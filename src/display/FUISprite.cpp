#include "FUISprite.h"
#include <cfloat>
#include "servers/rendering_server.h"

NS_FGUI_BEGIN

Ref<Texture2D> FUISprite::_empty;

// Const for radial fill boundary coords lookup
static const char kProgressTextureCoords = 0x4b; // {0,1} {0,0} {1,0} {1,1}
static const int kProgressTextureCoordsCount = 4;

FUISprite::FUISprite() :
    _fillMethod(FillMethod::None),
    _fillOrigin(FillOrigin::Left),
    _fillAmount(0),
    _fillClockwise(false),
    _scaleByTile(false),
    _grayed(false),
    _rotated(false),
    _scale9Enabled(false)
{
    set_centered(false); // FairyGUI uses top-left origin, NOT center origin
    item_rect_changed(); // enable NOTIFICATION_DRAW for Node2D
}

FUISprite::~FUISprite()
{
}

void FUISprite::_bind_methods()
{
    // Enum bindings
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "None", static_cast<GDExtensionInt>(FillMethod::None));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Horizontal", static_cast<GDExtensionInt>(FillMethod::Horizontal));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Vertical", static_cast<GDExtensionInt>(FillMethod::Vertical));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Radial90", static_cast<GDExtensionInt>(FillMethod::Radial90));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Radial180", static_cast<GDExtensionInt>(FillMethod::Radial180));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Radial360", static_cast<GDExtensionInt>(FillMethod::Radial360));

    ClassDB::bind_integer_constant(get_class_static(), "FillOrigin", "Top", static_cast<GDExtensionInt>(FillOrigin::Top));
    ClassDB::bind_integer_constant(get_class_static(), "FillOrigin", "Bottom", static_cast<GDExtensionInt>(FillOrigin::Bottom));
    ClassDB::bind_integer_constant(get_class_static(), "FillOrigin", "Left", static_cast<GDExtensionInt>(FillOrigin::Left));
    ClassDB::bind_integer_constant(get_class_static(), "FillOrigin", "Right", static_cast<GDExtensionInt>(FillOrigin::Right));

    ClassDB::bind_method(D_METHOD("clearContent"), &FUISprite::clearContent);
    ClassDB::bind_method(D_METHOD("setScale9Grid", "rect"), &FUISprite::setScale9Grid);

    ClassDB::bind_method(D_METHOD("setScaleByTile", "value"), &FUISprite::setScaleByTile);
    ClassDB::bind_method(D_METHOD("isScaleByTile"), &FUISprite::isScaleByTile);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "scaleByTile"), "setScaleByTile", "isScaleByTile");

    ClassDB::bind_method(D_METHOD("setGrayed", "value"), &FUISprite::setGrayed);
    ClassDB::bind_method(D_METHOD("isGrayed"), &FUISprite::isGrayed);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "grayed"), "setGrayed", "isGrayed");

    // Wrapped Sprite2D properties
    ClassDB::bind_method(D_METHOD("setFlippedH", "value"), &FUISprite::setFlippedH);
    ClassDB::bind_method(D_METHOD("isFlippedH"), &FUISprite::isFlippedH);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flippedH"), "setFlippedH", "isFlippedH");

    ClassDB::bind_method(D_METHOD("setFlippedV", "value"), &FUISprite::setFlippedV);
    ClassDB::bind_method(D_METHOD("isFlippedV"), &FUISprite::isFlippedV);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flippedV"), "setFlippedV", "isFlippedV");

    ClassDB::bind_method(D_METHOD("setColor", "color"), &FUISprite::setColor);
    ClassDB::bind_method(D_METHOD("getColor"), &FUISprite::getColor);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "setColor", "getColor");

    ClassDB::bind_method(D_METHOD("setRegion", "rect"), &FUISprite::setRegion);
    ClassDB::bind_method(D_METHOD("getRegion"), &FUISprite::getRegion);
    ADD_PROPERTY(PropertyInfo(Variant::RECT2, "region"), "setRegion", "getRegion");

    ClassDB::bind_method(D_METHOD("setRegionEnabled", "value"), &FUISprite::setRegionEnabled);
    ClassDB::bind_method(D_METHOD("isRegionEnabled"), &FUISprite::isRegionEnabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "regionEnabled"), "setRegionEnabled", "isRegionEnabled");

    ClassDB::bind_method(D_METHOD("setTexture", "texture"), &FUISprite::setTexture);
    ClassDB::bind_method(D_METHOD("getTexture"), &FUISprite::getTexture);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "setTexture", "getTexture");

    // Fill properties
    ClassDB::bind_method(D_METHOD("setFillMethod", "method"), &FUISprite::gd_setFillMethod);
    ClassDB::bind_method(D_METHOD("getFillMethod"), &FUISprite::gd_getFillMethod);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fillMethod"), "setFillMethod", "getFillMethod");

    ClassDB::bind_method(D_METHOD("setFillOrigin", "origin"), &FUISprite::gd_setFillOrigin);
    ClassDB::bind_method(D_METHOD("getFillOrigin"), &FUISprite::gd_getFillOrigin);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fillOrigin"), "setFillOrigin", "getFillOrigin");

    ClassDB::bind_method(D_METHOD("setFillClockwise", "value"), &FUISprite::setFillClockwise);
    ClassDB::bind_method(D_METHOD("isFillClockwise"), &FUISprite::isFillClockwise);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fillClockwise"), "setFillClockwise", "isFillClockwise");

    ClassDB::bind_method(D_METHOD("setFillAmount", "amount"), &FUISprite::setFillAmount);
    ClassDB::bind_method(D_METHOD("getFillAmount"), &FUISprite::getFillAmount);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fillAmount", PROPERTY_HINT_RANGE, "0,1,0.01"), "setFillAmount", "getFillAmount");
}

FUISprite* FUISprite::create()
{
    FUISprite* ret = memnew(FUISprite);
    return ret;
}

void FUISprite::_notification(int p_what)
{
    if (p_what == NOTIFICATION_DRAW) {
        _draw();
        return;
    }
    if (p_what == NOTIFICATION_ENTER_TREE)
        queue_redraw();
    else if (p_what == NOTIFICATION_ENTER_CANVAS)
    {
        if (_fillMethod != FillMethod::None)
            setupFill();
    }
    Sprite2D::_notification(p_what);
}

void FUISprite::clearContent()
{
    _realTexture.unref();
    set_texture(nullptr);
    _scale9Enabled = false;
    _empty = get_texture();
    queue_redraw();
}

void FUISprite::setScale9Grid(const Rect2& value)
{
    if (value.size.x == 0 || value.size.y == 0)
    {
        _scale9Enabled = false;
        return;
    }

    _scale9Enabled = true;
    // GODOT_TODO: convert Scale9Grid insets from untrimmed space to trimmed space
    // (cocos does this with _originalContentSize / _rect / _unflippedOffsetPositionFromCenter)
    _scale9Grid = value;

    queue_redraw();
}

void FUISprite::setScaleByTile(bool value)
{
    _scaleByTile = value;
    if (_scaleByTile)
        set_texture_repeat(CanvasItem::TEXTURE_REPEAT_ENABLED);
    else
        set_texture_repeat(CanvasItem::TEXTURE_REPEAT_DISABLED);
    queue_redraw();
}

void FUISprite::setGrayed(bool value)
{
    _grayed = value;
    if (_grayed)
        set_modulate(Color(0.3f, 0.59f, 0.11f, get_modulate().a));
    else
        set_modulate(Color(1, 1, 1, get_modulate().a));
}

void FUISprite::setFillMethod(FillMethod value)
{
    if (_fillMethod != value)
    {
        _fillMethod = value;
        if (_fillMethod != FillMethod::None)
            setupFill();
        else
        {
            _fillVertices.clear();
            _fillTexCoords.clear();
            _fillIndices.clear();
        }
        queue_redraw();
    }
}

void FUISprite::setFillOrigin(FillOrigin value)
{
    if (_fillOrigin != value)
    {
        _fillOrigin = value;
        if (_fillMethod != FillMethod::None)
            setupFill();
        queue_redraw();
    }
}

void FUISprite::setFillClockwise(bool value)
{
    if (_fillClockwise != value)
    {
        _fillClockwise = value;
        if (_fillMethod != FillMethod::None)
            setupFill();
        queue_redraw();
    }
}

void FUISprite::setFillAmount(float value)
{
    if (_fillAmount != value)
    {
        _fillAmount = value;
        if (_fillMethod != FillMethod::None)
            setupFill();
        queue_redraw();
    }
}

void FUISprite::setTexture(const Ref<Texture2D>& t)
{
    _realTexture = t;
    // Keep Sprite2D texture null to block auto-rendering via RenderingServer
    Sprite2D::set_texture(nullptr);
}

void FUISprite::set_content_size(const Vector2& size)
{
    _contentSize = size;
    if (_scaleByTile)
    {
        set_region_rect(Rect2(Vector2(), size));
    }
}

void FUISprite::setupFill()
{
    if (_fillMethod == FillMethod::Horizontal || _fillMethod == FillMethod::Vertical)
        drawFillHorizontal(); // both bar types handled
    else
        drawFillRadial();
}

// ===== Fill: Horizontal / Vertical progress bar =====

void FUISprite::drawFillHorizontal()
{
    Vector2 minT, maxT;
    bool isHorizontal = (_fillMethod == FillMethod::Horizontal);

    if (isHorizontal)
    {
        if (_fillOrigin == FillOrigin::Left || _fillOrigin == FillOrigin::Top)
        {
            minT = Vector2(0, 0);
            maxT = Vector2(_fillAmount, 1);
        }
        else
        {
            minT = Vector2(1 - _fillAmount, 0);
            maxT = Vector2(1, 1);
        }
    }
    else
    {
        if (_fillOrigin == FillOrigin::Left || _fillOrigin == FillOrigin::Top)
        {
            minT = Vector2(0, 1 - _fillAmount);
            maxT = Vector2(1, 1);
        }
        else
        {
            minT = Vector2(0, 0);
            maxT = Vector2(1, _fillAmount);
        }
    }

    _fillVertices.resize(4);
    _fillTexCoords.resize(4);
    _fillIndices = { 0, 1, 2, 2, 1, 3 };

    // Top-Left
    _fillTexCoords[0] = Vector2(minT.x, maxT.y);
    _fillVertices[0] = Vector2(minT.x, -(minT.y)) * get_rect().size; // Godot Y is inverted

    // Bottom-Left
    _fillTexCoords[1] = Vector2(minT.x, minT.y);
    _fillVertices[1] = Vector2(minT.x, -(maxT.y)) * get_rect().size;

    // Top-Right
    _fillTexCoords[2] = Vector2(maxT.x, maxT.y);
    _fillVertices[2] = Vector2(maxT.x, -(minT.y)) * get_rect().size;

    // Bottom-Right
    _fillTexCoords[3] = Vector2(maxT.x, minT.y);
    _fillVertices[3] = Vector2(maxT.x, -(maxT.y)) * get_rect().size;
}

void FUISprite::drawFillVertical()
{
    // Same as horizontal, handled by drawFillHorizontal() above
    drawFillHorizontal();
}

// ===== Fill: Radial progress (circle/pie) =====

void FUISprite::drawFillRadial()
{
    float angle = 2.0f * Math::PI * (_fillClockwise ? (1.0f - _fillAmount) : _fillAmount);

    Vector2 midpoint(0.5f, 0.5f);
    Vector2 topMid(0.5f, 0.0f);
    // Rotate topMid around midpoint by angle
    float dx = topMid.x - midpoint.x;
    float dy = topMid.y - midpoint.y;
    float cosA = cos(angle);
    float sinA = sin(angle);
    Vector2 percentagePt(midpoint.x + dx * cosA - dy * sinA,
                         midpoint.y + dx * sinA + dy * cosA);

    int index = 0;
    Vector2 hit;

    if (_fillAmount == 0.0f)
    {
        hit = topMid;
        index = 0;
    }
    else if (_fillAmount >= 1.0f)
    {
        hit = topMid;
        index = 4;
    }
    else
    {
        float min_t = FLT_MAX;
        for (int i = 0; i <= kProgressTextureCoordsCount; ++i)
        {
            int pIndex = (i + (kProgressTextureCoordsCount - 1)) % kProgressTextureCoordsCount;
            Vector2 edgePtA = boundaryTexCoord(i % kProgressTextureCoordsCount);
            Vector2 edgePtB = boundaryTexCoord(pIndex);

            if (i == 0)
                edgePtB = edgePtA.lerp(edgePtB, 1.0f - midpoint.x);
            else if (i == 4)
                edgePtA = edgePtA.lerp(edgePtB, 1.0f - midpoint.x);

            // Line intersection: midpoint->percentagePt vs edgePtA->edgePtB
            float s = 0, t = 0;
            // Solve: midpoint + t*(percentagePt-midpoint) = edgePtA + s*(edgePtB-edgePtA)
            Vector2 pv = percentagePt - midpoint;
            Vector2 ev = edgePtB - edgePtA;
            float cross = pv.x * ev.y - pv.y * ev.x;
            if (fabsf(cross) < 0.00001f) continue;

            float t_val = ((edgePtA.x - midpoint.x) * ev.y - (edgePtA.y - midpoint.y) * ev.x) / cross;
            float s_val = ((edgePtA.x - midpoint.x) * pv.y - (edgePtA.y - midpoint.y) * pv.x) / cross;

            if ((i == 0 || i == 4) && !(0.0f <= s_val && s_val <= 1.0f))
                continue;

            if (t_val >= 0.0f && t_val < min_t)
            {
                min_t = t_val;
                index = i;
            }
        }
        hit = midpoint + (percentagePt - midpoint) * min_t;
    }

    // Build vertex data
    int vertCount = index + 3;
    int triCount = vertCount - 2;

    _fillVertices.resize(vertCount);
    _fillTexCoords.resize(vertCount);
    _fillIndices.resize(triCount * 3);

    _fillVertices[0] = midpoint;
    _fillTexCoords[0] = midpoint;
    _fillVertices[1] = topMid;
    _fillTexCoords[1] = topMid;

    for (int i = 0; i < index; i++)
    {
        Vector2 pt = boundaryTexCoord(i);
        _fillVertices[i + 2] = pt;
        _fillTexCoords[i + 2] = pt;
    }

    _fillVertices[vertCount - 1] = hit;
    _fillTexCoords[vertCount - 1] = hit;

    for (int i = 0; i < triCount; i++)
    {
        _fillIndices[i * 3] = 0;
        _fillIndices[i * 3 + 1] = i + 1;
        _fillIndices[i * 3 + 2] = i + 2;
    }
}

Vector2 FUISprite::boundaryTexCoord(char index) const
{
    if (index < kProgressTextureCoordsCount)
    {
        if (!_fillClockwise)
        {
            return Vector2(
                (kProgressTextureCoords >> (7 - (index << 1))) & 1 ? 1.0f : 0.0f,
                (kProgressTextureCoords >> (7 - ((index << 1) + 1))) & 1 ? 1.0f : 0.0f);
        }
        else
        {
            return Vector2(
                (kProgressTextureCoords >> ((index << 1) + 1)) & 1 ? 1.0f : 0.0f,
                (kProgressTextureCoords >> (index << 1)) & 1 ? 1.0f : 0.0f);
        }
    }
    return Vector2();
}

// ===== Main draw =====

void FUISprite::_draw()
{
    if (_realTexture.is_null() || _realTexture == _empty)
        return;

    Ref<Texture2D> tex = _realTexture;

    Vector2 contentSize = _contentSize.x > 0 ? _contentSize : get_rect().size;
    Rect2 texRect = get_region_rect();
    if (texRect.size.x <= 0 || texRect.size.y <= 0)
        return; // region not set yet — nothing to draw

    if (_fillMethod != FillMethod::None)
    {
        // Fill mode: draw custom triangles
        if (_fillIndices.empty()) return;

        Color color = get_modulate();
        std::vector<PackedVector2Array> polys;
        std::vector<Color> colors;

        for (size_t i = 0; i < _fillIndices.size() / 3; i++)
        {
            PackedVector2Array tri;
            tri.resize(3);
            for (int j = 0; j < 3; j++)
            {
                int idx = _fillIndices[i * 3 + j];
                // Map vertex from normalized coords to pixel coords
                tri.set(j, Vector2(
                    _fillVertices[idx].x * contentSize.x,
                    -_fillVertices[idx].y * contentSize.y));
            }
            draw_colored_polygon(tri, color);
        }
        return;
    }

    if (_scale9Enabled)
    {
        drawScale9();
        return;
    }

    // Normal sprite draw
    if (_rotated)
    {
        // Sprite is rotated 90° CW in atlas. texRect has swapped dims (H_orig, W_orig).
        // Un-rotate by applying -90° transform, drawing atlas rect centered at origin.
        Vector2 atlasSize = texRect.size; // (H, W) for rotated sprite
        Vector2 displaySize = _contentSize.x > 0 ? _contentSize : Vector2(atlasSize.y, atlasSize.x);
        Vector2 center(displaySize.x * 0.5f, displaySize.y * 0.5f);
        draw_set_transform(center, Math::deg_to_rad(-90.0f), Vector2(1, 1));
        draw_texture_rect_region(tex,
            Rect2(-atlasSize.x * 0.5f, -atlasSize.y * 0.5f, atlasSize.x, atlasSize.y),
            texRect,
            get_modulate());
        draw_set_transform(Vector2(), 0, Vector2(1, 1));
    }
    else
    {
        draw_texture_rect_region(tex,
            Rect2(0, 0, contentSize.x, contentSize.y),
            texRect,
            get_modulate());
    }
}

void FUISprite::drawScale9()
{
    if (_realTexture.is_null()) return;

    Ref<Texture2D> tex = _realTexture;

    Vector2 contentSize = _contentSize.x > 0 ? _contentSize : get_rect().size;
    Rect2 texRect = get_region_rect();
    if (texRect.size.x == 0)
        texRect.size = tex->get_size();

    float srcX = texRect.position.x;
    float srcY = texRect.position.y;

    // scale9Grid coords are always in original (unrotated) sprite space.
    // If the atlas sprite was rotated 90° CW, texRect has swapped dimensions.
    float origW = texRect.size.x;
    float origH = texRect.size.y;

    float l = _scale9Grid.position.x;
    float t = _scale9Grid.position.y;
    float r = _scale9Grid.position.x + _scale9Grid.size.x;
    float b = _scale9Grid.position.y + _scale9Grid.size.y;
    float tw = origW;
    float th = origH;
    float sw = contentSize.x;
    float sh = contentSize.y;

    float marginLeft = l;
    float marginTop = t;
    float marginRight = tw - r;
    float marginBottom = th - b;
    float midW = r - l;
    float midH = b - t;
    float destMidW = sw - marginLeft - marginRight;
    float destMidH = sh - marginTop - marginBottom;

    if (destMidW < 0) destMidW = 0;
    if (destMidH < 0) destMidH = 0;

    // 9 source rects in original (unrotated) image space
    const float origRects[9][4] = {
        //  TL           TM            TR
        { 0, 0, l, t }, { l, 0, midW, t }, { r, 0, marginRight, t },
        //  ML           MM            MR
        { 0, t, l, midH }, { l, t, midW, midH }, { r, t, marginRight, midH },
        //  BL           BM            BR
        { 0, b, l, marginBottom }, { l, b, midW, marginBottom }, { r, b, marginRight, marginBottom },
    };

    // 9 destination rects in screen space
    const float destRects[9][4] = {
        { 0, 0, marginLeft, marginTop },
        { marginLeft, 0, destMidW, marginTop },
        { marginLeft + destMidW, 0, marginRight, marginTop },
        { 0, marginTop, marginLeft, destMidH },
        { marginLeft, marginTop, destMidW, destMidH },
        { marginLeft + destMidW, marginTop, marginRight, destMidH },
        { 0, marginTop + destMidH, marginLeft, marginBottom },
        { marginLeft, marginTop + destMidH, destMidW, marginBottom },
        { marginLeft + destMidW, marginTop + destMidH, marginRight, marginBottom },
    };

    if (_rotated)
    {
        print_line(vformat("rotated scale9: contentSize=%s origW=%f origH=%f grid=[%f,%f,%f,%f] texRect=%s",
            contentSize, origW, origH,
            _scale9Grid.position.x, _scale9Grid.position.y,
            _scale9Grid.size.x, _scale9Grid.size.y,
            texRect));

        // ... keep existing rotated code
        Vector2 center(contentSize.x * 0.5f, contentSize.y * 0.5f);
        draw_set_transform(center, Math::deg_to_rad(-90.0f), Vector2(1, 1));

        for (int i = 0; i < 9; i++)
        {
            float ox = origRects[i][0], oy = origRects[i][1];
            float ow = origRects[i][2], oh = origRects[i][3];
            if (ow <= 0 || oh <= 0) continue;

            Rect2 src(srcX + oy,
                      srcY + origW - ox - ow,
                      oh, ow);

            float dx = destRects[i][0], dy = destRects[i][1];
            float dw = destRects[i][2], dh = destRects[i][3];
            if (dw <= 0 || dh <= 0) continue;

            Rect2 localDst(dy - center.y,
                           center.x - dx - dw,
                           dh, dw);
            draw_texture_rect_region(tex, localDst, src, get_modulate());
        }
        draw_set_transform(Vector2(), 0, Vector2(1, 1));
        return;
    }

    for (int i = 0; i < 9; i++)
    {
        float ox = origRects[i][0], oy = origRects[i][1];
        float ow = origRects[i][2], oh = origRects[i][3];
        if (ow <= 0 || oh <= 0) continue;

        Rect2 src(srcX + ox, srcY + oy, ow, oh);

        Rect2 dst(destRects[i][0], destRects[i][1],
                  destRects[i][2], destRects[i][3]);
        if (dst.size.x <= 0 || dst.size.y <= 0) continue;

        draw_texture_rect_region(tex, dst, src, get_modulate());
    }
}

NS_FGUI_END
