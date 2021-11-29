#include "VideoRender.h"
#include <algorithm>

void VideoRender::initWindow(int width, int height) {
	mWidth = width;
	mHeight = height;
	mWindow.create(sf::VideoMode(mWidth, mHeight), "Player");
}

void VideoRender::prepareRenderData() {
	mpFrame = std::make_shared<sf::Uint8>(mWidth * mHeight * 4);
	mTexture.create(mWidth, mHeight);
	mTexture.setSmooth(false);
	mSprite.setTexture(mTexture);
}

void VideoRender::update(uint8_t* prawData) {
	std::copy(prawData, prawData + mWidth * mHeight * 4, mpFrame.get());
	mTexture.update(mpFrame.get());
}