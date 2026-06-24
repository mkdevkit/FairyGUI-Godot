#ifndef __GLABEL_H__
#define __GLABEL_H__

// cocos2d.h removed - see godot_types.h
#include "FairyGUIMacros.h"
#include "GComponent.h"

NS_FGUI_BEGIN

class GTextField;

class GLabel : public GComponent
{
    GDCLASS(GLabel, GComponent)

public:
    GLabel();
    virtual ~GLabel();

    FAIRYGUI_CREATE(GLabel)

    static void _bind_methods();

    const std::string& getTitle() { return getText(); }
    void setTitle(const std::string& value) { setText(value); };

    void gd_setTitle(const String& value);
    String gd_getTitle();

    virtual const std::string& getText() const override;
    virtual void setText(const std::string& value) override;

    virtual const std::string& getIcon() const override;
    virtual void setIcon(const std::string& value) override;

    void gd_setIcon(const String& value);
    String gd_getIcon() const;

    Color getTitleColor() const;
    void setTitleColor(const Color& value);

    int getTitleFontSize() const;
    void setTitleFontSize(int value);

    GTextField* getTextField() const;

    virtual Variant getProp(ObjectPropID propId) override;
    virtual void setProp(ObjectPropID propId, const Variant& value) override;

protected:
    virtual void constructExtension(ByteBuffer* buffer) override;
    virtual void setup_afterAdd(ByteBuffer* buffer, int beginPos) override;

private:
    GObject* _titleObject;
    GObject* _iconObject;
};

NS_FGUI_END

#endif
