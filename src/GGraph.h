#ifndef __GGRAPH_H__
#define __GGRAPH_H__

#include "FairyGUIMacros.h"
#include "GObject.h"
// cocos2d.h removed - see godot_types.h

NS_FGUI_BEGIN

class DrawNode;

class GGraph : public GObject
{
    GDCLASS(GGraph, GObject)

public:
    GGraph();
    virtual ~GGraph();

    FAIRYGUI_CREATE(GGraph)

    static void _bind_methods();

    void drawRect(float aWidth, float aHeight, int lineSize, const Color& lineColor, const Color& fillColor);
    void drawEllipse(float aWidth, float aHeight, int lineSize, const Color& lineColor, const Color& fillColor);
    void drawPolygon(int lineSize, const Color& lineColor, const Color& fillColor, const Vector2* points, int count);
    void drawRegularPolygon(int lineSize, const Color& lineColor, const Color& fillColor, int sides, float startAngle = 0, const float* distances = nullptr, int distanceCount = 0);
    bool isEmpty() const { return _type == 0; }

    Color getColor() const;
    void setColor(const Color& value);

    // Shape hit-area helper: geometry only, ignores touch/visibility (invisible hit graph child).
    bool hitTestShape(const Vector2& localPoint) const;

    virtual Variant getProp(ObjectPropID propId) override;
    virtual void setProp(ObjectPropID propId, const Variant& value) override;

    virtual GObject* hitTest(const Vector2& worldPoint, const Camera2D* camera) override;

protected:
    virtual void handleInit() override;
    virtual void setup_beforeAdd(ByteBuffer* buffer, int beginPos) override;
    virtual void handleSizeChanged() override;

private:
    void updateShape();

    int _type;
    Color _lineColor;
    Color _fillColor;
    int _lineSize;
    float* _cornerRadius;
    std::vector<Vector2>* _polygonPoints;
    float _polygonBaseWidth;
    float _polygonPointOffset;
    int _sides;
    float _startAngle;
    std::vector<float>* _distances;

    DrawNode* _shape;
};

NS_FGUI_END

#endif
