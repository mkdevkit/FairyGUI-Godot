#ifndef __INPUTEVENT_H__
#define __INPUTEVENT_H__

#include "FairyGUIMacros.h"
#include "godot_types.h"

NS_FGUI_BEGIN

class GObject;
class InputProcessor;

enum class MouseButton
{
    LEFT = 1,
    MIDDLE = 2,
    RIGHT = 3
};

enum class KeyCode
{
    NONE = 0,
    // Common key codes
    BACKSPACE = 8,
    TAB = 9,
    ENTER = 13,
    ESCAPE = 27,
    SPACE = 32,
    LEFT = 37,
    UP = 38,
    RIGHT = 39,
    DOWN = 40,
    DELETE = 46,
    A = 65,
    C = 67,
    V = 86,
    X = 88,
    Y = 89,
    Z = 90
};

class InputEvent
{
public:
    InputEvent();
    ~InputEvent();

    GObject* getTarget() const { return _target; }
    const int getX() const { return (int)_pos.x; }
    const int getY() const { return (int)_pos.y; }
    const Vector2& getPosition() const { return _pos; }
    int getTouchId() const { return _touchId; }
    int isDoubleClick() const { return _clickCount == 2; }
    MouseButton getButton() const { return _button; }
    KeyCode getKeyCode() const { return _keyCode; }
    bool isCtrlDown() const { return (_keyModifiers & 1) != 0; }
    bool isAltDown() const  { return (_keyModifiers & 2) != 0; }
    bool isShiftDown() const { return (_keyModifiers & 4) != 0; }
    int getMouseWheelDelta() const { return _mouseWheelDelta; }

    InputProcessor* getProcessor() const { return _inputProcessor; }

private:
    GObject* _target;
    Vector2 _pos;
    int _touchId;
    int _clickCount;
    int _mouseWheelDelta;
    MouseButton _button;
    KeyCode _keyCode;
    uint16_t _keyModifiers;
    InputProcessor* _inputProcessor;

    friend class InputProcessor;
};

NS_FGUI_END

#endif
