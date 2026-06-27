#ifndef __GTEXTFIELD_H__
#define __GTEXTFIELD_H__

#include "FairyGUIMacros.h"
#include "GObject.h"
#include "display/FUILabel.h"
#include "godot_types.h"

NS_FGUI_BEGIN

class GTextField : public GObject
{
    GDCLASS(GTextField, GObject)

public:
    GTextField();
    virtual ~GTextField();

    static void _bind_methods();

    virtual const std::string& getText() const override { return _text; }
    virtual void setText(const std::string& value) override;

    void gd_setTemplateVars(const Dictionary& vars);

    bool isUBBEnabled() const { return _ubbEnabled; }
    virtual void setUBBEnabled(bool value);

    AutoSizeType getAutoSize() const { return _autoSize; }
    virtual void setAutoSize(AutoSizeType value) {};
    int gd_getAutoSize() const { return static_cast<int>(getAutoSize()); }
    void gd_setAutoSize(int value) { setAutoSize(static_cast<AutoSizeType>(value)); }

    virtual bool isSingleLine() const { return false; }
    virtual void setSingleLine(bool value) {};

    virtual TextFormat* getTextFormat() const = 0;
    virtual void applyTextFormat() = 0;

    virtual Vector2 getTextSize();

    Color getColor() const { TextFormat* tf = getTextFormat(); return tf ? tf->color : Color(1,1,1,1); }
    void setColor(const Color& value);

    float getFontSize() const { TextFormat* tf = getTextFormat(); return tf ? tf->fontSize : 0.0f; }
    void setFontSize(float value);

    Color getOutlineColor() const { TextFormat* tf = getTextFormat(); return tf ? tf->outlineColor : Color(1,1,1,1); }
    void setOutlineColor(const Color& value);

    std::unordered_map<std::string, Variant>* getTemplateVars() { return _templateVars; }
    void setTemplateVars(std::unordered_map<std::string, Variant>* value);

    GTextField* setVar(const std::string& name, const Variant& value);
    void flushVars();

    virtual Variant getProp(ObjectPropID propId) override;
    virtual void setProp(ObjectPropID propId, const Variant& value) override;

protected:
    virtual void setTextFieldText() = 0;
    virtual void updateSize();

    virtual void setup_beforeAdd(ByteBuffer* buffer, int beginPos) override;
    virtual void setup_afterAdd(ByteBuffer* buffer, int beginPos) override;

    std::string parseTemplate(const char* text);

    std::string _text;
    bool _ubbEnabled;
    AutoSizeType _autoSize;
    std::unordered_map<std::string, Variant>* _templateVars;
};

class GBasicTextField : public GTextField
{
    GDCLASS(GBasicTextField, GTextField)

public:
    GBasicTextField();
    virtual ~GBasicTextField();

    FAIRYGUI_CREATE(GBasicTextField)

    static void _bind_methods();

    virtual void setAutoSize(AutoSizeType value) override;

    virtual bool isSingleLine() const override { return _label ? _label->isWrapEnabled() : false; }
    virtual void setSingleLine(bool value) override;

    virtual TextFormat* getTextFormat() const override { return _label ? _label->getTextFormat() : nullptr; }
    virtual void applyTextFormat() override;

protected:
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;
    virtual void handleGrayedChanged() override;

    virtual void setTextFieldText() override;
    virtual void updateSize() override;

private:
    FUILabel* _label;
    bool _updatingSize;
};

NS_FGUI_END

#endif
