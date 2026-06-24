#include "BitmapFont.h"

NS_FGUI_BEGIN

FontAtlas::FontAtlas(BitmapFont& font)
    : _font(font)
    , _lineHeight(0)
{
}

FontAtlas::~FontAtlas()
{
}

void FontAtlas::addLetterDefinition(unsigned short ch, const FontLetterDefinition& def)
{
    _letterDefinitions[ch] = def;
}

void FontAtlas::addTexture(Ref<Texture2D> texture, int index)
{
    if (index >= (int)_textures.size())
        _textures.resize(index + 1);
    _textures[index] = texture;
}

void FontAtlas::setLineHeight(float height)
{
    _lineHeight = height;
}

const FontLetterDefinition* FontAtlas::getLetterDefinition(unsigned short ch) const
{
    auto it = _letterDefinitions.find(ch);
    if (it != _letterDefinitions.end())
        return &it->second;
    return nullptr;
}

Ref<Texture2D> FontAtlas::getTexture(int index) const
{
    if (index >= 0 && index < (int)_textures.size())
        return _textures[index];
    return Ref<Texture2D>();
}

BitmapFont::BitmapFont()
    : _originalFontSize(0)
    , _fontAtlas(nullptr)
    , _canTint(false)
    , _resizable(false)
    , _fontSize(0)
{
    _font.instantiate();
}

BitmapFont::~BitmapFont()
{
    releaseAtlas();
}

BitmapFont* BitmapFont::create()
{
    return memnew(BitmapFont);
}

void BitmapFont::releaseAtlas()
{
    if (_fontAtlas)
    {
        delete _fontAtlas;
        _fontAtlas = nullptr;
    }
}

const FontLetterDefinition* BitmapFont::getLetterDefinition(unsigned short ch) const
{
    if (_fontAtlas)
        return _fontAtlas->getLetterDefinition(ch);
    return nullptr;
}

NS_FGUI_END
