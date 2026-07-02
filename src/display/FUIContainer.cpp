#include "FUIContainer.h"
#include "GComponent.h"
#include "GRoot.h"
#include "display/FUISprite.h"
#include "event/InputProcessor.h"
#include "scene/resources/shader.h"
#include "scene/resources/material.h"
#include "scene/main/viewport.h"
#include "servers/rendering_server.h"

NS_FGUI_BEGIN

static void mark_input_handled(Node* node)
{
    if (Viewport* vp = node->get_viewport())
        vp->set_input_as_handled();
}

static void _queue_redraw_all(Node* node) {
    CanvasItem* ci = Object::cast_to<CanvasItem>(node);
    if (ci) ci->queue_redraw();
    int count = node->get_child_count();
    for (int i = 0; i < count; i++)
        _queue_redraw_all(node->get_child(i));
}

static Ref<Shader> _mask_shader;

static Ref<Shader> get_mask_shader()
{
    if (_mask_shader.is_null())
    {
        _mask_shader.instantiate();
        _mask_shader->set_code(
            "shader_type canvas_item;\n"
            "uniform float alpha_threshold : hint_range(0.0, 1.0) = 0.05;\n"
            "uniform bool inverted = false;\n"
            "void fragment() {\n"
            "    float a = texture(TEXTURE, UV).a * COLOR.a;\n"
            "    bool keep = inverted ? (a <= alpha_threshold) : (a >= alpha_threshold);\n"
            "    if (!keep)\n"
            "        discard;\n"
            "}\n");
    }
    return _mask_shader;
}

static Ref<ShaderMaterial> create_mask_material(float threshold, bool inverted)
{
    Ref<ShaderMaterial> mat;
    mat.instantiate();
    mat->set_shader(get_mask_shader());
    mat->set_shader_parameter("alpha_threshold", threshold);
    mat->set_shader_parameter("inverted", inverted);
    return mat;
}

static void apply_mask_material_recursive(Node* node, float threshold, bool inverted)
{
    if (CanvasItem* ci = Object::cast_to<CanvasItem>(node))
    {
        if (threshold < 1.0f || inverted)
            ci->set_material(create_mask_material(threshold, inverted));
        else
            ci->set_material(Ref<Material>());
    }
    for (int i = 0; i < node->get_child_count(); i++)
        apply_mask_material_recursive(node->get_child(i), threshold, inverted);
}

FUIContainer::FUIContainer() :
    _clippingEnabled(false),
    _stencil(nullptr),
    _alphaThreshold(1.0f),
    _inverted(false),
    _clipMode(CanvasItem::CLIP_CHILDREN_AND_DRAW),
    gOwner(nullptr)
{
    item_rect_changed();
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
    return memnew(FUIContainer);
}

void FUIContainer::_notification(int p_what)
{
    if (p_what == NOTIFICATION_DRAW) {
        _draw();
        return;
    }
    if (p_what == NOTIFICATION_PROCESS)
    {
        if (_processCallback)
            _processCallback(get_process_delta_time());
        return;
    }
    if (p_what == NOTIFICATION_ENTER_CANVAS)
    {
        applyClipping();
        call_deferred("_deferred_redraw_all");
    }
    if (p_what == NOTIFICATION_PREDELETE)
    {
        while (get_child_count() > 0)
            remove_child(get_child(0));
    }
    Node2D::_notification(p_what);
}

void FUIContainer::_deferred_redraw_all()
{
    _queue_redraw_all(this);
}

void FUIContainer::_drawStencilSilhouette()
{
    FUISprite* sp = Object::cast_to<FUISprite>(_stencil);
    if (!sp)
        return;

    Ref<Texture2D> tex = sp->getRealTexture();
    if (tex.is_null())
        return;

    Transform2D xf = get_global_transform().affine_inverse() * sp->get_global_transform();
    draw_set_transform_matrix(xf);

    Rect2 texRect = sp->getRegion();
    if (texRect.size.x <= 0 || texRect.size.y <= 0)
        texRect.size = tex->get_size();

    Vector2 size = sp->getContentSize();
    if (size.x <= 0 || size.y <= 0)
        size = sp->get_rect().size;

    draw_texture_rect_region(tex, Rect2(Vector2(), size), texRect, Color(1, 1, 1, 1));

    draw_set_transform_matrix(Transform2D());
}

void FUIContainer::_draw()
{
    if (_stencil)
        _drawStencilSilhouette();
    else if (_clippingEnabled && _clippingRegion.size.x > 0 && _clippingRegion.size.y > 0)
        draw_rect(_clippingRegion, Color(1, 1, 1, 0));
}

void FUIContainer::applyStencilEffects()
{
    if (!_stencil)
        return;

    apply_mask_material_recursive(_stencil, _alphaThreshold, _inverted);

    if (CanvasItem* ci = Object::cast_to<CanvasItem>(_stencil))
        ci->set_visible(false);

    queue_redraw();
}

void FUIContainer::unhandled_input(const Ref<::InputEvent>& event)
{
    if (!gOwner) return;
    GRoot* root = Object::cast_to<GRoot>(gOwner);
    if (!root) root = gOwner->getRoot();
    if (!root) return;

    InputProcessor* ip = root->getInputProcessor();
    if (!ip || !ip->isTouchListenerEnabled())
        return;

    Ref<InputEventScreenTouch> st = event;
    if (st.is_valid())
    {
        if (st->is_pressed())
            ip->onTouchBegin(st->get_position(), st->get_index());
        else
            ip->onTouchEnd(st->get_position(), st->get_index());
        mark_input_handled(this);
        return;
    }

    Ref<InputEventScreenDrag> sd = event;
    if (sd.is_valid())
    {
        ip->onTouchMove(sd->get_position(), sd->get_index());
        mark_input_handled(this);
        return;
    }

    Ref<InputEventMouseButton> mb = event;
    if (mb.is_valid()) {
        int btn = (int)mb->get_button_index();
        if (mb->is_pressed()) {
            if (btn == (int)MouseButton::WHEEL_UP) {
                ip->onMouseScroll(mb->get_position(), 1);
                mark_input_handled(this);
                return;
            }
            if (btn == (int)MouseButton::WHEEL_DOWN) {
                ip->onMouseScroll(mb->get_position(), -1);
                mark_input_handled(this);
                return;
            }
            if (btn == (int)MouseButton::LEFT) {
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
        mark_input_handled(this);
        return;
    }

    Ref<InputEventMouseMotion> mm = event;
    if (mm.is_valid()) {
        if (mm->get_button_mask().has_flag(::MouseButtonMask::LEFT)) {
            ip->onTouchMove(mm->get_position(), 0);
        } else {
            ip->onMouseMove(mm->get_position());
        }
        mark_input_handled(this);
        return;
    }

    Ref<InputEventKey> key = event;
    if (key.is_valid())
    {
        if (key->is_echo())
            return;
        if (key->is_pressed())
            ip->onKeyDown((int)key->get_keycode());
        else
            ip->onKeyUp((int)key->get_keycode());
        mark_input_handled(this);
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
        queue_redraw();
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
    {
        applyClipping();
        queue_redraw();
    }
}

Node* FUIContainer::getStencil() const
{
    return _stencil;
}

void FUIContainer::setStencil(Node* stencil)
{
    if (_stencil == stencil)
        return;

    if (_stencil)
    {
        if (CanvasItem* ci = Object::cast_to<CanvasItem>(_stencil))
            ci->set_visible(true);
        _stencil->queue_free();
        _stencil = nullptr;
    }

    _stencil = stencil;

    if (_stencil)
    {
        add_child(_stencil);
        move_child(_stencil, 0);
        applyStencilEffects();
    }

    applyClipping();
    queue_redraw();
}

float FUIContainer::getAlphaThreshold() const
{
    return _alphaThreshold;
}

void FUIContainer::setAlphaThreshold(float alphaThreshold)
{
    if (_alphaThreshold != alphaThreshold)
    {
        _alphaThreshold = alphaThreshold;
        applyStencilEffects();
        queue_redraw();
    }
}

bool FUIContainer::isInverted() const
{
    return _inverted;
}

void FUIContainer::setInverted(bool inverted)
{
    if (_inverted != inverted)
    {
        _inverted = inverted;
        applyStencilEffects();
        queue_redraw();
    }
}

void FUIContainer::applyClipping()
{
    if (_stencil != nullptr)
        set_clip_children_mode(CanvasItem::CLIP_CHILDREN_AND_DRAW);
    else if (_clippingEnabled)
        set_clip_children_mode(_clipMode);
    else
        set_clip_children_mode(CanvasItem::CLIP_CHILDREN_DISABLED);
}

void FUIContainer::setClipMode(int mode)
{
    _clipMode = (CanvasItem::ClipChildrenMode)mode;
    applyClipping();
}

NS_FGUI_END
