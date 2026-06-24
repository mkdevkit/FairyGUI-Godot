#include "GScrollBar.h"
#include "PackageItem.h"
#include "ScrollPane.h"
#include "utils/ByteBuffer.h"

NS_FGUI_BEGIN
GScrollBar::GScrollBar()
    : _grip(nullptr),
      _arrowButton1(nullptr),
      _arrowButton2(nullptr),
      _bar(nullptr),
      _target(nullptr),
      _vertical(false),
      _scrollPerc(0),
      _fixedGripSize(false),
      _gripDragging(false)
{
}

GScrollBar::~GScrollBar()
{
}

void GScrollBar::setScrollPane(ScrollPane* target, bool vertical)
{
    _target = target;
    _vertical = vertical;
}

void GScrollBar::setDisplayPerc(float value)
{
    if (_vertical)
    {
        if (!_fixedGripSize)
            _grip->setHeight(floor(value * _bar->getHeight()));
        _grip->setY(round(_bar->getY() + (_bar->getHeight() - _grip->getHeight()) * _scrollPerc));
    }
    else
    {
        if (!_fixedGripSize)
            _grip->setWidth(floor(value * _bar->getWidth()));
        _grip->setX(round(_bar->getX() + (_bar->getWidth() - _grip->getWidth()) * _scrollPerc));
    }

    ((CanvasItem*)_grip->displayObject())->set_visible(value != 0 && value != 1);
}

void GScrollBar::setScrollPerc(float value)
{
    _scrollPerc = value;
    if (_vertical)
        _grip->setY(round(_bar->getY() + (_bar->getHeight() - _grip->getHeight()) * _scrollPerc));
    else
        _grip->setX(round(_bar->getX() + (_bar->getWidth() - _grip->getWidth()) * _scrollPerc));
}

float GScrollBar::getMinSize()
{
    if (_vertical)
        return (_arrowButton1 != nullptr ? _arrowButton1->getHeight() : 0) + (_arrowButton2 != nullptr ? _arrowButton2->getHeight() : 0);
    else
        return (_arrowButton1 != nullptr ? _arrowButton1->getWidth() : 0) + (_arrowButton2 != nullptr ? _arrowButton2->getWidth() : 0);
}

void GScrollBar::constructExtension(ByteBuffer* buffer)
{
    buffer->seek(0, 6);

    _fixedGripSize = buffer->readBool();

    _grip = getChild("grip");
    // CCASSERT(_grip != nullptr, "FairyGUI: should define grip")
    _bar = getChild("bar");
    // CCASSERT(_bar != nullptr, "FairyGUI: should define bar")

    _arrowButton1 = getChild("arrow1");
    _arrowButton2 = getChild("arrow2");

    _grip->addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { GScrollBar::onGripTouchBegin(ctx); });
    _grip->addEventListener(UIEventType::TouchMove, [this](EventContext* ctx) { GScrollBar::onGripTouchMove(ctx); });
    _grip->addEventListener(UIEventType::TouchEnd, [this](EventContext* ctx) { GScrollBar::onGripTouchEnd(ctx); });

    this->addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { GScrollBar::onTouchBegin(ctx); });

    if (_arrowButton1 != nullptr)
        _arrowButton1->addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { GScrollBar::onArrowButton1Click(ctx); });
    if (_arrowButton2 != nullptr)
        _arrowButton2->addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { GScrollBar::onArrowButton2Click(ctx); });
}

void GScrollBar::onTouchBegin(EventContext* context)
{
    context->stopPropagation();

    InputEvent* evt = context->getInput();
    Vector2 pt = _grip->globalToLocal(evt->getPosition());
    if (_vertical)
    {
        if (pt.y < 0)
            _target->scrollUp(4, false);
        else
            _target->scrollDown(4, false);
    }
    else
    {
        if (pt.x < 0)
            _target->scrollLeft(4, false);
        else
            _target->scrollRight(4, false);
    }
}

void GScrollBar::onGripTouchBegin(EventContext* context)
{
    if (_bar == nullptr)
        return;

    context->stopPropagation();
    context->captureTouch();

    _gripDragging = true;
    _target->updateScrollBarVisible();

    _dragOffset = globalToLocal(context->getInput()->getPosition()) - ((Node2D*)_grip->displayObject())->get_position();
}

void GScrollBar::onGripTouchMove(EventContext* context)
{
    Vector2 pt = globalToLocal(context->getInput()->getPosition());

    if (_vertical)
    {
        float curY = pt.y - _dragOffset.y;
        float diff = _bar->getHeight() - _grip->getHeight();
        if (diff == 0)
            _target->setPercY(0);
        else
            _target->setPercY((curY - _bar->getY()) / diff);
    }
    else
    {
        float curX = pt.x - _dragOffset.x;
        float diff = _bar->getWidth() - _grip->getWidth();
        if (diff == 0)
            _target->setPercX(0);
        else
            _target->setPercX((curX - _bar->getX()) / diff);
    }
}

void GScrollBar::onGripTouchEnd(EventContext* context)
{
    _gripDragging = false;
    _target->updateScrollBarVisible();
}

void GScrollBar::onArrowButton1Click(EventContext* context)
{
    context->stopPropagation();

    if (_vertical)
        _target->scrollUp();
    else
        _target->scrollLeft();
}

void GScrollBar::onArrowButton2Click(EventContext* context)
{
    context->stopPropagation();

    if (_vertical)
        _target->scrollDown();
    else
        _target->scrollRight();
}

void GScrollBar::_bind_methods()
{
    // setScrollPane uses ScrollPane* which is not a GDCLASS-registered type
    // ClassDB::bind_method(D_METHOD("setScrollPane", "target", "vertical"), &GScrollBar::setScrollPane);
    ClassDB::bind_method(D_METHOD("setDisplayPerc", "value"), &GScrollBar::setDisplayPerc);
    ClassDB::bind_method(D_METHOD("setScrollPerc", "value"), &GScrollBar::setScrollPerc);
    ClassDB::bind_method(D_METHOD("getMinSize"), &GScrollBar::getMinSize);
}

NS_FGUI_END


