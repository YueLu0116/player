#pragma once

#include <iostream>
#include <string>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "../include/types.h"

class MediaHelper{
public:

    MediaHelper(){
        video_stream_idx_ = -1;
        audio_stream_idx_ = -1;
        praw_frame_ = av_frame_alloc();
        pyuv_frame_ = av_frame_alloc();
        ppacket_ = av_packet_alloc();
        if(!praw_frame_ || !pyuv_frame_ || !ppacket_){
            std::cout << "ffmpeg: failed to alloc resource\n";
            if_init_success = false;
        }
    }

    ~MediaHelper(){
        cleaner();
    }

    bool openMediaFile(const std::string &_file_path);

    bool readStreamInfo();
    
    bool buildVideoCodec();

    bool initImage();

    bool readFrame();
    
    void wipePacket();

    bool initResult() const{
        return if_init_success;
    }

    int videoStreamId() const{
        return video_stream_idx_;
    }

    int audioStreamId() const{
        return audio_stream_idx_;
    }

    int width() const{
        return width_;
    }

    int height() const{
        return height_;
    }

    YuvData yuvData() const{
        return yuv_data_;
    }

private:
    void cleaner();
    bool prepareImageBuffer();
    bool initSws();
    bool if_init_success = true;
    int video_stream_idx_;
    int audio_stream_idx_;
    int width_;
    int height_;
    AVFormatContext *pfmt_context_ = nullptr;
    AVCodecContext *pcodec_context_ = nullptr;
    AVFrame *praw_frame_ = nullptr;
    AVFrame *pyuv_frame_ = nullptr;
    uint8_t *buffer = nullptr;
    SwsContext *psws_context = nullptr;
    AVPacket *ppacket_ = nullptr;
    YuvData yuv_data_;
};