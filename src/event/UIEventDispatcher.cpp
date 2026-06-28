#include "UIEventDispatcher.h"
#include "GComponent.h"
#include "InputProcessor.h"
#include "utils/WeakPtr.h"

NS_FGUI_BEGIN

const EventTag EventTag::None;

EventTag::EventTag() :_value(0)
{
}

EventTag::EventTag(void * ptr) : _value((uintptr_t)ptr)
{
}

EventTag::EventTag(int value) : _value(value)
{
}

EventTag::EventTag(const EventTag & other)
{
    *this = other;
}

EventTag::EventTag(EventTag && other)
{
    *this = std::move(other);
}

EventTag::~EventTag()
{
}

EventTag & EventTag::operator=(const EventTag & other)
{
    if (this != &other)
        _value = other._value;
    return *this;
}

EventTag & EventTag::operator=(EventTag && other)
{
    if (this != &other)
    {
        _value = other._value;
        other._value = 0;
    }
    return *this;
}

EventTag & EventTag::operator=(void * ptr)
{
    _value = (uintptr_t)ptr;
    return *this;
}

EventTag & EventTag::operator=(int v)
{
    _value = v;
    return *this;
}

bool EventTag::operator!=(const EventTag & v)
{
    return _value != v._value;
}

bool EventTag::operator!=(const EventTag & v) const
{
    return _value != v._value;
}

bool EventTag::operator==(const EventTag & v)
{
    return _value == v._value;
}

bool EventTag::operator==(const EventTag & v) const
{
    return _value == v._value;
}

UIEventDispatcher::UIEventDispatcher() :_dispatching(0)
{
}

UIEventDispatcher::~UIEventDispatcher()
{
    _dispatching = 0;
    removeEventListeners();
}

void UIEventDispatcher::addEventListener(int eventType, const EventCallback& callback, const EventTag& tag)
{
    if (!tag.isNone())
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); it++)
        {
            if ((*it)->eventType == eventType && (*it)->tag == tag)
            {
                (*it)->callback = callback;
                return;
            }
        }
    }

    EventCallbackItem* item = new EventCallbackItem();
    item->callback = callback;
    item->eventType = eventType;
    item->tag = tag;
    item->dispatching = 0;
    _callbacks.push_back(item);
}

void UIEventDispatcher::removeEventListener(int eventType, const EventTag& tag)
{
    if (_callbacks.empty())
        return;

    for (auto it = _callbacks.begin(); it != _callbacks.end(); )
    {
        if ((*it)->eventType == eventType && ((*it)->tag == tag || tag.isNone()))
        {
            if (_dispatching > 0)
            {
                (*it)->callback = nullptr;
                it++;
            }
            else
            {
                delete (*it);
                it = _callbacks.erase(it);
            }
        }
        else
            it++;
    }
}

void UIEventDispatcher::removeEventListeners()
{
    if (_callbacks.empty())
        return;

    if (_dispatching > 0)
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); ++it)
            (*it)->callback = nullptr;
    }
    else
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); it++)
            delete (*it);
        _callbacks.clear();
    }
}

bool UIEventDispatcher::hasEventListener(int eventType, const EventTag& tag) const
{
    if (_callbacks.empty())
        return false;

    for (auto it = _callbacks.cbegin(); it != _callbacks.cend(); ++it)
    {
        if ((*it)->eventType == eventType && ((*it)->tag == tag || tag.isNone()) && (*it)->callback != nullptr)
            return true;
    }
    return false;
}

bool UIEventDispatcher::dispatchEvent(int eventType, void* data, const Variant& dataValue)
{
    if (_callbacks.size() == 0)
        return false;

    EventContext context;
    context._sender = this;
    context._type = eventType;
    if (InputProcessor::_activeProcessor)
        context._inputEvent = InputProcessor::_activeProcessor->getRecentInput();
    context._dataValue = dataValue;
    context._data = data;

    doDispatch(eventType, &context);

    return context._defaultPrevented;
}

bool UIEventDispatcher::bubbleEvent(int eventType, void* data, const Variant& dataValue)
{
    EventContext context;
    if (InputProcessor::_activeProcessor)
        context._inputEvent = InputProcessor::_activeProcessor->getRecentInput();
    context._type = eventType;
    context._dataValue = dataValue;
    context._data = data;

    doBubble(eventType, &context);

    return context._defaultPrevented;
}

bool UIEventDispatcher::isDispatchingEvent(int eventType)
{
    for (auto it = _callbacks.begin(); it != _callbacks.end(); ++it)
    {
        if ((*it)->eventType == eventType)
            return (*it)->dispatching > 0;
    }
    return false;
}

void UIEventDispatcher::doDispatch(int eventType, EventContext* context)
{
    // retain();

    _dispatching++;
    context->_sender = this;
    bool hasDeletedItems = false;

    size_t cnt = _callbacks.size(); //dont use iterator, because new item would be added in callback.
    for (size_t i = 0; i < cnt; i++)
    {
        EventCallbackItem* ci = _callbacks[i];
        if (ci->callback == nullptr)
        {
            hasDeletedItems = true;
            continue;
        }
        if (ci->eventType == eventType)
        {
            ci->dispatching++;
            context->_touchCapture = 0;
            ci->callback(context);
            ci->dispatching--;
            if (context->_touchCapture != 0 && dynamic_cast<GObject*>(this))
            {
                if (context->_touchCapture == 1 && eventType == UIEventType::TouchBegin)
                    context->getInput()->getProcessor()->addTouchMonitor(context->getInput()->getTouchId(), dynamic_cast<GObject*>(this));
                else if (context->_touchCapture == 2)
                    context->getInput()->getProcessor()->removeTouchMonitor(dynamic_cast<GObject*>(this));
            }
        }
    }

    _dispatching--;
    if (hasDeletedItems && _dispatching == 0)
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); )
        {
            if ((*it)->callback == nullptr)
            {
                delete (*it);
                it = _callbacks.erase(it);
            }
            else
                it++;
        }
    }

    // release(); // GODOT_ADAPT: Godot ref counting handled by engine
}

void UIEventDispatcher::doBubble(int eventType, EventContext* context)
{
    //parent maybe disposed in callbacks
    WeakPtr wptr(((GObject*)this)->findParent());

    if (!_callbacks.empty())
    {
        context->_isStopped = false;
        doDispatch(eventType, context);
        if (context->_isStopped)
            return;
    }

    GObject* p = wptr.ptr();
    if (p)
        p->doBubble(eventType, context);
}

void UIEventDispatcher::gd_addEventListener(int eventType, const Callable& callable)
{
    addEventListener(eventType, [callable](EventContext* ctx) {
        callable.call();
    });
}

void UIEventDispatcher::gd_removeEventListener(int eventType)
{
    removeEventListener(eventType, EventTag::None);
}

void UIEventDispatcher::_bind_methods()
{
    // EventCallback and EventTag types can't be bound via ClassDB
    ClassDB::bind_integer_constant(get_class_static(), "", "ENTER", static_cast<int64_t>(UIEventType::Enter));
    ClassDB::bind_integer_constant(get_class_static(), "", "EXIT", static_cast<int64_t>(UIEventType::Exit));
    ClassDB::bind_integer_constant(get_class_static(), "", "CHANGED", static_cast<int64_t>(UIEventType::Changed));
    ClassDB::bind_integer_constant(get_class_static(), "", "SUBMIT", static_cast<int64_t>(UIEventType::Submit));
    ClassDB::bind_integer_constant(get_class_static(), "", "CLICK", static_cast<int64_t>(UIEventType::Click));
    ClassDB::bind_integer_constant(get_class_static(), "", "TOUCHBEGIN", static_cast<int64_t>(UIEventType::TouchBegin));
    ClassDB::bind_integer_constant(get_class_static(), "", "TOUCHMOVE", static_cast<int64_t>(UIEventType::TouchMove));
    ClassDB::bind_integer_constant(get_class_static(), "", "TOUCHEND", static_cast<int64_t>(UIEventType::TouchEnd));
    ClassDB::bind_integer_constant(get_class_static(), "", "ROLLOVER", static_cast<int64_t>(UIEventType::RollOver));
    ClassDB::bind_integer_constant(get_class_static(), "", "ROLLOUT", static_cast<int64_t>(UIEventType::RollOut));
    ClassDB::bind_integer_constant(get_class_static(), "", "MOUSEWHEEL", static_cast<int64_t>(UIEventType::MouseWheel));
    ClassDB::bind_integer_constant(get_class_static(), "", "RIGHTCLICK", static_cast<int64_t>(UIEventType::RightClick));
    ClassDB::bind_integer_constant(get_class_static(), "", "MIDDLECLICK", static_cast<int64_t>(UIEventType::MiddleClick));
    ClassDB::bind_integer_constant(get_class_static(), "", "KEYDOWN", static_cast<int64_t>(UIEventType::KeyDown));
    ClassDB::bind_integer_constant(get_class_static(), "", "KEYUP", static_cast<int64_t>(UIEventType::KeyUp));
    ClassDB::bind_integer_constant(get_class_static(), "", "SCROLL", static_cast<int64_t>(UIEventType::Scroll));
    ClassDB::bind_integer_constant(get_class_static(), "", "SCROLLEND", static_cast<int64_t>(UIEventType::ScrollEnd));
    ClassDB::bind_integer_constant(get_class_static(), "", "PULLDOWNRELEASE", static_cast<int64_t>(UIEventType::PullDownRelease));
    ClassDB::bind_integer_constant(get_class_static(), "", "PULLUPRELEASE", static_cast<int64_t>(UIEventType::PullUpRelease));
    ClassDB::bind_integer_constant(get_class_static(), "", "POSITIONCHANGE", static_cast<int64_t>(UIEventType::PositionChange));
    ClassDB::bind_integer_constant(get_class_static(), "", "SIZECHANGE", static_cast<int64_t>(UIEventType::SizeChange));
    ClassDB::bind_integer_constant(get_class_static(), "", "CLICKITEM", static_cast<int64_t>(UIEventType::ClickItem));
    ClassDB::bind_integer_constant(get_class_static(), "", "CLICKLINK", static_cast<int64_t>(UIEventType::ClickLink));
    ClassDB::bind_integer_constant(get_class_static(), "", "CLICKMENU", static_cast<int64_t>(UIEventType::ClickMenu));
    ClassDB::bind_integer_constant(get_class_static(), "", "RIGHTCLICKITEM", static_cast<int64_t>(UIEventType::RightClickItem));
    ClassDB::bind_integer_constant(get_class_static(), "", "DRAGSTART", static_cast<int64_t>(UIEventType::DragStart));
    ClassDB::bind_integer_constant(get_class_static(), "", "DRAGMOVE", static_cast<int64_t>(UIEventType::DragMove));
    ClassDB::bind_integer_constant(get_class_static(), "", "DRAGEND", static_cast<int64_t>(UIEventType::DragEnd));
    ClassDB::bind_integer_constant(get_class_static(), "", "DROP", static_cast<int64_t>(UIEventType::Drop));
    ClassDB::bind_integer_constant(get_class_static(), "", "GEARSTOP", static_cast<int64_t>(UIEventType::GearStop));

    // Callable-based event listener for GDScript
    ClassDB::bind_method(D_METHOD("addEventListener", "type", "callable"), &UIEventDispatcher::gd_addEventListener);
    ClassDB::bind_method(D_METHOD("removeEventListener", "type"), &UIEventDispatcher::gd_removeEventListener);
}

NS_FGUI_END