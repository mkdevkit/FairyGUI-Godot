#include "GSlider.h"
#include "PackageItem.h"
#include "utils/ByteBuffer.h"
#include <sstream>

NS_FGUI_BEGIN
GSlider::GSlider()
    : changeOnClick(false),
      canDrag(false),
      _min(0),
      _max(100),
      _value(0),
      _titleType(ProgressTitleType::PERCENT),
      _titleObject(nullptr),
      _barObjectH(nullptr),
      _barObjectV(nullptr),
      _barMaxWidth(0),
      _barMaxHeight(0),
      _barMaxWidthDelta(0),
      _barMaxHeightDelta(0),
      _gripObject(nullptr),
      _clickPercent(0),
      _barStartX(0),
      _barStartY(0),
      _wholeNumbers(false)
{
}

GSlider::~GSlider()
{
}

void GSlider::setTitleType(ProgressTitleType value)
{
    if (_titleType != value)
    {
        _titleType = value;
        update();
    }
}

void GSlider::setMin(double value)
{
    if (_min != value)
    {
        _min = value;
        update();
    }
}

void GSlider::setMax(double value)
{
    if (_max != value)
    {
        _max = value;
        update();
    }
}

void GSlider::setValue(double value)
{
    if (_value != value)
    {
        _value = value;
        update();
    }
}

void GSlider::setWholeNumbers(bool value)
{
    if (_wholeNumbers != value)
    {
        _wholeNumbers = value;
        update();
    }
}

void GSlider::update()
{
    float percent = (float)std::min((_value - _min) / (_max - _min), 1.0);
    updateWithPercent(percent, false);
}

void GSlider::updateWithPercent(float percent, bool manual)
{
    percent = std::clamp(percent, 0.0f, 1.0f);
    if (manual)
    {
        double newValue = _min + (_max - _min) * percent;
        if (newValue < _min)
            newValue = _min;
        if (newValue > _max)
            newValue = _max;
        if (_wholeNumbers)
        {
            newValue = round(newValue);
            percent = std::clamp((float)((newValue - _min) / (_max - _min)), 0.0f, 1.0f);
        }

        if (newValue != _value)
        {
            _value = newValue;
            if (dispatchEvent(UIEventType::Changed))
                return;
        }
    }

    if (_titleObject != nullptr)
    {
        std::ostringstream oss;
        switch (_titleType)
        {
        case ProgressTitleType::PERCENT:
            oss << floor(percent * 100) << "%";
            break;

        case ProgressTitleType::VALUE_MAX:
            oss << floor(_value) << "/" << floor(_max);
            break;

        case ProgressTitleType::VALUE:
            oss << _value;
            break;

        case ProgressTitleType::MAX:
            oss << _max;
            break;
        }
        _titleObject->setText(oss.str());
    }

    float fullWidth = this->getWidth() - _barMaxWidthDelta;
    float fullHeight = this->getHeight() - _barMaxHeightDelta;
    if (!_reverse)
    {
        if (_barObjectH != nullptr)
            _barObjectH->setWidth(round(fullWidth * percent));
        if (_barObjectV != nullptr)
            _barObjectV->setHeight(round(fullHeight * percent));
    }
    else
    {
        if (_barObjectH != nullptr)
        {
            _barObjectH->setWidth(round(fullWidth * percent));
            _barObjectH->setX(_barStartX + (fullWidth - _barObjectH->getWidth()));
        }
        if (_barObjectV != nullptr)
        {
            _barObjectV->setHeight(round(fullHeight * percent));
            _barObjectV->setY(_barStartY + (fullHeight - _barObjectV->getHeight()));
        }
    }
}

void GSlider::handleSizeChanged()
{
    GComponent::handleSizeChanged();

    if (_barObjectH != nullptr)
        _barMaxWidth = getWidth() - _barMaxWidthDelta;
    if (_barObjectV != nullptr)
        _barMaxHeight = getHeight() - _barMaxHeightDelta;

    if (!_underConstruct)
        update();
}

void GSlider::constructExtension(ByteBuffer* buffer)
{
    _titleType = (ProgressTitleType)buffer->readByte();
    _reverse = buffer->readBool();
    if (buffer->version >= 2)
    {
        _wholeNumbers = buffer->readBool();
        changeOnClick = buffer->readBool();
    }

    _titleObject = getChild("title");
    _barObjectH = getChild("bar");
    _barObjectV = getChild("bar_v");
    _gripObject = getChild("grip");

    if (_barObjectH != nullptr)
    {
        _barMaxWidth = _barObjectH->getWidth();
        _barMaxWidthDelta = getWidth() - _barMaxWidth;
        _barStartX = _barObjectH->getX();
    }
    if (_barObjectV != nullptr)
    {
        _barMaxHeight = _barObjectV->getHeight();
        _barMaxHeightDelta = getHeight() - _barMaxHeight;
        _barStartY = _barObjectV->getY();
    }

    if (_gripObject != nullptr)
    {
        _gripObject->addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { GSlider::onGripTouchBegin(ctx); });
        _gripObject->addEventListener(UIEventType::TouchMove, [this](EventContext* ctx) { GSlider::onGripTouchMove(ctx); });
    }

    addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { GSlider::onTouchBegin(ctx); });
}

void GSlider::setup_afterAdd(ByteBuffer* buffer, int beginPos)
{
    GComponent::setup_afterAdd(buffer, beginPos);

    if (!buffer->seek(beginPos, 6))
    {
        update();
        return;
    }

    if ((ObjectType)buffer->readByte() != _packageItem->objectType)
    {
        update();
        return;
    }

    _value = buffer->readInt();
    _max = buffer->readInt();
    if (buffer->version >= 2)
        _min = buffer->readInt();

    update();
}

void GSlider::onTouchBegin(EventContext* context)
{
    if (!changeOnClick)
        return;

    InputEvent* evt = context->getInput();
    if (evt->getButton() != MouseButton::LEFT)
        return;

    Vector2 pt = _gripObject->globalToLocal(evt->getPosition());
    float percent = (float)std::clamp((_value - _min) / (_max - _min), 0.0, 1.0);
    float delta = 0;
    if (_barObjectH != nullptr)
        delta = pt.x / _barMaxWidth;
    if (_barObjectV != nullptr)
        delta = pt.y / _barMaxHeight;
    if (_reverse)
        percent -= delta;
    else
        percent += delta;
    updateWithPercent(percent, true);
}

void GSlider::onGripTouchBegin(EventContext* context)
{
    if (context->getInput()->getButton() != MouseButton::LEFT)
        return;

    canDrag = true;
    context->stopPropagation();
    context->captureTouch();

    _clickPos = globalToLocal(context->getInput()->getPosition());
    _clickPercent = (float)std::clamp((_value - _min) / (_max - _min), 0.0, 1.0);
}

void GSlider::onGripTouchMove(EventContext* context)
{
    if (!canDrag)
        return;

    Vector2 pt = globalToLocal(context->getInput()->getPosition());

    float deltaX = pt.x - _clickPos.x;
    float deltaY = pt.y - _clickPos.y;
    if (_reverse)
    {
        deltaX = -deltaX;
        deltaY = -deltaY;
    }

    float percent;
    if (_barObjectH != nullptr)
        percent = _clickPercent + deltaX / _barMaxWidth;
    else
        percent = _clickPercent + deltaY / _barMaxHeight;
    updateWithPercent(percent, true);
}

void GSlider::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setMin", "value"), &GSlider::setMin);
    ClassDB::bind_method(D_METHOD("getMin"), &GSlider::getMin);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min"), "setMin", "getMin");

    ClassDB::bind_method(D_METHOD("setMax", "value"), &GSlider::setMax);
    ClassDB::bind_method(D_METHOD("getMax"), &GSlider::getMax);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max"), "setMax", "getMax");

    ClassDB::bind_method(D_METHOD("setValue", "value"), &GSlider::setValue);
    ClassDB::bind_method(D_METHOD("getValue"), &GSlider::getValue);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "value"), "setValue", "getValue");

    ClassDB::bind_method(D_METHOD("setWholeNumbers", "value"), &GSlider::setWholeNumbers);
    ClassDB::bind_method(D_METHOD("getWholeNumbers"), &GSlider::getWholeNumbers);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "wholeNumbers"), "setWholeNumbers", "getWholeNumbers");
}

NS_FGUI_END


