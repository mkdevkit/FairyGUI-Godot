#ifndef __GBUTTON_H
#define __GBUTTON_H

// cocos2d.h removed - see godot_types.h
#include "FairyGUIMacros.h"
#include "GComponent.h"

NS_FGUI_BEGIN

class GTextField;

class GButton : public GComponent
{
    GDCLASS(GButton, GComponent)

public:
    static const std::string UP;
    static const std::string DOWN;
    static const std::string OVER;
    static const std::string SELECTED_OVER;
    static const std::string DISABLED;
    static const std::string SELECTED_DISABLED;

    GButton();
    virtual ~GButton();

    FAIRYGUI_CREATE(GButton)

    static void _bind_methods();

    const std::string& getTitle() { return _title; }
    void setTitle(const std::string& value);

    void gd_setTitle(const String& value);
    String gd_getTitle();

    virtual const std::string& getText() const override { return _title; }
    virtual void setText(const std::string& value) override { setTitle(value); }

    virtual const std::string& getIcon() const override { return _icon; }
    virtual void setIcon(const std::string& value) override;

    const std::string& getSelectedTitle() const { return _selectedTitle; }
    void setSelectedTitle(const std::string& value);

    void gd_setSelectedTitle(const String& value);
    String gd_getSelectedTitle() const;

    const std::string& getSelectedIcon() const { return _selectedIcon; }
    void setSelectedIcon(const std::string& value);

    void gd_setSelectedIcon(const String& value);
    String gd_getSelectedIcon() const;

    void gd_setIcon(const String& value);
    String gd_getIcon() const;

    Color getTitleColor() const;
    void setTitleColor(const Color& value);

    int getTitleFontSize() const;
    void setTitleFontSize(int value);

    bool isSelected() const { return _selected; }
    void setSelected(bool value);

    GController* getRelatedController() const { return _relatedController.ptr(); }
    void setRelatedController(GController* c);

    bool isChangeStateOnClick() { return _changeStateOnClick; }
    void setChangeStateOnClick(bool value) { _changeStateOnClick = value; }

    GTextField* getTextField() const;

    virtual Variant getProp(ObjectPropID propId) override;
    virtual void setProp(ObjectPropID propId, const Variant& value) override;

protected:
    virtual void constructExtension(ByteBuffer* buffer) override;
    virtual void setup_afterAdd(ByteBuffer* buffer, int beginPos) override;
    virtual void handleControllerChanged(GController* c) override;

    void setState(const std::string& value);
    void setCurrentState();

private:
    void onRollOver(EventContext* context);
    void onRollOut(EventContext* context);
    void onTouchBegin(EventContext* context);
    void onTouchEnd(EventContext* context);
    void onClick(EventContext* context);
    void onExit(EventContext* context);

    ButtonMode _mode;
    GObject* _titleObject;
    GObject* _iconObject;
    Ref<GController> _buttonController;
    Ref<GController> _relatedController;
    std::string _relatedPageId;
    std::string _title;
    std::string _selectedTitle;
    std::string _icon;
    std::string _selectedIcon;
    std::string _sound;
    float _soundVolumeScale;
    bool _selected;
    bool _over;
    bool _down;
    int _downEffect;
    bool _downScaled;
    float _downEffectValue;
    bool _changeStateOnClick;
};

NS_FGUI_END

#endif
