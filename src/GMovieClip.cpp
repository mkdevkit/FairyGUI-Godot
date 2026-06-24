#include "GMovieClip.h"
#include "PackageItem.h"
#include "display/FUISprite.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

GMovieClip::GMovieClip()
    : _content(nullptr)
    , _playing(true)
    , _contentItem(nullptr)
    , _movieclipData(nullptr)
    , _frame(0)
    , _frameElapsed(0)
    , _repeatedCount(0)
    , _reversed(false)
    , _timeScale(1.0f)
    , _repeatDelay(0)
    , _swing(false)
    , _completeCallback(nullptr)
    , _displayFrame(-1)
    , _start(0)
    , _end(-1)
    , _times(0)
    , _endAt(-1)
    , _status(0)
    , _flip(FlipType::NONE)
{
    // GODOT_ADAPT: set_process(true) not available on GObject (non-Node)
    _sizeImplType = 1;
    _touchDisabled = true;
}

GMovieClip::~GMovieClip()
{
}

void GMovieClip::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setPlaying", "value"), &GMovieClip::setPlaying);
    ClassDB::bind_method(D_METHOD("isPlaying"), &GMovieClip::isPlaying);

    ClassDB::bind_method(D_METHOD("setFrame", "frame"), &GMovieClip::setFrame);
    ClassDB::bind_method(D_METHOD("getFrame"), &GMovieClip::getFrame);

    ClassDB::bind_method(D_METHOD("setTimeScale", "scale"), &GMovieClip::setTimeScale);
    ClassDB::bind_method(D_METHOD("getTimeScale"), &GMovieClip::getTimeScale);

    ClassDB::bind_method(D_METHOD("advance", "time"), &GMovieClip::advance);

    ClassDB::bind_method(D_METHOD("setFlip", "flip"), &GMovieClip::gd_setFlip);
    ClassDB::bind_method(D_METHOD("getFlip"), &GMovieClip::gd_getFlip);

    ClassDB::bind_method(D_METHOD("setColor", "color"), &GMovieClip::setColor);
    ClassDB::bind_method(D_METHOD("getColor"), &GMovieClip::getColor);
}

void GMovieClip::handleInit()
{
    _content = FUISprite::create();
}

void GMovieClip::setPlaySettings(int start, int end, int times, int endAt, std::function<void()> completeCallback)
{
    _start = start;
    _end = end;
    _times = times;
    _endAt = endAt;
    _status = 0;
    _completeCallback = completeCallback;

    int totalFrames = 0;
    if (_movieclipData)
        totalFrames = (int)_movieclipData->frames.size();

    if (_frame < 0 || _frame >= totalFrames)
        _frame = 0;
    _repeatedCount = 0;
    _reversed = false;
    _frameElapsed = 0;

    if (_start < 0)
        _start = 0;
    if (_start >= totalFrames)
        _start = totalFrames - 1;
    _frame = _start;

    drawFrame();
}

void GMovieClip::setPlaying(bool value)
{
    if (_playing != value)
    {
        _playing = value;
        // set_process(_playing);
    }
}

int GMovieClip::getFrame() const
{
    // if we have a complete frame list from our movieclip data
    if (_movieclipData && _frame >= 0 && _frame < (int)_movieclipData->frames.size())
        return _frame;
    return 0;
}

void GMovieClip::setFrame(int value)
{
    if (_movieclipData == nullptr)
        return;

    int totalFrames = (int)_movieclipData->frames.size();
    if (value < 0)
        value = 0;
    if (value >= totalFrames)
        value = totalFrames - 1;

    _frame = value;
    _frameElapsed = 0;
    _displayFrame = -1;
    drawFrame();
}


void GMovieClip::advance(float time)
{
    if (_movieclipData == nullptr)
        return;

    int totalFrames = (int)_movieclipData->frames.size();
    if (totalFrames == 0)
        return;

    int beginFrame = _frame;
    bool timeOver = false;

    _frameElapsed += time * _timeScale;

    if (_frameElapsed >= _movieclipData->interval + _movieclipData->frames[_frame].addDelay)
    {
        _frameElapsed -= _movieclipData->interval + _movieclipData->frames[_frame].addDelay;

        if (_reversed)
            _frame--;
        else
            _frame++;

        if (_status == 0)
        {
            int endFrame = _end;
            if (endFrame == -1)
                endFrame = totalFrames - 1;
            if (endFrame > totalFrames - 1)
                endFrame = totalFrames - 1;

            if (_reversed)
            {
                if (_frame == beginFrame - 1)
                {
                    // Wrap
                    _frame = endFrame;
                    _repeatedCount++;
                    if (_swing)
                    {
                        _reversed = !_reversed;
                        _frame = beginFrame + 1;
                        if (_frame > endFrame)
                            _frame = endFrame - 1;
                    }
                }
                else if (_frame < _start)
                {
                    _frame = endFrame;
                    _repeatedCount++;
                    if (_swing)
                    {
                        _reversed = !_reversed;
                        _frame = _start + 1;
                    }
                }
            }
            else
            {
                if (_frame > endFrame)
                {
                    _frame = _start;
                    _repeatedCount++;
                    if (_swing)
                    {
                        _reversed = !_reversed;
                        _frame = endFrame - 1;
                        if (_frame < _start)
                            _frame = _start;
                    }
                }
            }

            if (_times > 0 && _repeatedCount >= _times)
                timeOver = true;
        }
        else if (_status == 1)
        {
            _status = 2;
        }
        else if (_status == 2)
        {
            _status = 3;
            timeOver = true;
        }

        if (timeOver)
        {
            _playing = false;
            // set_process(false);

            if (_completeCallback)
                _completeCallback();
        }
    }

    drawFrame();
}

void GMovieClip::drawFrame()
{
    if (_movieclipData == nullptr || _content == nullptr)
        return;

    int totalFrames = (int)_movieclipData->frames.size();
    if (totalFrames == 0)
        return;

    if (_frame < 0)
        _frame = 0;
    if (_frame >= totalFrames)
        _frame = totalFrames - 1;

    if (_displayFrame == _frame)
        return;

    _displayFrame = _frame;

    MovieClipFrameData& frameData = _movieclipData->frames[_frame];

    _content->setTexture(frameData.imageData.texture);
    _content->setRegion(frameData.imageData.region);
    _content->setRegionEnabled(true);
}

FlipType GMovieClip::getFlip() const
{
    return _flip;
}

void GMovieClip::setFlip(FlipType value)
{
    _flip = value;
    if (_content)
    {
        _content->setFlippedH(value == FlipType::HORIZONTAL || value == FlipType::BOTH);
        _content->setFlippedV(value == FlipType::VERTICAL || value == FlipType::BOTH);
    }
}

Color GMovieClip::getColor() const
{
    if (_content)
        return _content->getColor();
    return Color(1, 1, 1, 1);
}

void GMovieClip::setColor(const Color& value)
{
    if (_content)
        _content->setColor(value);
}

void GMovieClip::handleGrayedChanged()
{
    GObject::handleGrayedChanged();

    if (_content)
        ((FUISprite*)_content)->setGrayed(_finalGrayed);
}

Variant GMovieClip::getProp(ObjectPropID propId)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        return Variant(ToolSet::colorToInt(getColor()));
    case ObjectPropID::Playing:
        return Variant(isPlaying());
    case ObjectPropID::Frame:
        return Variant(getFrame());
    case ObjectPropID::TimeScale:
        return Variant(getTimeScale());
    default:
        return GObject::getProp(propId);
    }
}

void GMovieClip::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        setColor(ToolSet::intToColor((unsigned int)value));
        break;
    case ObjectPropID::Playing:
        setPlaying((bool)value);
        break;
    case ObjectPropID::Frame:
        setFrame((int)value);
        break;
    case ObjectPropID::TimeScale:
        setTimeScale((float)value);
        break;
    case ObjectPropID::DeltaTime:
        advance((float)value);
        break;
    default:
        GObject::setProp(propId, value);
        break;
    }
}

void GMovieClip::constructFromResource()
{
    PackageItem* contentItem = _packageItem->getBranch();
    sourceSize.x = (float)contentItem->width;
    sourceSize.y = (float)contentItem->height;
    initSize = sourceSize;

    contentItem = contentItem->getHighResolution();
    contentItem->load();

    setAnimation(contentItem, contentItem->repeatDelay, contentItem->swing);

    setSize(sourceSize.x, sourceSize.y);
}

void GMovieClip::setAnimation(PackageItem* item, float repeatDelay, bool swing)
{
    _contentItem = item;
    _movieclipData = item->movieclip;
    _repeatDelay = repeatDelay;
    _swing = swing;

    if (_movieclipData)
    {
        _frame = 0;
        _frameElapsed = 0;
        _repeatedCount = 0;
        _displayFrame = -1;
        drawFrame();
    }
}

void GMovieClip::setup_beforeAdd(ByteBuffer* buffer, int beginPos)
{
    GObject::setup_beforeAdd(buffer, beginPos);

    buffer->seek(beginPos, 5);

    if (buffer->readBool())
        setColor(buffer->readColor());
    setFlip((FlipType)buffer->readByte());
    setFrame(buffer->readInt());
    setPlaying(buffer->readBool());
}

void GMovieClip::_process(double delta)
{
    if (_playing && _movieclipData)
        advance((float)delta);
}

void GMovieClip::setTimeScale(float value)
{
    _timeScale = value;
}

NS_FGUI_END
