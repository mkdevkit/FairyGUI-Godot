#include "FUIContainer.h"
#include "GComponent.h"
#include "servers/rendering_server.h"

NS_FGUI_BEGIN

FUIContainer::FUIContainer() :
    _clippingEnabled(false),
    _stencil(nullptr),
    _alphaThreshold(1.0f),
    _inverted(false),
    gOwner(nullptr)
{
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
    // ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clippingEnabled"), "setClippingEnabled", "isClippingEnabled");

    ClassDB::bind_method(D_METHOD("setClippingRegion", "region"), &FUIContainer::setClippingRegion);
    ClassDB::bind_method(D_METHOD("getClippingRegion"), &FUIContainer::getClippingRegion);
    // ADD_PROPERTY(PropertyInfo(Variant::RECT2, "clippingRegion"), "setClippingRegion", "getClippingRegion");

    ClassDB::bind_method(D_METHOD("setStencil", "stencil"), &FUIContainer::setStencil);
    ClassDB::bind_method(D_METHOD("getStencil"), &FUIContainer::getStencil);
    // ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stencil", PROPERTY_HINT_NODE_TYPE, "Node"), "setStencil", "getStencil");

    ClassDB::bind_method(D_METHOD("setAlphaThreshold", "threshold"), &FUIContainer::setAlphaThreshold);
    ClassDB::bind_method(D_METHOD("getAlphaThreshold"), &FUIContainer::getAlphaThreshold);
    // ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "alphaThreshold"), "setAlphaThreshold", "getAlphaThreshold");

    ClassDB::bind_method(D_METHOD("setInverted", "inverted"), &FUIContainer::setInverted);
    ClassDB::bind_method(D_METHOD("isInverted"), &FUIContainer::isInverted);
    // ADD_PROPERTY(PropertyInfo(Variant::BOOL, "inverted"), "setInverted", "isInverted");
}

FUIContainer* FUIContainer::create()
{
    FUIContainer* ret = memnew(FUIContainer);
    return ret;
}

void FUIContainer::_notification(int p_what)
{
    if (p_what == NOTIFICATION_ENTER_CANVAS)
    {
        applyClipping();
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
