#pragma once
#include <memory>
#include <SFML/Graphics.hpp>

#ifdef _DEBUG 
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
#endif

#ifndef _DEBUG
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")
#endif

class VideoRender {
public:
	VideoRender() : mWidth(-1), mHeight(-1) {}
	void initWindow(int width, int height);
	void prepareRenderData();
	bool isOpen() { return mWindow.isOpen(); }
	void update(uint8_t* prawData);
	void clear() { mWindow.clear(); }
	void display() { mWindow.display(); }
	void draw() { mWindow.draw(mSprite); }

private:
	sf::RenderWindow mWindow;
	std::shared_ptr<sf::Uint8> mpFrame = nullptr;
	sf::Texture mTexture;
	sf::Sprite mSprite;
	int mWidth;
	int mHeight;
};