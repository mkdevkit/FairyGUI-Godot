#include "GGraph.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

// Helper: convert std::vector<Vector2> to PackedVector2Array
static PackedVector2Array to_pva(const std::vector<Vector2>& v) {
    PackedVector2Array arr;
    arr.resize((int)v.size());
    for (size_t i = 0; i < v.size(); i++) arr.set((int)i, v[i]);
    return arr;
}

// Minimal DrawNode implementation for Godot
// Stores draw commands and replays them in _draw()

struct Cmd {
    enum Type { RECT, CIRCLE, POLYGON };
    Type type;
    std::vector<Vector2> pts;
    Color fillColor;
    Color borderColor;
    float borderWidth;
    Color color; // for triangles
};

class DrawNode : public Node2D {
    GDCLASS(DrawNode, Node2D)
    public:
        DrawNode() {
            item_rect_changed(); // enable NOTIFICATION_DRAW for Node2D
        }

        static DrawNode* create() { return memnew(DrawNode); }

        void clear() {
            _cmds.clear();
            _outlinePts.clear();
            queue_redraw();
        }

        void drawTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color) {
            Cmd c;
            c.type = Cmd::RECT; // reuse RECT type for triangles
            c.pts = {p1, p2, p3};
            c.color = color;
            _cmds.push_back(c);
            queue_redraw();
        }

        void drawCircle(const Vector2& center, float radius, float, float, bool, int, float scaleY, const Color& color) {
            // Draw outline circle/ellipse
            // In Godot, draw_arc draws outline
            std::vector<Vector2> pts;
            int segments = 64;
            for (int i = 0; i <= segments; i++) {
                float angle = 2.0f * (float)M_PI * i / segments;
                pts.push_back(Vector2(center.x + radius * cos(angle), center.y + radius * sin(angle) * scaleY));
            }
            _outlinePts.push_back({pts, color, _lineWidth});
            queue_redraw();
        }

        void drawSolidCircle(const Vector2& center, float radius, float, float, int, float scaleY, const Color& color) {
            Cmd c;
            c.type = Cmd::CIRCLE;
            c.pts = {center};
            c.fillColor = color;
            c.borderWidth = radius;
            c.borderColor = Color(scaleY, scaleY, scaleY, 1); // stash scaleY in borderColor
            _cmds.push_back(c);
            queue_redraw();
        }

        void drawPolygon(const Vector2* points, int count, const Color& fillColor, float borderWidth, const Color& borderColor) {
            Cmd c;
            c.type = Cmd::POLYGON;
            for (int i = 0; i < count; i++)
                c.pts.push_back(points[i]);
            c.fillColor = fillColor;
            c.borderWidth = borderWidth;
            c.borderColor = borderColor;
            _cmds.push_back(c);
            queue_redraw();
        }

        void setLineWidth(float width) { _lineWidth = width; }

        static void _bind_methods() {}
        void _notification(int p_what) {
            if (p_what == NOTIFICATION_DRAW) {
                _draw();
                return;
            }
            Node2D::_notification(p_what);
        }
        void _draw() {
            for (auto& cmd : _cmds) {
                switch (cmd.type) {
                case Cmd::RECT:
                    if (cmd.pts.size() == 3) {
                        draw_colored_polygon(to_pva(cmd.pts), cmd.color);
                    } else {
                        if (cmd.pts.size() == 2) {
                            Rect2 r(cmd.pts[0], cmd.pts[1] - cmd.pts[0]);
                            if (cmd.borderWidth > 0) {
                                draw_rect(r, cmd.fillColor, false, cmd.borderWidth);
                            } else {
                                draw_rect(r, cmd.fillColor);
                            }
                        }
                    }
                    break;
                case Cmd::CIRCLE: {
                    Vector2 center = cmd.pts[0];
                    float scaleY = cmd.borderColor.g;
                    float radius = cmd.borderWidth;
                    std::vector<Vector2> pts;
                    int segments = 64;
                    for (int i = 0; i <= segments; i++) {
                        float angle = 2.0f * (float)M_PI * i / segments;
                        pts.push_back(Vector2(center.x + radius * cos(angle), center.y + radius * sin(angle) * scaleY));
                    }
                    draw_colored_polygon(to_pva(pts), cmd.fillColor);
                    break;
                }
                case Cmd::POLYGON:
                    draw_colored_polygon(to_pva(cmd.pts), cmd.fillColor);
                    break;
                }
            }
            for (auto& [pts, color, width] : _outlinePts) {
                for (size_t i = 1; i < pts.size(); i++) {
                    draw_line(pts[i-1], pts[i], color, width);
                }
            }
        }

    private:
        std::vector<Cmd> _cmds;
        std::vector<std::tuple<std::vector<Vector2>, Color, float>> _outlinePts;
        float _lineWidth = 1.0f;
    };

static void drawVertRect(DrawNode* shape, float x, float y, float width, float height, const Color& color)
{
    float mx = x + width;
    float my = y + height;
    shape->drawTriangle(Vector2(x, y), Vector2(mx, y), Vector2(x, my), color);
    shape->drawTriangle(Vector2(mx, y), Vector2(mx, my), Vector2(x, my), color);
}

static void appendArc(std::vector<Vector2>& pts, const Vector2& center, float radius, float startRad, float endRad, int segments)
{
    if (radius <= 0)
        return;
    for (int i = 0; i <= segments; i++)
    {
        float t = startRad + (endRad - startRad) * i / segments;
        pts.push_back(center + Vector2(Math::cos(t) * radius, Math::sin(t) * radius));
    }
}

static void drawRoundedFill(DrawNode* shape, float x, float y, float w, float h, const float* radii, const Color& color)
{
    float rtl = radii ? radii[0] : 0;
    float rtr = radii ? radii[1] : 0;
    float rbl = radii ? radii[2] : 0;
    float rbr = radii ? radii[3] : 0;
    float maxR = std::min(w, h) * 0.5f;
    rtl = std::min(rtl, maxR);
    rtr = std::min(rtr, maxR);
    rbl = std::min(rbl, maxR);
    rbr = std::min(rbr, maxR);

    const int seg = 8;
    std::vector<Vector2> pts;
    pts.reserve(64);

    appendArc(pts, Vector2(x + rtl, y + rtl), rtl, (float)M_PI, (float)M_PI * 1.5f, seg);
    appendArc(pts, Vector2(x + w - rtr, y + rtr), rtr, (float)M_PI * 1.5f, (float)M_PI * 2.0f, seg);
    appendArc(pts, Vector2(x + w - rbr, y + h - rbr), rbr, 0, (float)M_PI * 0.5f, seg);
    appendArc(pts, Vector2(x + rbl, y + h - rbl), rbl, (float)M_PI * 0.5f, (float)M_PI, seg);

    if (pts.size() >= 3)
        shape->drawPolygon(pts.data(), (int)pts.size(), color, 0, Color(0, 0, 0, 0));
}

GGraph::GGraph() : _shape(nullptr),
                   _type(0),
                   _lineSize(1),
                   _lineColor(Color(0, 0, 0, 1)),
                   _fillColor(Color(1, 1, 1, 1)),
                   _cornerRadius(nullptr),
                   _polygonPoints(nullptr),
                   _distances(nullptr)
{
    _touchDisabled = true;
}

GGraph::~GGraph()
{
    FGUI_DELETE(_cornerRadius);
    FGUI_DELETE(_polygonPoints);
    FGUI_DELETE(_distances);
}

void GGraph::handleInit()
{
    _shape = DrawNode::create();
    _displayObject = _shape;

}

void GGraph::drawRect(float aWidth, float aHeight, int lineSize, const Color& lineColor, const Color& fillColor)
{
    _type = 0; //avoid updateshape call in handleSizeChange
    setSize(aWidth, aHeight);
    _type = 1;
    _lineSize = lineSize;
    _lineColor = lineColor;
    _fillColor = fillColor;
    updateShape();
}

void GGraph::drawEllipse(float aWidth, float aHeight, int lineSize, const Color& lineColor, const Color& fillColor)
{
    _type = 0; //avoid updateshape call in handleSizeChange
    setSize(aWidth, aHeight);
    _type = 2;
    _lineSize = lineSize;
    _lineColor = lineColor;
    _fillColor = fillColor;
    updateShape();
}

void GGraph::drawPolygon(int lineSize, const Color& lineColor, const Color& fillColor, const Vector2* points, int count)
{
    _type = 3;
    _lineSize = lineSize;
    _lineColor = lineColor;
    _fillColor = fillColor;
    if (_polygonPoints == nullptr)
        _polygonPoints = new std::vector<Vector2>();
    else
        _polygonPoints->clear();
    _polygonPointOffset = getHeight();
    for (int i = 0; i < count; i++)
        _polygonPoints->push_back(points[i]);
    updateShape();
}

void GGraph::drawRegularPolygon(int lineSize, const Color& lineColor, const Color& fillColor,
                                int sides, float startAngle, const float* distances, int count)
{
    _type = 4;
    _lineSize = lineSize;
    _lineColor = lineColor;
    _fillColor = fillColor;
    _sides = sides;
    _startAngle = startAngle;
    if (distances != nullptr)
    {
        if (_distances == nullptr)
            _distances = new std::vector<float>();
        else
            _distances->clear();
        for (int i = 0; i < count; i++)
            _distances->push_back(*(distances + i));
    }
    else if (_distances != nullptr)
        _distances->clear();
    updateShape();
}

void GGraph::updateShape()
{
    _shape->clear();
    if (_type == 0)
    {
        _touchDisabled = false;
        return;
    }

    switch (_type)
    {
    case 1:
    {
        bool hasRadius = _cornerRadius != nullptr &&
                (_cornerRadius[0] > 0 || _cornerRadius[1] > 0 || _cornerRadius[2] > 0 || _cornerRadius[3] > 0);

        if (_lineSize > 0)
        {
            float wl = _size.width - _lineSize;
            float hl = _size.height - _lineSize;
            if (hasRadius)
            {
                drawRoundedFill(_shape, 0, 0, _size.width, _size.height, _cornerRadius, _lineColor);
                float innerRadii[4];
                for (int i = 0; i < 4; i++)
                    innerRadii[i] = std::max(0.0f, _cornerRadius[i] - _lineSize);
                drawRoundedFill(_shape, _lineSize, _lineSize, wl, hl, innerRadii, _fillColor);
            }
            else
            {
                drawVertRect(_shape, 0, 0, wl, _lineSize, _lineColor);
                drawVertRect(_shape, wl, 0, _lineSize, hl, _lineColor);
                drawVertRect(_shape, _lineSize, hl, wl, _lineSize, _lineColor);
                drawVertRect(_shape, 0, _lineSize, _lineSize, hl, _lineColor);
                drawVertRect(_shape, _lineSize, _lineSize, wl, hl, _fillColor);
            }
        }
        else if (hasRadius)
            drawRoundedFill(_shape, 0, 0, _size.width, _size.height, _cornerRadius, _fillColor);
        else
            drawVertRect(_shape, 0, 0, _size.width, _size.height, _fillColor);
        break;
    }
    case 2:
    {
        if (_lineSize > 0)
        {
            _shape->setLineWidth((float)_lineSize);
            _shape->drawCircle(Vector2(_size.width / 2, _size.height / 2), _size.width / 2, 0, 360, false, 1, _size.height / _size.width, _lineColor);
        }
        _shape->drawSolidCircle(Vector2(_size.width / 2, _size.height / 2), _size.width / 2, 0, 360, 1, _size.height / _size.width, _fillColor);
        break;
    }
    case 3:
    {
        _shape->drawPolygon(_polygonPoints->data(), (int)_polygonPoints->size(), _fillColor, _lineSize * 0.5f, _lineColor);
        break;
    }

    case 4:
    {
        float h = getHeight();
        _polygonPointOffset = h;
        if (_polygonPoints == nullptr)
            _polygonPoints = new std::vector<Vector2>();
        else
            _polygonPoints->clear();

        float radius = std::min(getWidth(), getHeight()) * 0.5f;
        float angle = MATH_DEG_TO_RAD(_startAngle);
        float deltaAngle = 2 * (float)M_PI / _sides;
        float dist;
        for (int i = 0; i < _sides; i++)
        {
            if (_distances != nullptr && i < (int)_distances->size())
                dist = (*_distances)[i];
            else
                dist = 1;

            float xv = radius + radius * dist * cos(angle);
            float yv = radius + radius * dist * sin(angle);
            _polygonPoints->push_back(Vector2(xv, yv));

            angle += deltaAngle;
        }

        _shape->drawPolygon(_polygonPoints->data(), (int)_polygonPoints->size(), _fillColor, _lineSize * 0.5f, _lineColor);

        break;
    }
    }
}

Color GGraph::getColor() const
{
    return _fillColor;
}

void GGraph::setColor(const Color& value)
{
    _fillColor = Color(value, _fillColor.a);
    updateShape();
}

Variant GGraph::getProp(ObjectPropID propId)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        return Variant(ToolSet::colorToInt(getColor()));
    default:
        return GObject::getProp(propId);
    }
}

void GGraph::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        setColor(ToolSet::intToColor(value.operator int()));
        break;
    default:
        GObject::setProp(propId, value);
        break;
    }
}

void GGraph::handleSizeChanged()
{
    GObject::handleSizeChanged();

    if ((_type == 3 || _type == 4) && _polygonPoints != nullptr && _polygonPointOffset > 0)
    {
        float ratio = getHeight() / _polygonPointOffset;
        if (ratio != 1.0f)
        {
            for (size_t i = 0; i < _polygonPoints->size(); i++)
                (*_polygonPoints)[i].y *= ratio;
            _polygonPointOffset = getHeight();
        }
    }

    updateShape();
}

void GGraph::setup_beforeAdd(ByteBuffer* buffer, int beginPos)
{
    GObject::setup_beforeAdd(buffer, beginPos);

    buffer->seek(beginPos, 5);

    _type = buffer->readByte();
    if (_type != 0)
    {
        _lineSize = buffer->readInt();
        _lineColor = buffer->readColor();
        _fillColor = buffer->readColor();
        if (buffer->readBool())
        {
            _cornerRadius = new float[4];
            for (int i = 0; i < 4; i++)
                _cornerRadius[i] = buffer->readFloat();
        }

        if (_type == 3)
        {
            int cnt = buffer->readShort() / 2;
            _polygonPoints = new std::vector<Vector2>(cnt);
            _polygonPointOffset = getHeight();
            for (int i = 0; i < cnt; i++)
            {
                float f1 = buffer->readFloat();
                float f2 = buffer->readFloat();
                (*_polygonPoints)[i] = Vector2(f1, f2);
            }
        }
        else if (_type == 4)
        {
            _sides = buffer->readShort();
            _startAngle = buffer->readFloat();
            int cnt = buffer->readShort();
            if (cnt > 0)
            {
                _distances = new std::vector<float>(cnt);
                for (int i = 0; i < cnt; i++)
                    (*_distances)[i] = buffer->readFloat();
            }
        }

        updateShape();
    }
}

void GGraph::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("drawRect", "width", "height", "line_size", "line_color", "fill_color"), &GGraph::drawRect);
    ClassDB::bind_method(D_METHOD("drawEllipse", "width", "height", "line_size", "line_color", "fill_color"), &GGraph::drawEllipse);
    ClassDB::bind_method(D_METHOD("isEmpty"), &GGraph::isEmpty);
    ClassDB::bind_method(D_METHOD("setColor", "color"), &GGraph::setColor);
    ClassDB::bind_method(D_METHOD("getColor"), &GGraph::getColor);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "setColor", "getColor");
}

NS_FGUI_END


