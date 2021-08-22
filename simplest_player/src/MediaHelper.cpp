#include "../include/MediaHelper.h"


void MediaHelper::cleaner(){
    if(pfmt_context_ != nullptr){
        avformat_close_input(&pfmt_context_);
    }
    if(pcodec_context_ != nullptr){
        avcodec_free_context(&pcodec_context_);
    }
    if(praw_frame_ != nullptr){
        av_frame_free(&praw_frame_);
    }
    if(pyuv_frame_ != nullptr){
        av_frame_free(&pyuv_frame_);
    }
    if(buffer != nullptr){
        av_free(buffer);
    }
    if(ppacket_ != nullptr){
        av_packet_free(&ppacket_);
    }
    if(psws_context != nullptr){
        sws_freeContext(psws_context);
    }
}

bool MediaHelper::openMediaFile(const std::string &_file_path){
    int ret = avformat_open_input(&pfmt_context_, _file_path.c_str(), nullptr, nullptr);
    if(ret){
        std::cout << "ffmpeg: failed to open media file.\n";
        return false;
    }
    return true;
}

bool MediaHelper::readStreamInfo(){
    int ret = avformat_find_stream_info(pfmt_context_, nullptr);
    if(ret < 0){
        std::cout << "ffmpeg: failed to find stream information.\n";
        return false;
    }
    for(size_t id=0;id<pfmt_context_->nb_streams;id++){
        if(pfmt_context_->streams[id]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx_ = id;
        }
        else if(pfmt_context_->streams[id]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_idx_ = id;
        }
    }
    if(video_stream_idx_ == -1){
        std::cout << "ffmpeg: failed to cannot find video stream\n";
        return false;
    }
    // TODO: audio idx
    return true;
}

bool MediaHelper::buildVideoCodec(){
    // codec parameters->codec
    // codec parameters->codec context
    // open/initialize codec context using codec
    AVCodecParameters *pcodec_par = pfmt_context_->streams[video_stream_idx_]->codecpar;
    AVCodec *pcodec = avcodec_find_decoder(pcodec_par->codec_id);
    if(pcodec == nullptr){
        std::cout << "ffmpeg: failed to cannot find video decodec\n";
        return false;
    }
    pcodec_context_ = avcodec_alloc_context3(pcodec);
    if(pcodec_context_ == nullptr){
        std::cout << "ffmpeg: failed to alloc codec context.\n";
        return false;
    }
    int ret = avcodec_parameters_to_context(pcodec_context_, pcodec_par);
    if(ret < 0){
        std::cout << "ffmpeg: failed to change parameters to context.\n";
        return false;
    }
    ret = avcodec_open2(pcodec_context_, pcodec, nullptr);
    if(ret < 0){
        std::cout << "ffmpeg: failed to open codec context.\n";
        return false;
    }
    width_ = pcodec_context_->width;
    height_ = pcodec_context_->height;
    return true;
}

bool MediaHelper::initImage(){
    return prepareImageBuffer() && initSws();
}

bool MediaHelper::prepareImageBuffer(){
    int buf_size = av_image_get_buffer_size(
        AV_PIX_FMT_YUV420P,
        pcodec_context_->width,
        pcodec_context_->height,
        1);
    std::cout << "ffmpeg: [info] buffer size=" << buf_size << "\n";
    buffer = static_cast<uint8_t*>(av_malloc(buf_size));
    if(buffer == nullptr){
        std::cout << "ffmpeg: failed to alloc image buffer.\n";
        return false;
    }
    int ret = av_image_fill_arrays(
        pyuv_frame_->data,
        pyuv_frame_->linesize,
        buffer,
        AV_PIX_FMT_YUV420P,
        pcodec_context_->width,
        pcodec_context_->height,
        1
    );
    if(ret < 0){
        std::cout << "ffmpeg: failed to fill image arrays.\n";
        return false;
    }
    return true;
}

bool MediaHelper::initSws(){
    psws_context = sws_getContext(
        width_,
        height_,
        pcodec_context_->pix_fmt,
        width_,
        height_,
        AV_PIX_FMT_YUV420P,
        SWS_BICUBIC,
        nullptr,
        nullptr,
        nullptr
    );
    if(psws_context == nullptr){
        std::cout << "ffmpeg: failed to initialize sws.\n";
        return false;
    }
    return true;
}

bool MediaHelper::readFrame(){
    int ret = av_read_frame(pfmt_context_, ppacket_);
    if(ret != 0){
        return false;
    }
    if(ppacket_->stream_index == video_stream_idx_){
        ret = avcodec_send_packet(pcodec_context_, ppacket_);
        if(ret != 0){
            std::cout << "ffmpeg: failed to send packet.\n";
            return false;
        }
        ret = avcodec_receive_frame(pcodec_context_, praw_frame_);
        // if(ret != 0){
        //     std::cout << "ffmpeg: failed to receive frame.\n";
        //     return false;
        // }
        int times = 0;
        if(ret == AVERROR(EAGAIN)){
            while(ret == AVERROR(EAGAIN) && times++ < 5){
                ret = avcodec_receive_frame(pcodec_context_, praw_frame_);
                std::cout << "try to receive frame...\n";
            }
            if(times >= 5 && ret == AVERROR(EAGAIN)){
                std::cout << "ffmpeg: [warning] not a good frame.\n";
                return true;
            }
        }
        else if(ret == AVERROR(AVERROR_EOF)){
            std::cout << "ffmpeg: [warning] end of media file.\n";
            return false;
        }
        else if(ret != 0){
            std::cout << "ffmpeg: failed to receive frame.\n";
            return false;
        }
        sws_scale(
            psws_context,
            static_cast<const uint8_t* const*>(praw_frame_->data),
            praw_frame_->linesize,
            0,
            height_,
            pyuv_frame_->data,
            pyuv_frame_->linesize
        );
        yuv_data_.pY_plane = pyuv_frame_->data[0];
        yuv_data_.Y_pitch = pyuv_frame_->linesize[0];
        yuv_data_.pU_plane = pyuv_frame_->data[1];
        yuv_data_.U_pitch = pyuv_frame_->linesize[1];
        yuv_data_.pV_plane = pyuv_frame_->data[2];
        yuv_data_.V_pitch = pyuv_frame_->linesize[2];
        //std::cout << "one frame ready.\n";
    }
    return true;
}

void MediaHelper::wipePacket(){
    av_packet_unref(ppacket_);
}