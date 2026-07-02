#include "HitTest.h"
#include "GGraph.h"
#include "GComponent.h"
#include "GObject.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

PixelHitTestData::PixelHitTestData() :
    pixels(nullptr),
    pixelsLength(0),
    pixelWidth(0),
    scale(1)
{
}

PixelHitTestData::~PixelHitTestData()
{
    FGUI_DELETE(pixels);
}

void PixelHitTestData::load(ByteBuffer* buffer)
{
    buffer->skip(4);
    pixelWidth = buffer->readInt();
    scale = 1.0f / buffer->readByte();
    pixelsLength = buffer->readInt();
    pixels = new unsigned char[pixelsLength];
    for (size_t i = 0; i < pixelsLength; i++)
        pixels[i] = buffer->readByte();
}

PixelHitTest::PixelHitTest(PixelHitTestData * data, int offsetX, int offsetY) :
    offsetX(offsetX),
    offsetY(offsetY),
    scaleX(1),
    scaleY(1),
    _data(data)
{
}

bool PixelHitTest::hitTest(GComponent * obj, const Vector2 & localPoint)
{
    // FairyGUI pixel data is row-major from the top (Y-down). Cocos flips with
    // (height - y) because convertToNodeSpace is Y-up; Godot localPoint is already Y-down.
    int x = floor((localPoint.x / scaleX - offsetX) * _data->scale);
    int y = floor((localPoint.y / scaleY - offsetY) * _data->scale);
    if (x < 0 || y < 0 || x >= _data->pixelWidth)
        return false;

    ssize_t pos = y * _data->pixelWidth + x;
    ssize_t pos2 = pos / 8;
    ssize_t pos3 = pos % 8;

    if (pos2 >= 0 && pos2 < (ssize_t)_data->pixelsLength)
        return ((_data->pixels[pos2] >> pos3) & 0x1) > 0;
    else
        return false;
}

ChildHitArea::ChildHitArea(GObject* child) :
    _child(child)
{
}

bool ChildHitArea::hitTestCanvas(GComponent* obj, const Vector2& canvasPoint)
{
    if (_child == nullptr || obj == nullptr || !_child->displayObject())
        return false;

    CanvasItem* childDisplay = Object::cast_to<CanvasItem>(_child->displayObject());
    if (!childDisplay || !childDisplay->get_parent())
        return false;

    // Match Cocos ShapeHitTest: test in the hit-area child's display local space.
    Vector2 childLocal = childDisplay->get_global_transform_with_canvas().affine_inverse().xform(canvasPoint);

    if (GGraph* graph = dynamic_cast<GGraph*>(_child))
        return graph->hitTestShape(childLocal);

    Rect rect;
    rect.size = _child->getSize();
    return rect.has_point(childLocal);
}

bool ChildHitArea::hitTest(GComponent* obj, const Vector2& localPoint)
{
    if (_child == nullptr || obj == nullptr || !obj->displayObject())
        return false;

    CanvasItem* compDisplay = Object::cast_to<CanvasItem>(obj->displayObject());
    if (!compDisplay)
        return false;

    Vector2 canvasPoint = compDisplay->get_global_transform_with_canvas().xform(localPoint);
    return hitTestCanvas(obj, canvasPoint);
}

NS_FGUI_END
