#include "GMovieClip.h"
#include "PackageItem.h"
#include "display/FUISprite.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

GMovieClip::GMovieClip()
    : _content(nullptr)
    , _playAction(nullptr)
    , _playing(true)
    , _flip(FlipType::NONE)
{
    _sizeImplType = 1;
    _touchDisabled = true;
}

GMovieClip::~GMovieClip()
{
    FGUI_DELETE(_playAction);
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
    _displayObject = _content;

    _playAction = ActionMovieClip::create(nullptr);
    _playAction->setSprite(_content);

    _content->_processCallback = [this](float dt) {
        if (_playing && _playAction)
            _playAction->step(dt);
    };
    _content->set_process(_playing);
}

void GMovieClip::setPlaySettings(int start, int end, int times, int endAt, std::function<void()> completeCallback)
{
    if (_playAction)
        _playAction->setPlaySettings(start, end, times, endAt, completeCallback);
}

void GMovieClip::setPlaying(bool value)
{
    if (_playing != value)
    {
        _playing = value;
        if (_content)
            _content->set_process(_playing);
    }
}

int GMovieClip::getFrame() const
{
    if (_playAction)
        return _playAction->getFrame();
    return 0;
}

void GMovieClip::setFrame(int value)
{
    if (_playAction)
        _playAction->setFrame(value);
    syncContentSize();
}

float GMovieClip::getTimeScale() const
{
    if (_playAction)
        return _playAction->getTimeScale();
    return 1.0f;
}

void GMovieClip::setTimeScale(float value)
{
    if (_playAction)
        _playAction->setTimeScale(value);
}

void GMovieClip::advance(float time)
{
    if (_playAction)
        _playAction->advance(time);
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

void GMovieClip::syncContentSize()
{
    if (!_content || sourceSize.width <= 0 || sourceSize.height <= 0)
        return;
    // _sizeImplType=1 scales the node; FUISprite must draw at sourceSize (like GImage/GLoader).
    _content->set_content_size(sourceSize);
}

void GMovieClip::handleSizeChanged()
{
    GObject::handleSizeChanged();
    syncContentSize();
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

    if (_playAction)
        _playAction->setAnimation(contentItem->movieclip, contentItem->repeatDelay, contentItem->swing);

    setSize(sourceSize.x, sourceSize.y);
    syncContentSize();
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

NS_FGUI_END
