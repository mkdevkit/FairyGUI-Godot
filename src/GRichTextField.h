#ifndef __GRICHTEXTFIELD_H__
#define __GRICHTEXTFIELD_H__

// cocos2d.h removed - see godot_types.h
#include "FairyGUIMacros.h"
#include "GTextField.h"
#include "display/FUIRichText.h"

NS_FGUI_BEGIN

class GRichTextField : public GTextField
{
    GDCLASS(GRichTextField, GTextField)

public:
    GRichTextField();
    virtual ~GRichTextField();

    static GRichTextField* create(); // GODOT: implement create()

    static void _bind_methods();

    virtual void setAutoSize(AutoSizeType value) override;

    virtual bool isSingleLine() const override { return false; }
    virtual void setSingleLine(bool value) override;

    virtual TextFormat* getTextFormat() const override { return _richText->getTextFormat(); }
    virtual void applyTextFormat() override;

    HtmlObject* getControl(const std::string& name) const;

    virtual GObject* hitTest(const Vector2& worldPoint, const Camera2D* camera) override;
protected:
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;

    virtual void setTextFieldText() override;
    virtual void updateSize() override;

private:
    FUIRichText* _richText;
    bool _updatingSize;
};

NS_FGUI_END

#endif
