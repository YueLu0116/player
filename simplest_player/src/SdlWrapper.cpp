#include "../include/SdlWrapper.h"
#include "../include/types.h"

bool SdlWrapper::createWindow(const std::string &_win_name){
    std::cout << "sdl: [info] window size:" << width_ << "x" << height_ << "\n";
    pscreen_ = SDL_CreateWindow(
        _win_name.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width_,
        height_,
        SDL_WINDOW_SHOWN
    );
    if(pscreen_ == nullptr){
        std::cout << "sdl: failed to create window. error code: " << SDL_GetError() << "\n";
        return false;
    }

    //winThd_ = std::move(std::thread(&windowPoller));
    //winThd_.detach();

    psdl_renderer_ = SDL_CreateRenderer(pscreen_, -1, 0);
    if(psdl_renderer_ == nullptr){
        std::cout << "sdl: failed to create render. error code: " << SDL_GetError() << "\n";
        return false;
    }
    return true;
}

bool SdlWrapper::createTexture(){
    psdl_texture_ = SDL_CreateTexture(
        psdl_renderer_,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        width_,
        height_
        );
    if(psdl_texture_ == nullptr){
        std::cout << "sdl: failed to create texture. error code: " << SDL_GetError() << "\n";
        return false;
    }
    return true;
}

bool SdlWrapper::updateTexture(const YuvData &_yuv_data){

    int ret = SDL_UpdateYUVTexture(
        psdl_texture_,
        &rect_,
        _yuv_data.pY_plane,
        _yuv_data.Y_pitch,
        _yuv_data.pU_plane,
        _yuv_data.U_pitch,
        _yuv_data.pV_plane,
        _yuv_data.V_pitch
    );
    if(ret){
        std::cout << "sdl: failed to update texture. error code: " << SDL_GetError() << "\n";
        return false;
    }
    return true;
}

bool SdlWrapper::sdlRender(const int _fps){
    int ret = SDL_RenderClear(psdl_renderer_);
    if(ret){
        std::cout << "sdl: failed to clear render. error code: " << SDL_GetError() << "\n";
        return false;
    }
    ret = SDL_RenderCopy(psdl_renderer_, psdl_texture_, nullptr, &rect_);
    if(ret){
        std::cout << "sdl: failed to render copy. error code: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_RenderPresent(psdl_renderer_);
    SDL_Delay(1000 / _fps);
    return true;
}