#include "FUIInput.h"

#include "GObject.h"

#include "UIConfig.h"

#include "scene/gui/line_edit.h"

#include "scene/gui/text_edit.h"

#include "servers/display_server.h"



NS_FGUI_BEGIN



FUIInput::FUIInput() :

    _editor(nullptr),

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

    rebuildEditor();

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



void FUIInput::rebuildEditor()

{

    if (_editor)

    {

        remove_child(_editor);

        _editor->queue_free();

        _editor = nullptr;

    }



    if (_singleLine)

    {

        LineEdit* lineEdit = memnew(LineEdit);

        lineEdit->set_anchors_preset(Control::PRESET_FULL_RECT);

        lineEdit->set_mouse_filter(Control::MOUSE_FILTER_STOP);

        lineEdit->connect("text_changed", callable_mp(this, &FUIInput::_on_line_edit_changed));

        lineEdit->connect("text_submitted", callable_mp(this, &FUIInput::_on_line_edit_submitted));

        _editor = lineEdit;

    }

    else

    {

        TextEdit* textEdit = memnew(TextEdit);

        textEdit->set_anchors_preset(Control::PRESET_FULL_RECT);

        textEdit->set_mouse_filter(Control::MOUSE_FILTER_STOP);

        textEdit->set_line_wrapping_mode(TextEdit::LINE_WRAPPING_BOUNDARY);

        textEdit->connect("text_changed", callable_mp(this, &FUIInput::_on_text_edit_changed));

        _editor = textEdit;

    }



    add_child(_editor);

    setText(_text);

    setPassword(_password);

    setMaxLength(_maxLength);

    setEditable(_editable);

    setPlaceHolder(_placeHolder);

    applyEditorTheme();

}



void FUIInput::setText(const std::string& value)

{

    std::string text = filterText(value);

    if (_maxLength > 0 && (int)text.length() > _maxLength)

        text = text.substr(0, _maxLength);

    _text = text;



    if (!_editor)

        return;



    String godotText = GObject::toGodotStr(_text);

    if (LineEdit* lineEdit = Object::cast_to<LineEdit>(_editor))

    {

        if (lineEdit->get_text() != godotText)

            lineEdit->set_text(godotText);

    }

    else if (TextEdit* textEdit = Object::cast_to<TextEdit>(_editor))

    {

        if (textEdit->get_text() != godotText)

            textEdit->set_text(godotText);

    }

}



void FUIInput::setMaxLength(int value)

{

    _maxLength = value;

    if (LineEdit* lineEdit = Object::cast_to<LineEdit>(_editor))

        lineEdit->set_max_length(value);

}



void FUIInput::setPassword(bool value)

{

    _password = value;

    if (LineEdit* lineEdit = Object::cast_to<LineEdit>(_editor))

        lineEdit->set_secret(value);

}



void FUIInput::setSingleLine(bool value)

{

    if (_singleLine == value)

        return;

    _singleLine = value;

    rebuildEditor();

}



void FUIInput::setEditable(bool value)

{

    _editable = value;

    if (LineEdit* lineEdit = Object::cast_to<LineEdit>(_editor))

        lineEdit->set_editable(value);

    else if (TextEdit* textEdit = Object::cast_to<TextEdit>(_editor))

        textEdit->set_editable(value);

}



void FUIInput::setPlaceHolder(const std::string& value)

{

    _placeHolder = value;

    if (LineEdit* lineEdit = Object::cast_to<LineEdit>(_editor))

        lineEdit->set_placeholder(GObject::toGodotStr(_placeHolder));

}



void FUIInput::setKeyboardType(int value)

{

    _keyboardType = value;

}



bool FUIInput::isCharAllowed(char32_t ch) const

{

    if (_inputRestrict.empty())

        return true;



    if (_inputRestrict.length() >= 2 && _inputRestrict.front() == '[' && _inputRestrict.back() == ']')

    {

        std::string pattern = _inputRestrict.substr(1, _inputRestrict.length() - 2);

        for (size_t i = 0; i < pattern.length(); ++i)

        {

            if (i + 2 < pattern.length() && pattern[i + 1] == '-')

            {

                char32_t from = (unsigned char)pattern[i];

                char32_t to = (unsigned char)pattern[i + 2];

                if (ch >= from && ch <= to)

                    return true;

                i += 2;

            }

            else if ((unsigned char)pattern[i] == ch)

            {

                return true;

            }

        }

        return false;

    }



    return _inputRestrict.find((char)ch) != std::string::npos;

}



std::string FUIInput::filterText(const std::string& value) const

{

    if (_inputRestrict.empty())

        return value;



    String s = String::utf8(value.c_str());

    String filtered;

    for (int i = 0; i < s.length(); ++i)

    {

        char32_t ch = s[i];

        if (isCharAllowed(ch))

            filtered += String::chr(ch);

    }

    return filtered.utf8().get_data();

}



void FUIInput::_gui_input(const Ref<::InputEvent>& event)

{

    Ref<InputEventMouseButton> mb = event;

    if (mb.is_valid() && mb->is_pressed() && (int)mb->get_button_index() == (int)MouseButton::LEFT)

        openKeyboard();

}



void FUIInput::applyEditorTheme()

{

    if (!_editor || !_textFormat)

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

        _editor->add_theme_font_override("font", font);

        _editor->add_theme_font_size_override("font_size", _textFormat->fontSize);

    }



    _editor->add_theme_color_override("font_color", _textFormat->color);

    if (_placeholderFontSize > 0)

        _editor->add_theme_font_size_override("font_placeholder_size", _placeholderFontSize);

    _editor->add_theme_color_override("font_placeholder_color", _placeholderColor);

}



void FUIInput::applyTextFormat()

{

    applyEditorTheme();

}



void FUIInput::openKeyboard()

{

    if (!_editor)

        return;



    _editor->grab_focus();



    DisplayServer::VirtualKeyboardType vkType = DisplayServer::KEYBOARD_TYPE_DEFAULT;

    switch (_keyboardType)

    {

    case 1: vkType = DisplayServer::KEYBOARD_TYPE_NUMBER; break;

    case 2: vkType = DisplayServer::KEYBOARD_TYPE_NUMBER_DECIMAL; break;

    case 3: vkType = DisplayServer::KEYBOARD_TYPE_PHONE; break;

    case 4: vkType = DisplayServer::KEYBOARD_TYPE_EMAIL_ADDRESS; break;

    case 5: vkType = DisplayServer::KEYBOARD_TYPE_PASSWORD; break;

    case 6: vkType = DisplayServer::KEYBOARD_TYPE_URL; break;

    default: break;

    }



    if (!_singleLine)

        vkType = DisplayServer::KEYBOARD_TYPE_MULTILINE;



    DisplayServer::get_singleton()->virtual_keyboard_show(GObject::toGodotStr(_text), get_global_rect(), vkType);

}



void FUIInput::syncTextFromEditor()

{

    if (LineEdit* lineEdit = Object::cast_to<LineEdit>(_editor))

        _on_line_edit_changed(lineEdit->get_text());

    else if (TextEdit* textEdit = Object::cast_to<TextEdit>(_editor))

        _on_text_edit_changed();

}



void FUIInput::_on_line_edit_changed(const String& text)

{

    std::string value = filterText(text.utf8().get_data());

    if (_maxLength > 0 && (int)value.length() > _maxLength)

        value = value.substr(0, _maxLength);

    if (value != _text)

    {

        _text = value;

        if (LineEdit* lineEdit = Object::cast_to<LineEdit>(_editor))

        {

            String godotText = GObject::toGodotStr(_text);

            if (lineEdit->get_text() != godotText)

                lineEdit->set_text(godotText);

        }

    }

    else

    {

        _text = value;

    }

    if (_textChangedCallback)

        _textChangedCallback();

}



void FUIInput::_on_line_edit_submitted(const String& text)

{

    _on_line_edit_changed(text);

    if (_submittedCallback)

        _submittedCallback();

}



void FUIInput::_on_text_edit_changed()

{

    if (TextEdit* textEdit = Object::cast_to<TextEdit>(_editor))

        _on_line_edit_changed(textEdit->get_text());

}



void FUIInput::gd_setText(const String& text) { setText(text.utf8().get_data()); }

String FUIInput::gd_getText() const { return GObject::toGodotStr(getText()); }

void FUIInput::gd_setInputRestrict(const String& value) { setInputRestrict(value.utf8().get_data()); }

String FUIInput::gd_getInputRestrict() const { return GObject::toGodotStr(getInputRestrict()); }

void FUIInput::gd_setPlaceHolder(const String& value) { setPlaceHolder(value.utf8().get_data()); }



NS_FGUI_END

