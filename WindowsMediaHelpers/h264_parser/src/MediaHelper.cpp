#include "MediaHelper.h"

#pragma warning(disable : 4819)

bool MediaHelper::init() {
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
            mVideoIdx = static_cast<int>(id);
            mVideoCodecId = mPfmtCtx->streams[id]->codecpar->codec_id;
            std::cout << "[info] ffmpeg: video stream id=" << mVideoIdx << "\n";
            std::cout << "[info] ffmpeg: codec_id=" << static_cast<unsigned int>(mVideoCodecId) << "\n";
            break;
        }
    }
    if (mVideoIdx == -1 || mVideoCodecId == -1) {
        std::cout << "ffmpeg: failed to cannot find video stream\n";
        return false;
    }
    return true;
}

bool MediaHelper::initDecoder(AVCodecID codecId)
{
    av_register_all();
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

bool MediaHelper::initFrame() {
    mPframe = av_frame_alloc();
    if (!mPframe) {
        std::cout << "[error] ffmpeg: Cannot alloc frame.\n";
        return false;
    }
    return true;
}

void MediaHelper::freeFrame() {
    av_frame_free(&mPframe);
}

bool MediaHelper::initPacket() {
    mPpacket = av_packet_alloc();
    if (!mPpacket) {
        std::cout << "[error] ffmpeg: Cannot initialize packet.\n";
        return false;
    }
    return true;
}

void MediaHelper::freePacket() {
    av_packet_free(&mPpacket);
}

