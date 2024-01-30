#include "stable_fluid.hpp"
#include <vector>

using namespace std;

StableFluid::StableFluid(__uint32_t size, __uint32_t resolution)
: size(size), resolution(resolution)
{    
    density    = new float*[size+2];
    velocity_x = new float*[size+2];
    velocity_y = new float*[size+2];
    prev_density    = new float*[size+2];
    prev_velocity_x = new float*[size+2];
    prev_velocity_y = new float*[size+2];
    
    for(size_t i(0);i<size+2;i++)
    {
        density[i] = new float[size+2];
        velocity_x[i] = new float[size+2];
        velocity_y[i] = new float[size+2];
        prev_density[i] = new float[size+2];
        prev_velocity_x[i] = new float[size+2];
        prev_velocity_y[i] = new float[size+2];

        for(int j(0);j<size+2;j++)
        {
            prev_density[i][j] = 0;
            prev_velocity_x[i][j] = 0;
            prev_velocity_y[i][j] = 0;
            density[i][j] = 0;    
            velocity_x[i][j] = 0;
            velocity_y[i][j] = 0;
        }
    }
}

StableFluid::~StableFluid()
{
    for(size_t i(0);i<size+2;i++)
    {
        delete density[i];
        delete velocity_x[i];
        delete velocity_y[i];
        delete prev_density[i];
        delete prev_velocity_x[i];
        delete prev_velocity_y[i];
    }
    delete prev_density;
    delete prev_velocity_x;
    delete prev_velocity_y;
}

// add density
void StableFluid::add_source(float **x, float **s, float dt)
{
    for (size_t i(0); i<size+2 ;i++) 
        for (size_t j(0); j<size+2 ;j++) 
            x[i][j] += dt*s[i][j];
}

void StableFluid::diffuse(__uint32_t b, float **x, float **x0, float viscosity, float dt)
{
    float a=dt*viscosity*size*size;
    
    for(size_t k(0); k < K_MAX; k++)
        for(size_t i(1); i < size+1; i++)
            for(size_t j(1); j < size+1; j++)
                x[i][j] = (x0[i][j] + a*(x[i-1][j]+x[i+1][j]+x[i][j-1]+x[i][j+1]))/(1+4*a);

    set_bnd(b, x);
}

void StableFluid::advect(__uint32_t b, float **d, float **d0, float **u, float **v, float dt)
{
    __uint32_t i0, j0, i1, j1;
    float x, y, s0, t0, s1, t1, dt0 = dt*size;
    
    for(size_t i(1) ;i < size+1 ;i++)
    {
        for(size_t j(1) ; j < size+1 ; j++)
        {
            x = i-dt0*u[i][j]; 
            y = j-dt0*v[i][j];
            
            if(x < 0.5) 
                x=0.5; 
            if (x > size+0.5) 
                x=size+0.5; 
            if(y < 0.5) 
                y=0.5; 
            if (y > size+0.5) 
                y=size+0.5; 
            
            i0=(__uint32_t)x;
            i1=i0+1;
            j0=(__uint32_t)y; 
            j1=j0+1;
            s1 = x-i0; 
            s0 = 1-s1;
            t1 = y-j0;
            t0 = 1-t1;
            d[i][j] = s0*(t0*d0[i0][j0]+t1*d0[i0][j1])+s1*(t0*d0[i1][j0]+t1*d0[i1][j1]);
        }
    }
    set_bnd(b, d);
}

void StableFluid::project(float **u, float **v, float **p, float **div)
{
    float h = 1.0/size;

    for(size_t i(1); i < size+1; i++)
    {
        for(size_t j(1); j < size+1; j++)
        {
            div[i][j] = -0.5*h*(u[i+1][j]-u[i-1][j]+v[i][j+1]-v[i][j-1]);
            p[i][j] = 0;
        }
    }
    set_bnd(0, div);
    set_bnd(0, p);
    for(size_t k(0); k<K_MAX; k++)
    {
        for(size_t i(1); i < size+1; i++)
        {
            for(size_t j(1); j < size+1; j++)
            {
                p[i][j] = (div[i][j]+p[i-1][j]+p[i+1][j]+p[i][j-1]+p[i][j+1])/4;
            }
        }
        set_bnd(0, p);
    }

    for(size_t i(1); i < size+1; i++)
    {
        for(size_t j(1); j < size+1; j++)
        {
            u[i][j] -= 0.5*(p[i+1][j]-p[i-1][j])/h;
            v[i][j] -= 0.5*(p[i][j+1]-p[i][j-1])/h;
        }
    }
    set_bnd(1, u);
    set_bnd(2, v);
}

void StableFluid::set_bnd(__uint32_t b, float **x)
{
    for(size_t i(1); i < size+1; i++)
    {
        x[0][i]      = b==1 ? -x[1][i]      : x[1][i];
        x[size+1][i] = b==1 ? -x[size][i]   : x[size][i];
        x[i][0]      = b==2 ? -x[i][1]      : x[i][1];
        x[i][size+1] = b==2 ? -x[i][size]   : x[i][size];
    }
    // coins
    x[0][0]             = 0.5*(x[1][0]+x[0][1]);
    x[0][size+1]        = 0.5*(x[1][size+1]+x[0][size]);
    x[size+1][0]        = 0.5*(x[size][0]+x[size+1][1]);
    x[size+1][size+1]   = 0.5*(x[size][size+1]+x[size+1][size]);
}


float** StableFluid::step(float viscosity, float diffusion, float dt)
{
    float **u = this->velocity_x;
    float **v = this->velocity_y;
    float **u_prev = this->prev_velocity_x;
    float **v_prev = this->prev_velocity_y;
    float **x = this->density;
    float **x0 =  this->prev_density;

    //
    add_source(u, u_prev, dt); 
    add_source(v, v_prev, dt);
    SWAP(u_prev, u); 
    diffuse(1, u, u_prev, viscosity, dt);
    SWAP(v_prev, v); 
    diffuse(2, v, v_prev, viscosity, dt);
    
    project(u, v, u_prev, v_prev);
    SWAP(u_prev, u);
    SWAP(v_prev, v);
    advect(1, u, u_prev, u_prev, v_prev, dt); 
    advect(2, v, v_prev, u_prev, v_prev, dt);
    project(u, v, u_prev, v_prev);

    //
    add_source(x, x0, dt);
    SWAP(x0, x); 
    diffuse(0, x, x0, diffusion, dt);
    SWAP(x0, x);
    advect(0, x, x0, u, v, dt);

    return this->density;
}
