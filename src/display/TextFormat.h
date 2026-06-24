#ifndef __TEXTFORMAT_H__
#define __TEXTFORMAT_H__

#include "FairyGUIMacros.h"
#include <string>

NS_FGUI_BEGIN

class TextFormat
{
public:
    TextFormat();
    TextFormat(const TextFormat& other);
    TextFormat& operator=(const TextFormat& other);

    void setFormat(const TextFormat& format);
    void enableEffect(int effectFlag) { effect |= effectFlag; }
    void disableEffect(int effectFlag) { effect &= ~effectFlag; }
    bool hasEffect(int effectFlag) const { return (effect & effectFlag) != 0; }

    static const int OUTLINE = 1;
    static const int SHADOW = 2;
    static const int GLOW = 4;

    std::string face;
    float fontSize;
    Color color;
    bool bold;
    bool italics;
    bool underline;
    int lineSpacing;
    int letterSpacing;
    int align;
    int verticalAlign;

    int effect;
    Color outlineColor;
    int outlineSize;
    Color shadowColor;
    Vector2 shadowOffset;
    int shadowBlurRadius;
    Color glowColor;

    // Internal use
    bool _hasColor;
};

NS_FGUI_END

#endif
