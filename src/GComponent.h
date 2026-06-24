#ifndef __GCOMPONENT_H__
#define __GCOMPONENT_H__

#include "FairyGUIMacros.h"
#include "GObject.h"
#include "Margin.h"
#include "ScrollPane.h"
#include "Transition.h"
// cocos2d.h removed - see godot_types.h
#include "display/FUIContainer.h"
#include "event/HitTest.h"

NS_FGUI_BEGIN

class GComponent : public GObject
{
    GDCLASS(GComponent, GObject)

public:
    GComponent();
    virtual ~GComponent();

    FAIRYGUI_CREATE(GComponent)

    static void _bind_methods();

    GObject* addChild(GObject* child);
    virtual GObject* addChildAt(GObject* child, int index);

    void gd_addChild(GObject* child);
    void gd_addChildAt(GObject* child, int index);
    void gd_removeChild(GObject* child);
    void gd_removeChildren(int beginIndex, int endIndex);

    void removeChild(GObject* child);
    virtual void removeChildAt(int index);
    void gd_removeChildAt(int index);
    void removeChildren() { removeChildren(0, -1); }
    void removeChildren(int beginIndex, int endIndex);

    GObject* getChildAt(int index) const;
    GObject* getChild(const std::string& name) const;
    GObject* getChildByPath(const std::string& path) const;
    GObject* getChildInGroup(const GGroup* group, const std::string& name) const;
    GObject* getChildById(const std::string& id) const;
    const std::vector<GObject*>& getChildren() const { return _children; }

    GObject* gd_getChild(const String& name) const;
    GObject* gd_getChildByPath(const String& path) const;
    GObject* gd_getChildById(const String& id) const;

    int getChildIndex(const GObject* child) const;
    void setChildIndex(GObject* child, int index);
    int setChildIndexBefore(GObject* child, int index);
    void swapChildren(GObject* child1, GObject* child2);
    void swapChildrenAt(int index1, int index2);

    int numChildren() const;
    bool isAncestorOf(const GObject* obj) const;

    virtual bool isChildInView(GObject* child);
    virtual int getFirstChildInView();

    void addController(GController* c);
    GController* getControllerAt(int index) const;
    GController* getController(const std::string& name) const;

    GController* gd_getController(const String& name) const;

    void removeController(GController* c);
    void gd_removeController(GController* c) { removeController(c); }

    const std::vector<GController*>& getControllers() const { return _controllers; }

    void applyController(GController* c);
    void applyAllControllers();

    Transition* getTransition(const std::string& name) const;
    Transition* getTransitionAt(int index) const;

    Transition* gd_getTransition(const String& name) const;
    Transition* gd_getTransitionAt(int index) const { return getTransitionAt(index); }

    const std::vector<Transition*>& getTransitions() const { return _transitions; }

    bool getOpaque() const { return _opaque; }
    void setOpaque(bool value);

    const Margin& getMargin() { return _margin; }
    void setMargin(const Margin& value);

    ChildrenRenderOrder getChildrenRenderOrder() const { return _childrenRenderOrder; }
    void setChildrenRenderOrder(ChildrenRenderOrder value);
    int getApexIndex() const { return _apexIndex; }
    void setApexIndex(int value);

    Node* getMask() const;
    void setMask(Node* value, bool inverted = false);

    IHitTest* getHitArea() const { return _hitArea; }
    void setHitArea(IHitTest* value);

    ScrollPane* getScrollPane() const { return _scrollPane; }

    float getViewWidth() const;
    void setViewWidth(float value);
    float getViewHeight() const;
    void setViewHeight(float value);

    const std::string& getBaseUserData() const;

    void setBoundsChangedFlag();
    void ensureBoundsCorrect();

    virtual GObject* hitTest(const Vector2& worldPoint, const Camera2D* camera) override;
    virtual Vector2 getSnappingPosition(const Vector2& pt);

    //internal use
    void childSortingOrderChanged(GObject* child, int oldValue, int newValue);
    void childStateChanged(GObject* child);
    void adjustRadioGroupDepth(GObject* obj, GController* c);

    virtual void constructFromResource() override;
    void constructFromResource(std::vector<GObject*>* objectPool, int poolIndex);

    bool _buildingDisplayList;

protected:
    virtual void constructExtension(ByteBuffer* buffer);
    virtual void onConstruct();
    virtual void setup_afterAdd(ByteBuffer* buffer, int beginPos) override;
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;
    virtual void handleGrayedChanged() override;
    virtual void handleControllerChanged(GController* c) override;

    virtual void _enter_tree() override;
    virtual void _exit_tree() override;

    virtual void updateBounds();
    void setBounds(float ax, float ay, float aw, float ah);

    void setupOverflow(OverflowType overflow);
    void setupScroll(ByteBuffer* buffer);

    std::vector<GObject*> _children;
    std::vector<GController*> _controllers;
    std::vector<Transition*> _transitions;
    FUIInnerContainer* _container;
    ScrollPane* _scrollPane;
    Margin _margin;
    Vector2 _alignOffset;
    ChildrenRenderOrder _childrenRenderOrder;
    int _apexIndex;
    bool _boundsChanged;
    bool _trackBounds;
    GObject* _maskOwner;
    IHitTest* _hitArea;

private:
    int getInsertPosForSortingChild(GObject* target);
    int moveChild(GObject* child, int oldIndex, int index);

    CALL_LATER_FUNC(GComponent, doUpdateBounds);
    CALL_LATER_FUNC(GComponent, buildNativeDisplayList);

    bool _opaque;
    int _sortingChildCount;
    GController* _applyingController;

    friend class ScrollPane;
};

NS_FGUI_END

#endif
