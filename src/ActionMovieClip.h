#ifndef __ACTION_MOVIE_CLIP_H__
#define __ACTION_MOVIE_CLIP_H__

#include "FairyGUIMacros.h"
#include "PackageItem.h"
#include <functional>

NS_FGUI_BEGIN

class FUISprite;

class ActionMovieClip
{
public:
    ActionMovieClip();
    ~ActionMovieClip() = default;

    static ActionMovieClip* create(MovieClipData* md, float repeatDelay = 0, bool swing = false);

    void setSprite(FUISprite* sprite) { _sprite = sprite; }
    void setAnimation(MovieClipData* md, float repeatDelay = 0, bool swing = false);
    void setPlaySettings(int start, int end, int times, int endAt, std::function<void()> completeCallback = nullptr);

    void setTimeScale(float ts) { _timeScale = ts; }
    float getTimeScale() const { return _timeScale; }
    int getFrame() const { return _frame; }

    void setFrame(int f);
    void step(float dt);
    void advance(float time);

private:
    float getFrameDelay() const;
    void advanceOneFrame();
    void handlePlaySettingsAfterFrame();
    void drawFrame();

    FUISprite* _sprite;
    MovieClipData* _md;
    float _timeScale;
    float _repeatDelay;
    bool _swing;
    int _frame;
    int _displayFrame;
    float _frameElapsed;
    int _repeatedCount;
    bool _reversed;

    int _start;
    int _end;
    int _times;
    int _endAt;
    int _status; // 0-none, 1-next loop, 2-ending, 3-ended
    std::function<void()> _completeCallback;
};

NS_FGUI_END

#endif
