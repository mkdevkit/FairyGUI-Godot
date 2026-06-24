#include "TweenValue.h"

NS_FGUI_BEGIN
TweenValue::TweenValue():x(0),y(0),z(0),w(0),d(0)
{
}

Vector2 TweenValue::getVec2() const
{
    return Vector2(x, y);
}

void TweenValue::setVec2(const Vector2 & value)
{
    x = value.x;
    y = value.y;
}

Vector3 TweenValue::getVec3() const
{
    return Vector3(x, y, z);
}

void TweenValue::setVec3(const Vector3 & value)
{
    x = value.x;
    y = value.y;
    z = value.z;
}

Vector4 TweenValue::getVec4() const
{
    return Vector4(x, y, z, w);
}

void TweenValue::setVec4(const Vector4 & value)
{
    x = value.x;
    y = value.y;
    z = value.z;
    w = value.w;
}

Color TweenValue::getColor() const
{
    return Color(x, y, z, w);
}

void TweenValue::setColor(const Color & value)
{
    x = value.r;
    y = value.g;
    z = value.b;
    w = value.a;
}

float TweenValue::operator[](int index) const
{
    switch (index)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return w;
    default:
        fprintf(stderr, "TweenValue index out of bounds: %d\n", index);
        return x;
    }
}

float & TweenValue::operator[](int index)
{
    switch (index)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return w;
    default:
        fprintf(stderr, "TweenValue index out of bounds: %d\n", index);
        return x;
    }
}

void TweenValue::setZero()
{
    x = y = z = w = d = 0;
}

NS_FGUI_END