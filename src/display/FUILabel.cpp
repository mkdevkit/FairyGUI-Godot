#include "FUILabel.h"
#include "BitmapFont.h"
#include "UIConfig.h"
#include "UIPackage.h"
#include "scene/resources/font.h"

NS_FGUI_BEGIN

Color FUILabel::toGrayed(const Color& source)
{
    float gray = source.r * 0.299f + source.g * 0.587f + source.b * 0.114f;
    return Color(gray, gray, gray, source.a);
}

FUILabel::FUILabel() :
    _fontSize(-1),
    _bmFontCanTint(false),
    _textFormat(new TextFormat()),
    _grayed(false),
    _bmFontSize(0),
    _bmfontScale(1.0f)
{
    item_rect_changed(); // enable NOTIFICATION_DRAW for Node2D
}

FUILabel::~FUILabel()
{
    delete _textFormat;
}

FUILabel* FUILabel::create()
{
    return memnew(FUILabel);
}

void FUILabel::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setText", "text"), &FUILabel::gd_setText);
    ClassDB::bind_method(D_METHOD("getText"), &FUILabel::gd_getText);

    ClassDB::bind_method(D_METHOD("setGrayed", "value"), &FUILabel::setGrayed);
    ClassDB::bind_method(D_METHOD("isGrayed"), &FUILabel::isGrayed);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "grayed"), "setGrayed", "isGrayed");

    ClassDB::bind_method(D_METHOD("setWrapEnabled", "value"), &FUILabel::setWrapEnabled);
    ClassDB::bind_method(D_METHOD("isWrapEnabled"), &FUILabel::isWrapEnabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "wrapEnabled"), "setWrapEnabled", "isWrapEnabled");

    ClassDB::bind_method(D_METHOD("getTextSize"), &FUILabel::getTextSize);

    ClassDB::bind_method(D_METHOD("setUnderlineColor", "color"), &FUILabel::setUnderlineColor);

    ClassDB::bind_method(D_METHOD("getTextWidth"), &FUILabel::getTextWidth);
    ClassDB::bind_method(D_METHOD("getTextHeight"), &FUILabel::getTextHeight);

    ClassDB::bind_method(D_METHOD("applyTextFormat"), &FUILabel::applyTextFormat);
}

void FUILabel::setText(const std::string& value)
{
    if (_text != value)
    {
        _text = value;
        if (_fontSize < 0)
            applyTextFormat();
        updateText();
    }
}

void FUILabel::applyTextFormat()
{
    bool firstTime = (_fontSize < 0);

    if (firstTime || _fontName != _textFormat->face)
    {
        _fontName = _textFormat->face;

        if (_fontName.find("ui://") != std::string::npos)
        {
            // BM Font
            setBMFontFilePath(_fontName);
        }
        else
        {
            // System/TTF font
            _bmFontPath.clear();
            bool ttf = false;
            const std::string& fontName = UIConfig::getRealFontName(_fontName, &ttf);
            if (ttf)
            {
                // Load TTF/OTF font from file path
                Ref<FontFile> fontFile;
                fontFile.instantiate();
                Error err = fontFile->load_dynamic_font(String(fontName.c_str()));
                if (err == OK)
                    _bmFont = fontFile;
                else
                    _bmFont.instantiate(); // fallback to empty
            }
            else
            {
                // Use system font by name
                Ref<SystemFont> sysFont;
                sysFont.instantiate();
                Vector<String> names = String(fontName.c_str()).split(",");
                sysFont->set_font_names(PackedStringArray(names));
                _bmFont = sysFont;
            }
        }
    }

    if (_fontSize != _textFormat->fontSize)
    {
        _fontSize = _textFormat->fontSize;
    }

    if (!isBMFont() || _bmFontCanTint)
    {
        set_modulate(_grayed ? toGrayed(_textFormat->color) : _textFormat->color);
    }

    queue_redraw();
}

bool FUILabel::setBMFontFilePath(const std::string& bmfontFilePath, float fontSize)
{
    BitmapFont* bmFont = (BitmapFont*)UIPackage::getItemAssetByURL(bmfontFilePath, PackageItemType::FONT);
    if (bmFont == nullptr)
    {
        _bmFontPath.clear();
        _bmFont.unref();
        _bmAtlasTexture.unref();
        return false;
    }

    if (fontSize == 0)
    {
        _bmFontSize = bmFont->getOriginalFontSize();
    }
    else
    {
        _bmFontSize = fontSize;
    }

    _bmFontPath = bmfontFilePath;
    _bmFont = bmFont->getFont();
    _bmAtlasTexture = bmFont->getAtlasTexture();
    _bmFontCanTint = true; // GODOT: BM fonts can tint if GPU rendered

    return true;
}

void FUILabel::setGrayed(bool value)
{
    if (_grayed != value)
    {
        _grayed = value;
        applyTextFormat();
    }
}

void FUILabel::setUnderlineColor(const Color& value)
{
    // GODOT_TODO: implement underline color
}

void FUILabel::updateBMFontScale()
{
    _bmfontScale = 1.0f;
}

float FUILabel::getTextWidth() const
{
    if (_text.empty()) return 0;
    if (_bmFont.is_valid())
    {
        Vector2 size = _bmFont->get_string_size(GObject::toGodotStr(_text),
            HORIZONTAL_ALIGNMENT_LEFT, -1, (int)_textFormat->fontSize);
        return size.x;
    }
    return 0;
}

float FUILabel::getTextHeight() const
{
    if (_text.empty()) return 0;
    if (_bmFont.is_valid())
    {
        Vector2 size = _bmFont->get_string_size(GObject::toGodotStr(_text),
            HORIZONTAL_ALIGNMENT_LEFT, -1, (int)_textFormat->fontSize);
        return size.y;
    }
    return _textFormat->fontSize;
}

void FUILabel::updateText()
{
    queue_redraw();
}

void FUILabel::_notification(int p_what) {
    if (p_what == NOTIFICATION_DRAW) {
        _draw();
        return;
    }
    if (p_what == NOTIFICATION_ENTER_TREE) {
        queue_redraw();
    }
    Node2D::_notification(p_what);
}

void FUILabel::_draw()
{
    if (_text.empty()) return;

    Ref<Font> font = _bmFont;
    if (font.is_null()) return;

    Color textColor = _grayed ? toGrayed(_textFormat->color) : _textFormat->color;
    int fontSize = (int)(_fontSize > 0 ? _fontSize : _textFormat->fontSize);

    // Effects
    if (_textFormat->hasEffect(TextFormat::SHADOW))
    {
        Color shadowColor = _grayed ? toGrayed(_textFormat->shadowColor) : _textFormat->shadowColor;
        Vector2 shadowPos = _textFormat->shadowOffset;
        draw_string(font, shadowPos, GObject::toGodotStr(_text),
            HORIZONTAL_ALIGNMENT_LEFT, -1, fontSize, shadowColor);
    }

    if (_textFormat->hasEffect(TextFormat::OUTLINE) && _textFormat->outlineSize > 0)
    {
        Color outlineColor = _grayed ? toGrayed(_textFormat->outlineColor) : _textFormat->outlineColor;
        draw_string_outline(font, Vector2(0, 0), GObject::toGodotStr(_text),
            HORIZONTAL_ALIGNMENT_LEFT, -1, fontSize, _textFormat->outlineSize, outlineColor);
    }
    else if (_textFormat->hasEffect(TextFormat::GLOW))
    {
        Color glowColor = _grayed ? toGrayed(_textFormat->glowColor) : _textFormat->glowColor;
        draw_string(font, Vector2(0, 0), GObject::toGodotStr(_text),
            HORIZONTAL_ALIGNMENT_LEFT, -1, fontSize, glowColor);
    }

    // Main text
    draw_string(font, Vector2(0, 0), GObject::toGodotStr(_text),
        HORIZONTAL_ALIGNMENT_LEFT, -1, fontSize, textColor);

    // Underline
    if (_textFormat->underline)
    {
        float textWidth = getTextWidth();
        draw_line(Vector2(0, fontSize + 1), Vector2(textWidth, fontSize + 1), textColor);
    }
}

void FUILabel::gd_setText(const String& text) { setText(text.utf8().get_data()); }
String FUILabel::gd_getText() { return GObject::toGodotStr(getText()); }

NS_FGUI_END
