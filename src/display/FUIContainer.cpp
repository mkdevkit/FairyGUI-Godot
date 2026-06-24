#include "FUIContainer.h"
#include "GComponent.h"
#include "GRoot.h"
#include "event/InputProcessor.h"
#include "servers/rendering_server.h"

NS_FGUI_BEGIN

static void _queue_redraw_all(Node* node) {
    CanvasItem* ci = Object::cast_to<CanvasItem>(node);
    if (ci) ci->queue_redraw();
    int count = node->get_child_count();
    for (int i = 0; i < count; i++)
        _queue_redraw_all(node->get_child(i));
}

FUIContainer::FUIContainer() :
    _clippingEnabled(false),
    _stencil(nullptr),
    _alphaThreshold(1.0f),
    _inverted(false),
    gOwner(nullptr)
{
    item_rect_changed(); // enable NOTIFICATION_DRAW for Node2D
    set_process_unhandled_input(true);
}

FUIContainer::~FUIContainer()
{
    if (_stencil)
    {
        _stencil->queue_free();
        _stencil = nullptr;
    }
}

void FUIContainer::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setClippingEnabled", "value"), &FUIContainer::setClippingEnabled);
    ClassDB::bind_method(D_METHOD("isClippingEnabled"), &FUIContainer::isClippingEnabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clippingEnabled"), "setClippingEnabled", "isClippingEnabled");

    ClassDB::bind_method(D_METHOD("setClippingRegion", "region"), &FUIContainer::setClippingRegion);
    ClassDB::bind_method(D_METHOD("getClippingRegion"), &FUIContainer::getClippingRegion);
    ADD_PROPERTY(PropertyInfo(Variant::RECT2, "clippingRegion"), "setClippingRegion", "getClippingRegion");

    ClassDB::bind_method(D_METHOD("setStencil", "stencil"), &FUIContainer::setStencil);
    ClassDB::bind_method(D_METHOD("getStencil"), &FUIContainer::getStencil);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stencil", PROPERTY_HINT_NODE_TYPE, "Node"), "setStencil", "getStencil");

    ClassDB::bind_method(D_METHOD("setAlphaThreshold", "threshold"), &FUIContainer::setAlphaThreshold);
    ClassDB::bind_method(D_METHOD("getAlphaThreshold"), &FUIContainer::getAlphaThreshold);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "alphaThreshold"), "setAlphaThreshold", "getAlphaThreshold");

    ClassDB::bind_method(D_METHOD("setInverted", "inverted"), &FUIContainer::setInverted);
    ClassDB::bind_method(D_METHOD("isInverted"), &FUIContainer::isInverted);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "inverted"), "setInverted", "isInverted");

    ClassDB::bind_method(D_METHOD("_deferred_redraw_all"), &FUIContainer::_deferred_redraw_all);
}

FUIContainer* FUIContainer::create()
{
    FUIContainer* ret = memnew(FUIContainer);
    return ret;
}

void FUIContainer::_notification(int p_what)
{
    if (p_what == NOTIFICATION_DRAW) {
        _draw();
        return;
    }
    if (p_what == NOTIFICATION_ENTER_CANVAS)
    {
        applyClipping();
        call_deferred("_deferred_redraw_all");
    }
    Node2D::_notification(p_what);
}

void FUIContainer::_deferred_redraw_all()
{
    _queue_redraw_all(this);
}

void FUIContainer::_draw()
{
}

void FUIContainer::unhandled_input(const Ref<::InputEvent>& event)
{
    if (!gOwner) return;
    GRoot* root = Object::cast_to<GRoot>(gOwner);
    if (!root) root = gOwner->getRoot();
    if (!root) return;

    InputProcessor* ip = root->getInputProcessor();
    if (!ip) return;

    Ref<InputEventMouseButton> mb = event;
    if (mb.is_valid()) {
        int btn = (int)mb->get_button_index();
        if (mb->is_pressed()) {
            if (btn == (int)MouseButton::WHEEL_UP) {
                ip->onMouseScroll(mb->get_position(), 1);
                return;
            }
            if (btn == (int)MouseButton::WHEEL_DOWN) {
                ip->onMouseScroll(mb->get_position(), -1);
                return;
            }
            if (btn == (int)MouseButton::LEFT) {
                // Left button uses TouchBegin/TouchEnd for click+drag detection
                ip->onTouchBegin(mb->get_position(), 0);
            } else {
                ip->onMouseDown(mb->get_position(), btn);
            }
        } else {
            if (btn == (int)MouseButton::LEFT) {
                ip->onTouchEnd(mb->get_position(), 0);
            } else {
                ip->onMouseUp(mb->get_position(), btn);
            }
        }
        return;
    }

    Ref<InputEventMouseMotion> mm = event;
    if (mm.is_valid()) {
        // When left button is held, use onTouchMove for drag detection.
        // Otherwise use onMouseMove for hover/rollover.
        if (mm->get_button_mask().has_flag(::MouseButtonMask::LEFT)) {
            ip->onTouchMove(mm->get_position(), 0);
        } else {
            ip->onMouseMove(mm->get_position());
        }
        return;
    }
}

bool FUIContainer::isClippingEnabled() const
{
    return _clippingEnabled;
}

void FUIContainer::setClippingEnabled(bool value)
{
    if (_clippingEnabled != value)
    {
        _clippingEnabled = value;
        applyClipping();
    }
}

const Rect2& FUIContainer::getClippingRegion() const
{
    return _clippingRegion;
}

void FUIContainer::setClippingRegion(const Rect2& clippingRegion)
{
    _clippingRegion = clippingRegion;
    if (_clippingEnabled)
        applyClipping();
}

Node* FUIContainer::getStencil() const
{
    return _stencil;
}

void FUIContainer::setStencil(Node* stencil)
{
    // In Godot, stencil-style clipping is done via clip_children mode.
    // The stencil node is added as a child and used as a mask.
    if (_stencil == stencil)
        return;

    if (_stencil)
    {
        _stencil->queue_free();
        _stencil = nullptr;
    }

    _stencil = stencil;

    if (_stencil)
    {
        add_child(_stencil);
        // GODOT_TODO: set up alpha threshold shader for stencil
    }

    applyClipping();
}

float FUIContainer::getAlphaThreshold() const
{
    return _alphaThreshold;
}

void FUIContainer::setAlphaThreshold(float alphaThreshold)
{
    _alphaThreshold = alphaThreshold;
    // GODOT_TODO: apply alpha threshold to stencil children via shader
}

bool FUIContainer::isInverted() const
{
    return _inverted;
}

void FUIContainer::setInverted(bool inverted)
{
    _inverted = inverted;
    // GODOT_TODO: implement inverted stencil
}

void FUIContainer::applyClipping()
{
    // Godot's clip_children mode automatically clips children to the node's bounding rect
    if (_clippingEnabled || _stencil != nullptr)
        set_clip_children_mode(CanvasItem::CLIP_CHILDREN_AND_DRAW);
    else
        set_clip_children_mode(CanvasItem::CLIP_CHILDREN_DISABLED);
}

NS_FGUI_END
