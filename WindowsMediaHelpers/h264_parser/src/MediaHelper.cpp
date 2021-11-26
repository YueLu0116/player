#include "MediaHelper.h"

#pragma warning(disable : 4819)

bool MediaHelper::init() {
    mPframe = av_frame_alloc();
    if (!mPframe) {
        std::cout << "[error] ffmpeg: Cannot alloc frame.\n";
        return false;
    }
    mPpacket = av_packet_alloc();
    if (!mPpacket) {
        std::cout << "[error] ffmpeg: Cannot initialize packet.\n";
        return false;
    }

    return true;
}

void MediaHelper::unint() {
	if (mPfmtCtx) {
		avformat_close_input(&mPfmtCtx);
	}
    if (mPcodecCtx) {
        avcodec_free_context(&mPcodecCtx);
    }
    if (mPframe) {
        av_frame_free(&mPframe);
    }
    if (mPpacket) {
        av_packet_free(&mPpacket);
    }
}

int MediaHelper::openMediaFile(const std::string& filePath) {
	int ret = 0;
	if (mPfmtCtx) {
		std::cout << "ffmpeg: mPfmtCtx has been used.\n";
		return -1;
	}
    ret = avformat_open_input(&mPfmtCtx, filePath.c_str(), nullptr, nullptr);
	return ret;
}

bool MediaHelper::readStreamInfo() {
    int ret = avformat_find_stream_info(mPfmtCtx, nullptr);
    if (ret < 0) {
        std::cout << "ffmpeg: failed to find stream information.\n";
        return false;
    }
    std::cout << "[info] ffmpeg: the total number of streams=" << mPfmtCtx->nb_streams << "\n";
    for (size_t id = 0; id < mPfmtCtx->nb_streams; id++) {
        if (mPfmtCtx->streams[id]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            mVideoStreamIdx = static_cast<int>(id);
            mVideoCodecId = mPfmtCtx->streams[id]->codecpar->codec_id;
            std::cout << "[info] ffmpeg: video stream id=" << mVideoStreamIdx << "\n";
            std::cout << "[info] ffmpeg: codec_id=" << static_cast<unsigned int>(mVideoCodecId) << "\n";
            break;
        }
    }
    if (mVideoStreamIdx == -1 || mVideoCodecId == -1) {
        std::cout << "ffmpeg: failed to cannot find video stream\n";
        return false;
    }
    return true;
}

AVCodecID MediaHelper::originalCodecId() const{
    return mVideoCodecId;
}

bool MediaHelper::initDecoder(AVCodecID codecId)
{
    mPcodec = avcodec_find_decoder(codecId);
    if (!mPcodec) {
        std::cout << "[error] ffmpeg: Cannot find the specific decoder\n";
        return false;
    }
    mPcodecCtx = avcodec_alloc_context3(mPcodec);
    if (!mPcodecCtx) {
        std::cout << "[error] ffmpeg: Cannot alloc the context for decoder\n";
        return false;
    }
    if (avcodec_open2(mPcodecCtx, mPcodec, nullptr) < 0) {
        std::cout << "[error] ffmpeg: Cannot open the codec\n";
        return false;
    }
    return true;
}

bool MediaHelper::readFrame() {
    int ret = av_read_frame(mPfmtCtx, mPpacket);
    if (ret == AVERROR_EOF) {
        std::cout << "[warning] ffmpeg: End of file\n";
        return false;
    }
    else if (ret != 0) { // other reading errors
        std::cout << "[error] ffmpeg: Cannot read frames. Error code=" << ret << "\n";
        return false;
    }
    return true;
}

bool MediaHelper::isVideoFrame() {
    if (!mPpacket) {
        std::cout << "[error] ffmpeg: mPpacket is null\n";
        return false;
    }
    if (mVideoStreamIdx == -1) {
        std::cout << "[error] ffmpeg: Video stream is empty\n";
        return false;
    }
    return mPpacket->stream_index == mVideoStreamIdx;
}

bool MediaHelper::decode() {
    int ret = 0;
    while (!ret)
    {
        ret = avcodec_send_packet(mPcodecCtx, mPpacket);
        if (ret != 0) {
            std::cout << "[error] ffmpeg: Cannot send packets\n";
            return false;
        }

        ret = avcodec_receive_frame(mPcodecCtx, mPframe);
        if (ret == AVERROR(EAGAIN)) {
            ret = 0;
            std::cout << "[warning] ffmpeg: Not enough data\n";
            continue;
        }
        else if (ret == AVERROR_EOF) {
            std::cout << "[warning] ffmpeg: End of file\n";
            return false;
        }
        else if (ret != 0) {
            std::cout << "[error] ffmpeg: Failed to receive frame\n";
            return false;
        }
        // TODO: set to a texture
        break;
    }
    return true;
}

void MediaHelper::unrefPacket() {
    if (!mPpacket) {
        av_packet_unref(mPpacket);
    }
}