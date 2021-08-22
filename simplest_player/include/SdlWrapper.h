#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <utility>

extern "C"{
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDl2/SDL_rect.h>
}

struct YuvData;

class SdlWrapper{

public:

    SdlWrapper(const int _wid, const int _hei):width_(_wid), height_(_hei){
        rect_.x = 0;
        rect_.y = 0;
        rect_.w = width_;
        rect_.h = height_;
        init_return_code_ = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
    }

    ~SdlWrapper(){
        SDL_Quit();
        if(pscreen_){
            SDL_DestroyWindow(pscreen_);
        }
    }

    bool initResult()const{
        if(init_return_code_){
            std::cout << "sdl: initializaion error. error : " << SDL_GetError() << "\n";
            return false;
        }
        return true;
    }

    bool createWindow(const std::string &_win_name);

    bool createTexture();

    bool updateTexture(const YuvData &_yuv_data);

    bool sdlRender(const int _fps);

private:
    int init_return_code_ = 0;
    int width_;
    int height_;
    std::thread winThd_;
    SDL_Window *pscreen_ = nullptr;
    SDL_Renderer *psdl_renderer_ = nullptr;
    SDL_Texture *psdl_texture_ = nullptr;
    SDL_Rect rect_;
};