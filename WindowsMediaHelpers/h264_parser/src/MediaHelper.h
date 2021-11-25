#pragma once
#include <iostream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

class MediaHelper {
public:
	MediaHelper() : mVideoIdx(-1), mVideoCodecId(AV_CODEC_ID_NONE){}
	bool init();
	void unint();
	int openMediaFile(const std::string &filePath);
	bool readStreamInfo();
	bool initDecoder(AVCodecID codecId);
	bool initFrame();
	void freeFrame();
	bool initPacket();
	void freePacket();
private:
	AVFormatContext *mPfmtCtx = nullptr;
	AVCodec *mPcodec = nullptr;
	AVCodecContext *mPcodecCtx = nullptr;
	AVFrame* mPframe = nullptr;
	AVPacket* mPpacket = nullptr;
	int mVideoIdx;
	AVCodecID mVideoCodecId;
};

