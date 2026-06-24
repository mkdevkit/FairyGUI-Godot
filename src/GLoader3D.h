#ifndef __GLOADER3D_H__
#define __GLOADER3D_H__

#include "FairyGUIMacros.h"
#include "GObject.h"
#include "godot_types.h"

// Forward declarations for spine-godot
#ifndef SPINE_GODOT_DISABLED
class SpineSprite;
class SpineSkeletonDataResource;
class SpineSkeletonFileResource;
class SpineAtlasResource;
#endif

NS_FGUI_BEGIN

class GLoader3D : public GObject
{
    GDCLASS(GLoader3D, GObject)

public:
    GLoader3D();
    virtual ~GLoader3D();

    static GLoader3D* create();

    static void _bind_methods();

    void dispose();

    const std::string& getURL() const { return _url; }
    void setURL(const std::string& value);

    const std::string& getIcon() const { return _icon; }
    void setIcon(const std::string& value);

    void gd_setURL(const String& value);
    String gd_getURL() const;
    void gd_setIcon(const String& value);
    String gd_getIcon() const;

    AlignType getAlign() const { return _align; }
    void setAlign(AlignType value);

    void gd_setAlign(int value);
    int gd_getAlign() const;

    VertAlignType getVerticalAlign() const { return _verticalAlign; }
    void setVerticalAlign(VertAlignType value);

    void gd_setVerticalAlign(int value);
    int gd_getVerticalAlign() const;

    FillMethod getFillMethod() const { return _fillMethod; }
    void setFillMethod(FillMethod value);

    void gd_setFillMethod(int value);
    int gd_getFillMethod() const;

    int getFillOrigin() const { return _fillOrigin; }
    void setFillOrigin(int value);

    float getFillAmount() const { return _fillAmount; }
    void setFillAmount(float value);

    bool isFillClockwise() const { return _fillClockwise; }
    void setFillClockwise(bool value);

    bool isShrinkOnly() const { return _shrinkOnly; }
    void setShrinkOnly(bool value);

    const std::string& getPlayingAnimationName() const { return _animationName; }
    void setAnimationName(const std::string& value);

    void gd_setAnimationName(const String& value);
    String gd_getPlayingAnimationName() const;

    const std::string& getSkinName() const { return _skinName; }
    void setSkinName(const std::string& value);

    void gd_setSkinName(const String& value);
    String gd_getSkinName() const;

    bool isPlaying() const { return _playing; }
    bool isLoop() const { return _loop; }
    void setLoop(bool value);

    void setAutoPlay(bool value);

    void play(const std::string& name = EMPTY_STRING);
    void stop();

    void gd_play(const String& name);

protected:
    virtual void handleSizeChanged() override;
    virtual void handleGrayedChanged() override;
    void _ready();
    void _process(double delta);

private:
    void loadFromPackage();
    void loadContent();
    void clearContent();

#ifdef SPINE_GODOT_DISABLED
    void onExternalLoadSuccess(void* asset);
    void onExternalLoadFailed();
#else
    void updateSkeletonScale();
    void updateSpine();
#endif

    std::string _url;
    std::string _icon;
    AlignType _align;
    VertAlignType _verticalAlign;
    FillMethod _fillMethod;
    int _fillOrigin;
    float _fillAmount;
    bool _fillClockwise;
    bool _shrinkOnly;
    bool _autoPlay;
    bool _playing;
    bool _loop;

    std::string _animationName;
    std::string _skinName;

    bool _updatingLayout;
    PackageItem* _contentItem;

#ifndef SPINE_GODOT_DISABLED
    SpineSprite* _spineSprite;
#else
    void* _spineSprite;
#endif
};

NS_FGUI_END

#endif
