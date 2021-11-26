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
#endif


int main()
{
    // test sfml
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "First SFML");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;

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

    int frameCount = 0;
    while (mediaHelper.readFrame()) {
        std::cout << "[info] read " << frameCount++ << "th " << "frame\n";
        if (mediaHelper.isVideoFrame()) {
            if (!mediaHelper.decode()) {
                std::cout << "[error] Failed to decode video stream\n";
                mediaHelper.unrefPacket();
                break;
            }
            mediaHelper.unrefPacket();
        }
    }

    mediaHelper.unint();
    return 0;
}