#ifndef __GLOADER_H__
#define __GLOADER_H__

// cocos2d.h removed - see godot_types.h
#include "FairyGUIMacros.h"
#include "GObject.h"
#include "PackageItem.h"

NS_FGUI_BEGIN

class GComponent;
class FUISprite;

class ActionMovieClip
{
public:
    ActionMovieClip();
    ~ActionMovieClip() = default;

    static ActionMovieClip* create(MovieClipData* md, float repeatDelay = 0);

    void setSprite(FUISprite* sprite) { _sprite = sprite; }
    void setAnimation(MovieClipData* md, float repeatDelay = 0);
    void setTimeScale(float ts) { _timeScale = ts; }
    float getTimeScale() const { return _timeScale; }
    void setFrame(int f);
    void advance(float dt);

private:
    void drawFrame();

    FUISprite* _sprite;
    MovieClipData* _md;
    float _timeScale;
    float _repeatDelay;
    int _frame;
    int _displayFrame;
    float _frameElapsed;
};

class GLoader : public GObject
{
    GDCLASS(GLoader, GObject)

public:
    GLoader();
    virtual ~GLoader();

    FAIRYGUI_CREATE(GLoader)

    static void _bind_methods();

    const std::string& getURL() const { return _url; }
    void setURL(const std::string& value);

    void gd_setURL(const String& value);
    String gd_getURL() const;

    virtual const std::string& getIcon() const override { return _url; }
    virtual void setIcon(const std::string& value) override { setURL(value); }

    AlignType getAlign() const { return _align; }
    void setAlign(AlignType value);
    int gd_getAlign() const { return static_cast<int>(getAlign()); }
    void gd_setAlign(int value) { setAlign(static_cast<AlignType>(value)); }

    VertAlignType getVerticalAlign() const { return _verticalAlign; }
    void setVerticalAlign(VertAlignType value);
    int gd_getVerticalAlign() const { return static_cast<int>(getVerticalAlign()); }
    void gd_setVerticalAlign(int value) { setVerticalAlign(static_cast<VertAlignType>(value)); }

    bool getAutoSize() const { return _autoSize; }
    void setAutoSize(bool value);

    LoaderFillType getFill() const { return _fill; }
    void setFill(LoaderFillType value);
    int gd_getFillType() const { return static_cast<int>(getFill()); }
    void gd_setFillType(int value) { setFill(static_cast<LoaderFillType>(value)); }

    bool isShrinkOnly() const { return _shrinkOnly; }
    void setShrinkOnly(bool value);

    Vector2 getContentSize();

    Color getColor() const;
    void setColor(const Color& value);

    bool isPlaying() const { return _playing; }
    void setPlaying(bool value);

    int getFrame() const;
    void setFrame(int value);

    FillMethod getFillMethod() const;
    void setFillMethod(FillMethod value);
    int gd_getFillMethod() const { return static_cast<int>(getFillMethod()); }
    void gd_setFillMethod(int value) { setFillMethod(static_cast<FillMethod>(value)); }

    FillOrigin getFillOrigin() const;
    void setFillOrigin(FillOrigin value);
    int gd_getFillOrigin() const { return static_cast<int>(getFillOrigin()); }
    void gd_setFillOrigin(int value) { setFillOrigin(static_cast<FillOrigin>(value)); }

    bool isFillClockwise() const;
    void setFillClockwise(bool value);

    float getFillAmount() const;
    void setFillAmount(float value);

    GComponent* getComponent() const { return _content2; }

    virtual Variant getProp(ObjectPropID propId) override;
    virtual void setProp(ObjectPropID propId, const Variant& value) override;

protected:
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;
    virtual void handleGrayedChanged() override;
    virtual void setup_beforeAdd(ByteBuffer* buffer, int beginPos) override;
    virtual GObject* hitTest(const Vector2 & worldPoint, const Camera2D * camera) override;

    virtual void loadExternal();
    virtual void freeExternal(ImageFrame* spriteFrame);
    void onExternalLoadSuccess(ImageFrame* spriteFrame);
    void onExternalLoadFailed();

private:
    void loadContent();
    void loadFromPackage();
    void clearContent();
    void updateLayout();
    void setErrorState();
    void clearErrorState();
    void updateMovieClipProcess();

    std::string _url;
    AlignType _align;
    VertAlignType _verticalAlign;
    bool _autoSize;
    LoaderFillType _fill;
    bool _shrinkOnly;
    bool _updatingLayout;
    PackageItem* _contentItem;
    int _contentStatus;
    bool _playing;
    int _frame;

    FUISprite* _content;
    GComponent* _content2;
    ActionMovieClip* _playAction;
    Vector2 _sourceSize;
};

NS_FGUI_END

#endif
