#ifndef __FUILABEL_H__
#define __FUILABEL_H__

#include "FairyGUIMacros.h"
#include "TextFormat.h"

NS_FGUI_BEGIN

class BitmapFont;

class FUILabel : public Node2D
{
    GDCLASS(FUILabel, Node2D)

public:
    FUILabel();
    virtual ~FUILabel();

    static FUILabel* create();

    void setText(const std::string& value);
    std::string& getText() { return _text; }

    void gd_setText(const String& text);
    String gd_getText();

    int getTextLength() const { return (int)_text.length(); }

    TextFormat* getTextFormat() const { return _textFormat; }
    void applyTextFormat();

    void setGrayed(bool value);
    bool isGrayed() const { return _grayed; }

    void setWrapEnabled(bool value) { _wrapEnabled = value; }
    bool isWrapEnabled() const { return _wrapEnabled; }
    Vector2 getTextSize() const { return Vector2(getTextWidth(), getTextHeight()); }
    void setContentSize(const Vector2& size) { _contentSize = size; }
    Vector2 _contentSize;
    void setUnderlineColor(const Color& value);

    void setDrawFontSize(int value) { _drawFontSize = value; queue_redraw(); }
    int getDrawFontSize() const { return _drawFontSize > 0 ? _drawFontSize : (int)_textFormat->fontSize; }

    float getTextWidth() const;
    float getTextHeight() const;

    static void _bind_methods();
    void _notification(int p_what);
    void _draw();

protected:

private:
    bool setBMFontFilePath(const std::string& bmfontFilePath, float fontSize = 0);
    void updateBMFontScale();
    void updateText();
    bool isBMFont() const { return !_bmFontPath.empty(); }

    static Color toGrayed(const Color& source);

    std::string _text;
    std::string _fontName;
    TextFormat* _textFormat;
    float _fontSize;
    bool _grayed;
    bool _wrapEnabled;
    Color _underlineColor;
    bool _hasUnderlineColor;
    int _drawFontSize;

    // BM Font
    std::string _bmFontPath;
    float _bmFontSize;
    float _bmfontScale;
    bool _bmFontCanTint;
    Ref<Font> _bmFont;
    Ref<Texture2D> _bmAtlasTexture;

    friend class UIPackage;
};

NS_FGUI_END

#endif
