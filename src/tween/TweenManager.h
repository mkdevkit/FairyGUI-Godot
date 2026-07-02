#ifndef __TWEENMANAGER_H__
#define __TWEENMANAGER_H__

#include "FairyGUIMacros.h"
#include "TweenPropType.h"
// cocos2d.h removed - see godot_types.h

NS_FGUI_BEGIN

class GTweener;

class TweenManager
{
public:
    static GTweener* createTween();
    static bool isTweening(RefCounted* target, TweenPropType propType);
    static bool killTweens(RefCounted* target, TweenPropType propType, bool completed);
    static bool killTweensAny(void* target, bool completed);
    static GTweener* getTween(RefCounted* target, TweenPropType propType);
    static void update(float dt);
    static void clean();
    static void killAll(bool completed = false);
    static void init();
    static void beginCallback(GTweener* tweener);
    static void endCallback();
    static bool isUpdating() { return _updating; }
    static void flushPendingTweeners();

private:
    static void flushKilledTweeners();
    static void reset();

    static Ref<GTweener>* _activeTweens;
    static std::vector<Ref<GTweener>> _tweenerPool;
    static int _totalActiveTweens;
    static int _arrayLength;
    static bool _inited;
    static bool _updating;
    static GTweener* _executingTweener;
};

NS_FGUI_END

#endif
