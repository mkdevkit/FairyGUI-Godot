#include "GTextField.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN
GTextField::GTextField()
    : _templateVars(nullptr),
      _ubbEnabled(false),
      _autoSize(AutoSizeType::BOTH)
{
}

GTextField::~GTextField()
{
    delete _templateVars; _templateVars = nullptr;
}

void GTextField::setText(const std::string& value)
{
    _text = value;
    setTextFieldText();
    updateGear(6);
    updateSize();
}

void GTextField::setUBBEnabled(bool value)
{
    if (_ubbEnabled != value)
    {
        _ubbEnabled = value;
        setTextFieldText();
        updateSize();
    }
}

void GTextField::setColor(const Color& value)
{
    TextFormat* tf = getTextFormat();
    if (tf->color != value)
    {
        tf->color = value;
        applyTextFormat();
    }
}

void GTextField::setFontSize(float value)
{
    TextFormat* tf = getTextFormat();
    if (tf->fontSize != value)
    {
        tf->fontSize = value;
        applyTextFormat();
    }
}

void GTextField::setOutlineColor(const Color& value)
{
    TextFormat* tf = getTextFormat();
    if (tf->outlineColor != value)
    {
        tf->outlineColor = value;
        applyTextFormat();
    }
}

void GTextField::setTemplateVars(std::unordered_map<std::string, Variant>* value)
{
    if (_templateVars == nullptr && value == nullptr)
        return;

    if (value == nullptr)
    {
        delete _templateVars;
        _templateVars = nullptr;
    }
    else
    {
        if (_templateVars == nullptr)
            _templateVars = new std::unordered_map<std::string, Variant>();
        *_templateVars = *value;
    }

    flushVars();
}

GTextField* GTextField::setVar(const std::string& name, const Variant& value)
{
    if (_templateVars == nullptr)
        _templateVars = new std::unordered_map<std::string, Variant>();

    (*_templateVars)[name] = value;

    return this;
}

void GTextField::flushVars()
{
    setTextFieldText();
    updateSize();
}

void GTextField::updateSize()
{
}

Variant GTextField::getProp(ObjectPropID propId)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        return Variant(ToolSet::colorToInt(getColor()));
    case ObjectPropID::OutlineColor:
        return Variant(ToolSet::colorToInt(getOutlineColor()));
    case ObjectPropID::FontSize:
        return Variant(getFontSize());
    default:
        return GObject::getProp(propId);
    }
}

void GTextField::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        setColor(ToolSet::intToColor(value.operator int()));
        break;
    case ObjectPropID::OutlineColor:
        setOutlineColor(ToolSet::intToColor(value.operator int()));
        break;
    case ObjectPropID::FontSize:
        setFontSize(value.operator int64_t());
        break;
    default:
        GObject::setProp(propId, value);
        break;
    }
}

void GTextField::setup_beforeAdd(ByteBuffer* buffer, int beginPos)
{
    GObject::setup_beforeAdd(buffer, beginPos);

    buffer->seek(beginPos, 5);

    TextFormat* tf = getTextFormat();

    tf->face = buffer->readS();
    tf->fontSize = buffer->readShort();
    tf->color = (Color)buffer->readColor();
    tf->align = buffer->readByte();
    tf->verticalAlign = buffer->readByte();
    tf->lineSpacing = buffer->readShort();
    tf->letterSpacing = buffer->readShort();
    _ubbEnabled = buffer->readBool();
    setAutoSize((AutoSizeType)buffer->readByte());
    tf->underline = buffer->readBool();
    tf->italics = buffer->readBool();
    tf->bold = buffer->readBool();
    if (buffer->readBool())
        setSingleLine(true);
    if (buffer->readBool())
    {
        tf->outlineColor = (Color)buffer->readColor();
        tf->outlineSize = buffer->readFloat();
        tf->enableEffect(TextFormat::OUTLINE);
    }

    if (buffer->readBool())
    {
        tf->shadowColor = (Color)buffer->readColor();
        float f1 = buffer->readFloat();
        float f2 = buffer->readFloat();
        tf->shadowOffset = Vector2(f1, -f2);
        tf->enableEffect(TextFormat::SHADOW);
    }

    if (buffer->readBool())
        _templateVars = new std::unordered_map<std::string, Variant>();
}

void GTextField::setup_afterAdd(ByteBuffer* buffer, int beginPos)
{
    GObject::setup_afterAdd(buffer, beginPos);

    applyTextFormat();

    buffer->seek(beginPos, 6);

    const std::string& str = buffer->readS();
    if (!str.empty())
        setText(str);
}

std::string GTextField::parseTemplate(const char* text)
{
    const char* pString = text;

    ssize_t pos;
    ssize_t pos2;
    std::string tag, attr;
    std::string repl;
    std::string out;

    while (*pString != '\0')
    {
        const char* p = strchr(pString, '{');
        if (!p)
        {
            out.append(pString);
            break;
        }

        pos = p - pString;
        if (pos > 0 && *(p - 1) == '\\')
        {
            out.append(pString, pos - 1);
            out.append("{");
            pString += pos + 1;
            continue;
        }

        out.append(pString, pos);
        pString += pos;

        p = strchr(pString, '}');
        if (!p)
        {
            out.append(pString);
            break;
        }

        pos = p - pString;
        if (pos == 1)
        {
            out.append(pString, 0, 2);
            pString += 2;
            continue;
        }

        tag.assign(pString + 1, pos - 1);

        attr.clear();
        repl.clear();
        pos2 = tag.find('=');
        if (pos2 != -1)
        {
            auto it = _templateVars->find(tag.substr(0, pos2));
            if (it != _templateVars->end())
                out.append(std::string((const char*)it->second.stringify().utf8().ptr()));
            else
                out.append(tag.substr(pos2 + 1));
        }
        else
        {
            auto it = _templateVars->find(tag);
            if (it != _templateVars->end())
                out.append(std::string((const char*)it->second.stringify().utf8().ptr()));
        }
        pString += pos + 1;
    }
    return out;
}

//---------------------------

GBasicTextField::GBasicTextField() : _label(nullptr),
                                     _updatingSize(false)
{
    _touchDisabled = true;
}

GBasicTextField::~GBasicTextField()
{
}

void GBasicTextField::handleInit()
{
    _label = FUILabel::create();
    _displayObject = _label;

}

void GBasicTextField::applyTextFormat()
{
    _label->applyTextFormat();
    updateGear(4);
    if (!_underConstruct)
        updateSize();
}

void GBasicTextField::setAutoSize(AutoSizeType value)
{
    _autoSize = value;
    switch (value)
    {
    case AutoSizeType::NONE:
        // setOverflow not available on FUILabel
        break;
    case AutoSizeType::BOTH:
        // setOverflow not available on FUILabel
        break;
    case AutoSizeType::HEIGHT:
        // setOverflow not available on FUILabel
        break;
    case AutoSizeType::SHRINK:
        // setOverflow not available on FUILabel
        break;
    }

    if (_autoSize == AutoSizeType::BOTH) ;
    else if (_autoSize == AutoSizeType::HEIGHT) ;
        // GODOT_ADAPT: set_size not available on FUILabel

    if (!_underConstruct)
        updateSize();
}

void GBasicTextField::setSingleLine(bool value)
{
    _label->setWrapEnabled(!value);
    if (!_underConstruct)
        updateSize();
}

void GBasicTextField::setTextFieldText()
{
    if (_templateVars != nullptr)
        _label->setText(parseTemplate(_text.c_str()));
    else
        _label->setText(_text);
}

void GBasicTextField::updateSize()
{
    if (_updatingSize)
        return;

    _updatingSize = true;

    Vector2 sz = _label->getTextSize();
    if (_autoSize == AutoSizeType::BOTH)
        setSize(sz.x, sz.y);
    else if (_autoSize == AutoSizeType::HEIGHT)
        setHeight(sz.y);

    _updatingSize = false;
}

void GBasicTextField::handleSizeChanged()
{
    if (_updatingSize)
        return;

    _label->_contentSize = Vector2(_size.width, _size.height);
    _label->queue_redraw();

    if (_autoSize != AutoSizeType::BOTH)
    {
        // GODOT_ADAPT: set_size not available on FUILabel

        if (_autoSize == AutoSizeType::HEIGHT)
        {
            if (!_text.empty())
                setSizeDirectly(_size.width, _label->getTextSize().y);
        }
    }
}

void GBasicTextField::handleGrayedChanged()
{
    GObject::handleGrayedChanged();

    _label->setGrayed(_finalGrayed);
}

void GBasicTextField::_bind_methods()
{
    // set_text/get_text inherited from GTextField
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "setColor", "getColor");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fontSize"), "setFontSize", "getFontSize");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "singleLine"), "setSingleLine", "isSingleLine");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "autoSize"), "setAutoSize", "getAutoSize");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ubbEnabled"), "setUBBEnabled", "isUBBEnabled");
}

Vector2 GTextField::getTextSize()
{
    // GODOT_ADAPT: return default size - subclasses should override
    return Vector2(0, 0);
}

void GTextField::_bind_methods()
{
    // setText/getText inherited from GObject

    ClassDB::bind_method(D_METHOD("setColor", "color"), &GTextField::setColor);
    ClassDB::bind_method(D_METHOD("getColor"), &GTextField::getColor);

    ClassDB::bind_method(D_METHOD("setFontSize", "size"), &GTextField::setFontSize);
    ClassDB::bind_method(D_METHOD("getFontSize"), &GTextField::getFontSize);

    ClassDB::bind_method(D_METHOD("setSingleLine", "value"), &GTextField::setSingleLine);
    ClassDB::bind_method(D_METHOD("isSingleLine"), &GTextField::isSingleLine);

    ClassDB::bind_method(D_METHOD("setAutoSize", "value"), &GTextField::gd_setAutoSize);
    ClassDB::bind_method(D_METHOD("getAutoSize"), &GTextField::gd_getAutoSize);

    ClassDB::bind_method(D_METHOD("setUBBEnabled", "value"), &GTextField::setUBBEnabled);
    ClassDB::bind_method(D_METHOD("isUBBEnabled"), &GTextField::isUBBEnabled);

    ClassDB::bind_method(D_METHOD("setTemplateVars", "vars"), &GTextField::gd_setTemplateVars);
    ClassDB::bind_method(D_METHOD("getTextSize"), &GTextField::getTextSize);
}

void GTextField::gd_setTemplateVars(const Dictionary& vars) {
    if (_templateVars)
        delete static_cast<std::unordered_map<std::string, Variant>*>(_templateVars);
    _templateVars = nullptr;
}

NS_FGUI_END


