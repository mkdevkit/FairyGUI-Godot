#ifndef __FUISPRITE_H__
#define __FUISPRITE_H__

#include "FairyGUIMacros.h"

NS_FGUI_BEGIN

class FUISprite : public Sprite2D
{
    GDCLASS(FUISprite, Sprite2D)

public:
    FUISprite();
    virtual ~FUISprite();

    static FUISprite* create();

    void clearContent();
    void setScale9Grid(const Rect2& value);
    void setScaleByTile(bool value);
    bool isScaleByTile() const { return _scaleByTile; }
    void setGrayed(bool value);
    bool isGrayed() const { return _grayed; }

    // Wrapper methods for Godot Sprite2D API
    void setFlippedH(bool v) { set_flip_h(v); }
    bool isFlippedH() const { return is_flipped_h(); }
    void setFlippedV(bool v) { set_flip_v(v); }
    bool isFlippedV() const { return is_flipped_v(); }
    Color getColor() const { return get_modulate(); }
    void setColor(const Color& c) { set_modulate(c); }
    void setRegion(const Rect2& r) { set_region_rect(r); }
    Rect2 getRegion() const { return get_region_rect(); }
    void setRegionEnabled(bool v) { set_region_enabled(v); }
    bool isRegionEnabled() const { return is_region_enabled(); }
    void setTexture(const Ref<Texture2D>& t) { set_texture(t); }
    Ref<Texture2D> getTexture() const { return get_texture(); }

    FillMethod getFillMethod() const { return _fillMethod; }
    void setFillMethod(FillMethod value);
    int gd_getFillMethod() const { return static_cast<int>(getFillMethod()); }
    void gd_setFillMethod(int value) { setFillMethod(static_cast<FillMethod>(value)); }

    FillOrigin getFillOrigin() const { return _fillOrigin; }
    void setFillOrigin(FillOrigin value);
    int gd_getFillOrigin() const { return static_cast<int>(getFillOrigin()); }
    void gd_setFillOrigin(int value) { setFillOrigin(static_cast<FillOrigin>(value)); }

    bool isFillClockwise() const { return _fillClockwise; }
    void setFillClockwise(bool value);

    float getFillAmount() const { return _fillAmount; }
    void setFillAmount(float value);

    // Override Godot methods
    void set_content_size(const Vector2& size);

    static void _bind_methods();
    virtual void _draw() override;

protected:
    void _notification(int p_what);

private:
    void setupFill();
    void drawFillHorizontal();
    void drawFillVertical();
    void drawFillRadial();
    void drawScale9();

    Vector2 boundaryTexCoord(char index) const;

    FillMethod _fillMethod;
    FillOrigin _fillOrigin;
    float _fillAmount;
    bool _fillClockwise;
    bool _scaleByTile;
    bool _grayed;

    // 9-slice data
    bool _scale9Enabled;
    Rect2 _scale9Grid;
    Vector2 _originalContentSize;

    // Fill mode vertex data (for radial)
    std::vector<Vector2> _fillVertices;
    std::vector<Vector2> _fillTexCoords;
    std::vector<int> _fillIndices;

    static Ref<Texture2D> _empty;
};

NS_FGUI_END

#endif
