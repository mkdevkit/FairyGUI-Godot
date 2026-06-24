#ifndef __GGROUP_H__
#define __GGROUP_H__

// cocos2d.h removed - see godot_types.h
#include "FairyGUIMacros.h"
#include "GObject.h"

NS_FGUI_BEGIN

class GGroup : public GObject
{
    GDCLASS(GGroup, GObject)

public:
    GGroup();
    virtual ~GGroup();

    static GGroup* create(); // GODOT: implement create()

    static void _bind_methods();

    GroupLayoutType getLayout() const { return _layout; }
    void setLayout(GroupLayoutType value);

    void gd_setLayout(int value);
    int gd_getLayout() const;

    int getColumnGap() { return _columnGap; }
    void setColumnGap(int value);

    int getLineGap() { return _lineGap; }
    void setLineGap(int value);

    bool isExcludeInvisibles() { return _excludeInvisibles; }
    void setExcludeInvisibles(bool value);
    
    bool isAutoSizeDisabled() { return _autoSizeDisabled; }
    void setAutoSizeDisabled(bool value);

    int getMainGridIndex() { return _mainGridIndex; }
    void setMainGridIndex(int value);

    int getMainGridMinSize() { return _mainGridMinSize; }
    void setMainGridMinSize(int value);

    void setBoundsChangedFlag(bool positionChangedOnly = false);
    void moveChildren(float dx, float dy);
    void resizeChildren(float dw, float dh);

    int _updating;

protected:
    virtual void setup_beforeAdd(ByteBuffer* buffer, int beginPos) override;
    virtual void setup_afterAdd(ByteBuffer* buffer, int beginPos) override;
    virtual void handleAlphaChanged() override;
    virtual void handleVisibleChanged() override;

private:
    void updateBounds();
    void handleLayout();
    CALL_LATER_FUNC(GGroup, ensureBoundsCorrect);

    GroupLayoutType _layout;
    int _lineGap;
    int _columnGap;
    bool _excludeInvisibles;
    bool _autoSizeDisabled;
    int _mainGridIndex;
    int _mainGridMinSize;

    bool _percentReady;
    bool _boundsChanged;
    int _mainChildIndex;
    float _totalSize;
    int _numChildren;

};

NS_FGUI_END

#endif
