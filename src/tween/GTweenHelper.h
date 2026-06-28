#ifndef __GTWEEN_HELPER_H__
#define __GTWEEN_HELPER_H__

#include "FairyGUIMacros.h"
#include "GTweener.h"

NS_FGUI_BEGIN

class GTweenHelper : public RefCounted
{
    GDCLASS(GTweenHelper, RefCounted)

public:
    static GTweenHelper* getInstance();

    GTweener* to_float(float start, float end, float duration);
    GTweener* to_vec2(const Vector2& start, const Vector2& end, float duration);
    GTweener* to_vec3(const Vector3& start, const Vector3& end, float duration);
    GTweener* to_vec4(const Vector4& start, const Vector4& end, float duration);
    GTweener* to_color(const Color& start, const Color& end, float duration);
    GTweener* to_double(double start, double end, float duration);
    GTweener* delayedCall(float delay);
    GTweener* shake(const Vector2& start, float amplitude, float duration);
    bool isTweening(RefCounted* target);
    void kill(RefCounted* target, bool complete);
    void clean();

    static void _bind_methods();

private:
    static GTweenHelper* _inst;
};

NS_FGUI_END

#endif
