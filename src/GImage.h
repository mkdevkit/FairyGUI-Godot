#ifndef __GIMAGE_H__
#define __GIMAGE_H__

#include "FairyGUIMacros.h"
#include "GObject.h"
// cocos2d.h removed - see godot_types.h
#include "display/FUISprite.h"

NS_FGUI_BEGIN

class FUISprite;

class GImage : public GObject
{
    GDCLASS(GImage, GObject)

public:
    GImage();
    virtual ~GImage();

    static GImage* create(); // GODOT: implement create()

    static void _bind_methods();

    FlipType getFlip() const;
    void setFlip(FlipType value);

    void gd_setFlip(int value);
    int gd_getFlip() const;

    Color getColor() const;
    void setColor(const Color& value);

    FillMethod getFillMethod() const;
    void setFillMethod(FillMethod value);

    void gd_setFillMethod(int value);
    int gd_getFillMethod() const;

    FillOrigin getFillOrigin() const;
    void setFillOrigin(FillOrigin value);

    void gd_setFillOrigin(int value);
    int gd_getFillOrigin() const;

    bool isFillClockwise() const;
    void setFillClockwise(bool value);

    float getFillAmount() const;
    void setFillAmount(float value);

    virtual void constructFromResource() override;

    virtual Variant getProp(ObjectPropID propId) override;
    virtual void setProp(ObjectPropID propId, const Variant& value) override;

protected:
    virtual void handleInit() override;
    virtual void handleGrayedChanged() override;
    virtual void setup_beforeAdd(ByteBuffer* buffer, int beginPos) override;

private:
    FUISprite* _content;
};

NS_FGUI_END

#endif
