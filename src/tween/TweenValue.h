#ifndef __TWEENVALUE_H__
#define __TWEENVALUE_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h

NS_FGUI_BEGIN

class TweenValue
{
public:
    float x;
    float y;
    float z;
    float w;
    double d;

    TweenValue();

    Vector2 getVec2() const;
    void setVec2(const Vector2& value);
    Vector3 getVec3() const;
    void setVec3(const Vector3& value);
    Vector4 getVec4() const;
    void setVec4(const Vector4& value);
    Color getColor() const;
    void setColor(const Color& value);
    float operator[] (int index) const;
    float& operator[] (int index);
    void setZero();
};

NS_FGUI_END

#endif
