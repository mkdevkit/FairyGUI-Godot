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

    FAIRYGUI_CREATE(GRichTextField)

    static void _bind_methods();

    virtual void setAutoSize(AutoSizeType value) override;

    virtual bool isSingleLine() const override { return _singleLine; }
    virtual void setSingleLine(bool value) override;

    virtual TextFormat* getTextFormat() const override { return _richText->getTextFormat(); }
    virtual void applyTextFormat() override;

    virtual Vector2 getTextSize() override;

    HtmlObject* getControl(const std::string& name) const;

    bool isAnchorTextUnderline() const;
    void setAnchorTextUnderline(bool value);
    Color getAnchorFontColor() const;
    void setAnchorFontColor(const Color& value);

    virtual GObject* hitTest(const Vector2& worldPoint, const Camera2D* camera) override;
protected:
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;

    virtual void setTextFieldText() override;
    virtual void updateSize() override;

private:
    FUIRichText* _richText;
    bool _updatingSize;
    bool _singleLine;
};

NS_FGUI_END

#endif
