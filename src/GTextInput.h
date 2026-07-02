#ifndef __GTEXTINPUT_H__
#define __GTEXTINPUT_H__

#include "FairyGUIMacros.h"
#include "GTextField.h"
#include "display/FUIInput.h"

NS_FGUI_BEGIN

class GTextInput : public GTextField
{
    GDCLASS(GTextInput, GTextField)

public:
    GTextInput();
    virtual ~GTextInput();

    FAIRYGUI_CREATE(GTextInput)

    static void _bind_methods();

    virtual bool isSingleLine() const override;
    virtual void setSingleLine(bool value) override;

    virtual TextFormat* getTextFormat() const override { return _input->getTextFormat(); }
    virtual void applyTextFormat() override;

    virtual Vector2 getTextSize() override;

    void setPrompt(const std::string& value);
    void gd_setPrompt(const String& value) { setPrompt(value.utf8().get_data()); }
    void setPassword(bool value);
    void setKeyboardType(int value);
    void setMaxLength(int value);
    void setRestrict(const std::string& value);

    void editBoxReturn();
    void editBoxTextChanged(const std::string& text);
    void onTextSubmitted();

protected:
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;
    virtual void setup_beforeAdd(ByteBuffer* buffer, int beginPos) override;

    virtual void setTextFieldText() override;

private:
    FUIInput* _input;
};

NS_FGUI_END

#endif
