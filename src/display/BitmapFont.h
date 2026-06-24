#ifndef __BITMAPFONT_H__
#define __BITMAPFONT_H__

#include "FairyGUIMacros.h"
#include "PackageItem.h"
#include <unordered_map>
#include <vector>

NS_FGUI_BEGIN

struct FontLetterDefinition
{
    float U;            // left texture coordinate
    float V;            // top texture coordinate
    float width;        // width in texture
    float height;       // height in texture
    float offsetX;      // x offset
    float offsetY;      // y offset
    float xAdvance;     // x advance
    bool validDefinition;

    FontLetterDefinition()
        : U(0), V(0), width(0), height(0)
        , offsetX(0), offsetY(0), xAdvance(0)
        , validDefinition(false)
    {
    }
};

class FontAtlas
{
public:
    FontAtlas(BitmapFont& font);
    ~FontAtlas();

    void addLetterDefinition(unsigned short ch, const FontLetterDefinition& def);
    void addTexture(Ref<Texture2D> texture, int index);
    void setLineHeight(float height);

    float getLineHeight() const { return _lineHeight; }
    const FontLetterDefinition* getLetterDefinition(unsigned short ch) const;
    Ref<Texture2D> getTexture(int index) const;

private:
    BitmapFont& _font;
    std::unordered_map<unsigned short, FontLetterDefinition> _letterDefinitions;
    std::vector<Ref<Texture2D>> _textures;
    float _lineHeight;
};

class BitmapFont
{
public:
    BitmapFont();
    virtual ~BitmapFont();

    static BitmapFont* create();
    void releaseAtlas();

    Ref<Font> getFont() const { return _font; }
    Ref<Texture2D> getAtlasTexture() const { return _atlasTexture; }

    float getOriginalFontSize() const { return _originalFontSize; }
    int getFontSize() const { return (int)_originalFontSize; }

    const FontLetterDefinition* getLetterDefinition(unsigned short ch) const;
    const FontAtlas* getFontAtlas() const { return _fontAtlas; }

private:
    float _originalFontSize;
    Ref<Font> _font;
    Ref<Texture2D> _atlasTexture;
    FontAtlas* _fontAtlas;

    // Bits from cocos BitmapFont
    bool _canTint;
    bool _resizable;
    int _fontSize;

    friend class UIPackage;
    friend class FontAtlas;
};

NS_FGUI_END

#endif
