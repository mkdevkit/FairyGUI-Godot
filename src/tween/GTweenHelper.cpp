#include "GTweenHelper.h"
#include "GTween.h"

NS_FGUI_BEGIN

GTweenHelper* GTweenHelper::_inst = nullptr;

GTweenHelper* GTweenHelper::getInstance()
{
    if (_inst == nullptr)
    {
        _inst = memnew(GTweenHelper);
        _inst->reference();
    }
    return _inst;
}

GTweener* GTweenHelper::to_float(float start, float end, float duration)
{
    return GTween::to(start, end, duration);
}

GTweener* GTweenHelper::to_vec2(const Vector2& start, const Vector2& end, float duration)
{
    return GTween::to(start, end, duration);
}

GTweener* GTweenHelper::to_vec3(const Vector3& start, const Vector3& end, float duration)
{
    return GTween::to(start, end, duration);
}

GTweener* GTweenHelper::to_vec4(const Vector4& start, const Vector4& end, float duration)
{
    return GTween::to(start, end, duration);
}

GTweener* GTweenHelper::to_color(const Color& start, const Color& end, float duration)
{
    return GTween::to(start, end, duration);
}

GTweener* GTweenHelper::to_double(double start, double end, float duration)
{
    return GTween::toDouble(start, end, duration);
}

GTweener* GTweenHelper::delayedCall(float delay)
{
    return GTween::delayedCall(delay);
}

GTweener* GTweenHelper::shake(const Vector2& start, float amplitude, float duration)
{
    return GTween::shake(start, amplitude, duration);
}

bool GTweenHelper::isTweening(RefCounted* target)
{
    return GTween::isTweening(target);
}

void GTweenHelper::kill(RefCounted* target, bool complete)
{
    GTween::kill(target, complete);
}

void GTweenHelper::clean()
{
    GTween::clean();
}

void GTweenHelper::_bind_methods()
{
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getInstance"), &GTweenHelper::getInstance);

    ClassDB::bind_method(D_METHOD("toFloat", "start", "end", "duration"), &GTweenHelper::to_float);
    ClassDB::bind_method(D_METHOD("toVec2", "start", "end", "duration"), &GTweenHelper::to_vec2);
    ClassDB::bind_method(D_METHOD("toVec3", "start", "end", "duration"), &GTweenHelper::to_vec3);
    ClassDB::bind_method(D_METHOD("toVec4", "start", "end", "duration"), &GTweenHelper::to_vec4);
    ClassDB::bind_method(D_METHOD("toColor", "start", "end", "duration"), &GTweenHelper::to_color);
    ClassDB::bind_method(D_METHOD("toDouble", "start", "end", "duration"), &GTweenHelper::to_double);
    ClassDB::bind_method(D_METHOD("delayedCall", "delay"), &GTweenHelper::delayedCall);
    ClassDB::bind_method(D_METHOD("shake", "start", "amplitude", "duration"), &GTweenHelper::shake);
    ClassDB::bind_method(D_METHOD("isTweening", "target"), &GTweenHelper::isTweening);
    ClassDB::bind_method(D_METHOD("kill", "target", "complete"), &GTweenHelper::kill, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("clean"), &GTweenHelper::clean);
}

NS_FGUI_END
