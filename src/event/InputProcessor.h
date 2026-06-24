#ifndef __INPUTPROCESSOR_H__
#define __INPUTPROCESSOR_H__

#include "FairyGUIMacros.h"
#include "InputEvent.h"
#include <functional>
#include <vector>

NS_FGUI_BEGIN

class GComponent;
class TouchInfo;

class InputProcessor
{
public:
    typedef std::function<void(int eventType)> CaptureEventCallback;

    InputProcessor(GComponent* owner);
    ~InputProcessor();

    InputEvent* getRecentInput() { return &_recentInput; }
    static bool isTouchOnUI();

    Vector2 getTouchPosition(int touchId);

    void addTouchMonitor(int touchId, GObject* target);
    void removeTouchMonitor(GObject* target);

    void cancelClick(int touchId);
    void simulateClick(GObject* target, int touchId = -1);

    void setCaptureCallback(CaptureEventCallback value) { _captureCallback = value; }

    // === Godot input entry points ===
    void disableDefaultTouchEvent();
    bool onTouchBegin(const Vector2& screenPos, int touchId);
    void onTouchMove(const Vector2& screenPos, int touchId);
    void onTouchEnd(const Vector2& screenPos, int touchId);
    void onTouchCancel(const Vector2& screenPos, int touchId);

    bool onMouseDown(const Vector2& screenPos, int button);
    void onMouseUp(const Vector2& screenPos, int button);
    void onMouseMove(const Vector2& screenPos);
    void onMouseScroll(const Vector2& screenPos, int delta);

    void onKeyDown(int keyCode);
    void onKeyUp(int keyCode);

    void onFrameUpdate();

    static InputProcessor* _activeProcessor;

private:
    TouchInfo* getTouch(int touchId, bool createIfNotExisits = true);
    void updateRecentInput(TouchInfo* touch, GObject* target);
    void handleRollOver(TouchInfo* touch, GObject* target);
    void setBegin(TouchInfo* touch, GObject* target);
    void setEnd(TouchInfo* touch, GObject* target);
    GObject* clickTest(TouchInfo* touch, GObject* target);

    std::vector<TouchInfo*> _touches;
    GComponent* _owner;
    CaptureEventCallback _captureCallback;
    InputEvent _recentInput;
    uint16_t _keyModifiers;

    static bool _touchOnUI;
    static unsigned int _touchOnUIFlagFrameId;

    friend class UIEventDispatcher;
};

NS_FGUI_END

#endif
