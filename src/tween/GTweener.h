#ifndef __GTWEENER_H__
#define __GTWEENER_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h
#include "EaseType.h"
#include "TweenValue.h"
#include "TweenPropType.h"

NS_FGUI_BEGIN

class GPath;

class GTweener : public RefCounted
{
    GDCLASS(GTweener, RefCounted)
public:
    typedef std::function<void(GTweener* tweener)> GTweenCallback;
    typedef std::function<void()> GTweenCallback0;

    GTweener();
    ~GTweener();
    GTweener* setDelay(float value);
    float getDelay() const { return _delay; }
    GTweener* setDuration(float value);
    float getDuration() const { return _duration; }
    GTweener* setBreakpoint(float value);
    GTweener* setEase(EaseType value);
    GTweener* setEasePeriod(float value);
    GTweener* setEaseOvershootOrAmplitude(float value);
    GTweener* setRepeat(int repeat, bool yoyo = false);
    int getRepeat() const { return _repeat; }
    GTweener* setTimeScale(float value);
    GTweener* setSnapping(bool value);
    GTweener* setTargetAny(void* value);
    GTweener* setTarget(RefCounted* target);
    GTweener* setTarget(RefCounted* target, TweenPropType propType);
    void* getTarget() const { return _target; }
    GTweener* setGtUserData(const Variant& value);
    GTweener* setPath(GPath* path);
    const Variant& getUserData() const { return _userData; }
    GTweener* onUpdate(GTweenCallback callback);
    GTweener* onStart(GTweenCallback callback);
    GTweener* onComplete(GTweenCallback0 callback);
    GTweener* onComplete1(GTweenCallback callback);

    float getNormalizedTime() const { return _normalizedTime; }
    bool isCompleted() const { return _ended != 0; }
    bool allCompleted() const { return _ended == 1; }
    GTweener* setPaused(bool paused);
    void seek(float time);
    void kill(bool complete = false);

    TweenValue startValue;
    TweenValue endValue;
    TweenValue value;
    TweenValue deltaValue;

    // GDScript bindings
    static void _bind_methods();

    // GDScript callback setters (Callable wrappers)
    Ref<GTweener> gd_setDelay(float value);
    Ref<GTweener> gd_setDuration(float value);
    Ref<GTweener> gd_setRepeat(int repeat, bool yoyo);
    Ref<GTweener> gd_setTimeScale(float value);
    Ref<GTweener> gd_setSnapping(bool value);
    Ref<GTweener> gd_setPaused(bool paused);
    Ref<GTweener> gd_onUpdate(const Callable& callable);
    Ref<GTweener> gd_onStart(const Callable& callable);
    Ref<GTweener> gd_onComplete(const Callable& callable);
    Ref<GTweener> gd_setEase(int value);
    Ref<GTweener> gd_setTarget(Object* target, int prop_type);

private:
    GTweener* _to(float start, float end, float duration);
    GTweener* _to(const Vector2& start, const Vector2& end, float duration);
    GTweener* _to(const Vector3& start, const Vector3& end, float duration);
    GTweener* _to(const Vector4& start, const Vector4& end, float duration);
    GTweener* _to(const Color& start, const Color& end, float duration);
    GTweener* _to(double start, double end, float duration);
    GTweener* _shake(const Vector2& start, float amplitude, float duration);
    void _init();
    void _reset();
    void _update(float dt);
    void update();
    void callStartCallback();
    void callUpdateCallback();
    void callCompleteCallback();

private:
    void* _target;
    Ref<RefCounted> _refTarget;
    TweenPropType _propType;
    bool _killed;
    bool _paused;

    float _delay;
    float _duration;
    float _breakpoint;
    EaseType _easeType;
    float _easeOvershootOrAmplitude;
    float _easePeriod;
    int _repeat;
    bool _yoyo;
    float _timeScale;
    bool _snapping;
    Variant _userData;
    int _valueSize;
    GPath* _path;

    GTweenCallback _onUpdate;
    GTweenCallback _onStart;
    GTweenCallback _onComplete;
    GTweenCallback0 _onComplete0;

    bool _started;
    int _ended;
    float _elapsedTime;
    float _normalizedTime;

    friend class GTween;
    friend class TweenManager;
};

NS_FGUI_END

#endif
