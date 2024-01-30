#include <inttypes.h>
#include <vector>

#define K_MAX 10

#define SWAP(x0, x) {float **tmp=x0;x0=x;x=tmp;}

class StableFluid
{
    private:
        __uint32_t size;
        __uint32_t resolution;
    
    public:
        float **prev_density;
        float **prev_velocity_x;
        float **prev_velocity_y;
        float **density;
        float **velocity_x;
        float **velocity_y;
    
    public:
        StableFluid(__uint32_t, __uint32_t);
        ~StableFluid();
        void add_source(float**, float**, float);
        void diffuse(__uint32_t, float**, float**, float, float);
        void advect(__uint32_t, float**, float**, float**, float**, float);
        void project(float**, float**, float**, float**);
        void set_bnd(__uint32_t, float**);
        float** step(float, float, float);
};
