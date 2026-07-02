#include "GTweener.h"
#include "EaseManager.h"
#include "GObject.h"
#include "GPath.h"
#include "TweenManager.h"
#include <cstdlib>
#include <cstring>
#include <new>

NS_FGUI_BEGIN

void GTweener::clearRefTarget()
{
    if (_refTargetId.is_valid())
    {
        if (ObjectDB::get_instance(_refTargetId))
            _refTarget = Ref<RefCounted>();
        else
        {
            // Target already freed; do not dereference a stale Ref.
            RefCounted** pref = reinterpret_cast<RefCounted**>(&_refTarget);
            *pref = nullptr;
        }
        _refTargetId = ObjectID();
    }
    else if (!_refTarget.is_null())
    {
        RefCounted** pref = reinterpret_cast<RefCounted**>(&_refTarget);
        *pref = nullptr;
    }
}

GTweener::GTweener() : _target(nullptr),
                       _refTarget(nullptr),
                       _refTargetId(),
                       _userData(),
                       _onStart(nullptr),
                       _onUpdate(nullptr),
                       _onComplete(nullptr),
                       _onComplete0(nullptr),
                       _scriptOnStartId(),
                       _scriptOnUpdateId(),
                       _scriptOnCompleteId(),
                       _path(nullptr)
{
}

GTweener::~GTweener()
{
}

GTweener* GTweener::setDelay(float value)
{
    _delay = value;
    return this;
}

GTweener* GTweener::setDuration(float value)
{
    _duration = value;
    return this;
}

GTweener* GTweener::setBreakpoint(float value)
{
    _breakpoint = value;
    return this;
}

GTweener* GTweener::setEase(EaseType value)
{
    _easeType = value;
    return this;
}

GTweener* GTweener::setEasePeriod(float value)
{
    _easePeriod = value;
    return this;
}

GTweener* GTweener::setEaseOvershootOrAmplitude(float value)
{
    _easeOvershootOrAmplitude = value;
    return this;
}

GTweener* GTweener::setRepeat(int repeat, bool yoyo)
{
    _repeat = repeat;
    _yoyo = yoyo;
    return this;
}

GTweener* GTweener::setTimeScale(float value)
{
    _timeScale = value;
    return this;
}

GTweener* GTweener::setSnapping(bool value)
{
    _snapping = value;
    return this;
}

GTweener* GTweener::setTargetAny(void* value)
{
    clearRefTarget();
    _target = value;
    return this;
}

GTweener* GTweener::setTarget(RefCounted* value)
{
    return setTarget(value, TweenPropType::None);
}

GTweener* GTweener::setTarget(RefCounted* target, TweenPropType propType)
{
    clearRefTarget();
    _target = target;
    _propType = propType;
    if (target != nullptr)
    {
        if (Object* obj = Object::cast_to<Object>(target))
            _refTargetId = obj->get_instance_id();
        _refTarget = Ref<RefCounted>(target);
    }
    return this;
}

GTweener* GTweener::setGtUserData(const Variant& value)
{
    _userData = value;
    return this;
}

GTweener* GTweener::setPath(GPath* path)
{
    _path = path;
    return this;
}

GTweener* GTweener::onUpdate(GTweenCallback callback)
{
    _onUpdate = callback;
    return this;
}

GTweener* GTweener::onStart(GTweenCallback callback)
{
    _onStart = callback;
    return this;
}

GTweener* GTweener::onComplete(GTweenCallback0 callback)
{
    _onComplete0 = callback;
    return this;
}

GTweener* GTweener::onComplete1(GTweenCallback callback)
{
    _onComplete = callback;
    return this;
}

GTweener* GTweener::setPaused(bool paused)
{
    _paused = paused;
    return this;
}

void GTweener::seek(float time)
{
    if (_killed)
        return;

    _elapsedTime = time;
    if (_elapsedTime < _delay)
    {
        if (_started)
            _elapsedTime = _delay;
        else
            return;
    }

    update();
}

void GTweener::kill(bool complete)
{
    if (_killed)
        return;

    if (complete)
    {
        if (_ended == 0)
        {
            if (_breakpoint >= 0)
                _elapsedTime = _delay + _breakpoint;
            else if (_repeat >= 0)
                _elapsedTime = _delay + _duration * (_repeat + 1);
            else
                _elapsedTime = _delay + _duration * 2;
            update();
        }

        callCompleteCallback();
    }

    _killed = true;
}

GTweener* GTweener::_to(float start, float end, float duration)
{
    _valueSize = 1;
    startValue.x = start;
    endValue.x = end;
    value.x = start;
    _duration = duration;
    return this;
}

GTweener* GTweener::_to(const Vector2& start, const Vector2& end, float duration)
{
    _valueSize = 2;
    startValue.setVec2(start);
    endValue.setVec2(end);
    value.setVec2(start);
    _duration = duration;
    return this;
}

GTweener* GTweener::_to(const Vector3& start, const Vector3& end, float duration)
{
    _valueSize = 3;
    startValue.setVec3(start);
    endValue.setVec3(end);
    value.setVec3(start);
    _duration = duration;
    return this;
}

GTweener* GTweener::_to(const Vector4& start, const Vector4& end, float duration)
{
    _valueSize = 4;
    startValue.setVec4(start);
    endValue.setVec4(end);
    value.setVec4(start);
    _duration = duration;
    return this;
}

GTweener* GTweener::_to(const Color& start, const Color& end, float duration)
{
    _valueSize = 4;
    startValue.setColor(start);
    endValue.setColor(end);
    value.setColor(start);
    _duration = duration;
    return this;
}

GTweener* GTweener::_to(double start, double end, float duration)
{
    _valueSize = 5;
    startValue.d = start;
    endValue.d = end;
    value.d = start;
    _duration = duration;
    return this;
}

GTweener* GTweener::_shake(const Vector2& start, float amplitude, float duration)
{
    _valueSize = 6;
    startValue.setVec2(start);
    startValue.w = amplitude;
    _duration = duration;
    _easeType = EaseType::Linear;
    return this;
}

void GTweener::_init()
{
    _delay = 0;
    _duration = 0;
    _breakpoint = -1;
    _easeType = EaseType::QuadOut;
    _timeScale = 1;
    _easePeriod = 0;
    _easeOvershootOrAmplitude = 1.70158f;
    _snapping = false;
    _repeat = 0;
    _yoyo = false;
    _valueSize = 0;
    _started = false;
    _paused = false;
    _killed = false;
    _elapsedTime = 0;
    _normalizedTime = 0;
    _ended = 0;
    startValue.setZero();
    endValue.setZero();
    value.setZero();
    deltaValue.setZero();
}

void GTweener::abandonVariant(Variant& value)
{
    // Never run ~Variant() or operator=; the stored value may hold dangling refs.
    memset(static_cast<void*>(&value), 0, sizeof(Variant));
    new (&value) Variant();
}

void GTweener::abandonCallable(Callable& callable, ObjectID& id)
{
    // Never run ~Callable() or operator=; unref on stale internals will crash.
    memset(static_cast<void*>(&callable), 0, sizeof(Callable));
    new (&callable) Callable();
    id = ObjectID();
}

void GTweener::abandonCallback(GTweenCallback& callback)
{
    memset(static_cast<void*>(&callback), 0, sizeof(GTweenCallback));
    new (&callback) GTweenCallback();
}

void GTweener::abandonCallback0(GTweenCallback0& callback)
{
    memset(static_cast<void*>(&callback), 0, sizeof(GTweenCallback0));
    new (&callback) GTweenCallback0();
}

void GTweener::clearScriptBindings()
{
    abandonCallable(_scriptOnStart, _scriptOnStartId);
    abandonCallable(_scriptOnUpdate, _scriptOnUpdateId);
    abandonCallable(_scriptOnComplete, _scriptOnCompleteId);
}

void GTweener::callScriptOnStart(GTweener* tweener)
{
    if (_scriptOnStartId.is_valid() && ObjectDB::get_instance(_scriptOnStartId))
        _scriptOnStart.call(tweener);
}

void GTweener::callScriptOnUpdate(GTweener* tweener)
{
    if (_scriptOnUpdateId.is_valid() && ObjectDB::get_instance(_scriptOnUpdateId))
        _scriptOnUpdate.call(tweener);
}

void GTweener::callScriptOnComplete()
{
    if (_scriptOnCompleteId.is_valid() && ObjectDB::get_instance(_scriptOnCompleteId))
        _scriptOnComplete.call();
}

void GTweener::_reset()
{
    clearRefTarget();
    _target = nullptr;
    _path = nullptr;
    abandonCallback(_onStart);
    abandonCallback(_onUpdate);
    abandonCallback(_onComplete);
    abandonCallback0(_onComplete0);
    clearScriptBindings();
    abandonVariant(_userData);
}

void GTweener::_update(float dt)
{
    if (_ended != 0) //Maybe completed by seek
    {
        callCompleteCallback();
        _killed = true;
        return;
    }

    if (_timeScale != 1)
        dt *= _timeScale;
    if (dt == 0)
        return;

    _elapsedTime += dt;
    update();

    if (_ended != 0)
    {
        if (!_killed)
        {
            callCompleteCallback();
            _killed = true;
        }
    }
}

void GTweener::update()
{
    _ended = 0;

    if (_valueSize == 0) //DelayedCall
    {
        if (_elapsedTime >= _delay + _duration)
            _ended = 1;

        return;
    }

    if (!_started)
    {
        if (_elapsedTime < _delay)
            return;

        _started = true;
        callStartCallback();
        if (_killed)
            return;
    }

    bool reversed = false;
    float tt = _elapsedTime - _delay;
    if (_breakpoint >= 0 && tt >= _breakpoint)
    {
        tt = _breakpoint;
        _ended = 2;
    }

    if (_repeat != 0)
    {
        int round = (int)floor(tt / _duration);
        tt -= _duration * round;
        if (_yoyo)
            reversed = round % 2 == 1;

        if (_repeat > 0 && _repeat - round < 0)
        {
            if (_yoyo)
                reversed = _repeat % 2 == 1;
            tt = _duration;
            _ended = 1;
        }
    }
    else if (tt >= _duration)
    {
        tt = _duration;
        _ended = 1;
    }

    _normalizedTime = EaseManager::evaluate(_easeType, reversed ? (_duration - tt) : tt, _duration,
                                            _easeOvershootOrAmplitude, _easePeriod);

    value.setZero();
    deltaValue.setZero();

    if (_valueSize == 5)
    {
        double d = startValue.d + (endValue.d - startValue.d) * _normalizedTime;
        if (_snapping)
            d = round(d);
        deltaValue.d = d - value.d;
        value.d = d;
        value.x = (float)d;
    }
    else if (_valueSize == 6)
    {
        if (_ended == 0)
        {
            float r = startValue.w * (1 - _normalizedTime);
            float rx = (((float)rand() / RAND_MAX) * 2 - 1) * r;
            float ry = (((float)rand() / RAND_MAX) * 2 - 1) * r;
            rx = rx > 0 ? ceil(rx) : floor(rx);
            ry = ry > 0 ? ceil(ry) : floor(ry);

            deltaValue.x = rx;
            deltaValue.y = ry;
            value.x = startValue.x + rx;
            value.y = startValue.y + ry;
        }
        else
            value.setVec3(startValue.getVec3());
    }
    else if (_path != nullptr)
    {
        Vector3 vec3 = _path->getPointAt(_normalizedTime);
        if (_snapping)
        {
            vec3.x = round(vec3.x);
            vec3.y = round(vec3.y);
            vec3.z = round(vec3.z);
        }
        deltaValue.setVec3(vec3 - value.getVec3());
        value.setVec3(vec3);
    }
    else
    {
        for (int i = 0; i < _valueSize; i++)
        {
            float n1 = startValue[i];
            float n2 = endValue[i];
            float f = n1 + (n2 - n1) * _normalizedTime;
            if (_snapping)
                f = round(f);
            deltaValue[i] = f - value[i];
            value[i] = f;
        }
        value.d = value.x;
    }

    if (_refTargetId.is_valid() && _propType != TweenPropType::None)
    {
        Object* obj = ObjectDB::get_instance(_refTargetId);
        if (!obj)
        {
            _killed = true;
        }
        else if (GObject* gobj = Object::cast_to<GObject>(obj))
        {
            TweenPropTypeUtils::setProps(gobj, _propType, value);
        }
        else if (Node* node = Object::cast_to<Node>(obj))
        {
            TweenPropTypeUtils::setProps(node, _propType, value);
        }
    }

    callUpdateCallback();
}

void GTweener::callStartCallback()
{
    TweenManager::beginCallback(this);
    if (_onStart != nullptr)
        _onStart(this);
    TweenManager::endCallback();
}

void GTweener::callUpdateCallback()
{
    TweenManager::beginCallback(this);
    if (_onUpdate != nullptr)
        _onUpdate(this);
    TweenManager::endCallback();
}

void GTweener::callCompleteCallback()
{
    TweenManager::beginCallback(this);
    if (_onComplete != nullptr)
        _onComplete(this);
    if (_onComplete0 != nullptr)
        _onComplete0();
    TweenManager::endCallback();
}

Ref<GTweener> GTweener::gd_setDelay(float value)
{
    return Ref<GTweener>(setDelay(value));
}

Ref<GTweener> GTweener::gd_setDuration(float value)
{
    return Ref<GTweener>(setDuration(value));
}

Ref<GTweener> GTweener::gd_setRepeat(int repeat, bool yoyo)
{
    return Ref<GTweener>(setRepeat(repeat, yoyo));
}

Ref<GTweener> GTweener::gd_setTimeScale(float value)
{
    return Ref<GTweener>(setTimeScale(value));
}

Ref<GTweener> GTweener::gd_setSnapping(bool value)
{
    return Ref<GTweener>(setSnapping(value));
}

Ref<GTweener> GTweener::gd_setPaused(bool paused)
{
    return Ref<GTweener>(setPaused(paused));
}

Ref<GTweener> GTweener::gd_onUpdate(const Callable& callable)
{
    _scriptOnUpdate = callable;
    if (callable.is_valid())
    {
        Object* obj = callable.get_object();
        _scriptOnUpdateId = obj ? obj->get_instance_id() : ObjectID();
    }
    else
        _scriptOnUpdateId = ObjectID();

    onUpdate([this](GTweener* tweener) {
        callScriptOnUpdate(tweener);
    });
    return Ref<GTweener>(this);
}

Ref<GTweener> GTweener::gd_onStart(const Callable& callable)
{
    _scriptOnStart = callable;
    if (callable.is_valid())
    {
        Object* obj = callable.get_object();
        _scriptOnStartId = obj ? obj->get_instance_id() : ObjectID();
    }
    else
        _scriptOnStartId = ObjectID();

    onStart([this](GTweener* tweener) {
        callScriptOnStart(tweener);
    });
    return Ref<GTweener>(this);
}

Ref<GTweener> GTweener::gd_onComplete(const Callable& callable)
{
    _scriptOnComplete = callable;
    if (callable.is_valid())
    {
        Object* obj = callable.get_object();
        _scriptOnCompleteId = obj ? obj->get_instance_id() : ObjectID();
    }
    else
        _scriptOnCompleteId = ObjectID();

    onComplete([this]() {
        callScriptOnComplete();
    });
    return Ref<GTweener>(this);
}

Ref<GTweener> GTweener::gd_setEase(int value)
{
    return Ref<GTweener>(setEase((EaseType)value));
}

Ref<GTweener> GTweener::gd_setTarget(Object* target, int prop_type)
{
    RefCounted* ref = Object::cast_to<RefCounted>(target);
    if (ref)
        setTarget(ref, (TweenPropType)prop_type);
    return Ref<GTweener>(this);
}

void GTweener::_bind_methods()
{
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "NONE", static_cast<int64_t>(TweenPropType::None));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "X", static_cast<int64_t>(TweenPropType::X));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "Y", static_cast<int64_t>(TweenPropType::Y));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "POSITION", static_cast<int64_t>(TweenPropType::Position));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "WIDTH", static_cast<int64_t>(TweenPropType::Width));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "HEIGHT", static_cast<int64_t>(TweenPropType::Height));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "SIZE", static_cast<int64_t>(TweenPropType::Size));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "SCALE_X", static_cast<int64_t>(TweenPropType::ScaleX));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "SCALE_Y", static_cast<int64_t>(TweenPropType::ScaleY));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "SCALE", static_cast<int64_t>(TweenPropType::Scale));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "ROTATION", static_cast<int64_t>(TweenPropType::Rotation));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "ALPHA", static_cast<int64_t>(TweenPropType::Alpha));
    ClassDB::bind_integer_constant(get_class_static(), "TweenPropType", "PROGRESS", static_cast<int64_t>(TweenPropType::Progress));

    ClassDB::bind_method(D_METHOD("setDelay", "value"), &GTweener::gd_setDelay);
    ClassDB::bind_method(D_METHOD("getDelay"), &GTweener::getDelay);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "delay"), "setDelay", "getDelay");

    ClassDB::bind_method(D_METHOD("setDuration", "value"), &GTweener::gd_setDuration);
    ClassDB::bind_method(D_METHOD("getDuration"), &GTweener::getDuration);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"), "setDuration", "getDuration");

    ClassDB::bind_method(D_METHOD("setEase", "value"), &GTweener::gd_setEase);
    ClassDB::bind_method(D_METHOD("setRepeat", "repeat", "yoyo"), &GTweener::gd_setRepeat, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("getRepeat"), &GTweener::getRepeat);
    ClassDB::bind_method(D_METHOD("setTimeScale", "value"), &GTweener::gd_setTimeScale);
    ClassDB::bind_method(D_METHOD("setSnapping", "value"), &GTweener::gd_setSnapping);
    ClassDB::bind_method(D_METHOD("setTarget", "target", "prop_type"), &GTweener::gd_setTarget, DEFVAL((int)TweenPropType::None));
    ClassDB::bind_method(D_METHOD("setPaused", "value"), &GTweener::gd_setPaused);
    ClassDB::bind_method(D_METHOD("seek", "time"), &GTweener::seek);
    ClassDB::bind_method(D_METHOD("kill", "complete"), &GTweener::kill, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("getNormalizedTime"), &GTweener::getNormalizedTime);
    ClassDB::bind_method(D_METHOD("isCompleted"), &GTweener::isCompleted);
    ClassDB::bind_method(D_METHOD("allCompleted"), &GTweener::allCompleted);

    ClassDB::bind_method(D_METHOD("onUpdate", "callable"), &GTweener::gd_onUpdate);
    ClassDB::bind_method(D_METHOD("onStart", "callable"), &GTweener::gd_onStart);
    ClassDB::bind_method(D_METHOD("onComplete", "callable"), &GTweener::gd_onComplete);
}

NS_FGUI_END