#include "FUIInput.h"
#include "GObject.h"
#include "UIConfig.h"
#include "scene/gui/line_edit.h"

NS_FGUI_BEGIN

FUIInput::FUIInput() :
    _lineEdit(nullptr),
    _maxLength(0),
    _password(false),
    _singleLine(true),
    _editable(true),
    _placeholderColor(Color(0.5, 0.5, 0.5, 1)),
    _placeholderFontSize(0),
    _keyboardType(0),
    _focused(false),
    _textFormat(new TextFormat())
{
    set_focus_mode(FOCUS_ALL);
    set_mouse_filter(MOUSE_FILTER_STOP);

    _lineEdit = memnew(::LineEdit);
    _lineEdit->set_anchors_preset(Control::PRESET_FULL_RECT);
    _lineEdit->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    add_child(_lineEdit);

    _lineEdit->connect("text_changed", callable_mp(this, &FUIInput::_on_line_edit_changed));
    _lineEdit->connect("text_submitted", callable_mp(this, &FUIInput::_on_line_edit_submitted));

    applyTextFormat();
}

FUIInput::~FUIInput()
{
    delete _textFormat;
}

void FUIInput::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setText", "text"), &FUIInput::gd_setText);
    ClassDB::bind_method(D_METHOD("getText"), &FUIInput::gd_getText);

    ClassDB::bind_method(D_METHOD("setMaxLength", "value"), &FUIInput::setMaxLength);
    ClassDB::bind_method(D_METHOD("getMaxLength"), &FUIInput::getMaxLength);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "maxLength"), "setMaxLength", "getMaxLength");

    ClassDB::bind_method(D_METHOD("setPassword", "value"), &FUIInput::setPassword);
    ClassDB::bind_method(D_METHOD("isPassword"), &FUIInput::isPassword);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "password"), "setPassword", "isPassword");

    ClassDB::bind_method(D_METHOD("setSingleLine", "value"), &FUIInput::setSingleLine);
    ClassDB::bind_method(D_METHOD("isSingleLine"), &FUIInput::isSingleLine);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "singleLine"), "setSingleLine", "isSingleLine");

    ClassDB::bind_method(D_METHOD("setEditable", "value"), &FUIInput::setEditable);
    ClassDB::bind_method(D_METHOD("isEditable"), &FUIInput::isEditable);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editable"), "setEditable", "isEditable");

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
    std::string text = value;
    if (_maxLength > 0 && (int)text.length() > _maxLength)
        text = text.substr(0, _maxLength);
    _text = text;
    if (_lineEdit && _lineEdit->get_text() != GObject::toGodotStr(_text))
        _lineEdit->set_text(GObject::toGodotStr(_text));
}

void FUIInput::setMaxLength(int value)
{
    _maxLength = value;
    if (_lineEdit)
        _lineEdit->set_max_length(value);
}

void FUIInput::setPassword(bool value)
{
    _password = value;
    if (_lineEdit)
        _lineEdit->set_secret(value);
}

void FUIInput::setSingleLine(bool value)
{
    _singleLine = value;
}

void FUIInput::setEditable(bool value)
{
    _editable = value;
    if (_lineEdit)
        _lineEdit->set_editable(value);
}

void FUIInput::setPlaceHolder(const std::string& value)
{
    _placeHolder = value;
    if (_lineEdit)
        _lineEdit->set_placeholder(GObject::toGodotStr(_placeHolder));
}

void FUIInput::_gui_input(const Ref<::InputEvent>& event)
{
    Ref<InputEventMouseButton> mb = event;
    if (mb.is_valid() && mb->is_pressed() && (int)mb->get_button_index() == (int)MouseButton::LEFT)
        openKeyboard();
}

void FUIInput::applyTextFormat()
{
    if (!_lineEdit || !_textFormat)
        return;

    bool ttf = false;
    const std::string& fontName = UIConfig::getRealFontName(_textFormat->face, &ttf);
    Ref<Font> font;
    if (ttf)
    {
        Ref<FontFile> fontFile;
        fontFile.instantiate();
        if (fontFile->load_dynamic_font(GObject::toGodotStr(fontName)) == OK)
            font = fontFile;
    }
    else
    {
        Ref<SystemFont> sysFont;
        sysFont.instantiate();
        sysFont->set_font_names(PackedStringArray(GObject::toGodotStr(fontName).split(",")));
        font = sysFont;
    }

    if (font.is_valid())
    {
        _lineEdit->add_theme_font_override("font", font);
        _lineEdit->add_theme_font_size_override("font_size", _textFormat->fontSize);
    }

    _lineEdit->add_theme_color_override("font_color", _textFormat->color);
    if (_placeholderFontSize > 0)
        _lineEdit->add_theme_font_size_override("font_placeholder_size", _placeholderFontSize);
    _lineEdit->add_theme_color_override("font_placeholder_color", _placeholderColor);
}

void FUIInput::openKeyboard()
{
    if (_lineEdit)
        _lineEdit->grab_focus();
}

void FUIInput::_on_line_edit_changed(const String& text)
{
    std::string value = text.utf8().get_data();
    if (_maxLength > 0 && (int)value.length() > _maxLength)
        value = value.substr(0, _maxLength);
    _text = value;
}

void FUIInput::_on_line_edit_submitted(const String& text)
{
    _on_line_edit_changed(text);
}

void FUIInput::gd_setText(const String& text) { setText(text.utf8().get_data()); }
String FUIInput::gd_getText() const { return GObject::toGodotStr(getText()); }
void FUIInput::gd_setInputRestrict(const String& value) { setInputRestrict(value.utf8().get_data()); }
String FUIInput::gd_getInputRestrict() const { return GObject::toGodotStr(getInputRestrict()); }
void FUIInput::gd_setPlaceHolder(const String& value) { setPlaceHolder(value.utf8().get_data()); }

NS_FGUI_END
