#ifndef __GPATH_H__
#define __GPATH_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h

NS_FGUI_BEGIN

struct GPathPoint
{
    enum class CurveType
    {
        CRSpline,
        Bezier,
        CubicBezier,
        Straight
    };

    Vector3 pos;
    Vector3 control1;
    Vector3 control2;
    CurveType curveType;

    GPathPoint(const Vector3& pos);
    GPathPoint(const Vector3& pos, const Vector3& control);
    GPathPoint(const Vector3& pos, const Vector3& control1, const Vector3& control2);
    GPathPoint(const Vector3& pos, CurveType curveType);
};

class GPath
{
public:
    GPath();
    void create(GPathPoint* points, int count);
    void clear();
    Vector3 getPointAt(float t);

    float getLength() { return _fullLength; }
    int getSegmentCount() { return (int)_segments.size(); }
    float getSegmentLength(int segmentIndex);
    void getPointsInSegment(int segmentIndex, float t0, float t1,
                            std::vector<Vector3>& points, std::vector<float>* ts = nullptr, float pointDensity = 0.1f);
    void getAllPoints(std::vector<Vector3>& points, float pointDensity = 0.1f);

    struct Segment
    {
        GPathPoint::CurveType type;
        float length;
        int ptStart;
        int ptCount;
    };

private:
    void createSplineSegment();
    Vector3 onCRSplineCurve(int ptStart, int ptCount, float t);
    Vector3 onBezierCurve(int ptStart, int ptCount, float t);

    std::vector<Segment> _segments;
    std::vector<Vector3> _points;
    float _fullLength;
};

NS_FGUI_END

#endif
