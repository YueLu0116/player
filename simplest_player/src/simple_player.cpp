#include <iostream>
#include <cstdlib>

#include "../include/MediaHelper.h"
#include "../include/SdlWrapper.h"

#define CHECK(ret) if(!ret) {system("pause"); return -1;}

namespace{
    void windowPoller(){
        // https://stackoverflow.com/questions/34424816/sdl-window-does-not-show
        SDL_Event e;
        bool quit = false;
        while (!quit){
            while (SDL_PollEvent(&e)){
                if (e.type == SDL_QUIT){
                    quit = true;
                }
                if (e.type == SDL_KEYDOWN){
                    quit = true;
                }
                if (e.type == SDL_MOUSEBUTTONDOWN){
                    quit = true;
                }
            }
        }
    }
}

int main(){


    MediaHelper media_helper;
    CHECK(media_helper.initResult());
    CHECK(media_helper.openMediaFile("../../resources/bunny.mp4"));
    CHECK(media_helper.readStreamInfo());
    std::cout << "video stream: " << media_helper.videoStreamId() << "\n";
    std::cout << "audio stream: " << media_helper.audioStreamId() << "\n";
    CHECK(media_helper.buildVideoCodec());
    std::cout << "video size: " << media_helper.width() << "x" << media_helper.height() << "\n";
    CHECK(media_helper.initImage());

    SdlWrapper sdl(media_helper.width(), media_helper.height());
    CHECK(sdl.initResult());
    CHECK(sdl.createWindow("simple player"));

    windowPoller();

    CHECK(sdl.createTexture());
    
    while(media_helper.readFrame()){
        if(media_helper.yuvData().pY_plane == nullptr){
            continue;
        }
        CHECK(sdl.updateTexture(media_helper.yuvData()));
        CHECK(sdl.sdlRender(30));
        media_helper.wipePacket();
    }

    return 0;
}