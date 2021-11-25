#include <iostream>
#include <string>
#include "MediaHelper.h"

int main()
{
    MediaHelper mediaHelper;
    mediaHelper.init();

    int ret = mediaHelper.openMediaFile("E:\\opensource\\player\\resources\\h264\\test.h264");
    if (!ret) {
        std::cout << ":) I can open raw h264 file" << std::endl;
    }
    else {
        std::cout << "Failed to open h264 file. Error code=" << ret << std::endl;
    }
    if (!mediaHelper.readStreamInfo())
    {
        std::cout << "Failed to readStreamInfo\n";
    }
    mediaHelper.unint();

}