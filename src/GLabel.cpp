#include "GLabel.h"
#include "GButton.h"
#include "GTextField.h"
#include "GTextInput.h"
#include "PackageItem.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN
GLabel::GLabel() :
    _titleObject(nullptr),
    _iconObject(nullptr)
{
}
GLabel::~GLabel()
{
}

const std::string& GLabel::getText() const
{
    if (_titleObject != nullptr)
        return _titleObject->getText();
    else
        return "";
}

void GLabel::setText(const std::string & value)
{
    if (_titleObject != nullptr)
        _titleObject->setText(value);
    updateGear(6);
}

const std::string& GLabel::getIcon() const
{
    if (_iconObject != nullptr)
        return _iconObject->getIcon();
    else
        return "";
}

void GLabel::setIcon(const std::string & value)
{
    if (_iconObject != nullptr)
        _iconObject->setIcon(value);
    updateGear(7);
}

Color GLabel::getTitleColor() const
{
    GTextField* tf = getTextField();
    if (tf)
        return tf->getColor();
    else
        return Color(0,0,0,1);
}

void GLabel::setTitleColor(const Color & value)
{
    GTextField* tf = getTextField();
    if (tf)
        tf->setColor(value);
}

int GLabel::getTitleFontSize() const
{
    GTextField* tf = getTextField();
    if (tf)
        return tf->getFontSize();
    else
        return 0;
}

void GLabel::setTitleFontSize(int value)
{
    GTextField* tf = getTextField();
    if (tf)
        tf->setFontSize(value);
}

GTextField * GLabel::getTextField() const
{
    if (dynamic_cast<GTextField*>(_titleObject))
        return dynamic_cast<GTextField*>(_titleObject);
    else if (dynamic_cast<GLabel*>(_titleObject))
        return dynamic_cast<GLabel*>(_titleObject)->getTextField();
    else if (dynamic_cast<GButton*>(_titleObject))
        return dynamic_cast<GButton*>(_titleObject)->getTextField();
    else
        return nullptr;
}

Variant GLabel::getProp(ObjectPropID propId)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        return Variant(ToolSet::colorToInt(getTitleColor()));
    case ObjectPropID::OutlineColor:
    {
        GTextField* tf = getTextField();
        if (tf != nullptr)
            return Variant(ToolSet::colorToInt(tf->getOutlineColor()));
        else
            return Variant();
    }
    case ObjectPropID::FontSize:
        return Variant(getTitleFontSize());
    default:
        return GComponent::getProp(propId);
    }
}

void GLabel::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        setTitleColor(ToolSet::intToColor(value.operator int()));
        break;
    case ObjectPropID::OutlineColor:
    {
        GTextField* tf = getTextField();
        if (tf != nullptr)
            tf->setOutlineColor(ToolSet::intToColor(value.operator int()));
        break;
    }
    case ObjectPropID::FontSize:
        setTitleFontSize(value.operator int64_t());
        break;
    default:
        GComponent::setProp(propId, value);
        break;
    }
}

void GLabel::constructExtension(ByteBuffer* buffer)
{
    _titleObject = getChild("title");
    _iconObject = getChild("icon");
}

void GLabel::setup_afterAdd(ByteBuffer* buffer, int beginPos)
{
    GComponent::setup_afterAdd(buffer, beginPos);

    if (!buffer->seek(beginPos, 6))
        return;

    if ((ObjectType)buffer->readByte() != _packageItem->objectType)
        return;

    const std::string* str;

    if ((str = buffer->readSP()))
        setTitle(*str);
    if ((str = buffer->readSP()))
        setIcon(*str);
    if (buffer->readBool())
        setTitleColor((Color)buffer->readColor());
    int iv = buffer->readInt();
    if (iv != 0)
        setTitleFontSize(iv);

    if (buffer->readBool())
    {
        // GODOT_ADAPT: GTextInput not yet ported, skip its data
        buffer->skip(13);
    }
}

void GLabel::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setTitle", "title"), &GLabel::gd_setTitle);
    ClassDB::bind_method(D_METHOD("getTitle"), &GLabel::gd_getTitle);

    ClassDB::bind_method(D_METHOD("setText", "text"), &GLabel::gd_setText);
    ClassDB::bind_method(D_METHOD("getText"), &GLabel::gd_getText);

    ClassDB::bind_method(D_METHOD("setIcon", "icon"), &GLabel::gd_setIcon);
    ClassDB::bind_method(D_METHOD("getIcon"), &GLabel::gd_getIcon);

    ClassDB::bind_method(D_METHOD("setTitleColor", "color"), &GLabel::setTitleColor);
    ClassDB::bind_method(D_METHOD("getTitleColor"), &GLabel::getTitleColor);

    ClassDB::bind_method(D_METHOD("setTitleFontSize", "size"), &GLabel::setTitleFontSize);
    ClassDB::bind_method(D_METHOD("getTitleFontSize"), &GLabel::getTitleFontSize);
}

void GLabel::gd_setTitle(const String& value) { setTitle(value.utf8().get_data()); }
String GLabel::gd_getTitle() { return String(getTitle().c_str()); }
void GLabel::gd_setText(const String& value) { setText(value.utf8().get_data()); }
String GLabel::gd_getText() const { return String(getText().c_str()); }
void GLabel::gd_setIcon(const String& value) { setIcon(value.utf8().get_data()); }
String GLabel::gd_getIcon() const { return String(getIcon().c_str()); }

NS_FGUI_END

