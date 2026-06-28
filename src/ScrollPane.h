#ifndef __SCROLLPANE_H__
#define __SCROLLPANE_H__

#include "FairyGUIMacros.h"
#include "Margin.h"
// cocos2d.h removed - see godot_types.h

NS_FGUI_BEGIN

class GObject;
class GComponent;
class GScrollBar;
class FUIContainer;
class FUIInnerContainer;
class GController;
class EventContext;
class ByteBuffer;
class GTweener;

class ScrollPane : public RefCounted
{
    GDCLASS(ScrollPane, RefCounted)
public:
    ScrollPane();
    explicit ScrollPane(GComponent* owner);
    virtual ~ScrollPane();

    void setup(ByteBuffer* buffer);

    GComponent* getOwner() const { return _owner; }
    GComponent* getHeader() const { return _header; }
    GComponent* getFooter() const { return _footer; }
    GScrollBar* getVtScrollBar() const { return _vtScrollBar; }
    GScrollBar* getHzScrollBar() const { return _hzScrollBar; }

    static void _bind_methods();

    // GDScript wrapper for getPosX/setPosX returning Variant
    float gd_getPosX() const { return getPosX(); }
    void gd_setPosX(float value, bool ani) { setPosX(value, ani); }
    float gd_getPosY() const { return getPosY(); }
    void gd_setPosY(float value, bool ani) { setPosY(value, ani); }
    void gd_scrollTop(bool ani) { scrollTop(ani); }
    void gd_scrollBottom(bool ani) { scrollBottom(ani); }
    bool gd_isBottomMost() const { return isBottomMost(); }

    bool isBouncebackEffect() const { return _bouncebackEffect; }
    void setBouncebackEffect(bool value) { _bouncebackEffect = value; }

    bool isTouchEffect() const { return _touchEffect; }
    void setTouchEffect(bool value) { _touchEffect = value; }

    bool isInertiaDisabled() const { return _inertiaDisabled; }
    void setInertiaDisabled(bool value) { _inertiaDisabled = value; }

    float getScrollStep() const { return _scrollStep; }
    void setScrollStep(float value);

    bool isSnapToItem() const { return _snapToItem; }
    void setSnapToItem(bool value) { _snapToItem = value; }

    bool isPageMode() const { return _pageMode; }
    void setPageMode(bool value) { _pageMode = value; }

    GController* getPageController() const { return _pageController; }
    void setPageController(GController* value) { _pageController = value; }

    bool isMouseWheelEnabled() const { return _mouseWheelEnabled; }
    void setMouseWheelEnabled(bool value) { _mouseWheelEnabled = value; }

    float getDecelerationRate() const { return _decelerationRate; }
    void setDecelerationRate(float value) { _decelerationRate = value; }

    float getPosX() const { return _xPos; }
    void setPosX(float value, bool ani = false);
    float getPosY() const { return _yPos; }
    void setPosY(float value, bool ani = false);

    float getPercX() const;
    void setPercX(float value, bool ani = false);
    float getPercY() const;
    void setPercY(float value, bool ani = false);

    bool isBottomMost() const;
    bool isRightMost() const;

    void scrollLeft(float ratio = 1, bool ani = false);
    void scrollRight(float ratio = 1, bool ani = false);
    void scrollUp(float ratio = 1, bool ani = false);
    void scrollDown(float ratio = 1, bool ani = false);
    void scrollTop(bool ani = false);
    void scrollBottom(bool ani = false);
    void scrollToView(GObject* obj, bool ani = false, bool setFirst = false);
    void scrollToView(const Rect2& rect, bool ani = false, bool setFirst = false);
    bool isChildInView(GObject* obj) const;

    int getPageX() const;
    void setPageX(int value, bool ani = false);
    int getPageY() const;
    void setPageY(int value, bool ani = false);

    float getScrollingPosX() const;
    float getScrollingPosY() const;

    const Vector2& getContentSize() const { return _contentSize; }
    const Vector2& getViewSize() const { return _viewSize; }

    void lockHeader(int size);
    void lockFooter(int size);

    void cancelDragging();
    static ScrollPane* getDraggingPane() { return _draggingPane; }

private:
    void onOwnerSizeChanged();
    void adjustMaskContainer();
    void setContentSize(float wv, float hv);
    void changeContentSizeOnScrolling(float deltaWidth, float deltaHeight, float deltaPosX, float deltaPosY);
    void setViewWidth(float value);
    void setViewHeight(float value);
    void setSize(float wv, float hv);
    void handleSizeChanged();

    void handleControllerChanged(GController* c);
    void updatePageController();

    GObject* hitTest(const Vector2& pt, const Camera2D* camera);

    void posChanged(bool ani);
    CALL_LATER_FUNC(ScrollPane, refresh);
    void refresh2();

    void updateScrollBarPos();
    void updateScrollBarVisible();
    void updateScrollBarVisible2(GScrollBar* bar);

    float getLoopPartSize(float division, int axis);
    bool loopCheckingCurrent();
    void loopCheckingTarget(Vector2& endPos);
    void loopCheckingTarget(Vector2& endPos, int axis);
    void loopCheckingNewPos(float& value, int axis);
    void alignPosition(Vector2& pos, bool inertialScrolling);
    float alignByPage(float pos, int axis, bool inertialScrolling);
    Vector2 updateTargetAndDuration(const Vector2& orignPos);
    float updateTargetAndDuration(float pos, int axis);
    void fixDuration(int axis, float oldChange);
    void startTween(int type);
    void killTween();
    void tweenUpdate(float dt);
    float runTween(int axis, float dt);

    void checkRefreshBar();

    void onTouchBegin(EventContext* context);
    void onTouchMove(EventContext* context);
    void onTouchEnd(EventContext* context);
    void onMouseWheel(EventContext* context);
    void onRollOver(EventContext* context);
    void onRollOut(EventContext* context);
    void onBarTweenComplete(GTweener* tweener);

    ScrollType _scrollType;
    float _scrollStep;
    float _mouseWheelStep;
    Margin _scrollBarMargin;
    bool _bouncebackEffect;
    bool _touchEffect;
    bool _scrollBarDisplayAuto;
    bool _vScrollNone;
    bool _hScrollNone;
    bool _needRefresh;
    int _refreshBarAxis;
    bool _displayOnLeft;
    bool _snapToItem;
    bool _displayInDemand;
    bool _mouseWheelEnabled;
    bool _inertiaDisabled;
    float _decelerationRate;
    bool _pageMode;
    bool _floating;
    bool _dontClipMargin;

    float _xPos;
    float _yPos;

    Vector2 _viewSize;
    Vector2 _contentSize;
    Vector2 _overlapSize;
    Vector2 _pageSize;

    Vector2 _containerPos;
    Vector2 _beginTouchPos;
    Vector2 _lastTouchPos;
    Vector2 _lastTouchGlobalPos;
    Vector2 _velocity;
    float _velocityScale;
    clock_t _lastMoveTime;
    bool _dragged;
    bool _isHoldAreaDone;
    int _aniFlag;
    int _loop;
    bool _hover;

    int _headerLockedSize;
    int _footerLockedSize;

    int _tweening;
    Vector2 _tweenStart;
    Vector2 _tweenChange;
    Vector2 _tweenTime;
    Vector2 _tweenDuration;

    GComponent* _owner;
    FUIContainer* _maskContainer;
    FUIInnerContainer* _container;
    GScrollBar* _hzScrollBar;
    GScrollBar* _vtScrollBar;
    GComponent* _header;
    GComponent* _footer;
    GController* _pageController;

    static int _gestureFlag;
    static ScrollPane* _draggingPane;

    friend class GComponent;
    friend class GList;
    friend class GScrollBar;
};

NS_FGUI_END

#endif
