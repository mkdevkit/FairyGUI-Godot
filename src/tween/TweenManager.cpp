#include "TweenManager.h"
#include "GTweener.h"

NS_FGUI_BEGIN
Ref<GTweener>* TweenManager::_activeTweens = nullptr;
std::vector<Ref<GTweener>> TweenManager::_tweenerPool;
int TweenManager::_totalActiveTweens = 0;
int TweenManager::_arrayLength = 0;
bool TweenManager::_inited = false;
bool TweenManager::_updating = false;
GTweener* TweenManager::_executingTweener = nullptr;

void TweenManager::beginCallback(GTweener* tweener)
{
    _executingTweener = tweener;
}

void TweenManager::endCallback()
{
    _executingTweener = nullptr;
}

void TweenManager::flushPendingTweeners()
{
    flushKilledTweeners();
}

class TweenEngine
{
public:
    void update(float dt)
    {
        TweenManager::update(dt);
    }
};
static TweenEngine tweenEngine;

void TweenManager::flushKilledTweeners()
{
    if (_updating)
        return;

    int cnt = _totalActiveTweens;
    int freePosStart = -1;
    for (int i = 0; i < cnt; i++)
    {
        Ref<GTweener>& tweenerRef = _activeTweens[i];
        if (tweenerRef.is_null())
        {
            if (freePosStart == -1)
                freePosStart = i;
        }
        else if (tweenerRef->_killed)
        {
            GTweener* tweener = tweenerRef.ptr();
            tweener->_reset();
            _tweenerPool.push_back(tweenerRef);
            tweenerRef = Ref<GTweener>();

            if (freePosStart == -1)
                freePosStart = i;
        }
        else if (freePosStart != -1)
        {
            _activeTweens[freePosStart] = tweenerRef;
            tweenerRef = Ref<GTweener>();
            freePosStart++;
        }
    }

    if (freePosStart >= 0)
    {
        if (_totalActiveTweens != cnt)
        {
            int j = cnt;
            cnt = _totalActiveTweens - cnt;
            for (int i = 0; i < cnt; i++)
                _activeTweens[freePosStart++] = _activeTweens[j++];
        }
        _totalActiveTweens = freePosStart;
    }
}

GTweener* TweenManager::createTween()
{
    if (!_inited)
        init();

    Ref<GTweener> tweenerRef;
    int cnt = (int)_tweenerPool.size();
    if (cnt > 0)
    {
        tweenerRef = _tweenerPool[cnt - 1];
        _tweenerPool.pop_back();
    }
    else
        tweenerRef = Ref<GTweener>(memnew(GTweener));

    GTweener* tweener = tweenerRef.ptr();
    tweener->_reset();
    tweener->_init();
    _activeTweens[_totalActiveTweens++] = tweenerRef;

    if (_totalActiveTweens == _arrayLength)
    {
        int newLen = _arrayLength + ceil(_arrayLength * 0.5f);
        Ref<GTweener>* newArray = new Ref<GTweener>[newLen];
        for (int i = 0; i < _arrayLength; i++)
            newArray[i] = _activeTweens[i];
        delete[] _activeTweens;
        _activeTweens = newArray;
        _arrayLength = newLen;
    }

    return tweener;
}

bool TweenManager::isTweening(RefCounted* target, TweenPropType propType)
{
    if (target == nullptr)
        return false;

    bool anyType = propType == TweenPropType::None;
    for (int i = 0; i < _totalActiveTweens; i++)
    {
        GTweener* tweener = _activeTweens[i].ptr();
        if (tweener != nullptr && tweener->_target == target && !tweener->_killed && (anyType || tweener->_propType == propType))
            return true;
    }

    return false;
}

bool TweenManager::killTweens(RefCounted* target, TweenPropType propType, bool completed)
{
    if (target == nullptr)
        return false;

    bool flag = false;
    int cnt = _totalActiveTweens;
    bool anyType = propType == TweenPropType::None;
    for (int i = 0; i < cnt; i++)
    {
        GTweener* tweener = _activeTweens[i].ptr();
        if (tweener != nullptr && tweener->_target == target && !tweener->_killed && tweener != _executingTweener && (anyType || tweener->_propType == propType))
        {
            tweener->kill(completed);
            flag = true;
        }
    }

    return flag;
}

bool TweenManager::killTweensAny(void* target, bool completed)
{
    if (target == nullptr)
        return false;

    bool flag = false;
    int cnt = _totalActiveTweens;
    for (int i = 0; i < cnt; i++)
    {
        GTweener* tweener = _activeTweens[i].ptr();
        if (tweener != nullptr && tweener->_target == target && !tweener->_killed && tweener != _executingTweener)
        {
            tweener->kill(completed);
            flag = true;
        }
    }

    return flag;
}

GTweener* TweenManager::getTween(RefCounted* target, TweenPropType propType)
{
    if (target == nullptr)
        return nullptr;

    int cnt = _totalActiveTweens;
    bool anyType = propType == TweenPropType::None;
    for (int i = 0; i < cnt; i++)
    {
        GTweener* tweener = _activeTweens[i].ptr();
        if (tweener != nullptr && tweener->_target == target && !tweener->_killed && (anyType || tweener->_propType == propType))
        {
            return tweener;
        }
    }

    return nullptr;
}

void TweenManager::update(float dt)
{
    _updating = true;

    int cnt = _totalActiveTweens;
    int freePosStart = -1;
    for (int i = 0; i < cnt; i++)
    {
        Ref<GTweener>& tweenerRef = _activeTweens[i];
        GTweener* tweener = tweenerRef.ptr();
        if (tweener == nullptr)
        {
            if (freePosStart == -1)
                freePosStart = i;
        }
        else if (tweener->_killed || (tweener->_refTargetId.is_valid() && !ObjectDB::get_instance(tweener->_refTargetId)))
        {
            if (!tweener->_killed)
                tweener->_killed = true;

            tweener->_reset();
            _tweenerPool.push_back(tweenerRef);
            tweenerRef = Ref<GTweener>();

            if (freePosStart == -1)
                freePosStart = i;
        }
        else
        {
            if (!tweener->_paused)
                tweener->_update(dt);

            if (freePosStart != -1)
            {
                _activeTweens[freePosStart] = tweenerRef;
                tweenerRef = Ref<GTweener>();
                freePosStart++;
            }
        }
    }

    if (freePosStart >= 0)
    {
        if (_totalActiveTweens != cnt)
        {
            int j = cnt;
            cnt = _totalActiveTweens - cnt;
            for (int i = 0; i < cnt; i++)
                _activeTweens[freePosStart++] = _activeTweens[j++];
        }
        _totalActiveTweens = freePosStart;
    }

    _updating = false;
    flushKilledTweeners();
}

void TweenManager::clean()
{
    killAll(true);

    for (Ref<GTweener>& tweenerRef : _tweenerPool)
    {
        if (GTweener* tweener = tweenerRef.ptr())
            tweener->kill(true);
    }
    _tweenerPool.clear();
}

void TweenManager::killAll(bool completed)
{
    if (!_inited)
        return;

    int cnt = _totalActiveTweens;
    for (int i = 0; i < cnt; i++)
    {
        GTweener* tweener = _activeTweens[i].ptr();
        if (tweener != nullptr && !tweener->_killed)
            tweener->kill(completed);
    }

    flushKilledTweeners();
}

void TweenManager::init()
{
    _inited = true;

    if (_activeTweens == nullptr)
    {
        _arrayLength = 30;
        _activeTweens = new Ref<GTweener>[_arrayLength];
    }
}

void TweenManager::reset()
{
    int cnt = _totalActiveTweens;
    for (int i = 0; i < cnt; i++)
    {
        Ref<GTweener>& tweenerRef = _activeTweens[i];
        if (GTweener* tweener = tweenerRef.ptr())
        {
            tweener->_reset();
            _tweenerPool.push_back(tweenerRef);
            tweenerRef = Ref<GTweener>();
        }
    }

    _totalActiveTweens = 0;
    _inited = false;
}

NS_FGUI_END
