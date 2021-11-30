#include <algorithm>
#include <iostream>
#include <string>

#include <SFML/Graphics.hpp>

#include "MediaHelper.h"

#ifdef _DEBUG 
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
#endif

#ifndef _DEBUG
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")  // don't show console window
#endif

int main()
{
    MediaHelper mediaHelper;
    mediaHelper.init();
    int ret = mediaHelper.openMediaFile("E:\\opensource\\player\\resources\\h264\\test.h264");
    if (!ret) {
        std::cout << "[info] I can open raw h264 file" << std::endl;
    }
    else {
        std::cout << "[error] Failed to open h264 file. Error code=" << ret << std::endl;
        mediaHelper.unint();
        return -1;
    }

    if (!mediaHelper.readStreamInfo()){
        std::cout << "[error] Failed to readStreamInfo\n";
        mediaHelper.unint();
        return -2;
    }

    if (!mediaHelper.initDecoder(mediaHelper.originalCodecId())) {  // or other codec?
        std::cout << "[error] Failed to initialize decoder\n";
        mediaHelper.unint();
        return -3;
    }

    if (!mediaHelper.initTransformer()) {
        std::cout << "[error] Failed to initialize transformer\n";
        mediaHelper.unint();
        return -4;
    }

    // initialize rendered window
    sf::RenderWindow window;
    if (mediaHelper.width() > 0 && mediaHelper.height() > 0) {
        window.create(sf::VideoMode(mediaHelper.width(), mediaHelper.height()), "Player");
    }
    else {
        std::cout << "[error] Ileagal window size\n";
        return -5;
    }
    window.setFramerateLimit(30);

    // prepare drawing resources
    sf::Texture videoTexture;
    if (!videoTexture.create(mediaHelper.width(), mediaHelper.height())) {
        std::cout << "[error] Cannot create video texture\n";
        return -6;
    }
    videoTexture.setSmooth(false);
    sf::Sprite sprite(videoTexture);
    sf::Uint8* pData = new (std::nothrow) sf::Uint8[mediaHelper.width() * mediaHelper.height() * 4];
    if (!pData) {
        std::cout << "[error] Cannot alloc memory for texture\n";
        return -7;
    }

    int frameCount = 0;
    while (window.isOpen()) {
        sf::Event event;
        // message loop
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // decode
        if (mediaHelper.readFrame()) {
            std::cout << "[info] read " << frameCount++ << "th " << "frame\n";
            if (mediaHelper.isVideoFrame()) {
                if (!mediaHelper.decode()) {
                    std::cout << "[error] Failed to decode video stream\n";
                    mediaHelper.unrefPacket();
                    break;
                }
                if (!mediaHelper.Yuv2RGB()) {
                    std::cout << "[error] Failed to transform video frames\n";
                    break;
                }
                // render
                mediaHelper.dumpRGBData(pData);
                videoTexture.update(pData);
                window.clear();
                window.draw(sprite);
                window.display();

                mediaHelper.unrefPacket();
            }
        }
    }
    mediaHelper.unint();
    delete[] pData;
    pData = nullptr;
    return 0;
}