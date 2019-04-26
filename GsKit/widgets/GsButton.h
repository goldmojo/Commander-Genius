/*
 * CGUIText.h
 *
 *  Created on: 30.10.2011
 *      Author: gerstrong
 *
 *  This is the text control for our new GUI
 */

#ifndef GsButton_H_
#define GsButton_H_

#include "GsControl.h"
#include <string>
#include <memory>
#include <map>
#include <graphics/GsSurface.h>
#include <widgets/GsText.h>

#include <functional>

class GsButton : public GsControl
{
public:

    GsButton(const std::string& text,
             const GsRect<float> &rect,
             CEvent *ev = nullptr,
             const Style style = Style::NONE,
             const float red = 0.875f,
             const float green = 0.875f,
             const float blue = 1.0f);

    GsButton(const std::string& text,
             CEvent *ev = nullptr,
             const Style style = Style::NONE,
             const float red = 0.875f,
             const float green = 0.875f,
             const float blue = 1.0f);

    bool sendEvent(const InputCommand command) override;

    void updateGraphics() override;

    void setActivationEvent(const std::function <void ()>& f)
    {
        mFunction = f;
    }

    /**
     * @brief drawEnabledButton Drawing of a button with fading effects.
     * @param blitsfc sfc where to draw
     * @param lRect Coordinates
     * @param alternate Alterante color wanted?
     */
    void drawEnabledButton(GsWeakSurface &blitsfc,
                           const SDL_Rect &lRect,
                           const bool alternate);

    virtual void processLogic() override;

    void drawNoStyle(const SDL_Rect &lRect);

	void drawVorticonStyle(SDL_Rect& lRect);

    virtual void processRender(const GsRect<float> &RectDispCoordFloat) override;

    virtual void processRender(const GsRect<float> &srcRectFloat,
                               const GsRect<float> &dstRectFloat) override;

    void setText(const std::string& text)
	{
		mText = text;
        setupButtonSurface("  " + mText);
    }

    std::shared_ptr<CEvent> &event()
    {
        return mEvent;
    }

	std::string getText()
	{	return mText;	}

    virtual void setupButtonSurface(const std::string &text = "");

protected:

    std::string mText;

    GsSurface mTextDarkSfc;
    GsSurface mTextLightSfc;
    GsSurface mTextRedSfc;
    GsSurface mTextDisabledSfc;


private:        

	std::shared_ptr<CEvent> mEvent;

    // Default-> Blue. This will colorize the buttons a bit. Effects continue working.
    const float mRed = 0.875f;
    const float mGreen = 0.875f;
    const float mBlue = 1.0f;

    CGUIText mTextWidget;

    std::function <void ()> mFunction;
};


#endif /* GsButton_H_ */
