#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "stable_fluid.hpp"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

#define SF_SIZE 100

bool is_lclick_held = 0;
bool display_velocity_field = 0;

__uint64_t prev_tick=0;

int main(void)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[debug] > %s", SDL_GetError());
        return 1;
    }
    if(TTF_Init() == -1)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[debug] > %s", SDL_GetError());
        return 1;
    }
    SDL_Window *win; SDL_Renderer *re;
    SDL_CreateWindowAndRenderer(1, 1, 0, &win, &re);
    if(win == NULL || re == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[debug] > %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // bordered fullscreen
    SDL_DisplayMode display0;
    SDL_Rect client_screen;
    // SDL_GetDisplayBounds(0, &client_screen); Pour avoir la taille totale de l'écran
    SDL_GetCurrentDisplayMode(0, &display0); // total screen size
    SDL_GetDisplayUsableBounds(0, &client_screen);
    SDL_SetWindowSize(win, client_screen.w, client_screen.h);    

    SDL_SetWindowTitle(win, "Stable Fluid");
    TTF_Font *font  = TTF_OpenFont("./arial/arial.ttf", 20);
    if(font == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[debug] > %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);
    SDL_bool run = SDL_TRUE;
    __int32_t sf_size = 8.0/10.0*((float)client_screen.h);
    SDL_Event events;

    StableFluid s(SF_SIZE, 1);

    prev_tick = SDL_GetTicks64();
    while(run)
    {
        // reset velocity and density arrays
        int max_speed = 10;
        for(size_t j(0);j<SF_SIZE+2;j++)
        {
            for(size_t i(0);i<SF_SIZE+2;i++)
            {
                s.prev_density[i][j] = 0;// 0 en 0 et SF+2 1 EN SF+2/2
                s.prev_velocity_x[i][j] = 0;
                s.prev_velocity_y[i][j] = 0;

                // cercle
                float angle = M_PI_2/2;

                float x_norm = 2.0f*((float)i/(SF_SIZE+2))-1.0f;//(float)((SF_SIZE+2))-1.0f;
                float y_norm = 2.0f*((float)j/(SF_SIZE+2))-1.0f;//(float)((SF_SIZE+2))-1.0f;

                // obligatoire a cause du cas x=0 qui 
                if(x_norm < 0.0f)
                {
                    // s.prev_velocity_y[i][j] = max_speed*1*sin(atan2(x_norm, y_norm)-angle);
                    // s.prev_velocity_x[i][j] = max_speed*1*cos(atan2(x_norm, y_norm)-angle);
                    s.prev_velocity_y[i][j] = max_speed*sin(atan(y_norm/x_norm)-angle);
                    s.prev_velocity_x[i][j] = max_speed*cos(atan(y_norm/x_norm)-angle);
                }
                else if(x_norm > 0.0f)
                {
                    angle += M_PI;
                    // s.prev_velocity_y[i][j] = max_speed*1*sin(atan2(x_norm, y_norm)-angle);
                    // s.prev_velocity_x[i][j] = max_speed*1*cos(atan2(x_norm, y_norm)-angle);
                    s.prev_velocity_y[i][j] = max_speed*sin(atan(y_norm/x_norm)-angle);
                    s.prev_velocity_x[i][j] = max_speed*cos(atan(y_norm/x_norm)-angle);
                }

                // front montant et descendant
                /*s.prev_velocity_x[i][j] = 0;
                s.prev_velocity_y[i][j] = 0;

                s.prev_velocity_y[i][j] = 10;
                s.prev_velocity_x[i][j] = 0;
                if(i > 50)
                {       
                    s.prev_velocity_y[i][j] = -10;
                    s.prev_velocity_x[i][j] = -0;
                }*/
            }
        }
        // add mouse held event
        if(is_lclick_held)
        {
            SDL_Event e;
            e.type = SDL_MOUSEBUTTONDOWN;
            e.button.button = 1;
            SDL_GetMouseState(&e.button.x, &e.button.y);
            SDL_PushEvent(&e);
        }

        while(SDL_PollEvent(&events))
        {
            switch (events.type)
            {
            case SDL_QUIT:
                run = SDL_FALSE;
            break;
            case SDL_KEYDOWN:
                switch(events.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    run = SDL_FALSE;
                break;
                case SDLK_a:
                    display_velocity_field = !display_velocity_field;
                break;
                case SDLK_c:    // reset density
                    for(__int32_t i(0); i < SF_SIZE+2; i++)
                    {
                        for(__int32_t j(0); j < SF_SIZE+2; j++)
                        {
                            s.density[i][j] = 0;
                        }
                    }
                break;
                }
            break;
            // empeche de bouger la fenetre
            case SDL_EventType::SDL_WINDOWEVENT:
                SDL_SetWindowPosition(win, display0.w-client_screen.w, 0);
            break;
            case SDL_MOUSEBUTTONUP:
                if(events.button.button == 1)
                    is_lclick_held = 0;
            break;
            case SDL_MOUSEBUTTONDOWN:
                if(events.button.button == 1) //clic droit
                {
                    // is_lclick_held = 1;
                    __int32_t x = (__int32_t)(((float)(events.button.x-display0.w+client_screen.h))/((float)sf_size)*((float)SF_SIZE));
                    __int32_t y = (__int32_t)(((float)(events.button.y-client_screen.h/10))/((float)sf_size)*((float)SF_SIZE));
                    
                    __int32_t dsize = SF_SIZE/10;
                    for(__int32_t i(x-dsize < 1 ? 1 : x-dsize); i < (x+dsize > SF_SIZE ? SF_SIZE : x+dsize); i++)
                    {
                        for(__int32_t j(y-dsize < 1 ? 1 : y-dsize); j < (y+dsize > SF_SIZE ? SF_SIZE : y+dsize); j++)
                        {
                            s.prev_density[i][j] = 1000;
                        }
                    }
                }
            break;
            }
        }

        float dt = 0.001;
        float **im = s.step(0.1, 0.01, dt);

        std::vector<uint32_t> image;
        image.reserve(SF_SIZE*SF_SIZE);
        for(int j(1);j<SF_SIZE+1;j++)
        {
            for(int i(1);i<SF_SIZE+1;i++)
            {
                uint32_t color = 0X000000FF;
                // color |= (uint8_t)((im[i][j]/max)*255) << 24;
                color |= (uint8_t)(im[i][j] > 1.0f ? 255 : im[i][j]*255) << 24;
                image.push_back(color);
            }
        }
        

        SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(&image[0], SF_SIZE, SF_SIZE, 32, SF_SIZE*4, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        SDL_Texture *text = SDL_CreateTextureFromSurface(re, surf);
        SDL_Color white = {0, 255, 0};
        std::stringstream fps;  fps << 1000.0f/((float)(SDL_GetTicks64()-prev_tick));  // v=d/t    fps=1/tick

        SDL_Surface *stex = TTF_RenderText_Solid(font, fps.str().c_str(), white);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(re, stex);

        SDL_SetTextureBlendMode(text, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

        SDL_SetRenderDrawColor(re, 0, 0, 0, 255);
        SDL_RenderClear(re);

        SDL_Rect dest = {display0.w-client_screen.h, client_screen.h/10, sf_size, sf_size};
        SDL_Rect dest2 = {0, 0, stex->w, stex->h};
        SDL_RenderCopy(re, text, nullptr, &dest);
        SDL_RenderCopy(re, tex, nullptr, &dest2);
        
        SDL_SetRenderDrawColor(re, 255, 255, 255, 255);
        SDL_RenderDrawLine(re, display0.w-client_screen.h, client_screen.h/10, display0.w-client_screen.h*2/10, client_screen.h/10);
        SDL_RenderDrawLine(re, display0.w-client_screen.h, client_screen.h*9/10, display0.w-client_screen.h*2/10, client_screen.h*9/10);
        SDL_RenderDrawLine(re, display0.w-client_screen.h, client_screen.h/10, display0.w-client_screen.h, client_screen.h*9/10);
        SDL_RenderDrawLine(re, display0.w-client_screen.h*2/10, client_screen.h*9/10, display0.w-client_screen.h*2/10, client_screen.h/10);

        // draw velocity vectors
        if(display_velocity_field)
        {
            for(int j(5);j<SF_SIZE;j+=5)
            {
                for(int i(5);i<SF_SIZE;i+=5)
                {
                    SDL_RenderDrawLine(re, ((float)i)/SF_SIZE*sf_size+display0.w-client_screen.h,
                                           ((float)j)/SF_SIZE*sf_size+client_screen.h/10,
                                           ((float)i)/SF_SIZE*sf_size+display0.w-client_screen.h+s.velocity_x[i][j]/max_speed*4*sf_size/SF_SIZE, 
                                           ((float)j)/SF_SIZE*sf_size+client_screen.h/10+s.velocity_y[i][j]/max_speed*4*sf_size/SF_SIZE);
                }
            }
        }
        
        SDL_RenderPresent(re);
        prev_tick = SDL_GetTicks64();
    }

    SDL_DestroyRenderer(re);
    SDL_DestroyWindow(win);
    return 0;
}