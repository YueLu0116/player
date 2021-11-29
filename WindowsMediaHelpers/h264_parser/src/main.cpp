#include <iostream>
#include <string>

#include "MediaHelper.h"
#include "VideoRender.h"

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


    int frameCount = 0;
    while (mediaHelper.readFrame()) {
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

            mediaHelper.unrefPacket();
        }
    }
    mediaHelper.unint();

    return 0;
}