/*
 * CGUIText.h
 *
 *  Created on: 30.10.2011
 *      Author: gerstrong
 *
 *  This is the text control for our new GUI
 */

#ifndef CGUITEXT_H_
#define CGUITEXT_H_

#include <string>
#include <list>
#include <memory>

#include "GsControl.h"
#include <graphics/GsTexture.h>
#include <graphics/GsSurface.h>
#include <graphics/GsTrueTypeFont.h>

class CGUIText : public GsControl
{
public:
	CGUIText(const std::string& text);

    void setupButtonSurface(const std::string& text = "");

	virtual void setText(const std::string& text);

	virtual void processLogic();
    virtual void processRender(const GsRect<float> &RectDispCoordFloat);

	// Defines in character units how tall and wide the entire text is.
    GsRect<unsigned int> mTextDim;

protected:
	std::list<std::string> mTextList;
	
private:
	std::unique_ptr<SDL_Surface> mpTextDarkSfc;
	std::unique_ptr<SDL_Surface> mpTextLightSfc;
	std::unique_ptr<SDL_Surface> mpTextDisabledSfc;


    /// Automatic horizontal scrolling in case the text is too big
    float mScrollPosMax = 0.0f;
    float mScrollPos = 0.0f;
    float mScrollVel = 0.3f;


#if defined(USE_SDL_TTF)
    GsTrueTypeFont mTrueTypeFont;
#endif

    GsTexture mTexture;
    GsSurface mTextSfc;

    enum class ScrollDir
    {
        NONE,
        LEFT,
        RIGHT
    } mScrollDir = ScrollDir::NONE;

};

#endif /* CGUITEXT_H_ */