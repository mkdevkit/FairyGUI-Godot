#include "TextFormat.h"

NS_FGUI_BEGIN

TextFormat::TextFormat() :
    fontSize(12),
    color(0, 0, 0, 1),
    bold(false),
    italics(false),
    underline(false),
    lineSpacing(0),
    letterSpacing(0),
    align(0),
    verticalAlign(0),
    effect(0),
    outlineColor(0, 0, 0, 1),
    outlineSize(0),
    shadowColor(0, 0, 0, 0.5),
    shadowBlurRadius(0),
    glowColor(0, 0, 0, 1),
    _hasColor(false)
{
}

TextFormat::TextFormat(const TextFormat& other)
{
    *this = other;
}

TextFormat& TextFormat::operator=(const TextFormat& other)
{
    face = other.face;
    fontSize = other.fontSize;
    color = other.color;
    bold = other.bold;
    italics = other.italics;
    underline = other.underline;
    lineSpacing = other.lineSpacing;
    letterSpacing = other.letterSpacing;
    align = other.align;
    verticalAlign = other.verticalAlign;
    effect = other.effect;
    outlineColor = other.outlineColor;
    outlineSize = other.outlineSize;
    shadowColor = other.shadowColor;
    shadowOffset = other.shadowOffset;
    shadowBlurRadius = other.shadowBlurRadius;
    glowColor = other.glowColor;
    _hasColor = other._hasColor;
    return *this;
}

void TextFormat::setFormat(const TextFormat& format)
{
    *this = format;
}

NS_FGUI_END
