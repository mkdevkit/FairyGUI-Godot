#ifndef __GTWEEN_H__
#define __GTWEEN_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h
#include "GTweener.h"
#include "EaseType.h"
#include "TweenValue.h"
#include "TweenPropType.h"

NS_FGUI_BEGIN

class GTween
{
public:
    static GTweener* to(float startValue, float endValue, float duration);
    static GTweener* to(const Vector2& startValue, const Vector2& endValue, float duration);
    static GTweener* to(const Vector3& startValue, const Vector3& endValue, float duration);
    static GTweener* to(const Vector4& startValue, const Vector4& endValue, float duration);
    static GTweener* to(const Color& startValue, const Color& endValue, float duration);
    static GTweener* toDouble(double startValue, double endValue, float duration);
    static GTweener* delayedCall(float delay);
    static GTweener* shake(const Vector2& startValue, float amplitude, float duration);
    static bool isTweening(RefCounted* target);
    static bool isTweening(RefCounted* target, TweenPropType propType);
    static void kill(RefCounted* target);
    static void kill(RefCounted* target, bool complete);
    static void kill(RefCounted* target, TweenPropType propType, bool complete);
    static GTweener* getTween(RefCounted* target);
    static GTweener* getTween(RefCounted* target, TweenPropType propType);
    static void clean();
};

NS_FGUI_END

#endif
