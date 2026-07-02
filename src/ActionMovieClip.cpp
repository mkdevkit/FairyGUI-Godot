#include "ActionMovieClip.h"
#include "display/FUISprite.h"

NS_FGUI_BEGIN

ActionMovieClip::ActionMovieClip()
    : _sprite(nullptr)
    , _md(nullptr)
    , _timeScale(1.0f)
    , _repeatDelay(0)
    , _swing(false)
    , _frame(0)
    , _displayFrame(-1)
    , _frameElapsed(0)
    , _repeatedCount(0)
    , _reversed(false)
    , _start(0)
    , _end(-1)
    , _times(0)
    , _endAt(-1)
    , _status(0)
{
}

ActionMovieClip* ActionMovieClip::create(MovieClipData* md, float repeatDelay, bool swing)
{
    ActionMovieClip* amc = new ActionMovieClip();
    amc->setAnimation(md, repeatDelay, swing);
    return amc;
}

void ActionMovieClip::setAnimation(MovieClipData* md, float repeatDelay, bool swing)
{
    _md = md;
    _repeatDelay = repeatDelay;
    _swing = swing;
    _completeCallback = nullptr;

    if (_md == nullptr)
        return;

    int frameCount = (int)_md->frames.size();
    if (_end == -1 || _end > frameCount - 1)
        _end = frameCount - 1;
    if (_endAt == -1 || _endAt > frameCount - 1)
        _endAt = _end;

    if (_frame < 0 || _frame > frameCount - 1)
        _frame = frameCount > 0 ? frameCount - 1 : 0;

    _displayFrame = -1;
    _frameElapsed = 0;
    _repeatedCount = 0;
    _reversed = false;
    _status = 0;
    drawFrame();
}

void ActionMovieClip::setPlaySettings(int start, int end, int times, int endAt, std::function<void()> completeCallback)
{
    if (_md == nullptr)
        return;

    int frameCount = (int)_md->frames.size();
    if (frameCount == 0)
        return;

    _start = start;
    _end = end;
    if (_end == -1 || _end > frameCount - 1)
        _end = frameCount - 1;
    _times = times;
    _endAt = endAt;
    if (_endAt == -1)
        _endAt = _end;
    _status = 0;
    _completeCallback = completeCallback;

    setFrame(start);
}

void ActionMovieClip::setFrame(int f)
{
    if (_md == nullptr)
        return;

    int totalFrames = (int)_md->frames.size();
    if (totalFrames == 0)
        return;

    if (f >= totalFrames)
        f = totalFrames - 1;
    if (f < 0)
        f = 0;

    _frame = f;
    _frameElapsed = 0;
    _displayFrame = -1;
    drawFrame();
}

float ActionMovieClip::getFrameDelay() const
{
    if (_md == nullptr)
        return 0;

    float delay = _md->interval + _md->frames[_frame].addDelay;
    if (_frame == 0 && _repeatedCount > 0)
        delay += _repeatDelay;
    return delay;
}

void ActionMovieClip::advanceOneFrame()
{
    if (_md == nullptr)
        return;

    int frameCount = (int)_md->frames.size();
    if (frameCount == 0)
        return;

    if (_swing)
    {
        if (_reversed)
        {
            _frame--;
            if (_frame <= 0)
            {
                _frame = 0;
                _repeatedCount++;
                _reversed = !_reversed;
            }
        }
        else
        {
            _frame++;
            if (_frame > frameCount - 1)
            {
                _frame = MAX(0, frameCount - 2);
                _repeatedCount++;
                _reversed = !_reversed;
            }
        }
    }
    else
    {
        _frame++;
        if (_frame > frameCount - 1)
        {
            _frame = 0;
            _repeatedCount++;
        }
    }
}

void ActionMovieClip::handlePlaySettingsAfterFrame()
{
    if (_status == 1)
    {
        _frame = _start;
        _frameElapsed = 0;
        _status = 0;
    }
    else if (_status == 2)
    {
        _frame = _endAt;
        _frameElapsed = 0;
        _status = 3;

        if (_completeCallback)
            _completeCallback();
    }
    else if (_status == 0)
    {
        if (_frame == _end)
        {
            if (_times > 0)
            {
                _times--;
                if (_times == 0)
                    _status = 2;
                else
                    _status = 1;
            }
            else if (_start != 0)
            {
                _status = 1;
            }
        }
    }
}

void ActionMovieClip::step(float dt)
{
    if (_md == nullptr || _sprite == nullptr)
        return;

    int frameCount = (int)_md->frames.size();
    if (frameCount == 0 || _status == 3)
        return;

    if (_timeScale != 1.0f)
        dt *= _timeScale;

    _frameElapsed += dt;
    float frameDelay = getFrameDelay();
    if (_frameElapsed < frameDelay)
        return;

    _frameElapsed -= frameDelay;
    if (_frameElapsed > _md->interval)
        _frameElapsed = _md->interval;

    advanceOneFrame();
    handlePlaySettingsAfterFrame();
    drawFrame();
}

void ActionMovieClip::advance(float time)
{
    if (_md == nullptr)
        return;

    int frameCount = (int)_md->frames.size();
    if (frameCount == 0)
        return;

    if (_timeScale != 1.0f)
        time *= _timeScale;

    int beginFrame = _frame;
    bool beginReversed = _reversed;
    float backupTime = time;

    while (true)
    {
        float frameDelay = getFrameDelay();
        if (time < frameDelay)
        {
            _frameElapsed = 0;
            break;
        }

        time -= frameDelay;
        advanceOneFrame();

        if (_frame == beginFrame && _reversed == beginReversed)
        {
            float roundTime = backupTime - time;
            if (roundTime > 0)
                time -= (int)floor(time / roundTime) * roundTime;
        }
    }

    drawFrame();
}

void ActionMovieClip::drawFrame()
{
    if (_md == nullptr || _sprite == nullptr)
        return;

    int totalFrames = (int)_md->frames.size();
    if (totalFrames == 0)
        return;

    if (_frame < 0)
        _frame = 0;
    if (_frame >= totalFrames)
        _frame = totalFrames - 1;

    if (_displayFrame == _frame)
        return;

    _displayFrame = _frame;
    MovieClipFrameData& frameData = _md->frames[_frame];
    _sprite->setRegion(frameData.imageData.region);
    _sprite->setRotated(frameData.imageData.rotated);
    _sprite->setImageFrameInfo(frameData.imageData.originalSize, frameData.imageData.offset);
    _sprite->setTexture(frameData.imageData.texture);
    _sprite->setRegionEnabled(true);
}

NS_FGUI_END
