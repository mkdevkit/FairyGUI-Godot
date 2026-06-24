#ifndef __FUIINPUT_H__
#define __FUIINPUT_H__

#include "FairyGUIMacros.h"
#include "TextFormat.h"
#include "core/input/input_event.h"

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

    void setMaxLength(int value) { _maxLength = value; }
    int getMaxLength() const { return _maxLength; }

    void setPassword(bool value) { _password = value; }
    bool isPassword() const { return _password; }

    void setSingleLine(bool value) { _singleLine = value; }
    bool isSingleLine() const { return _singleLine; }

    void setEditable(bool value) { _editable = value; }
    bool isEditable() const { return _editable; }

    void setInputRestrict(const std::string& value) { _inputRestrict = value; }
    const std::string& getInputRestrict() const { return _inputRestrict; }

    void gd_setInputRestrict(const String& value);
    String gd_getInputRestrict() const;

    void setPlaceHolder(const std::string& value) { _placeHolder = value; }
    const std::string& getPlaceHolder() const { return _placeHolder; }
    void gd_setPlaceHolder(const String& value);

    void setPlaceholderColor(const Color& value) { _placeholderColor = value; }
    Color getPlaceholderColor() const { return _placeholderColor; }

    void setPlaceholderFontSize(int value) { _placeholderFontSize = value; }
    int getPlaceholderFontSize() const { return _placeholderFontSize; }

    void setKeyboardType(int value) { _keyboardType = value; }
    int getKeyboardType() const { return _keyboardType; }

    void applyTextFormat();
    void openKeyboard();

    TextFormat* getTextFormat() const { return _textFormat; }

    static void _bind_methods();
    void _gui_input(const Ref<::InputEvent>& event);

protected:

private:
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
};

NS_FGUI_END

#endif
