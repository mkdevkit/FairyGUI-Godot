#include "FUIInput.h"
#include "GObject.h"

NS_FGUI_BEGIN

FUIInput::FUIInput() :
    _maxLength(0),
    _password(false),
    _singleLine(true),
    _editable(true),
    _placeholderColor(Color(0.5, 0.5, 0.5, 1)),
    _placeholderFontSize(0),
    _keyboardType(0),
    _focused(false),
    _textFormat(nullptr)
{
    set_focus_mode(FOCUS_ALL);
}

FUIInput::~FUIInput()
{
}

void FUIInput::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setText", "text"), &FUIInput::gd_setText);
    ClassDB::bind_method(D_METHOD("getText"), &FUIInput::gd_getText);

    ClassDB::bind_method(D_METHOD("setMaxLength", "value"), &FUIInput::setMaxLength);
    ClassDB::bind_method(D_METHOD("getMaxLength"), &FUIInput::getMaxLength);
    // ADD_PROPERTY(PropertyInfo(Variant::INT, "maxLength"), "setMaxLength", "getMaxLength");

    ClassDB::bind_method(D_METHOD("setPassword", "value"), &FUIInput::setPassword);
    ClassDB::bind_method(D_METHOD("isPassword"), &FUIInput::isPassword);
    // ADD_PROPERTY(PropertyInfo(Variant::BOOL, "password"), "setPassword", "isPassword");

    ClassDB::bind_method(D_METHOD("setSingleLine", "value"), &FUIInput::setSingleLine);
    ClassDB::bind_method(D_METHOD("isSingleLine"), &FUIInput::isSingleLine);
    // ADD_PROPERTY(PropertyInfo(Variant::BOOL, "singleLine"), "setSingleLine", "isSingleLine");

    ClassDB::bind_method(D_METHOD("setEditable", "value"), &FUIInput::setEditable);
    ClassDB::bind_method(D_METHOD("isEditable"), &FUIInput::isEditable);
    // ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editable"), "setEditable", "isEditable");

    ClassDB::bind_method(D_METHOD("setInputRestrict", "value"), &FUIInput::gd_setInputRestrict);
    ClassDB::bind_method(D_METHOD("getInputRestrict"), &FUIInput::gd_getInputRestrict);

    ClassDB::bind_method(D_METHOD("setPlaceholder", "value"), &FUIInput::gd_setPlaceHolder);
    ClassDB::bind_method(D_METHOD("setPlaceholderColor", "value"), &FUIInput::setPlaceholderColor);
    ClassDB::bind_method(D_METHOD("setPlaceholderFontSize", "value"), &FUIInput::setPlaceholderFontSize);

    ClassDB::bind_method(D_METHOD("setKeyboardType", "value"), &FUIInput::setKeyboardType);
    ClassDB::bind_method(D_METHOD("getKeyboardType"), &FUIInput::getKeyboardType);

    ClassDB::bind_method(D_METHOD("openKeyboard"), &FUIInput::openKeyboard);

    ClassDB::bind_method(D_METHOD("applyTextFormat"), &FUIInput::applyTextFormat);
}

FUIInput* FUIInput::create()
{
    return memnew(FUIInput);
}

void FUIInput::setText(const std::string& value)
{
    if (_maxLength > 0 && (int)value.length() > _maxLength)
        _text = value.substr(0, _maxLength);
    else
        _text = value;
}

void FUIInput::_gui_input(const Ref<::InputEvent>& event)
{
    // GODOT_TODO: handle text input events
}

void FUIInput::gd_setText(const String& text) { setText(text.utf8().get_data()); }
String FUIInput::gd_getText() const { return GObject::toGodotStr(getText()); }
void FUIInput::gd_setInputRestrict(const String& value) { setInputRestrict(value.utf8().get_data()); }
String FUIInput::gd_getInputRestrict() const { return GObject::toGodotStr(getInputRestrict()); }
void FUIInput::gd_setPlaceHolder(const String& value) { setPlaceHolder(value.utf8().get_data()); }

void FUIInput::applyTextFormat()
{
    // GODOT_TODO: apply text format properties to rendered text
}

void FUIInput::openKeyboard()
{
    grab_focus();
}

NS_FGUI_END
