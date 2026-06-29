#include "InputProcessor.h"
#include "GComponent.h"
#include "GRoot.h"
#include "GRichTextField.h"
#include "GTextInput.h"
#include "utils/WeakPtr.h"
#include <ctime>

NS_FGUI_BEGIN

using namespace std;

InputProcessor* InputProcessor::_activeProcessor = nullptr;
bool InputProcessor::_touchOnUI = false;
unsigned int InputProcessor::_touchOnUIFlagFrameId = 0;

class TouchInfo
{
public:
    TouchInfo();
    ~TouchInfo();

    void reset();

    Vector2 pos;
    int touchId;
    int clickCount;
    int mouseWheelDelta;
    int button;
    Vector2 downPos;
    bool began;
    bool clickCancelled;
    clock_t lastClickTime;
    WeakPtr lastRollOver;
    std::vector<WeakPtr> downTargets;
    std::vector<WeakPtr> touchMonitors;
};

TouchInfo::TouchInfo() :
    touchId(-1),
    clickCount(0),
    mouseWheelDelta(0),
    button(0),
    began(false),
    clickCancelled(false),
    lastClickTime(0)
{
}

TouchInfo::~TouchInfo()
{
}

void TouchInfo::reset()
{
    touchId = -1;
    clickCount = 0;
    began = false;
    clickCancelled = false;
    downTargets.clear();
    touchMonitors.clear();
}

InputProcessor::InputProcessor(GComponent* owner) :
    _keyModifiers(0)
{
    _owner = owner;
    _recentInput._inputProcessor = this;
}

InputProcessor::~InputProcessor()
{
    for (auto& ti : _touches)
        delete ti;
    _touches.clear();
}

Vector2 InputProcessor::getTouchPosition(int touchId)
{
    for (auto& ti : _touches)
    {
        if (ti->touchId == touchId)
            return ti->pos;
    }
    return Vector2((float)_recentInput.getX(), (float)_recentInput.getY());
}

TouchInfo* InputProcessor::getTouch(int touchId, bool createIfNotExisits)
{
    TouchInfo* ret = nullptr;
    for (auto& ti : _touches)
    {
        if (ti->touchId == touchId)
            return ti;
        else if (ti->touchId == -1)
            ret = ti;
    }

    if (!ret)
    {
        if (!createIfNotExisits)
            return nullptr;

        ret = new TouchInfo();
        _touches.push_back(ret);
    }
    ret->touchId = touchId;
    return ret;
}

void InputProcessor::updateRecentInput(TouchInfo* ti, GObject* target)
{
    _recentInput._pos.x = (int)ti->pos.x;
    _recentInput._pos.y = (int)ti->pos.y;
    _recentInput._target = target;
    _recentInput._clickCount = ti->clickCount;
    _recentInput._button = (MouseButton)ti->button;
    _recentInput._mouseWheelDelta = ti->mouseWheelDelta;
    _recentInput._touchId = ti->touchId;

    bool flag = target != _owner;
    _touchOnUIFlagFrameId++;
    if (flag)
        _touchOnUI = true;
}

void InputProcessor::handleRollOver(TouchInfo* touch, GObject* target)
{
    if (touch->lastRollOver == target)
        return;

    std::vector<WeakPtr> rollOutChain;
    std::vector<WeakPtr> rollOverChain;
    GObject* element = touch->lastRollOver.ptr();
    while (element != nullptr)
    {
        rollOutChain.push_back(WeakPtr(element));
        element = element->findParent();
    }

    element = target;
    while (element != nullptr)
    {
        auto iter = std::find(rollOutChain.cbegin(), rollOutChain.cend(), element);
        if (iter != rollOutChain.cend())
        {
            rollOutChain.resize(iter - rollOutChain.cbegin());
            break;
        }
        rollOverChain.push_back(WeakPtr(element));
        element = element->findParent();
    }

    touch->lastRollOver = target;

    for (auto& wptr : rollOutChain)
    {
        element = wptr.ptr();
        if (element && element->onStage())
            element->dispatchEvent(UIEventType::RollOut);
    }

    for (auto& wptr : rollOverChain)
    {
        element = wptr.ptr();
        if (element && element->onStage())
            element->dispatchEvent(UIEventType::RollOver);
    }
}

void InputProcessor::addTouchMonitor(int touchId, GObject* target)
{
    TouchInfo* ti = getTouch(touchId, false);
    if (!ti)
        return;

    auto it = std::find(ti->touchMonitors.cbegin(), ti->touchMonitors.cend(), target);
    if (it == ti->touchMonitors.cend())
        ti->touchMonitors.push_back(WeakPtr(target));
}

void InputProcessor::removeTouchMonitor(GObject* target)
{
    for (auto it = _touches.cbegin(); it != _touches.cend(); ++it)
    {
        auto it2 = std::find((*it)->touchMonitors.begin(), (*it)->touchMonitors.end(), target);
        if (it2 != (*it)->touchMonitors.end())
            *it2 = nullptr;
    }
}

void InputProcessor::cancelClick(int touchId)
{
    TouchInfo* ti = getTouch(touchId, false);
    if (ti)
        ti->clickCancelled = true;
}

void InputProcessor::simulateClick(GObject* target, int touchId)
{
    _activeProcessor = this;

    Vector2 pt = target->localToGlobal(Vector2());
    _recentInput._pos.x = (int)pt.x;
    _recentInput._pos.y = (int)pt.y;
    _recentInput._target = target;
    _recentInput._clickCount = 1;
    _recentInput._button = (MouseButton)0 /*LEFT*/;
    _recentInput._touchId = touchId;

    if (_captureCallback)
        _captureCallback(UIEventType::TouchBegin);

    WeakPtr wptr(target);
    target->bubbleEvent(UIEventType::TouchBegin);

    target = wptr.ptr();
    if (target)
    {
        target->bubbleEvent(UIEventType::TouchEnd);

        target = wptr.ptr();
        if (target)
            target->bubbleEvent(UIEventType::Click);
    }

    _activeProcessor = nullptr;
}

void InputProcessor::setBegin(TouchInfo* touch, GObject* target)
{
    touch->began = true;
    touch->clickCancelled = false;
    touch->downPos = touch->pos;

    touch->downTargets.clear();
    GObject* obj = target;
    while (obj != nullptr)
    {
        touch->downTargets.push_back(WeakPtr(obj));
        obj = obj->findParent();
    }
}

void InputProcessor::setEnd(TouchInfo* touch, GObject* target)
{
    touch->began = false;

    auto now = clock();
    float elapsed = (now - touch->lastClickTime) / (double)CLOCKS_PER_SEC;

    if (elapsed < 0.45f)
    {
        if (touch->clickCount == 2)
            touch->clickCount = 1;
        else
            touch->clickCount++;
    }
    else
        touch->clickCount = 1;
    touch->lastClickTime = now;
}

GObject* InputProcessor::clickTest(TouchInfo* touch, GObject* target)
{
    if (touch->downTargets.empty()
        || touch->clickCancelled
        || std::abs(touch->pos.x - touch->downPos.x) > 50 || std::abs(touch->pos.y - touch->downPos.y) > 50)
        return nullptr;

    GObject* obj = touch->downTargets[0].ptr();
    if (obj && obj->onStage())
        return obj;

    obj = target;
    while (obj != nullptr)
    {
        auto it = std::find(touch->downTargets.cbegin(), touch->downTargets.cend(), obj);
        if (it != touch->downTargets.cend() && it->onStage())
        {
            obj = it->ptr();
            break;
        }

        obj = obj->findParent();
    }

    return obj;
}

bool InputProcessor::isTouchOnUI()
{
    return _touchOnUI;
}

void InputProcessor::disableDefaultTouchEvent()
{
    // GODOT_TODO: disable default touch handling
}

// === Godot input handlers ===

bool InputProcessor::onTouchBegin(const Vector2& screenPos, int touchId)
{
    if (!(_owner->isTouchable() && _owner->isVisible()))
        return false;

    GObject* target = _owner->hitTest(screenPos, nullptr);
    if (!target)
        target = _owner;

    TouchInfo* ti = getTouch(touchId);
    ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
    ti->button = (int)MouseButton::LEFT;

    updateRecentInput(ti, target);
    _activeProcessor = this;

    if (_captureCallback)
        _captureCallback(UIEventType::TouchBegin);

    WeakPtr wptr(target);
    target->bubbleEvent(UIEventType::TouchBegin);
    target = wptr.ptr();

    handleRollOver(ti, target);

    _activeProcessor = nullptr;
    return true;
}

void InputProcessor::onTouchMove(const Vector2& screenPos, int touchId)
{
    GObject* target = _owner->hitTest(screenPos, nullptr);
    if (!target)
        target = _owner;

    TouchInfo* ti = getTouch(touchId);
    ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
    ti->button = (int)MouseButton::LEFT;

    updateRecentInput(ti, target);
    _activeProcessor = this;

    if (_captureCallback)
        _captureCallback(UIEventType::TouchMove);

    handleRollOver(ti, target);

    if (ti->began)
    {
        bool done = false;
        size_t cnt = ti->touchMonitors.size();
        if (cnt > 0)
        {
            for (size_t i = 0; i < cnt; i++)
            {
                GObject* mm = ti->touchMonitors[i].ptr();
                if (!mm)
                    continue;

                WeakPtr wptr(mm);
                mm->dispatchEvent(UIEventType::TouchMove);
                if (!(wptr == mm)) done = true;
            }
        }
        if (!done)
        {
            WeakPtr wptr(target);
            target->bubbleEvent(UIEventType::TouchMove);
        }
    }

    _activeProcessor = nullptr;
}

void InputProcessor::onTouchEnd(const Vector2& screenPos, int touchId)
{
    TouchInfo* ti = getTouch(touchId, false);
    if (!ti) return;

    GObject* target = _owner->hitTest(screenPos, nullptr);
    if (!target)
        target = _owner;

    ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
    ti->button = (int)MouseButton::LEFT;
    setEnd(ti, target);

    updateRecentInput(ti, target);
    _activeProcessor = this;

    if (_captureCallback)
        _captureCallback(UIEventType::TouchEnd);

    bool done = false;
    size_t cnt = ti->touchMonitors.size();
    if (cnt > 0)
    {
        for (size_t i = 0; i < cnt; i++)
        {
            GObject* mm = ti->touchMonitors[i].ptr();
            if (!mm)
                continue;

            WeakPtr wptr(mm);
            mm->dispatchEvent(UIEventType::TouchEnd);
            if (!(wptr == mm)) done = true;
        }
    }
    if (!done)
    {
        WeakPtr wptr(target);
        target->bubbleEvent(UIEventType::TouchEnd);
    }

    handleRollOver(ti, nullptr);

    GObject* clickTarget = clickTest(ti, target);
    if (clickTarget)
    {
        ti->clickCount++;
        _recentInput._clickCount = ti->clickCount;
        WeakPtr cwptr(clickTarget);
        clickTarget->bubbleEvent(UIEventType::Click);
        clickTarget = cwptr.ptr();
    }

    ti->reset();
    _activeProcessor = nullptr;
}

void InputProcessor::onTouchCancel(const Vector2& screenPos, int touchId)
{
    TouchInfo* ti = getTouch(touchId, false);
    if (!ti) return;

    GObject* target = _owner->hitTest(screenPos, nullptr);
    if (!target)
        target = _owner;

    ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
    setEnd(ti, target);

    updateRecentInput(ti, target);
    _activeProcessor = this;

    handleRollOver(ti, target);

    ti->reset();
    _activeProcessor = nullptr;
}

bool InputProcessor::onMouseDown(const Vector2& screenPos, int button)
{
    if (!(_owner->isTouchable() && _owner->isVisible()))
        return false;

    GObject* target = _owner->hitTest(screenPos, nullptr);
    if (!target)
        target = _owner;

    TouchInfo* ti = getTouch(-1);
    ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
    ti->button = button;

    updateRecentInput(ti, target);
    _activeProcessor = this;

    if (button == (int)MouseButton::RIGHT)
        target->bubbleEvent(UIEventType::RightClick);
    else if (button == (int)MouseButton::MIDDLE)
        target->bubbleEvent(UIEventType::MiddleClick);
    else if (button == (int)MouseButton::LEFT)
    {
        // Nothing extra for left click - handled by TouchBegin
    }

    _activeProcessor = nullptr;
    return true;
}

void InputProcessor::onMouseUp(const Vector2& screenPos, int button)
{
    TouchInfo* ti = getTouch(-1);
    GObject* target = _owner->hitTest(screenPos, nullptr);
    if (!target)
        target = _owner;

    ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
    ti->button = button;
    setEnd(ti, target);

    updateRecentInput(ti, target);
    _activeProcessor = this;

    GObject* clickTarget = clickTest(ti, target);
    if (clickTarget)
    {
        WeakPtr cwptr(clickTarget);
        clickTarget->bubbleEvent(UIEventType::Click);
    }

    _activeProcessor = nullptr;
}

void InputProcessor::onMouseMove(const Vector2& screenPos)
{
    TouchInfo* ti = getTouch(-1, false);
    if (!ti)
    {
        ti = getTouch(-1);
        ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
        ti->button = 0;
    }

    GObject* target = _owner->hitTest(screenPos, nullptr);
    if (!target)
        target = _owner;

    ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
    updateRecentInput(ti, target);
    _activeProcessor = this;
    handleRollOver(ti, target);
    _activeProcessor = nullptr;
}

void InputProcessor::onMouseScroll(const Vector2& screenPos, int delta)
{
    TouchInfo* ti = getTouch(-1);
    GObject* target = _owner->hitTest(screenPos, nullptr);
    if (!target)
        target = _owner;

    ti->pos = GRoot::getInstance()->worldToRoot(screenPos);
    ti->mouseWheelDelta = delta;
    updateRecentInput(ti, target);
    _activeProcessor = this;

    target->bubbleEvent(UIEventType::MouseWheel);

    _activeProcessor = nullptr;
}

void InputProcessor::onKeyDown(int keyCode)
{
    _activeProcessor = this;
    _recentInput._keyCode = (KeyCode)(Key)keyCode;
    _recentInput._keyModifiers = _keyModifiers;
    _owner->bubbleEvent(UIEventType::KeyDown);
    _activeProcessor = nullptr;
}

void InputProcessor::onKeyUp(int keyCode)
{
    _activeProcessor = this;
    _recentInput._keyCode = (KeyCode)(Key)keyCode;
    _recentInput._keyModifiers = _keyModifiers;
    _owner->bubbleEvent(UIEventType::KeyUp);
    _activeProcessor = nullptr;
}

void InputProcessor::onFrameUpdate()
{
    _touchOnUI = false;
}

NS_FGUI_END
