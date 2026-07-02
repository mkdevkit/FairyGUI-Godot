#ifndef __GMOVIECLIP_H__
#define __GMOVIECLIP_H__

#include "FairyGUIMacros.h"
#include "GObject.h"
#include "ActionMovieClip.h"

NS_FGUI_BEGIN

class FUISprite;

class GMovieClip : public GObject
{
    GDCLASS(GMovieClip, GObject)

public:
    GMovieClip();
    virtual ~GMovieClip();

    FAIRYGUI_CREATE(GMovieClip)

    static void _bind_methods();

    bool isPlaying() const { return _playing; }
    void setPlaying(bool value);

    int getFrame() const;
    void setFrame(int value);

    float getTimeScale() const;
    void setTimeScale(float value);

    void advance(float time);

    FlipType getFlip() const;
    void setFlip(FlipType value);
    int gd_getFlip() const { return static_cast<int>(getFlip()); }
    void gd_setFlip(int value) { setFlip(static_cast<FlipType>(value)); }

    Color getColor() const;
    void setColor(const Color& value);

    void setPlaySettings(int start = 0, int end = -1, int times = 0, int endAt = -1,
        std::function<void()> completeCallback = nullptr);

    virtual void constructFromResource() override;

    virtual Variant getProp(ObjectPropID propId) override;
    virtual void setProp(ObjectPropID propId, const Variant& value) override;

protected:
    virtual void handleInit() override;
    virtual void setup_beforeAdd(ByteBuffer* buffer, int beginPos) override;
    virtual void handleGrayedChanged() override;
    virtual void handleSizeChanged() override;

private:
    void syncContentSize();
    FUISprite* _content;
    ActionMovieClip* _playAction;
    bool _playing;
    FlipType _flip;
};

NS_FGUI_END

#endif
