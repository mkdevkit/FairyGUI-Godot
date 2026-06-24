#include "GTween.h"
#include "TweenManager.h"

NS_FGUI_BEGIN
GTweener* GTween::to(float startValue, float endValue, float duration)
{
    return TweenManager::createTween()->_to(startValue, endValue, duration);
}

GTweener* GTween::to(const Vector2& startValue, const Vector2 & endValue, float duration)
{
    return TweenManager::createTween()->_to(startValue, endValue, duration);
}

GTweener* GTween::to(const Vector3& startValue, const Vector3 & endValue, float duration)
{
    return TweenManager::createTween()->_to(startValue, endValue, duration);
}

GTweener* GTween::to(const Vector4& startValue, const Vector4 & endValue, float duration)
{
    return TweenManager::createTween()->_to(startValue, endValue, duration);
}

GTweener* GTween::to(const Color& startValue, const Color & endValue, float duration)
{
    return TweenManager::createTween()->_to(startValue, endValue, duration);
}

GTweener* GTween::toDouble(double startValue, double endValue, float duration)
{
    return TweenManager::createTween()->_to(startValue, endValue, duration);
}

GTweener* GTween::delayedCall(float delay)
{
    return TweenManager::createTween()->setDelay(delay);
}

GTweener* GTween::shake(const Vector2& startValue, float amplitude, float duration)
{
    return TweenManager::createTween()->_shake(startValue, amplitude, duration);
}

bool GTween::isTweening(RefCounted * target)
{
    return TweenManager::isTweening(target, TweenPropType::None);
}

bool GTween::isTweening(RefCounted * target, TweenPropType propType)
{
    return TweenManager::isTweening(target, propType);
}

void GTween::kill(RefCounted * target)
{
    TweenManager::killTweens(target, TweenPropType::None, false);
}

void GTween::kill(RefCounted * target, bool complete)
{
    TweenManager::killTweens(target, TweenPropType::None, complete);
}

void GTween::kill(RefCounted * target, TweenPropType propType, bool complete)
{
    TweenManager::killTweens(target, propType, complete);
}

GTweener* GTween::getTween(RefCounted * target)
{
    return TweenManager::getTween(target, TweenPropType::None);
}

GTweener* GTween::getTween(RefCounted * target, TweenPropType propType)
{
    return TweenManager::getTween(target, propType);
}

void GTween::clean()
{
    TweenManager::clean();
}

NS_FGUI_END