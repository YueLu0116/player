#pragma once
#include <iostream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

class MediaHelper {
public:
	MediaHelper() : mVideoStreamIdx(-1), mVideoCodecId(AV_CODEC_ID_NONE), mWidth(-1), mHeight(-1){}
	bool init();
	void unint();
	int openMediaFile(const std::string &filePath);
	bool readStreamInfo();
	AVCodecID originalCodecId() const;
	bool initDecoder(AVCodecID codecId);
	bool readFrame();
	bool isVideoFrame();
	bool decode();
	void unrefPacket();
	bool initTransformer();
	bool Yuv2RGB();
	int width() const { return mWidth; }
	int height() const { return mHeight; }
	uint8_t* frame() { return reinterpret_cast<uint8_t*>(mPRGBframe->data[0]); }
private:
	bool initRGBAImageBuffer();
	bool initSwsCtx();

	AVFormatContext *mPfmtCtx = nullptr;
	AVCodec *mPcodec = nullptr;
	AVCodecContext *mPcodecCtx = nullptr;
	AVFrame *mPframe = nullptr;
	AVFrame *mPRGBframe = nullptr;
	AVPacket *mPpacket = nullptr;
	int mVideoStreamIdx;
	AVCodecID mVideoCodecId;
	uint8_t *mbufferRGB = nullptr;
	SwsContext *mPswsCtx = nullptr;
	int mWidth;
	int mHeight;
};

