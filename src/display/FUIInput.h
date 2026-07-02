#ifndef __FUIINPUT_H__
#define __FUIINPUT_H__

#include "FairyGUIMacros.h"
#include "TextFormat.h"
#include "core/input/input_event.h"
#include "scene/gui/control.h"
#include <functional>

class LineEdit;
class TextEdit;

NS_FGUI_BEGIN

class FUIInput : public Control
{
    GDCLASS(FUIInput, Control)

public:
    FUIInput();
    virtual ~FUIInput();

    static FUIInput* create();

    void setText(const std::string& value);
    const std::string& getText() const { return _text; }

    void gd_setText(const String& value);
    String gd_getText() const;

    void setMaxLength(int value);
    int getMaxLength() const { return _maxLength; }

    void setPassword(bool value);
    bool isPassword() const { return _password; }

    void setSingleLine(bool value);
    bool isSingleLine() const { return _singleLine; }

    void setEditable(bool value);
    bool isEditable() const { return _editable; }

    void setInputRestrict(const std::string& value) { _inputRestrict = value; }
    const std::string& getInputRestrict() const { return _inputRestrict; }

    void gd_setInputRestrict(const String& value);
    String gd_getInputRestrict() const;

    void setPlaceHolder(const std::string& value);
    const std::string& getPlaceHolder() const { return _placeHolder; }
    void gd_setPlaceHolder(const String& value);

    void setPlaceholderColor(const Color& value) { _placeholderColor = value; }
    Color getPlaceholderColor() const { return _placeholderColor; }

    void setPlaceholderFontSize(int value) { _placeholderFontSize = value; }
    int getPlaceholderFontSize() const { return _placeholderFontSize; }

    void setKeyboardType(int value);
    int getKeyboardType() const { return _keyboardType; }

    void applyTextFormat();
    void openKeyboard();
    void setSubmittedCallback(const std::function<void()>& callback) { _submittedCallback = callback; }
    void setTextChangedCallback(const std::function<void()>& callback) { _textChangedCallback = callback; }

    TextFormat* getTextFormat() const { return _textFormat; }

    static void _bind_methods();
    void _gui_input(const Ref<::InputEvent>& event);

private:
    void rebuildEditor();
    void applyEditorTheme();
    bool isCharAllowed(char32_t ch) const;
    std::string filterText(const std::string& value) const;
    void syncTextFromEditor();
    void _on_line_edit_changed(const String& text);
    void _on_line_edit_submitted(const String& text);
    void _on_text_edit_changed();

    Control* _editor;
    std::string _text;
    int _maxLength;
    bool _password;
    bool _singleLine;
    bool _editable;
    std::string _inputRestrict;
    std::string _placeHolder;
    Color _placeholderColor;
    int _placeholderFontSize;
    int _keyboardType;
    bool _focused;
    TextFormat* _textFormat;
    std::function<void()> _submittedCallback;
    std::function<void()> _textChangedCallback;
};

NS_FGUI_END

#endif
