#include "MediaHelper.h"

#pragma warning(disable : 4819)

namespace {
    void saveFrame(AVFrame* pFrame, int width, int height, int frameId) {
        FILE* pFile;
        char szFilename[32];
        int  y;

        // Open file
        sprintf(szFilename, "frame%d.rgb", frameId);
        pFile = fopen(szFilename, "wb");
        if (pFile == NULL)
            return;

        // Write header
        fprintf(pFile, "P6\n%d %d\n255\n", width, height);

        // Write pixel data
        for (y = 0; y < height; y++)
            fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);

        // Close file
        fclose(pFile);
    }
}

bool MediaHelper::init() {
    mPframe = av_frame_alloc();
    if (!mPframe) {
        std::cout << "[error] ffmpeg: Cannot alloc frame.\n";
        return false;
    }
    mPRGBframe = av_frame_alloc();
    if (!mPRGBframe) {
        std::cout << "[error] ffmpeg: Cannot alloc rgba frame.\n";
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
    if (mPRGBframe) {
        av_frame_free(&mPRGBframe);
    }
    if (mPpacket) {
        av_packet_free(&mPpacket);
    }
    if (mbufferRGB) {
        av_free(mbufferRGB);
    }
    if (mPswsCtx) {
        sws_freeContext(mPswsCtx);
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
            mWidth = mPfmtCtx->streams[id]->codecpar->width;
            mHeight = mPfmtCtx->streams[id]->codecpar->height;
            std::cout << "[info] ffmpeg: video stream id=" << mVideoStreamIdx << "\n";
            std::cout << "[info] ffmpeg: codec_id=" << static_cast<unsigned int>(mVideoCodecId) << "\n";
            std::cout << "[info] ffmpeg: frame size=" << mWidth << "x" << mHeight << "\n";
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
    int ret = avcodec_parameters_to_context(mPcodecCtx, mPfmtCtx->streams[mVideoStreamIdx]->codecpar);
    if (ret < 0) {
        std::cout << "[error] ffmpeg: Cannot trans avcodec parameters to context\n";
        return false;
    }
    if (avcodec_open2(mPcodecCtx, mPcodec, nullptr) < 0) {
        std::cout << "[error] ffmpeg: Cannot open the codec\n";
        return false;
    }
    std::cout << "[info] ffmpeg: Codec context: widthxheight=" << mPcodecCtx->width << "x" << mPcodecCtx->height << "\n";
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
        break;
    }
    return true;
}

void MediaHelper::unrefPacket() {
    if (!mPpacket) {
        av_packet_unref(mPpacket);
    }
}

bool MediaHelper::initRGBAImageBuffer() {
    if (!mPcodecCtx) {
        std::cout << "[error] ffmpeg: Unintialized codec context\n";
        return false;
    }
    if (mWidth < 0 || mHeight < 0) {
        std::cout << "[error] ffmpeg: Unknown video frams size\n";
        return false;
    }
    int bufSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, mWidth, mHeight, 1);
    mbufferRGB = static_cast<uint8_t*>(av_malloc(bufSize));
    if (!mbufferRGB) {
        std::cout << "[error] Cannot alloc rgba buffer\n";
        return false;
    }
    std::cout << "[info] ffmpeg: Required RGBA buffer size is " << bufSize << "\n";
    int ret = av_image_fill_arrays(mPRGBframe->data, mPRGBframe->linesize, mbufferRGB, AV_PIX_FMT_RGB24, mWidth, mHeight, 1);
    if (ret < 0) {
        std::cout << "[error] ffmpeg: Failed to av_image_fill_arrays\n";
        return false;
    }
    std::cout << "[info] ffmpeg: the size in bytes required for mbufferRGB is " << ret << "\n";
    return true;
}

bool MediaHelper::initSwsCtx() {
    mPswsCtx = sws_getContext(
        mWidth,
        mHeight,
        mPcodecCtx->pix_fmt,
        mWidth,
        mHeight,
        AV_PIX_FMT_RGB24,
        SWS_BICUBIC,
        nullptr,
        nullptr,
        nullptr
        );
    if (!mPswsCtx) {
        std::cout << "[error] Cannot initialize sws_context\n";
        return false;
    }
    return true;
}

bool MediaHelper::initTransformer() {
    return initRGBAImageBuffer() && initSwsCtx();
}

bool MediaHelper::Yuv2RGB() {
    if (!mPswsCtx) {
        std::cout << "[error] sws_context is null\n";
        return false;
    }
    sws_scale(mPswsCtx, static_cast<const uint8_t* const*>(mPframe->data), mPframe->linesize, 0, 
        mPcodecCtx->height, mPRGBframe->data, mPRGBframe->linesize);
    // debug
    //saveFrame(mPRGBframe, mPcodecCtx->width, mPcodecCtx->height, 0);
    return true;
}

void MediaHelper::dumpRGBData(uint8_t* pData) {
    for (int i = 0, j = 0; i < mPcodecCtx->width*mPcodecCtx->height*3; i += 3, j += 4)
    {
        pData[j + 0] = mPRGBframe->data[0][i + 0];
        pData[j + 1] = mPRGBframe->data[0][i + 1];
        pData[j + 2] = mPRGBframe->data[0][i + 2];
        pData[j + 3] = 255;
    }
}