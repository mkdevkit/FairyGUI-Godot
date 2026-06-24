#ifndef __GEARCOLOR_H__
#define __GEARCOLOR_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h
#include "GearBase.h"

NS_FGUI_BEGIN

class GObject;
class GTweener;

class GearColor : public GearBase
{
public:
    GearColor(GObject* owner);
    virtual ~GearColor();

    void apply() override;
    void updateState() override;

protected:
    void addStatus(const std::string&  pageId, ByteBuffer* buffer) override;
    void init() override;

private:
    void onTweenUpdate(GTweener* tweener);
    void onTweenComplete();

    class GearColorValue
    {
    public:
        Color color;
        Color outlineColor;

        GearColorValue();
        GearColorValue(const Color& color, const Color& outlineColor);
    };

    std::unordered_map<std::string, GearColorValue> _storage;
    GearColorValue _default;
};

NS_FGUI_END

#endif
