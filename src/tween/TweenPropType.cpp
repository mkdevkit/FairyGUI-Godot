#include "TweenPropType.h"
#include "TweenValue.h"
#include "GObject.h"
#include "GProgressBar.h"


NS_FGUI_BEGIN
void TweenPropTypeUtils::setProps(GObject * target, TweenPropType propType, const TweenValue & value)
{
    if (target == nullptr)
        return;

    switch (propType)
    {
    case TweenPropType::X:
        target->setX(value.x);
        break;

    case TweenPropType::Y:
        target->setY(value.x);
        break;

    case TweenPropType::Position:
        target->setPosition(value.x, value.y);
        break;

    case TweenPropType::Width:
        target->setWidth(value.x);
        break;

    case TweenPropType::Height:
        target->setHeight(value.x);
        break;

    case TweenPropType::Size:
        target->setSize(value.x, value.y);
        break;

    case TweenPropType::ScaleX:
        target->setScaleX(value.x);
        break;

    case TweenPropType::ScaleY:
        target->setScaleY(value.x);
        break;

    case TweenPropType::Scale:
        target->setScale(value.x, value.y);
        break;

    case TweenPropType::Rotation:
        target->setRotation(value.x);
        break;

    case TweenPropType::Alpha:
        target->setAlpha(value.x);
        break;

    case TweenPropType::Progress:
        target->as<GProgressBar>()->update(value.d);
        break;
    default:
        break;
    }
}

void TweenPropTypeUtils::setProps(Node * target, TweenPropType propType, const TweenValue & value)
{
    switch (propType)
    {
    case TweenPropType::X:
        ((Node2D*)target)->set_position(Vector2(value.x, ((Node2D*)target)->get_position().y));
        break;

    case TweenPropType::Y:
        ((Node2D*)target)->set_position(Vector2(((Node2D*)target)->get_position().x, value.x));
        break;

    case TweenPropType::Position:
        ((Node2D*)target)->set_position(Vector2(value.x, value.y));
        break;

    case TweenPropType::ScaleX:
        ((Node2D*)target)->set_scale(Vector2(value.x, ((Node2D*)target)->get_scale().y));
        break;

    case TweenPropType::ScaleY:
        ((Node2D*)target)->set_scale(Vector2(((Node2D*)target)->get_scale().x, value.x));
        break;

    case TweenPropType::Scale:
        ((Node2D*)target)->set_scale(Vector2(value.x, value.y));
        break;

    case TweenPropType::Rotation:
        ((Node2D*)target)->set_rotation(value.x);
        break;

    case TweenPropType::Alpha:
    {
        CanvasItem* ci = Object::cast_to<CanvasItem>(target);
        if (ci)
        {
            Color c = ci->get_modulate();
            c.a = value.x;
            ci->set_modulate(c);
        }
        break;
    }

    default:
        break;
    }
}

NS_FGUI_END

