#include "ScrollPane.h"
#include "GList.h"
#include "GScrollBar.h"
#include "UIConfig.h"
#include "UIPackage.h"
#include "event/InputProcessor.h"
#include "tween/GTween.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"
#include "scene/main/viewport.h"

NS_FGUI_BEGIN

// Control-based clip container. Uses Control.clip_contents (like C# NClipContainer)
// which clips children to the Control rect without depending on _draw() commands.
// Avoids the Node2D CLIP_CHILDREN_AND_DRAW frame-delay deadloop.
class MaskContainer : public Control
{
    GDCLASS(MaskContainer, Control)
public:
    std::function<void(float)> _processCallback;

    static void _bind_methods() {}

    MaskContainer()
    {
        set_clip_contents(true);
        set_mouse_filter(MOUSE_FILTER_IGNORE);
    }

protected:
    void _notification(int p_what)
    {
        if (p_what == NOTIFICATION_PROCESS && _processCallback)
            _processCallback(get_process_delta_time());
        Control::_notification(p_what);
    }
};
ScrollPane* ScrollPane::_draggingPane = nullptr;
int ScrollPane::_gestureFlag = 0;

static const float TWEEN_TIME_GO = 0.5f;      //tween time for SetPos(ani)
static const float TWEEN_TIME_DEFAULT = 0.3f; //min tween time for inertial scroll
static const float PULL_RATIO = 0.5f;         //pull down/up ratio

static inline float sp_getField(const Vector2& pt, int axis) { return axis == 0 ? pt.x : pt.y; }
static void sp_setField(Vector2& pt, int axis, float value)
{
    if (axis == 0)
        pt.x = value;
    else
        pt.y = value;
}
static void sp_incField(Vector2& pt, int axis, float value)
{
    if (axis == 0)
        pt.x += value;
    else
        pt.y += value;
}

static inline float sp_EaseFunc(float t, float d)
{
    t = t / d - 1;
    return t * t * t + 1; //cubicOut
}

ScrollPane::ScrollPane()
    : _vtScrollBar(nullptr),
    _hzScrollBar(nullptr),
    _header(nullptr),
    _footer(nullptr),
    _pageController(nullptr),
    _needRefresh(false),
    _refreshBarAxis(0),
    _aniFlag(0),
    _loop(0),
    _headerLockedSize(0),
    _footerLockedSize(0),
    _vScrollNone(false),
    _hScrollNone(false),
    _tweening(0),
    _xPos(0),
    _yPos(0),
    _floating(false),
    _dontClipMargin(false),
    _mouseWheelEnabled(true),
    _hover(false),
    _dragged(false),
    _owner(nullptr),
    _maskContainer(nullptr),
    _container(nullptr),
    _deferredCallsCancelled(false)
{
    _scrollStep = UIConfig::defaultScrollStep;
    _mouseWheelStep = _scrollStep * 2;
    _decelerationRate = UIConfig::defaultScrollDecelerationRate;
    _touchEffect = UIConfig::defaultScrollTouchEffect;
    _bouncebackEffect = UIConfig::defaultScrollBounceEffect;
    _pageSize = Vector2(1, 1);
}

ScrollPane::ScrollPane(GComponent* owner)
    : _vtScrollBar(nullptr),
    _hzScrollBar(nullptr),
    _header(nullptr),
    _footer(nullptr),
    _pageController(nullptr),
    _needRefresh(false),
    _refreshBarAxis(0),
    _aniFlag(0),
    _loop(0),
    _headerLockedSize(0),
    _footerLockedSize(0),
    _vScrollNone(false),
    _hScrollNone(false),
    _tweening(0),
    _xPos(0),
    _yPos(0),
    _floating(false),
    _dontClipMargin(false),
    _mouseWheelEnabled(true),
    _hover(false),
    _dragged(false),
    _deferredCallsCancelled(false)
{
    _owner = owner;

    _scrollStep = UIConfig::defaultScrollStep;
    _mouseWheelStep = _scrollStep * 2;
    _decelerationRate = UIConfig::defaultScrollDecelerationRate;
    _touchEffect = UIConfig::defaultScrollTouchEffect;
    _bouncebackEffect = UIConfig::defaultScrollBounceEffect;
    _pageSize = Vector2(1, 1);

    _maskContainer = memnew(MaskContainer);
    _owner->displayObject()->add_child(_maskContainer);

    // Use the container already created in GComponent::handleInit()
    _container = _owner->_container;
    if (_container)
        _container->get_parent()->remove_child(_container);
    if (_container)
        _maskContainer->add_child(_container);

    _owner->addEventListener(UIEventType::MouseWheel, [this](EventContext* ctx) { onMouseWheel(ctx); });
    _owner->addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { onTouchBegin(ctx); });
    _owner->addEventListener(UIEventType::TouchMove, [this](EventContext* ctx) { onTouchMove(ctx); });
    _owner->addEventListener(UIEventType::TouchEnd, [this](EventContext* ctx) { onTouchEnd(ctx); });
    _owner->addEventListener(UIEventType::Exit, [this](EventContext*) {
        if (_draggingPane == this)
            _draggingPane = nullptr;
    });
}

ScrollPane::~ScrollPane()
{
    _deferredCallsCancelled = true;

    if (_draggingPane == this)
        _draggingPane = nullptr;
}

void ScrollPane::setup(ByteBuffer* buffer)
{
    _scrollType = (ScrollType)buffer->readByte();
    ScrollBarDisplayType scrollBarDisplay = (ScrollBarDisplayType)buffer->readByte();
    int flags = buffer->readInt();

    if (buffer->readBool())
    {
        _scrollBarMargin.top = buffer->readInt();
        _scrollBarMargin.bottom = buffer->readInt();
        _scrollBarMargin.left = buffer->readInt();
        _scrollBarMargin.right = buffer->readInt();
    }

    const std::string& vtScrollBarRes = buffer->readS();
    const std::string& hzScrollBarRes = buffer->readS();
    const std::string& headerRes = buffer->readS();
    const std::string& footerRes = buffer->readS();

    _displayOnLeft = (flags & 1) != 0;
    _snapToItem = (flags & 2) != 0;
    _displayInDemand = (flags & 4) != 0;
    _pageMode = (flags & 8) != 0;
    if ((flags & 16) != 0)
        _touchEffect = true;
    else if ((flags & 32) != 0)
        _touchEffect = false;
    if ((flags & 64) != 0)
        _bouncebackEffect = true;
    else if ((flags & 128) != 0)
        _bouncebackEffect = false;
    _inertiaDisabled = (flags & 256) != 0;
    _maskContainer->set_clip_contents((flags & 512) == 0);
    _floating = (flags & 1024) != 0;
    _dontClipMargin = (flags & 2048) != 0;

    if (scrollBarDisplay == ScrollBarDisplayType::DEFAULT)
    {
        if (ToolSet::isDesktopInput())
            scrollBarDisplay = UIConfig::defaultScrollBarDisplay;
        else
            scrollBarDisplay = ScrollBarDisplayType::AUTO;
    }

    if (scrollBarDisplay != ScrollBarDisplayType::HIDDEN)
    {
        if (_scrollType == ScrollType::BOTH || _scrollType == ScrollType::VERTICAL)
        {
            const std::string& res = vtScrollBarRes.size() == 0 ? UIConfig::verticalScrollBar : vtScrollBarRes;
            if (res.length() > 0)
            {
                Ref<GObject> obj = UIPackage::createObjectFromURL(res);
                _vtScrollBar = Object::cast_to<GScrollBar>(obj.ptr());
                if (_vtScrollBar == nullptr)
                    // CCLOGWARN("FairyGUI: cannot create scrollbar from %s", res.c_str());
                ;
                {
                    _vtScrollBar->setScrollPane(this, true);
                    _vtScrollBar->_alignToBL = true;
                    _owner->displayObject()->add_child(_vtScrollBar->displayObject());
                }
            }
        }
        if (_scrollType == ScrollType::BOTH || _scrollType == ScrollType::HORIZONTAL)
        {
            const std::string& res = hzScrollBarRes.length() == 0 ? UIConfig::horizontalScrollBar : hzScrollBarRes;
            if (res.length() > 0)
            {
                Ref<GObject> obj = UIPackage::createObjectFromURL(res);
                _hzScrollBar = Object::cast_to<GScrollBar>(obj.ptr());
                if (_hzScrollBar == nullptr)
                    // CCLOGWARN("FairyGUI: cannot create scrollbar from %s", res.c_str());
                ;
                {
                    _hzScrollBar->setScrollPane(this, false);
                    _hzScrollBar->_alignToBL = true;
                    _owner->displayObject()->add_child(_hzScrollBar->displayObject());
                }
            }
        }

        _scrollBarDisplayAuto = scrollBarDisplay == ScrollBarDisplayType::AUTO;
        if (_scrollBarDisplayAuto)
        {
            if (_vtScrollBar != nullptr)
                _vtScrollBar->setVisible(false);
            if (_hzScrollBar != nullptr)
                _hzScrollBar->setVisible(false);

            _owner->addEventListener(UIEventType::RollOver, [this](EventContext* ctx) { ScrollPane::onRollOver(ctx); });
            _owner->addEventListener(UIEventType::RollOut, [this](EventContext* ctx) { ScrollPane::onRollOut(ctx); });
        }
    }
    else
        _mouseWheelEnabled = false;

    if (headerRes.length() > 0)
    {
        Ref<GObject> obj = UIPackage::createObjectFromURL(headerRes);
        _header = Object::cast_to<GComponent>(obj.ptr());
        if (_header == nullptr)
            // CCLOGWARN("FairyGUI: cannot create scrollPane header from %s", headerRes.c_str());
            ;
        {
            _header->setVisible(false);
            _header->_alignToBL = true;
            _owner->displayObject()->add_child(_header->displayObject());
        }
    }

    if (footerRes.length() > 0)
    {
        Ref<GObject> obj = UIPackage::createObjectFromURL(footerRes);
        _footer = Object::cast_to<GComponent>(obj.ptr());
        if (_footer == nullptr)
            // CCLOGWARN("FairyGUI: cannot create scrollPane footer from %s", footerRes.c_str());
            ;
        {
            _footer->setVisible(false);
            _footer->_alignToBL = true;
            _owner->displayObject()->add_child(_footer->displayObject());
        }
    }

    if (_header != nullptr || _footer != nullptr)
        _refreshBarAxis = (_scrollType == ScrollType::BOTH || _scrollType == ScrollType::VERTICAL) ? 1 : 0;

    syncOverlayZOrder();
    setSize(_owner->getWidth(), _owner->getHeight());
}

void ScrollPane::syncOverlayZOrder()
{
    if (!_owner)
        return;

    Node* ownerDisplay = _owner->displayObject();
    if (!ownerDisplay)
        return;

    const int kContentZ = 0;
    const int kOverlayZ = 100;

    if (_maskContainer && _maskContainer->get_parent() == ownerDisplay)
    {
        ownerDisplay->move_child(_maskContainer, 0);
        _maskContainer->set_z_index(kContentZ);
    }

    auto raiseOverlay = [&](GObject* obj) {
        if (obj == nullptr)
            return;
        Node* node = obj->displayObject();
        if (node == nullptr || node->get_parent() != ownerDisplay)
            return;
        ownerDisplay->move_child(node, ownerDisplay->get_child_count() - 1);
        if (CanvasItem* ci = Object::cast_to<CanvasItem>(node))
            ci->set_z_index(kOverlayZ);
    };

    raiseOverlay(_header.ptr());
    raiseOverlay(_footer.ptr());
    raiseOverlay(_vtScrollBar.ptr());
    raiseOverlay(_hzScrollBar.ptr());
}

void ScrollPane::setScrollStep(float value)
{
    _scrollStep = value;
    if (_scrollStep == 0)
        _scrollStep = UIConfig::defaultScrollStep;
    _mouseWheelStep = _scrollStep * 2;
}

void ScrollPane::setPosX(float value, bool ani)
{
    _owner->ensureBoundsCorrect();

    if (_loop == 1)
        loopCheckingNewPos(value, 0);

    value = std::clamp(value, 0.0f, _overlapSize.width);
    if (value != _xPos)
    {
        _xPos = value;
        posChanged(ani);
    }
}

void ScrollPane::setPosY(float value, bool ani)
{
    _owner->ensureBoundsCorrect();

    if (_loop == 2)
        loopCheckingNewPos(value, 1);

    value = std::clamp(value, 0.0f, _overlapSize.height);
    if (value != _yPos)
    {
        _yPos = value;
        posChanged(ani);
    }
}

float ScrollPane::getPercX() const
{
    return _overlapSize.width == 0 ? 0 : _xPos / _overlapSize.width;
}

void ScrollPane::setPercX(float value, bool ani)
{
    _owner->ensureBoundsCorrect();
    setPosX(_overlapSize.width * std::clamp(value, 0.0f, 1.0f), ani);
}

float ScrollPane::getPercY() const
{
    return _overlapSize.height == 0 ? 0 : _yPos / _overlapSize.height;
}

void ScrollPane::setPercY(float value, bool ani)
{
    _owner->ensureBoundsCorrect();
    setPosY(_overlapSize.height * std::clamp(value, 0.0f, 1.0f), ani);
}

bool ScrollPane::isBottomMost() const
{
    return _yPos == _overlapSize.height || _overlapSize.height == 0;
}

bool ScrollPane::isRightMost() const
{
    return _xPos == _overlapSize.width || _overlapSize.width == 0;
}

void ScrollPane::scrollLeft(float ratio, bool ani)
{
    if (_pageMode)
        setPosX(_xPos - _pageSize.width * ratio, ani);
    else
        setPosX(_xPos - _scrollStep * ratio, ani);
}

void ScrollPane::scrollRight(float ratio, bool ani)
{
    if (_pageMode)
        setPosX(_xPos + _pageSize.width * ratio, ani);
    else
        setPosX(_xPos + _scrollStep * ratio, ani);
}

void ScrollPane::scrollUp(float ratio, bool ani)
{
    if (_pageMode)
        setPosY(_yPos - _pageSize.height * ratio, ani);
    else
        setPosY(_yPos - _scrollStep * ratio, ani);
}

void ScrollPane::scrollDown(float ratio, bool ani)
{
    if (_pageMode)
        setPosY(_yPos + _pageSize.height * ratio, ani);
    else
        setPosY(_yPos + _scrollStep * ratio, ani);
}

void ScrollPane::scrollTop(bool ani)
{
    setPercY(0, ani);
}

void ScrollPane::scrollBottom(bool ani)
{
    setPercY(1, ani);
}

void ScrollPane::scrollToView(GObject* obj, bool ani, bool setFirst)
{
    _owner->ensureBoundsCorrect();
    if (_needRefresh)
        refresh();

    Rect rect = Rect(obj->getX(), obj->getY(), obj->getWidth(), obj->getHeight());
    if (obj->getParent() != _owner)
        rect = obj->getParent()->transformRect(rect, _owner);
    scrollToView(rect, ani, setFirst);
}

void ScrollPane::scrollToView(const Rect2& rect, bool ani, bool setFirst)
{
    _owner->ensureBoundsCorrect();
    if (_needRefresh)
        refresh();

    if (_overlapSize.height > 0)
    {
        float bottom = _yPos + _viewSize.height;
        if (setFirst || rect.position.y <= _yPos || rect.size.y >= _viewSize.height)
        {
            if (_pageMode)
                setPosY(floor(rect.position.y / _pageSize.height) * _pageSize.height, ani);
            else
                setPosY(rect.position.y, ani);
        }
        else if (rect.position.y + rect.size.y > bottom)
        {
            if (_pageMode)
                setPosY(floor(rect.position.y / _pageSize.height) * _pageSize.height, ani);
            else if (rect.size.y <= _viewSize.height / 2)
                setPosY(rect.position.y + rect.size.y * 2 - _viewSize.height, ani);
            else
                setPosY(rect.position.y + rect.size.y - _viewSize.height, ani);
        }
    }
    if (_overlapSize.width > 0)
    {
        float right = _xPos + _viewSize.width;
        if (setFirst || rect.position.x <= _xPos || rect.size.x >= _viewSize.width)
        {
            if (_pageMode)
                setPosX(floor(rect.position.x / _pageSize.width) * _pageSize.width, ani);
            setPosX(rect.position.x, ani);
        }
        else if (rect.position.x + rect.size.x > right)
        {
            if (_pageMode)
                setPosX(floor(rect.position.x / _pageSize.width) * _pageSize.width, ani);
            else if (rect.size.x <= _viewSize.width / 2)
                setPosX(rect.position.x + rect.size.x * 2 - _viewSize.width, ani);
            else
                setPosX(rect.position.x + rect.size.x - _viewSize.width, ani);
        }
    }

    if (!ani && _needRefresh)
        refresh();
}

bool ScrollPane::isChildInView(GObject* obj) const
{
    if (_overlapSize.height > 0)
    {
        float dist = obj->getY() + _container->get_position().y;
        if (dist <= -obj->getHeight() || dist >= _viewSize.height)
            return false;
    }
    if (_overlapSize.width > 0)
    {
        float dist = obj->getX() + _container->get_position().x;
        if (dist <= -obj->getWidth() || dist >= _viewSize.width)
            return false;
    }

    return true;
}

int ScrollPane::getPageX() const
{
    if (!_pageMode)
        return 0;

    int page = floor(_xPos / _pageSize.width);
    if (_xPos - page * _pageSize.width > _pageSize.width * 0.5f)
        page++;

    return page;
}

void ScrollPane::setPageX(int value, bool ani)
{
    if (!_pageMode)
        return;

    _owner->ensureBoundsCorrect();

    if (_overlapSize.width > 0)
        setPosX(value * _pageSize.width, ani);
}

int ScrollPane::getPageY() const
{
    if (!_pageMode)
        return 0;

    int page = floor(_yPos / _pageSize.height);
    if (_yPos - page * _pageSize.height > _pageSize.height * 0.5f)
        page++;

    return page;
}

void ScrollPane::setPageY(int value, bool ani)
{
    if (!_pageMode)
        return;

    _owner->ensureBoundsCorrect();

    if (_overlapSize.height > 0)
        setPosY(value * _pageSize.height, ani);
}

float ScrollPane::getScrollingPosX() const
{
    return std::clamp(-_container->get_position().x, 0.0f, _overlapSize.width);
}

float ScrollPane::getScrollingPosY() const
{
    return std::clamp(-_container->get_position().y, 0.0f, _overlapSize.height);
}

void ScrollPane::setViewWidth(float value)
{
    value = value + _owner->_margin.left + _owner->_margin.right;
    if (_vtScrollBar != nullptr && !_floating)
        value += _vtScrollBar->getWidth();
    _owner->setWidth(value);
}

void ScrollPane::setViewHeight(float value)
{
    value = value + _owner->_margin.top + _owner->_margin.bottom;
    if (_hzScrollBar != nullptr && !_floating)
        value += _hzScrollBar->getHeight();
    _owner->setHeight(value);
}

void ScrollPane::lockHeader(int size)
{
    if (_headerLockedSize == size)
        return;

    Vector2 cpos = _container->get_position();

    _headerLockedSize = size;
    if (!_owner->isDispatchingEvent(UIEventType::PullDownRelease) && sp_getField(cpos, _refreshBarAxis) >= 0)
    {
        _tweenStart = cpos;
        _tweenChange = Vector2();
        sp_setField(_tweenChange, _refreshBarAxis, _headerLockedSize - sp_getField(_tweenStart, _refreshBarAxis));
        _tweenDuration = Vector2(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        startTween(2);
    }
}

void ScrollPane::lockFooter(int size)
{
    if (_footerLockedSize == size)
        return;

    Vector2 cpos = _container->get_position();

    _footerLockedSize = size;
    if (!_owner->isDispatchingEvent(UIEventType::PullUpRelease) && sp_getField(cpos, _refreshBarAxis) >= 0)
    {
        _tweenStart = cpos;
        _tweenChange = Vector2();
        float max = sp_getField(_overlapSize, _refreshBarAxis);
        if (max == 0)
            max = std::max(sp_getField(_contentSize, _refreshBarAxis) + _footerLockedSize - sp_getField(_viewSize, _refreshBarAxis), 0.0f);
        else
            max += _footerLockedSize;
        sp_setField(_tweenChange, _refreshBarAxis, -max - sp_getField(_tweenStart, _refreshBarAxis));
        _tweenDuration = Vector2(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        startTween(2);
    }
}

void ScrollPane::cancelDragging()
{
    if (_draggingPane == this)
        _draggingPane = nullptr;

    _gestureFlag = 0;
    _dragged = false;
}

void ScrollPane::handleControllerChanged(GController* c)
{
    if (_pageController == c)
    {
        if (_scrollType == ScrollType::HORIZONTAL)
            setPageX(c->getSelectedIndex(), true);
        else
            setPageY(c->getSelectedIndex(), true);
    }
}

void ScrollPane::updatePageController()
{
    if (_pageController != nullptr && !_pageController->changing)
    {
        int index;
        if (_scrollType == ScrollType::HORIZONTAL)
            index = getPageX();
        else
            index = getPageY();
        if (index < _pageController->getPageCount())
        {
            GController* c = _pageController;
            _pageController = nullptr; //avoid calling handleControllerChanged
            c->setSelectedIndex(index);
            _pageController = c;
        }
    }
}

void ScrollPane::adjustMaskContainer()
{
    float mx, my;
    if (_displayOnLeft && _vtScrollBar != nullptr && !_floating)
        mx = floor(_owner->_margin.left + _vtScrollBar->getWidth());
    else
        mx = floor(_owner->_margin.left);
    my = floor(_owner->_margin.top);
    mx += _owner->_alignOffset.x;
    my += _owner->_alignOffset.y;

    _maskContainer->set_position(Vector2(mx, my));
}

void ScrollPane::onOwnerSizeChanged()
{
    setSize(_owner->getWidth(), _owner->getHeight());
    posChanged(false);
}

void ScrollPane::setSize(float wv, float hv)
{
    if (_hzScrollBar != nullptr)
    {
        _hzScrollBar->setY(hv - _hzScrollBar->getHeight());
        if (_vtScrollBar != nullptr)
        {
            _hzScrollBar->setWidth(wv - _vtScrollBar->getWidth() - _scrollBarMargin.left - _scrollBarMargin.right);
            if (_displayOnLeft)
                _hzScrollBar->setX(_scrollBarMargin.left + _vtScrollBar->getWidth());
            else
                _hzScrollBar->setX(_scrollBarMargin.left);
        }
        else
        {
            _hzScrollBar->setWidth(wv - _scrollBarMargin.left - _scrollBarMargin.right);
            _hzScrollBar->setX(_scrollBarMargin.left);
        }
    }
    if (_vtScrollBar != nullptr)
    {
        if (!_displayOnLeft)
            _vtScrollBar->setX(wv - _vtScrollBar->getWidth());
        if (_hzScrollBar != nullptr)
            _vtScrollBar->setHeight(hv - _hzScrollBar->getHeight() - _scrollBarMargin.top - _scrollBarMargin.bottom);
        else
            _vtScrollBar->setHeight(hv - _scrollBarMargin.top - _scrollBarMargin.bottom);
        _vtScrollBar->setY(_scrollBarMargin.top);
    }

    _viewSize.width = wv;
    _viewSize.height = hv;
    if (_hzScrollBar != nullptr && !_floating)
        _viewSize.height -= _hzScrollBar->getHeight();
    if (_vtScrollBar != nullptr && !_floating)
        _viewSize.width -= _vtScrollBar->getWidth();
    _viewSize.width -= (_owner->_margin.left + _owner->_margin.right);
    _viewSize.height -= (_owner->_margin.top + _owner->_margin.bottom);

    _viewSize.width = std::max(1.0f, _viewSize.width);
    _viewSize.height = std::max(1.0f, _viewSize.height);
    _pageSize = _viewSize;

    adjustMaskContainer();
    handleSizeChanged();
}

void ScrollPane::setContentSize(float wv, float hv)
{
    if (_contentSize.width == wv && _contentSize.height == hv)
        return;

    _contentSize.width = wv;
    _contentSize.height = hv;
    handleSizeChanged();
}

void ScrollPane::changeContentSizeOnScrolling(float deltaWidth, float deltaHeight, float deltaPosX, float deltaPosY)
{
    bool isRightmost = _xPos == _overlapSize.width;
    bool isBottom = _yPos == _overlapSize.height;

    _contentSize.width += deltaWidth;
    _contentSize.height += deltaHeight;
    handleSizeChanged();

    if (_tweening == 1)
    {
        if (deltaWidth != 0 && isRightmost && _tweenChange.x < 0)
        {
            _xPos = _overlapSize.width;
            _tweenChange.x = -_xPos - _tweenStart.x;
        }

        if (deltaHeight != 0 && isBottom && _tweenChange.y < 0)
        {
            _yPos = _overlapSize.height;
            _tweenChange.y = -_yPos - _tweenStart.y;
        }
    }
    else if (_tweening == 2)
    {
        if (deltaPosX != 0)
        {
            _container->set_position(Vector2(_container->get_position().x - deltaPosX, _container->get_position().y));
            _tweenStart.x -= deltaPosX;
            _xPos = -_container->get_position().x;
        }
        if (deltaPosY != 0)
        {
            _container->set_position(Vector2(_container->get_position().x, _container->get_position().y - deltaPosY));
            _tweenStart.y -= deltaPosY;
            _yPos = -_container->get_position().y;
        }
    }
    else if (_dragged)
    {
        if (deltaPosX != 0)
        {
            _container->set_position(Vector2(_container->get_position().x - deltaPosX, _container->get_position().y));
            _containerPos.x -= deltaPosX;
            _xPos = -_container->get_position().x;
        }
        if (deltaPosY != 0)
        {
            _container->set_position(Vector2(_container->get_position().x, _container->get_position().y - deltaPosY));
            _containerPos.y -= deltaPosY;
            _yPos = -_container->get_position().y;
        }
    }
    else
    {
        if (deltaWidth != 0 && isRightmost)
        {
            _xPos = _overlapSize.width;
            _container->set_position(Vector2(-_xPos, _container->get_position().y));
        }

        if (deltaHeight != 0 && isBottom)
        {
            _yPos = _overlapSize.height;
            _container->set_position(Vector2(_container->get_position().x, -_yPos));
        }
    }

    if (_pageMode)
        updatePageController();
}

void ScrollPane::handleSizeChanged()
{
    if (_displayInDemand)
    {
        _vScrollNone = _contentSize.height <= _viewSize.height;
        _hScrollNone = _contentSize.width <= _viewSize.width;
    }

    if (_vtScrollBar != nullptr)
    {
        if (_contentSize.height == 0)
            _vtScrollBar->setDisplayPerc(0);
        else
            _vtScrollBar->setDisplayPerc(std::min(1.0f, _viewSize.height / _contentSize.height));
    }
    if (_hzScrollBar != nullptr)
    {
        if (_contentSize.width == 0)
            _hzScrollBar->setDisplayPerc(0);
        else
            _hzScrollBar->setDisplayPerc(std::min(1.0f, _viewSize.width / _contentSize.width));
    }

    updateScrollBarVisible();

    // Viewport origin in owner space (Y-down). Cocos adjustMaskContainer used a Y-up flip;
    // Godot places the mask directly at margin + alignOffset.
    float mx, my;
    if (_displayOnLeft && _vtScrollBar != nullptr && !_floating)
        mx = floor(_owner->_margin.left + _vtScrollBar->getWidth());
    else
        mx = floor(_owner->_margin.left);
    my = floor(_owner->_margin.top);
    mx += _owner->_alignOffset.x;
    my += _owner->_alignOffset.y;

    // _viewSize already has scrollbar widths and margins subtracted in setSize().
    // Re-add scrollbar size when scrollNone (not subtracted from _viewSize) and
    // margin when dontClipMargin is set.
    Rect2 maskRect(mx, my, _viewSize.width, _viewSize.height);
    if (_vScrollNone && _vtScrollBar != nullptr)
        maskRect.size.x += _vtScrollBar->getWidth();
    if (_hScrollNone && _hzScrollBar != nullptr)
        maskRect.size.y += _hzScrollBar->getHeight();
    if (_dontClipMargin)
    {
        maskRect.position.x -= _owner->_margin.left;
        maskRect.size.x += _owner->_margin.left + _owner->_margin.right;
        maskRect.position.y -= _owner->_margin.top;
        maskRect.size.y += _owner->_margin.top + _owner->_margin.bottom;
    }
    // MaskContainer is a Control with clip_contents=true.
    // The clip region is the Control's rect (position + size).
    _maskContainer->set_position(maskRect.position);
    _maskContainer->set_size(maskRect.size);

    if (_vtScrollBar.is_valid())
        _vtScrollBar->handlePositionChanged();
    if (_hzScrollBar.is_valid())
        _hzScrollBar->handlePositionChanged();
	if (_header.is_valid())
        _header->handlePositionChanged();
	if (_footer.is_valid())
        _footer->handlePositionChanged();

    if (_scrollType == ScrollType::HORIZONTAL || _scrollType == ScrollType::BOTH)
        _overlapSize.width = ceil(std::max(0.0f, _contentSize.width - _viewSize.width));
    else
        _overlapSize.width = 0;
    if (_scrollType == ScrollType::VERTICAL || _scrollType == ScrollType::BOTH)
        _overlapSize.height = ceil(std::max(0.0f, _contentSize.height - _viewSize.height));
    else
        _overlapSize.height = 0;

    _xPos = std::clamp(_xPos, 0.0f, _overlapSize.width);
    _yPos = std::clamp(_yPos, 0.0f, _overlapSize.height);
    // Only adjust container position when it exists (may not during construction)
    if (_container != nullptr)
    {
        float max = sp_getField(_overlapSize, _refreshBarAxis);
        if (max == 0)
            max = std::max(sp_getField(_contentSize, _refreshBarAxis) + _footerLockedSize - sp_getField(_viewSize, _refreshBarAxis), 0.0f);
        else
            max += _footerLockedSize;
        if (_container->get_parent() != nullptr)
        {
            Vector2 containerPos = _container->get_position();
            if (_refreshBarAxis == 0)
                _container->set_position(Vector2(
                    std::clamp(containerPos.x, -max, (float)_headerLockedSize),
                    std::clamp(containerPos.y, -_overlapSize.height, 0.0f)));
            else
                _container->set_position(Vector2(
                    std::clamp(containerPos.x, -_overlapSize.width, 0.0f),
                    std::clamp(containerPos.y, -max, (float)_headerLockedSize)));
        }
    }

    if (_header != nullptr)
    {
        if (_refreshBarAxis == 0)
            _header->setHeight(_viewSize.height);
        else
            _header->setWidth(_viewSize.width);
    }

    if (_footer != nullptr)
    {
        if (_refreshBarAxis == 0)
            _footer->setHeight(_viewSize.height);
        else
            _footer->setWidth(_viewSize.width);
    }

    updateScrollBarPos();
    if (_pageMode)
        updatePageController();

    syncOverlayZOrder();
}

GObject* ScrollPane::hitTest(const Vector2& pt, const Camera2D* camera)
{
    GObject* target = nullptr;
    if (_vtScrollBar.is_valid())
    {
        target = _vtScrollBar->hitTest(pt, camera);
        if (target)
            return target;
    }
    if (_hzScrollBar.is_valid())
    {
        target = _hzScrollBar->hitTest(pt, camera);
        if (target)
            return target;
    }
    if (_header.is_valid() && _header->displayObject()->get_parent() != nullptr)
    {
        target = _header->hitTest(pt, camera);
        if (target)
            return target;
    }
    if (_footer.is_valid() && _footer->displayObject()->get_parent() != nullptr)
    {
        target = _footer->hitTest(pt, camera);
        if (target)
            return target;
    }
    if (_maskContainer->is_clipping_contents())
    {
        if (_maskContainer->get_global_rect().has_point(pt))
            return _owner;
        else
            return nullptr;
    }
    else
        return _owner;
}

void ScrollPane::posChanged(bool ani)
{
    if (_aniFlag == 0)
        _aniFlag = ani ? 1 : -1;
    else if (_aniFlag == 1 && !ani)
        _aniFlag = -1;

    _needRefresh = true;
    _deferredCallsCancelled = false;
    call_deferred(StringName("refresh"));
}

void ScrollPane::refresh()
{
    if (_deferredCallsCancelled)
        return;
    if (_owner && _owner->isDeferredCallCancelled())
        return;
    _needRefresh = false;

    if (_pageMode || _snapToItem)
    {
        Vector2 pos(-_xPos, -_yPos);
        alignPosition(pos, false);
        _xPos = -pos.x;
        _yPos = -pos.y;
    }

    refresh2();

    _owner->dispatchEvent(UIEventType::Scroll);
    if (_needRefresh) //pos may change in onScroll
    {
        _needRefresh = false;
        // CALL_LATER_CANCEL removed

        refresh2();
    }

    updateScrollBarPos();
    _aniFlag = 0;
}

void ScrollPane::refresh2()
{
    if (_aniFlag == 1 && !_dragged)
    {
        Vector2 pos;

        if (_overlapSize.width > 0)
            pos.x = -(int)_xPos;
        else
        {
            if (_container->get_position().x != 0)
                _container->set_position(Vector2(0, _container->get_position().y));
            pos.x = 0;
        }
        if (_overlapSize.height > 0)
            pos.y = -(int)_yPos;
        else
        {
            if (_container->get_position().y != 0)
                _container->set_position(Vector2(_container->get_position().x, 0));
            pos.y = 0;
        }

        if (pos.x != _container->get_position().x || pos.y != _container->get_position().y)
        {
            _tweenDuration = Vector2(TWEEN_TIME_GO, TWEEN_TIME_GO);
            _tweenStart = _container->get_position();
            _tweenChange = pos - _tweenStart;
            startTween(1);
        }
        else if (_tweening != 0)
            killTween();
    }
    else
    {
        if (_tweening != 0)
            killTween();

        _container->set_position(Vector2((int)-_xPos, (int)-_yPos));

        loopCheckingCurrent();
    }

    if (_pageMode)
        updatePageController();
}

void ScrollPane::updateScrollBarPos()
{
    if (_vtScrollBar != nullptr)
        _vtScrollBar->setScrollPerc(_overlapSize.height == 0 ? 0 : std::clamp(-_container->get_position().y, 0.0f, _overlapSize.height) / _overlapSize.height);

    if (_hzScrollBar != nullptr)
        _hzScrollBar->setScrollPerc(_overlapSize.width == 0 ? 0 : std::clamp(-_container->get_position().x, 0.0f, _overlapSize.width) / _overlapSize.width);

    checkRefreshBar();
}

void ScrollPane::updateScrollBarVisible()
{
    if (_vtScrollBar != nullptr)
    {
        if (_viewSize.height <= _vtScrollBar->getMinSize() || _vScrollNone)
            _vtScrollBar->setVisible(false);
        else
            updateScrollBarVisible2(_vtScrollBar.ptr());
    }

    if (_hzScrollBar != nullptr)
    {
        if (_viewSize.width <= _hzScrollBar->getMinSize() || _hScrollNone)
            _hzScrollBar->setVisible(false);
        else
            updateScrollBarVisible2(_hzScrollBar.ptr());
    }
}

void ScrollPane::updateScrollBarVisible2(GScrollBar* bar)
{
    if (_scrollBarDisplayAuto)
        GTween::kill(bar, TweenPropType::Alpha, false);

    if (_scrollBarDisplayAuto && !_hover && _tweening == 0 && !_dragged && !bar->_gripDragging)
    {
        if (((CanvasItem*)bar->displayObject())->is_visible())
            GTween::to(1, 0, 0.5f)
            ->setDelay(0.5f)
            ->onComplete1([this](GTweener* tweener) { ScrollPane::onBarTweenComplete(tweener); })
            ->setTarget(bar, TweenPropType::Alpha);
    }
    else
    {
        bar->setAlpha(1);
        bar->setVisible(true);
    }
}

void ScrollPane::onBarTweenComplete(GTweener* tweener)
{
    GObject* bar = (GObject*)tweener->getTarget();
    bar->setAlpha(1);
    bar->setVisible(false);
}

float ScrollPane::getLoopPartSize(float division, int axis)
{
    return (sp_getField(_contentSize, axis) + (axis == 0 ? ((GList*)_owner)->getColumnGap() : ((GList*)_owner)->getLineGap())) / division;
}

bool ScrollPane::loopCheckingCurrent()
{
    bool changed = false;
    if (_loop == 1 && _overlapSize.width > 0)
    {
        if (_xPos < 0.001f)
        {
            _xPos += getLoopPartSize(2, 0);
            changed = true;
        }
        else if (_xPos >= _overlapSize.width)
        {
            _xPos -= getLoopPartSize(2, 0);
            changed = true;
        }
    }
    else if (_loop == 2 && _overlapSize.height > 0)
    {
        if (_yPos < 0.001f)
        {
            _yPos += getLoopPartSize(2, 1);
            changed = true;
        }
        else if (_yPos >= _overlapSize.height)
        {
            _yPos -= getLoopPartSize(2, 1);
            changed = true;
        }
    }

    if (changed)
        _container->set_position(Vector2((int)-_xPos, (int)-_yPos));

    return changed;
}

void ScrollPane::loopCheckingTarget(Vector2& endPos)
{
    if (_loop == 1)
        loopCheckingTarget(endPos, 0);

    if (_loop == 2)
        loopCheckingTarget(endPos, 1);
}

void ScrollPane::loopCheckingTarget(Vector2& endPos, int axis)
{
    if (sp_getField(endPos, axis) > 0)
    {
        float halfSize = getLoopPartSize(2, axis);
        float tmp = sp_getField(_tweenStart, axis) - halfSize;
        if (tmp <= 0 && tmp >= -sp_getField(_overlapSize, axis))
        {
            sp_incField(endPos, axis, -halfSize);
            sp_setField(_tweenStart, axis, tmp);
        }
    }
    else if (sp_getField(endPos, axis) < -sp_getField(_overlapSize, axis))
    {
        float halfSize = getLoopPartSize(2, axis);
        float tmp = sp_getField(_tweenStart, axis) + halfSize;
        if (tmp <= 0 && tmp >= -sp_getField(_overlapSize, axis))
        {
            sp_incField(endPos, axis, halfSize);
            sp_setField(_tweenStart, axis, tmp);
        }
    }
}

void ScrollPane::loopCheckingNewPos(float& value, int axis)
{
    float overlapSize = sp_getField(_overlapSize, axis);
    if (overlapSize == 0)
        return;

    float pos = axis == 0 ? _xPos : _yPos;
    bool changed = false;
    if (value < 0.001f)
    {
        value += getLoopPartSize(2, axis);
        if (value > pos)
        {
            float v = getLoopPartSize(6, axis);
            v = ceil((value - pos) / v) * v;
            pos = std::clamp(pos + v, 0.0f, overlapSize);
            changed = true;
        }
    }
    else if (value >= overlapSize)
    {
        value -= getLoopPartSize(2, axis);
        if (value < pos)
        {
            float v = getLoopPartSize(6, axis);
            v = ceil((pos - value) / v) * v;
            pos = std::clamp(pos - v, 0.0f, overlapSize);
            changed = true;
        }
    }

    if (changed)
    {
        if (axis == 0)
            _container->set_position(Vector2(-(int)pos, _container->get_position().y));
        else
            _container->set_position(Vector2(_container->get_position().x, -(int)pos));
    }
}

void ScrollPane::alignPosition(Vector2& pos, bool inertialScrolling)
{
    if (_pageMode)
    {
        pos.x = alignByPage(pos.x, 0, inertialScrolling);
        pos.y = alignByPage(pos.y, 1, inertialScrolling);
    }
    else if (_snapToItem)
    {
        Vector2 tmp = _owner->getSnappingPosition(-pos);
        if (pos.x < 0 && pos.x > -_overlapSize.width)
            pos.x = -tmp.x;
        if (pos.y < 0 && pos.y > -_overlapSize.height)
            pos.y = -tmp.y;
    }
}

float ScrollPane::alignByPage(float pos, int axis, bool inertialScrolling)
{
    int page;
    float pageSize = sp_getField(_pageSize, axis);
    float overlapSize = sp_getField(_overlapSize, axis);
    float contentSize = sp_getField(_contentSize, axis);

    if (pos > 0)
        page = 0;
    else if (pos < -overlapSize)
        page = ceil(contentSize / pageSize) - 1;
    else
    {
        page = floor(-pos / pageSize);
        float change = inertialScrolling ? (pos - sp_getField(_containerPos, axis)) : (pos - sp_getField(_container->get_position(), axis));
        float testPageSize = std::min(pageSize, contentSize - (page + 1) * pageSize);
        float delta = -pos - page * pageSize;

        if (std::abs(change) > pageSize)
        {
            if (delta > testPageSize * 0.5f)
                page++;
        }
        else
        {
            if (delta > testPageSize * (change < 0 ? 0.3f : 0.7f))
                page++;
        }

        pos = -page * pageSize;
        if (pos < -overlapSize)
            pos = -overlapSize;
    }

    if (inertialScrolling)
    {
        float oldPos = sp_getField(_tweenStart, axis);
        int oldPage;
        if (oldPos > 0)
            oldPage = 0;
        else if (oldPos < -overlapSize)
            oldPage = ceil(contentSize / pageSize) - 1;
        else
            oldPage = floor(-oldPos / pageSize);
        int startPage = floor(-sp_getField(_containerPos, axis) / pageSize);
        if (abs(page - startPage) > 1 && abs(oldPage - startPage) <= 1)
        {
            if (page > startPage)
                page = startPage + 1;
            else
                page = startPage - 1;
            pos = -page * pageSize;
        }
    }

    return pos;
}

Vector2 ScrollPane::updateTargetAndDuration(const Vector2& orignPos)
{
    Vector2 ret(0, 0);
    ret.x = updateTargetAndDuration(orignPos.x, 0);
    ret.y = updateTargetAndDuration(orignPos.y, 1);
    return ret;
}

float ScrollPane::updateTargetAndDuration(float pos, int axis)
{
    float v = sp_getField(_velocity, axis);
    float duration = 0;

    if (pos > 0)
        pos = 0;
    else if (pos < -sp_getField(_overlapSize, axis))
        pos = -sp_getField(_overlapSize, axis);
    else
    {
        float v2 = std::abs(v) * _velocityScale;
        float ratio = 0;
        if (ToolSet::isDesktopInput())
        {
            if (v2 > 500)
                ratio = pow((v2 - 500) / 500, 2);
        }
        else
        {
            const Vector2 winSize = ((Node*)_owner->displayObject())->get_viewport()->get_visible_rect().size;
            v2 *= 1136.0f / std::max(winSize.x, winSize.y);

            if (_pageMode)
            {
                if (v2 > 500)
                    ratio = pow((v2 - 500) / 500, 2);
            }
            else
            {
                if (v2 > 1000)
                    ratio = pow((v2 - 1000) / 1000, 2);
            }
        }

        if (ratio != 0)
        {
            if (ratio > 1)
                ratio = 1;

            v2 *= ratio;
            v *= ratio;
            sp_setField(_velocity, axis, v);

            duration = log(60 / v2) / log(_decelerationRate) / 60;
            float change = (int)(v * duration * 0.4f);
            pos += change;
        }
    }

    if (duration < TWEEN_TIME_DEFAULT)
        duration = TWEEN_TIME_DEFAULT;
    sp_setField(_tweenDuration, axis, duration);

    return pos;
}

void ScrollPane::fixDuration(int axis, float oldChange)
{
    float tweenChange = sp_getField(_tweenChange, axis);
    if (tweenChange == 0 || std::abs(tweenChange) >= std::abs(oldChange))
        return;

    float newDuration = std::abs(tweenChange / oldChange) * sp_getField(_tweenDuration, axis);
    if (newDuration < TWEEN_TIME_DEFAULT)
        newDuration = TWEEN_TIME_DEFAULT;

    sp_setField(_tweenDuration, axis, newDuration);
}

void ScrollPane::startTween(int type)
{
    _tweenTime = Vector2();
    _tweening = type;
    // Drive tweenUpdate via _maskContainer's _process callback.
    _maskContainer->set_process(true);
    _maskContainer->_processCallback = [this](float dt) { tweenUpdate(dt); };
    updateScrollBarVisible();
}

void ScrollPane::killTween()
{
    if (_tweening == 1)
    {
        Vector2 t = _tweenStart + _tweenChange;
        _container->set_position(t);
        _owner->dispatchEvent(UIEventType::Scroll);
    }

    _tweening = 0;
    _maskContainer->set_process(false);
    _maskContainer->_processCallback = nullptr;
    _owner->dispatchEvent(UIEventType::ScrollEnd);
}

void ScrollPane::checkRefreshBar()
{
    if (_header == nullptr && _footer == nullptr)
        return;

    float pos = sp_getField(_container->get_position(), _refreshBarAxis);
    if (_header != nullptr)
    {
        if (pos > 0)
        {
            _header->setVisible(true);
            Vector2 vec;

            vec = _header->getSize();
            sp_setField(vec, _refreshBarAxis, pos);
            _header->setSize(vec.x, vec.y);
        }
        else
            _header->setVisible(false);
    }

    if (_footer != nullptr)
    {
        float max = sp_getField(_overlapSize, _refreshBarAxis);
        if (pos < -max || (max == 0 && _footerLockedSize > 0))
        {
            _footer->setVisible(true);

            Vector2 vec;

            vec = _footer->getPosition();
            if (max > 0)
                sp_setField(vec, _refreshBarAxis, pos + sp_getField(_contentSize, _refreshBarAxis));
            else
                sp_setField(vec, _refreshBarAxis, std::max(std::min(pos + sp_getField(_viewSize, _refreshBarAxis), sp_getField(_viewSize, _refreshBarAxis) - _footerLockedSize), sp_getField(_viewSize, _refreshBarAxis) - sp_getField(_contentSize, _refreshBarAxis)));
            _footer->setPosition(vec.x, vec.y);

            vec = _footer->getSize();
            if (max > 0)
                sp_setField(vec, _refreshBarAxis, -max - pos);
            else
                sp_setField(vec, _refreshBarAxis, sp_getField(_viewSize, _refreshBarAxis) - sp_getField(_footer->getPosition(), _refreshBarAxis));
            _footer->setSize(vec.x, vec.y);
        }
        else
            _footer->setVisible(false);
    }
}

void ScrollPane::tweenUpdate(float dt)
{
    float nx = runTween(0, dt);
    float ny = runTween(1, dt);

    _container->set_position(Vector2(nx, ny));

    if (_tweening == 2)
    {
        if (_overlapSize.width > 0)
            _xPos = std::clamp(-nx, 0.0f, _overlapSize.width);
        if (_overlapSize.height > 0)
            _yPos = std::clamp(-ny, 0.0f, _overlapSize.height);

        if (_pageMode)
            updatePageController();
    }

    if (_tweenChange.x == 0 && _tweenChange.y == 0)
    {
        _tweening = 0;
        _maskContainer->set_process(false);
        _maskContainer->_processCallback = nullptr;

        loopCheckingCurrent();

        updateScrollBarPos();
        updateScrollBarVisible();

        _owner->dispatchEvent(UIEventType::Scroll);
        _owner->dispatchEvent(UIEventType::ScrollEnd);
    }
    else
    {
        updateScrollBarPos();
        _owner->dispatchEvent(UIEventType::Scroll);
    }
}

float ScrollPane::runTween(int axis, float dt)
{
    float newValue;
    if (sp_getField(_tweenChange, axis) != 0)
    {
        sp_incField(_tweenTime, axis, dt);
        if (sp_getField(_tweenTime, axis) >= sp_getField(_tweenDuration, axis))
        {
            newValue = sp_getField(_tweenStart, axis) + sp_getField(_tweenChange, axis);
            sp_setField(_tweenChange, axis, 0);
        }
        else
        {
            float ratio = sp_EaseFunc(sp_getField(_tweenTime, axis), sp_getField(_tweenDuration, axis));
            newValue = sp_getField(_tweenStart, axis) + (int)(sp_getField(_tweenChange, axis) * ratio);
        }

        float threshold1 = 0;
        float threshold2 = -sp_getField(_overlapSize, axis);
        if (_headerLockedSize > 0 && _refreshBarAxis == axis)
            threshold1 = _headerLockedSize;
        if (_footerLockedSize > 0 && _refreshBarAxis == axis)
        {
            float max = sp_getField(_overlapSize, _refreshBarAxis);
            if (max == 0)
                max = std::max(sp_getField(_contentSize, _refreshBarAxis) + _footerLockedSize - sp_getField(_viewSize, _refreshBarAxis), 0.0f);
            else
                max += _footerLockedSize;
            threshold2 = -max;
        }

        if (_tweening == 2 && _bouncebackEffect)
        {
            if ((newValue > 20 + threshold1 && sp_getField(_tweenChange, axis) > 0) || (newValue > threshold1 && sp_getField(_tweenChange, axis) == 0))
            {
                sp_setField(_tweenTime, axis, 0);
                sp_setField(_tweenDuration, axis, TWEEN_TIME_DEFAULT);
                sp_setField(_tweenChange, axis, -newValue + threshold1);
                sp_setField(_tweenStart, axis, newValue);
            }
            else if ((newValue < threshold2 - 20 && sp_getField(_tweenChange, axis) < 0) || (newValue < threshold2 && sp_getField(_tweenChange, axis) == 0))
            {
                sp_setField(_tweenTime, axis, 0);
                sp_setField(_tweenDuration, axis, TWEEN_TIME_DEFAULT);
                sp_setField(_tweenChange, axis, threshold2 - newValue);
                sp_setField(_tweenStart, axis, newValue);
            }
        }
        else
        {
            if (newValue > threshold1)
            {
                newValue = threshold1;
                sp_setField(_tweenChange, axis, 0);
            }
            else if (newValue < threshold2)
            {
                newValue = threshold2;
                sp_setField(_tweenChange, axis, 0);
            }
        }
    }
    else
        newValue = sp_getField(_container->get_position(), axis);

    return newValue;
}

void ScrollPane::onTouchBegin(EventContext* context)
{
    if (!_touchEffect)
        return;

    context->captureTouch();
    InputEvent* evt = context->getInput();
    Vector2 pt = _owner->globalToLocal(evt->getPosition());

    if (_tweening != 0)
    {
        killTween();
        evt->getProcessor()->cancelClick(evt->getTouchId());

        _dragged = true;
    }
    else
        _dragged = false;

    _containerPos = _container->get_position();
    _beginTouchPos = _lastTouchPos = pt;
    _lastTouchGlobalPos = evt->getPosition();
    _isHoldAreaDone = false;
    _velocity = Vector2();
    _velocityScale = 1;
    _lastMoveTime = clock();
}

void ScrollPane::onTouchMove(EventContext* context)
{
    if (!_touchEffect)
        return;

    if ((_draggingPane != nullptr && _draggingPane != this) || GObject::getDraggingObject() != nullptr)
        return;

    InputEvent* evt = context->getInput();
    Vector2 pt = _owner->globalToLocal(evt->getPosition());

    int sensitivity = ToolSet::isDesktopInput() ? 8 : UIConfig::touchScrollSensitivity;

    float diff;
    bool sv = false, sh = false;

    if (_scrollType == ScrollType::VERTICAL)
    {
        if (!_isHoldAreaDone)
        {
            _gestureFlag |= 1;

            diff = std::abs(_beginTouchPos.y - pt.y);
            if (diff < sensitivity)
                return;

            if ((_gestureFlag & 2) != 0)
            {
                float diff2 = std::abs(_beginTouchPos.x - pt.x);
                if (diff < diff2)
                    return;
            }
        }

        sv = true;
    }
    else if (_scrollType == ScrollType::HORIZONTAL)
    {
        if (!_isHoldAreaDone)
        {
            _gestureFlag |= 2;

            diff = std::abs(_beginTouchPos.x - pt.x);
            if (diff < sensitivity)
                return;

            if ((_gestureFlag & 1) != 0)
            {
                float diff2 = std::abs(_beginTouchPos.y - pt.y);
                if (diff < diff2)
                    return;
            }
        }

        sh = true;
    }
    else
    {
        _gestureFlag = 3;

        if (!_isHoldAreaDone)
        {
            diff = std::abs(_beginTouchPos.y - pt.y);
            if (diff < sensitivity)
            {
                diff = std::abs(_beginTouchPos.x - pt.x);
                if (diff < sensitivity)
                    return;
            }
        }

        sv = sh = true;
    }

    Vector2 newPos = _containerPos + pt - _beginTouchPos;
    newPos.x = (int)newPos.x;
    newPos.y = (int)newPos.y;

    if (sv)
    {
        if (newPos.y > 0)
        {
            if (!_bouncebackEffect)
                _container->set_position(Vector2(_container->get_position().x, 0));
            else if (_header != nullptr && _header->maxSize.height != 0)
                _container->set_position(Vector2(_container->get_position().x, (int)std::min(newPos.y * 0.5f, _header->maxSize.height)));
            else
                _container->set_position(Vector2(_container->get_position().x, (int)std::min(newPos.y * 0.5f, _viewSize.height * PULL_RATIO)));
        }
        else if (newPos.y < -_overlapSize.height)
        {
            if (!_bouncebackEffect)
                _container->set_position(Vector2(_container->get_position().x, -_overlapSize.height));
            else if (_footer != nullptr && _footer->maxSize.height > 0)
                _container->set_position(Vector2(_container->get_position().x, (int)std::max((newPos.y + _overlapSize.height) * 0.5f, -_footer->maxSize.height) - _overlapSize.height));
            else
                _container->set_position(Vector2(_container->get_position().x, (int)std::max((newPos.y + _overlapSize.height) * 0.5f, -_viewSize.height * PULL_RATIO) - _overlapSize.height));
        }
        else
            _container->set_position(Vector2(_container->get_position().x, newPos.y));
    }

    if (sh)
    {
        if (newPos.x > 0)
        {
            if (!_bouncebackEffect)
                _container->set_position(Vector2(0, _container->get_position().y));
            else if (_header != nullptr && _header->maxSize.width != 0)
                _container->set_position(Vector2((int)std::min(newPos.x * 0.5f, _header->maxSize.width), _container->get_position().y));
            else
                _container->set_position(Vector2((int)std::min(newPos.x * 0.5f, _viewSize.width * PULL_RATIO), _container->get_position().y));
        }
        else if (newPos.x < 0 - _overlapSize.width)
        {
            if (!_bouncebackEffect)
                _container->set_position(Vector2(-_overlapSize.width, _container->get_position().y));
            else if (_footer != nullptr && _footer->maxSize.width > 0)
                _container->set_position(Vector2((int)std::max((newPos.x + _overlapSize.width) * 0.5f, -_footer->maxSize.width) - _overlapSize.width, _container->get_position().y));
            else
                _container->set_position(Vector2((int)std::max((newPos.x + _overlapSize.width) * 0.5f, -_viewSize.width * PULL_RATIO) - _overlapSize.width, _container->get_position().y));
        }
        else
            _container->set_position(Vector2(newPos.x, _container->get_position().y));
    }

    float deltaTime = ((Node*)_owner->displayObject())->get_process_delta_time();
    if (deltaTime <= 0) deltaTime = 0.016f;
    float elapsed = (clock() - _lastMoveTime) / (double)CLOCKS_PER_SEC;
    elapsed = elapsed * 60 - 1;
    if (elapsed > 1)
        _velocity = _velocity * pow(0.833f, elapsed);
    Vector2 deltaPosition = pt - _lastTouchPos;
    if (!sh)
        deltaPosition.x = 0;
    if (!sv)
        deltaPosition.y = 0;
    _velocity = _velocity.lerp(deltaPosition / deltaTime, deltaTime * 10);

    Vector2 deltaGlobalPosition = _lastTouchGlobalPos - evt->getPosition();
    if (deltaPosition.x != 0)
        _velocityScale = std::abs(deltaGlobalPosition.x / deltaPosition.x);
    else if (deltaPosition.y != 0)
        _velocityScale = std::abs(deltaGlobalPosition.y / deltaPosition.y);

    _lastTouchPos = pt;
    _lastTouchGlobalPos = evt->getPosition();
    _lastMoveTime = clock();

    if (_overlapSize.width > 0)
        _xPos = std::clamp(-_container->get_position().x, 0.0f, _overlapSize.width);
    if (_overlapSize.height > 0)
        _yPos = std::clamp(-_container->get_position().y, 0.0f, _overlapSize.height);

    if (_loop != 0)
    {
        newPos = _container->get_position();
        if (loopCheckingCurrent())
            _containerPos += _container->get_position() - newPos;
    }

    _draggingPane = this;
    _isHoldAreaDone = true;
    _dragged = true;

    updateScrollBarPos();
    updateScrollBarVisible();
    if (_pageMode)
        updatePageController();
    _owner->dispatchEvent(UIEventType::Scroll);
}

void ScrollPane::onTouchEnd(EventContext* context)
{
    if (_draggingPane == this)
        _draggingPane = nullptr;

    _gestureFlag = 0;

    if (!_dragged || !_touchEffect)
    {
        _dragged = false;
        return;
    }

    _dragged = false;
    _tweenStart = _container->get_position();

    Vector2 endPos = _tweenStart;
    bool flag = false;
    if (_container->get_position().x > 0)
    {
        endPos.x = 0;
        flag = true;
    }
    else if (_container->get_position().x < -_overlapSize.width)
    {
        endPos.x = -_overlapSize.width;
        flag = true;
    }
    if (_container->get_position().y > 0)
    {
        endPos.y = 0;
        flag = true;
    }
    else if (_container->get_position().y < -_overlapSize.height)
    {
        endPos.y = -_overlapSize.height;
        flag = true;
    }

    if (flag)
    {
        _tweenChange = endPos - _tweenStart;
        if (_tweenChange.x < -UIConfig::touchDragSensitivity || _tweenChange.y < -UIConfig::touchDragSensitivity)
            _owner->dispatchEvent(UIEventType::PullDownRelease);
        else if (_tweenChange.x > UIConfig::touchDragSensitivity || _tweenChange.y > UIConfig::touchDragSensitivity)
            _owner->dispatchEvent(UIEventType::PullUpRelease);

        if (_headerLockedSize > 0 && sp_getField(endPos, _refreshBarAxis) == 0)
        {
            sp_setField(endPos, _refreshBarAxis, _headerLockedSize);
            _tweenChange = endPos - _tweenStart;
        }
        else if (_footerLockedSize > 0 && sp_getField(endPos, _refreshBarAxis) == -sp_getField(_overlapSize, _refreshBarAxis))
        {
            float max = sp_getField(_overlapSize, _refreshBarAxis);
            if (max == 0)
                max = std::max(sp_getField(_contentSize, _refreshBarAxis) + _footerLockedSize - sp_getField(_viewSize, _refreshBarAxis), 0.0f);
            else
                max += _footerLockedSize;
            sp_setField(endPos, _refreshBarAxis, -max);
            _tweenChange = endPos - _tweenStart;
        }

        _tweenDuration = Vector2(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
    }
    else
    {
        if (!_inertiaDisabled)
        {
            float elapsed = (clock() - _lastMoveTime) / (double)CLOCKS_PER_SEC;
            elapsed = elapsed * 60 - 1;
            if (elapsed > 1)
                _velocity = _velocity * pow(0.833f, elapsed);

            endPos = updateTargetAndDuration(_tweenStart);
        }
        else
            _tweenDuration = Vector2(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        Vector2 oldChange = endPos - _tweenStart;

        loopCheckingTarget(endPos);
        if (_pageMode || _snapToItem)
            alignPosition(endPos, true);

        _tweenChange = endPos - _tweenStart;
        if (_tweenChange.x == 0 && _tweenChange.y == 0)
        {
            updateScrollBarVisible();
            return;
        }

        if (_pageMode || _snapToItem)
        {
            fixDuration(0, oldChange.x);
            fixDuration(1, oldChange.y);
        }
    }

    startTween(2);
}

void ScrollPane::onMouseWheel(EventContext* context)
{
    if (!_mouseWheelEnabled)
        return;

    InputEvent* evt = context->getInput();
    int delta = evt->getMouseWheelDelta();
    delta = delta > 0 ? 1 : -1;
    if (_overlapSize.width > 0 && _overlapSize.height == 0)
    {
        if (_pageMode)
            setPosX(_xPos + _pageSize.width * delta, false);
        else
            setPosX(_xPos + _mouseWheelStep * delta, false);
    }
    else
    {
        if (_pageMode)
            setPosY(_yPos + _pageSize.height * delta, false);
        else
            setPosY(_yPos + _mouseWheelStep * delta, false);
    }
}

void ScrollPane::onRollOver(EventContext* context)
{
    _hover = true;
    updateScrollBarVisible();
}

void ScrollPane::onRollOut(EventContext* context)
{
    _hover = false;
    updateScrollBarVisible();
}

void ScrollPane::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getPosX"), &ScrollPane::gd_getPosX);
    ClassDB::bind_method(D_METHOD("setPosX", "value", "ani"), &ScrollPane::gd_setPosX, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("getPosY"), &ScrollPane::gd_getPosY);
    ClassDB::bind_method(D_METHOD("setPosY", "value", "ani"), &ScrollPane::gd_setPosY, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("scrollTop", "ani"), &ScrollPane::gd_scrollTop, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("scrollBottom", "ani"), &ScrollPane::gd_scrollBottom, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("isBottomMost"), &ScrollPane::gd_isBottomMost);
    ClassDB::bind_method(D_METHOD("getHeader"), &ScrollPane::getHeader);
    ClassDB::bind_method(D_METHOD("getFooter"), &ScrollPane::getFooter);
    ClassDB::bind_method(D_METHOD("lockHeader", "size"), &ScrollPane::lockHeader);
    ClassDB::bind_method(D_METHOD("lockFooter", "size"), &ScrollPane::lockFooter);
    ClassDB::bind_method(D_METHOD("cancelDragging"), &ScrollPane::cancelDragging);
}

NS_FGUI_END
