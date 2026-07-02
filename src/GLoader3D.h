#ifndef __GLOADER3D_H__
#define __GLOADER3D_H__

#include "FairyGUIMacros.h"
#include "GObject.h"
#include "godot_types.h"

#ifndef SPINE_GODOT_DISABLED
class SpineSprite;
#endif

NS_FGUI_BEGIN

class FUIContainer;
class PackageItem;

class GLoader3D : public GObject
{
    GDCLASS(GLoader3D, GObject)

public:
    GLoader3D();
    virtual ~GLoader3D();

    FAIRYGUI_CREATE(GLoader3D)

    static void _bind_methods();

    void dispose();

    const std::string& getURL() const { return _url; }
    void setURL(const std::string& value);

    virtual const std::string& getIcon() const override { return _url; }
    virtual void setIcon(const std::string& value) override { setURL(value); }

    void gd_setURL(const String& value);
    String gd_getURL() const;

    AlignType getAlign() const { return _align; }
    void setAlign(AlignType value);
    void gd_setAlign(int value);
    int gd_getAlign() const;

    VertAlignType getVerticalAlign() const { return _verticalAlign; }
    void setVerticalAlign(VertAlignType value);
    void gd_setVerticalAlign(int value);
    int gd_getVerticalAlign() const;

    bool getAutoSize() const { return _autoSize; }
    void setAutoSize(bool value);

    LoaderFillType getFill() const { return _fill; }
    void setFill(LoaderFillType value);
    void gd_setFillType(int value);
    int gd_getFillType() const;

    bool isShrinkOnly() const { return _shrinkOnly; }
    void setShrinkOnly(bool value);

    Color getColor() const;
    void setColor(const Color& value);

    bool isPlaying() const { return _playing; }
    void setPlaying(bool value);

    int getFrame() const { return _frame; }
    void setFrame(int value);

    const std::string& getAnimationName() const { return _animationName; }
    void setAnimationName(const std::string& value);
    void gd_setAnimationName(const String& value);
    String gd_getPlayingAnimationName() const;

    const std::string& getSkinName() const { return _skinName; }
    void setSkinName(const std::string& value);
    void gd_setSkinName(const String& value);
    String gd_getSkinName() const;

    bool isLoop() const { return _loop; }
    void setLoop(bool value);

    void play(const std::string& name = EMPTY_STRING);
    void stop();
    void gd_play(const String& name);

    virtual Variant getProp(ObjectPropID propId) override;
    virtual void setProp(ObjectPropID propId, const Variant& value) override;

protected:
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;
    virtual void handleGrayedChanged() override;
    virtual void setup_beforeAdd(ByteBuffer* buffer, int beginPos) override;
    virtual GObject* hitTest(const Vector2& worldPoint, const Camera2D* camera) override;

private:
    void loadContent();
    void loadFromPackage();
    void clearContent();
    void updateLayout();
    void onChange();
    void onChangeSpine();
    void setErrorState();
    void clearErrorState();

#ifndef SPINE_GODOT_DISABLED
    bool loadSpineContent();
#endif

    std::string _url;
    AlignType _align;
    VertAlignType _verticalAlign;
    bool _autoSize;
    LoaderFillType _fill;
    bool _shrinkOnly;
    bool _updatingLayout;
    PackageItem* _contentItem;
    bool _playing;
    int _frame;
    bool _loop;
    std::string _animationName;
    std::string _skinName;
    Color _color;

    FUIContainer* _container;

#ifndef SPINE_GODOT_DISABLED
    SpineSprite* _spineSprite;
#endif
};

NS_FGUI_END

#endif
