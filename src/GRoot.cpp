#include "GRoot.h"
#include "GGraph.h"
#include "UIConfig.h"
#include "UIPackage.h"
#include "display/FUIContainer.h"
#include "tween/TweenManager.h"
#include "core/object/class_db.h"

#include "scene/audio/audio_stream_player.h"
#include "servers/audio/audio_stream.h"
#include "core/io/resource_loader.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"
#include "servers/display_server.h"

#include <algorithm>

NS_FGUI_BEGIN

GRoot* GRoot::_inst = nullptr;
bool GRoot::_soundEnabled = true;
float GRoot::_soundVolumeScale = 1.0f;
int GRoot::contentScaleLevel = 0;

GRoot* GRoot::create(SceneTree* tree, int zOrder)
{
    Ref<GRoot> ref = memnew(GRoot);
    GRoot* pRet = ref.ptr();
    
    Node* root = Object::cast_to<Node>(tree->get_root());
    if (pRet->initWithParent(Object::cast_to<Node>(tree->get_root()), zOrder))
    {
        pRet->onInitWithParent(root, zOrder);
        pRet->reference(); // keep alive after ref dtor (2→1)
        return pRet;
    }
    // ref dtor cleans up (1→0→freed)
    return nullptr;
}

GRoot* GRoot::createDeferred(SceneTree* tree, int zOrder)
{
    Ref<GRoot> ref = memnew(GRoot);
    GRoot* pRet = ref.ptr();
    
    Node* root = Object::cast_to<Node>(tree->get_root());
    if (pRet->initWithParent(root, zOrder))
    {
        pRet->onInitWithParent(root, zOrder, true);
        pRet->reference(); // keep alive after ref dtor (2→1)
        return pRet;
    }

    // ref dtor cleans up (1→0→freed)
    return nullptr;
}

void GRoot::cleanup()
{
    if (_inst != nullptr)
    {
        Node* displayNode = _inst->displayObject();
        if (displayNode != nullptr)
        {
            Node* parent = displayNode->get_parent();
            if (parent != nullptr)
                parent->remove_child(displayNode);
        }
        _inst = nullptr;
    }
}

GRoot::GRoot()
    : _inputProcessor(nullptr),
      _modalLayer(nullptr),
      _modalWaitPane(nullptr),
      _tooltipWin(nullptr),
      _defaultTooltipWin(nullptr),
      _hasDesignResolution(false),
      _viewportSizeConnected(false),
      _soundPlayerCount(0),
      _soundPoolIndex(0)
{
    for (int i = 0; i < SOUND_POOL_MAX; i++)
        _soundPlayers[i] = nullptr;
}

GRoot::~GRoot()
{
    delete _inputProcessor;

    if (_modalWaitPane.is_valid())
    {
        _modalWaitPane = nullptr;
    }
    if (_defaultTooltipWin)
    {
        if (!_defaultTooltipWin->displayObject()->is_inside_tree())
            memdelete(_defaultTooltipWin);
        _defaultTooltipWin = nullptr;
    }
    if (_modalLayerRef.is_valid())
    {
        _modalLayerRef = Ref<GGraph>();
        _modalLayer = nullptr;
    }

    CALL_LATER_CANCEL(GRoot, doShowTooltipsWin);
}

void GRoot::_bind_methods()
{
    ClassDB::bind_integer_constant(get_class_static(), "ScreenMatchMode", "MATCH_WIDTH_OR_HEIGHT", static_cast<GDExtensionInt>(ScreenMatchMode::MatchWidthOrHeight));
    ClassDB::bind_integer_constant(get_class_static(), "ScreenMatchMode", "MATCH_WIDTH", static_cast<GDExtensionInt>(ScreenMatchMode::MatchWidth));
    ClassDB::bind_integer_constant(get_class_static(), "ScreenMatchMode", "MATCH_HEIGHT", static_cast<GDExtensionInt>(ScreenMatchMode::MatchHeight));
    ClassDB::bind_integer_constant(get_class_static(), "ScreenMatchMode", "MATCH_FILL", static_cast<GDExtensionInt>(ScreenMatchMode::MatchFill));

    ClassDB::bind_integer_constant(get_class_static(), "PopupDirection", "AUTO", static_cast<GDExtensionInt>(PopupDirection::AUTO));
    ClassDB::bind_integer_constant(get_class_static(), "PopupDirection", "UP", static_cast<GDExtensionInt>(PopupDirection::UP));
    ClassDB::bind_integer_constant(get_class_static(), "PopupDirection", "DOWN", static_cast<GDExtensionInt>(PopupDirection::DOWN));

    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "tree", "z_order"), &GRoot::create, DEFVAL(1000));
    ClassDB::bind_static_method(get_class_static(), D_METHOD("createDeferred", "tree", "z_order"), &GRoot::createDeferred, DEFVAL(1000));
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getInstance"), &GRoot::getInstance);

    ClassDB::bind_method(D_METHOD("showWindow", "win"), &GRoot::gd_showWindow);
    ClassDB::bind_method(D_METHOD("hideWindow", "win"), &GRoot::gd_hideWindow);
    ClassDB::bind_method(D_METHOD("hideWindowImmediately", "win"), &GRoot::gd_hideWindowImmediately);
    ClassDB::bind_method(D_METHOD("bringToFront", "win"), &GRoot::gd_bringToFront);
    ClassDB::bind_method(D_METHOD("getTopWindow"), &GRoot::gd_getTopWindow);

    ClassDB::bind_method(D_METHOD("showModalWait"), &GRoot::showModalWait);
    ClassDB::bind_method(D_METHOD("closeModalWait"), &GRoot::closeModalWait);
    ClassDB::bind_method(D_METHOD("closeAllExceptModals"), &GRoot::closeAllExceptModals);
    ClassDB::bind_method(D_METHOD("closeAllWindows"), &GRoot::gd_closeAllWindows);

    ClassDB::bind_method(D_METHOD("getModalWaitingPane"), &GRoot::gd_getModalWaitingPane);
    ClassDB::bind_method(D_METHOD("getModalLayer"), &GRoot::getModalLayer);
    ClassDB::bind_method(D_METHOD("hasModalWindow"), &GRoot::gd_hasModalWindow);
    ClassDB::bind_method(D_METHOD("isModalWaiting"), &GRoot::isModalWaiting);

    ClassDB::bind_method(D_METHOD("getTouchPosition", "touch_id"), &GRoot::getTouchPosition, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("getTouchTarget"), &GRoot::gd_getTouchTarget);

    ClassDB::bind_method(D_METHOD("worldToRoot", "pt"), &GRoot::worldToRoot);
    ClassDB::bind_method(D_METHOD("rootToWorld", "pt"), &GRoot::rootToWorld);

    ClassDB::bind_method(D_METHOD("hasAnyPopup"), &GRoot::hasAnyPopup);
    ClassDB::bind_method(D_METHOD("hidePopup"), &GRoot::gd_hidePopup);

    ClassDB::bind_method(D_METHOD("showTooltips", "msg"), &GRoot::gd_showTooltips);
    ClassDB::bind_method(D_METHOD("showTooltipsWin", "tooltip_win"), &GRoot::gd_showTooltipsWin);
    ClassDB::bind_method(D_METHOD("hideTooltips"), &GRoot::hideTooltips);

    ClassDB::bind_method(D_METHOD("playSound", "url", "volume_scale"), &GRoot::gd_playSound, DEFVAL(1.0f));

    ClassDB::bind_method(D_METHOD("setContentScaleFactor", "designWidth", "designHeight", "matchMode"), &GRoot::setContentScaleFactor);
    ClassDB::bind_method(D_METHOD("onWindowSizeChanged"), &GRoot::onWindowSizeChanged);
    ClassDB::bind_method(D_METHOD("getDisplayObject"), &GRoot::gd_getDisplayObject);

    ClassDB::bind_method(D_METHOD("setSoundEnabled", "value"), &GRoot::setSoundEnabled);
    ClassDB::bind_method(D_METHOD("isSoundEnabled"), &GRoot::isSoundEnabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "soundEnabled"), "setSoundEnabled", "isSoundEnabled");

    ClassDB::bind_method(D_METHOD("setSoundVolumeScale", "value"), &GRoot::setSoundVolumeScale);
    ClassDB::bind_method(D_METHOD("getSoundVolumeScale"), &GRoot::getSoundVolumeScale);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "soundVolumeScale", PROPERTY_HINT_RANGE, "0,1,0.01"), "setSoundVolumeScale", "getSoundVolumeScale");

    ClassDB::bind_method(D_METHOD("showPopup", "popup", "target", "dir"), &GRoot::gd_showPopup);
    ClassDB::bind_method(D_METHOD("showPopupSimple", "popup"), &GRoot::gd_showPopupSimple);
}

void GRoot::showWindow(GWindow* win)
{
    if (!win)
        return;

    if (!win->displayObject())
        win->init();

    // Load content before attaching to GRoot so size/layout are valid for display.
    win->initWindow();

    static const int kPopupWindowSortingOrder = 50000;
    if (win->getSortingOrder() < kPopupWindowSortingOrder)
        win->setSortingOrder(kPopupWindowSortingOrder);

    Ref<GObject> ref(win);
    addChild(ref);
    syncNativeChildrenZOrder();
    win->center();
    bringToFront(win);
    adjustModalLayer();
}

void GRoot::hideWindow(GWindow* win)
{
    win->hide();
}

void GRoot::hideWindowImmediately(GWindow* win)
{
    if (win->getParent() == this)
        removeChild(win);

    adjustModalLayer();
}

void GRoot::bringToFront(GWindow* win)
{
    static const int kOverlayBaseOrder = 50000;
    static const int kOverlayMaxOrder = 99999;

    int maxOrder = kOverlayBaseOrder - 1;
    int cnt = numChildren();
    for (int i = 0; i < cnt; i++)
    {
        GObject* g = getChildAt(i);
        if (g != win && dynamic_cast<GWindow*>(g))
            maxOrder = std::max(maxOrder, g->getSortingOrder());
    }

    int newOrder = std::min(maxOrder + 1, kOverlayMaxOrder);
    if (win->getSortingOrder() < newOrder)
        win->setSortingOrder(newOrder);

    int i;
    if (_modalLayer && _modalLayer->getParent() != nullptr && !win->isModal())
        i = getChildIndex(_modalLayer) - 1;
    else
        i = cnt - 1;

    for (; i >= 0; i--)
    {
        GObject* g = getChildAt(i);
        if (g == win)
            return;
        if (dynamic_cast<GWindow*>(g))
            break;
    }

    if (i >= 0 && win->getSortingOrder() == 0)
        setChildIndex(win, i);
}

void GRoot::closeAllExceptModals()
{
    std::vector<GObject*> map;
    map.reserve(_children.size());
    for (const auto& child : _children)
        map.push_back(child.ptr());

    for (const auto& child : map)
    {
        if (dynamic_cast<GWindow*>(child) && !((GWindow*)child)->isModal())
            hideWindowImmediately((GWindow*)child);
    }
}

void GRoot::closeAllWindows()
{
    std::vector<GObject*> map;
    map.reserve(_children.size());
    for (const auto& child : _children)
        map.push_back(child.ptr());

    for (const auto& child : map)
    {
        if (dynamic_cast<GWindow*>(child))
            hideWindowImmediately((GWindow*)child);
    }
}

GWindow* GRoot::getTopWindow()
{
    int cnt = numChildren();
    for (int i = cnt - 1; i >= 0; i--)
    {
        GObject* child = getChildAt(i);
        if (dynamic_cast<GWindow*>(child))
        {
            return (GWindow*)child;
        }
    }

    return nullptr;
}

GGraph* GRoot::getModalLayer()
{
    if (_modalLayer == nullptr)
        createModalLayer();

    return _modalLayer;
}

void GRoot::createModalLayer()
{
    _modalLayerRef = GGraph::create();
    _modalLayer = _modalLayerRef.ptr();
    if (!_modalLayer)
        return;

    _modalLayer->drawRect(getWidth(), getHeight(), 0, Color(1.0f, 1.0f, 1.0f, 1.0f), UIConfig::modalLayerColor);
    _modalLayer->addRelation(this, RelationType::Size);
}

void GRoot::adjustModalLayer()
{
    if (_modalLayer == nullptr)
        createModalLayer();

    if (_modalLayer == nullptr)
        return;

    int cnt = numChildren();

    if (_modalWaitPane.is_valid() && _modalWaitPane->getParent() != nullptr)
        setChildIndex(_modalWaitPane.ptr(), cnt - 1);

    for (int i = cnt - 1; i >= 0; i--)
    {
        GObject* child = getChildAt(i);
        if (dynamic_cast<GWindow*>(child) && ((GWindow*)child)->isModal())
        {
            if (_modalLayer->getParent() == nullptr)
                addChildAt(Ref<GObject>(_modalLayer), i);
            else
                setChildIndexBefore(_modalLayer, i);
            return;
        }
    }

    if (_modalLayer->getParent() != nullptr)
        removeChild(_modalLayer);
}

bool GRoot::hasModalWindow()
{
    return _modalLayer != nullptr && _modalLayer->getParent() != nullptr;
}

void GRoot::showModalWait()
{
    getModalWaitingPane();
    if (_modalWaitPane.is_valid())
        addChild(_modalWaitPane);
}

void GRoot::closeModalWait()
{
    if (_modalWaitPane.is_valid() && _modalWaitPane->getParent() != nullptr)
        removeChild(_modalWaitPane.ptr());
}

GObject* GRoot::getModalWaitingPane()
{
    if (!UIConfig::globalModalWaiting.empty())
    {
        if (!_modalWaitPane.is_valid())
        {
            _modalWaitPane = UIPackage::createObjectFromURL(UIConfig::globalModalWaiting);
            _modalWaitPane->setSortingOrder(INT_MAX);
        }

        _modalWaitPane->setSize(getWidth(), getHeight());
        _modalWaitPane->addRelation(this, RelationType::Size);

        return _modalWaitPane.ptr();
    }
    else
        return nullptr;
}

bool GRoot::isModalWaiting()
{
    return (_modalWaitPane.is_valid()) && _modalWaitPane->onStage();
}

Vector2 GRoot::getTouchPosition(int touchId)
{
    return _inputProcessor->getTouchPosition(touchId);
}

GObject* GRoot::getTouchTarget()
{
    return _inputProcessor->getRecentInput()->getTarget();
}

Vector2 GRoot::worldToRoot(const Vector2& pt)
{
    // FairyGUI + Godot both Y-down: no Y flip (Cocos flipped Y here for Y-up engine)
    return ((CanvasItem*)_displayObject)->get_global_transform_with_canvas().affine_inverse().xform(pt);
}

Vector2 GRoot::rootToWorld(const Vector2& pt)
{
    return ((CanvasItem*)_displayObject)->get_global_transform_with_canvas().xform(pt);
}

void GRoot::showPopup(GObject* popup)
{
    showPopup(popup, nullptr, PopupDirection::AUTO);
}

void GRoot::showPopup(GObject* popup, GObject* target, PopupDirection dir)
{
    if (!_popupStack.empty())
        hidePopup(popup);

    _popupStack.push_back(WeakPtr(popup));

    if (target != nullptr)
    {
        GObject* p = target;
        while (p != nullptr)
        {
            if (p->getParent() == this)
            {
                if (popup->getSortingOrder() < p->getSortingOrder())
                {
                    popup->setSortingOrder(p->getSortingOrder());
                }
                break;
            }
            p = p->getParent();
        }
    }

    static const int kPopupWindowSortingOrder = 50000;
    if (popup->getSortingOrder() < kPopupWindowSortingOrder)
        popup->setSortingOrder(kPopupWindowSortingOrder);

    popup->setVisible(true);
    addChild(Ref<GObject>(popup));
    syncNativeChildrenZOrder();
    adjustModalLayer();

    if (dynamic_cast<GWindow*>(popup) && target == nullptr && dir == PopupDirection::AUTO)
        return;

    Vector2 pos = getPoupPosition(popup, target, dir);
    popup->setPosition(pos.x, pos.y);
}

void GRoot::togglePopup(GObject* popup)
{
    togglePopup(popup, nullptr, PopupDirection::AUTO);
}

void GRoot::togglePopup(GObject* popup, GObject* target, PopupDirection dir)
{
    if (std::find(_justClosedPopups.cbegin(), _justClosedPopups.cend(), popup) != _justClosedPopups.cend())
        return;

    showPopup(popup, target, dir);
}

void GRoot::hidePopup()
{
    hidePopup(nullptr);
}

void GRoot::hidePopup(GObject* popup)
{
    if (popup != nullptr)
    {
        auto it = std::find(_popupStack.cbegin(), _popupStack.cend(), popup);
        if (it != _popupStack.cend())
        {
            int k = (int)(it - _popupStack.cbegin());
            for (int i = (int)_popupStack.size() - 1; i >= k; i--)
            {
                closePopup(_popupStack.back().ptr());
                _popupStack.pop_back();
            }
        }
    }
    else
    {
        for (const auto& it : _popupStack)
            closePopup(it.ptr());
        _popupStack.clear();
    }
}

void GRoot::closePopup(GObject* target)
{
    if (target && target->getParent() != nullptr)
    {
        if (dynamic_cast<GWindow*>(target))
            ((GWindow*)target)->hide();
        else
            removeChild(target);
    }
}

void GRoot::checkPopups()
{
    _justClosedPopups.clear();
    if (!_popupStack.empty())
    {
        GObject* mc = _inputProcessor->getRecentInput()->getTarget();
        bool handled = false;
        while (mc != this && mc != nullptr)
        {
            auto it = std::find(_popupStack.cbegin(), _popupStack.cend(), mc);
            if (it != _popupStack.cend())
            {
                int k = (int)(it - _popupStack.cbegin());
                for (int i = (int)_popupStack.size() - 1; i > k; i--)
                {
                    closePopup(_popupStack.back().ptr());
                    _popupStack.pop_back();
                }
                handled = true;
                break;
            }
            mc = mc->findParent();
        }

        if (!handled)
        {
            for (int i = (int)_popupStack.size() - 1; i >= 0; i--)
            {
                GObject* popup = _popupStack[i].ptr();
                if (popup)
                {
                    _justClosedPopups.push_back(WeakPtr(popup));
                    closePopup(popup);
                }
            }
            _popupStack.clear();
        }
    }
}

bool GRoot::hasAnyPopup()
{
    return !_popupStack.empty();
}

Vector2 GRoot::getPoupPosition(GObject* popup, GObject* target, PopupDirection dir)
{
    Vector2 pos;
    Vector2 size;
    if (target != nullptr)
    {
        pos = target->localToGlobal(Vector2());
        pos = this->globalToLocal(pos);
        size = target->localToGlobal(target->getSize());
        size = this->globalToLocal(size);
        size -= pos;
    }
    else
    {
        pos = globalToLocal(_inputProcessor->getRecentInput()->getPosition());
    }
    float xx, yy;
    xx = pos.x;
    if (xx + popup->getWidth() > getWidth())
        xx = xx + size.x - popup->getWidth();
    yy = pos.y + size.y;
    if ((dir == PopupDirection::AUTO && yy + popup->getHeight() > getHeight()) || dir == PopupDirection::UP)
    {
        yy = pos.y - popup->getHeight() - 1;
        if (yy < 0)
        {
            yy = 0;
            xx += size.x / 2;
        }
    }

    return Vector2(round(xx), round(yy));
}

void GRoot::showTooltips(const std::string& msg)
{
    if (_defaultTooltipWin == nullptr)
    {
        const std::string& resourceURL = UIConfig::tooltipsWin;
        if (resourceURL.empty())
        {
            print_line("FairyGUI: UIConfig.tooltipsWin not defined");
            return;
        }

        _defaultTooltipWin = UIPackage::createObjectFromURL(resourceURL).ptr();
        _defaultTooltipWin->setTouchable(false);
    }

    _defaultTooltipWin->setText(msg);
    showTooltipsWin(_defaultTooltipWin);
}

void GRoot::showTooltipsWin(GObject* tooltipWin)
{
    hideTooltips();

    _tooltipWin = tooltipWin;
    CALL_LATER(GRoot, doShowTooltipsWin, 0.1f);
}

void GRoot::doShowTooltipsWin()
{
    if (_deferredCallsCancelled)
        return;
    if (_tooltipWin == nullptr)
        return;

    Vector2 pt = _inputProcessor->getRecentInput()->getPosition();
    float xx = pt.x + 10;
    float yy = pt.y + 20;

    pt = globalToLocal(Vector2(xx, yy));
    xx = pt.x;
    yy = pt.y;

    if (xx + _tooltipWin->getWidth() > getWidth())
        xx = xx - _tooltipWin->getWidth();
    if (yy + _tooltipWin->getHeight() > getHeight())
    {
        yy = yy - _tooltipWin->getHeight() - 1;
        if (yy < 0)
            yy = 0;
    }

    _tooltipWin->setPosition(round(xx), round(yy));
    addChild(Ref<GObject>(_tooltipWin));
}

void GRoot::hideTooltips()
{
    if (_tooltipWin != nullptr)
    {
        if (_tooltipWin->getParent() != nullptr)
            removeChild(_tooltipWin);
        _tooltipWin = nullptr;
    }
}

void GRoot::playSound(const std::string& url, float volumeScale)
{
    if (!_soundEnabled)
        return;

    PackageItem* pi = UIPackage::getItemByURL(url);
    if (!pi)
        return;

    Ref<AudioStream> stream = ResourceLoader::load(String(pi->file.c_str()));
    if (!stream.is_valid())
        return;

    AudioStreamPlayer* player = nullptr;
    for (int i = 0; i < _soundPlayerCount; i++)
    {
        if (_soundPlayers[i] && !_soundPlayers[i]->is_playing())
        {
            player = _soundPlayers[i];
            break;
        }
    }

    if (player == nullptr)
    {
        if (_soundPlayerCount < SOUND_POOL_MAX)
        {
            player = memnew(AudioStreamPlayer);
            _displayObject->add_child(player);
            _soundPlayers[_soundPlayerCount++] = player;
        }
        else
        {
            player = _soundPlayers[_soundPoolIndex % SOUND_POOL_MAX];
            _soundPoolIndex++;
            if (player)
                player->stop();
        }
    }

    if (player == nullptr)
        return;

    player->set_stream(stream);
    float db = _soundVolumeScale * volumeScale;
    if (db > 0.0f)
        player->set_volume_db(Math::linear_to_db(db));
    else
        player->set_volume_db(-80.0f);
    player->play();
}

void GRoot::setSoundEnabled(bool value)
{
    _soundEnabled = value;
}

void GRoot::setSoundVolumeScale(float value)
{
    _soundVolumeScale = value;
}

void GRoot::onTouchEvent(int eventType)
{
    if (eventType == UIEventType::TouchBegin)
    {
        if (_tooltipWin != nullptr)
            hideTooltips();

        checkPopups();
    }
}

void GRoot::handlePositionChanged()
{
    // Position is managed by applyContentScale / onWindowSizeChanged
}

void GRoot::handleSizeChanged()
{
    // Scale is managed by applyContentScale — do NOT let base reset it.
    // Only run GComponent's layout bookkeeping (container position, clipping, etc.).
    // ScrollPane and mask are inaccessible / not needed for GRoot.
    if (_container)
        _container->set_position(Vector2(_margin.left, _margin.top));

    if (((FUIContainer*)_displayObject)->isClippingEnabled())
        ((FUIContainer*)_displayObject)->setClippingRegion(Rect(_margin.left, _margin.top, _size.width - _margin.left - _margin.right, _size.height - _margin.top - _margin.bottom));
}

void GRoot::_enter_tree()
{
    GComponent::_enter_tree();
    _inst = this;

    if (_displayObject && _displayObject->is_inside_tree())
    {
        Viewport* viewport = _displayObject->get_viewport();
        if (viewport && !_viewportSizeConnected)
        {
            viewport->connect("size_changed", callable_mp(this, &GRoot::onWindowSizeChanged));
            _viewportSizeConnected = true;
        }
        onWindowSizeChanged();
    }
}

void GRoot::_exit_tree()
{
    if (_displayObject)
    {
        Viewport* viewport = _displayObject->get_viewport();
        if (viewport && _viewportSizeConnected)
        {
            viewport->disconnect("size_changed", callable_mp(this, &GRoot::onWindowSizeChanged));
            _viewportSizeConnected = false;
        }
    }

    FUIContainer* fc = Object::cast_to<FUIContainer>(_displayObject);
    if (fc)
    {
        fc->set_process(false);
        fc->_processCallback = nullptr;
    }
    TweenManager::clean();

    GComponent::_exit_tree();
    if (_inst == this)
        _inst = nullptr;
}

bool GRoot::initWithParent(Node* parent, int zOrder)
{
    if (!GComponent::init())
        return false;

    if (_inst == nullptr)
        _inst = this;

    _inputProcessor = new InputProcessor(this);
    _inputProcessor->setCaptureCallback([this](int eventType) {
        onTouchEvent(eventType);
    });

    return true;
}

void GRoot::onInitWithParent(Node* parent, int zOrder, bool deferAdd)
{
    FUIContainer* fc = Object::cast_to<FUIContainer>(_displayObject);
    if (fc)
    {
        fc->set_process(true);
        fc->_processCallback = [](float dt) {
            TweenManager::update(dt);
            GRoot* root = GRoot::getInstance();
            if (root && root->getInputProcessor())
                root->getInputProcessor()->onFrameUpdate();
        };
    }

    if (parent)
    {
        if (deferAdd) {
            parent->call_deferred("add_child", _displayObject);
            if (zOrder > 0 && zOrder < parent->get_child_count())
                parent->call_deferred("move_child", _displayObject, zOrder);
        } else {
            parent->add_child(_displayObject);
            if (zOrder > 0 && zOrder < parent->get_child_count())
                parent->move_child(_displayObject, zOrder);
        }
    }
}

void GRoot::onWindowSizeChanged()
{
    if (!_displayObject || !_displayObject->is_inside_tree())
        return;

    if (_hasDesignResolution)
    {
        applyContentScale();
    }
    else
    {
        Viewport* viewport = _displayObject->get_viewport();
        if (viewport)
        {
            Rect2 visibleRect = viewport->get_visible_rect();
            setSize(visibleRect.size.x, visibleRect.size.y);
        }
    }

    updateContentScaleLevel();
}

Node* GRoot::gd_getDisplayObject()
{
    return _displayObject;
}

void GRoot::updateContentScaleLevel()
{
    if (!_hasDesignResolution)
    {
        contentScaleLevel = 0;
        return;
    }

    Viewport* viewport = _displayObject->get_viewport();
    if (!viewport)
    {
        contentScaleLevel = 0;
        return;
    }

    Rect2 visibleRect = viewport->get_visible_rect();
    float screenW = visibleRect.size.x;
    float screenH = visibleRect.size.y;
    if (screenW <= 0 || screenH <= 0)
    {
        contentScaleLevel = 0;
        return;
    }

    float designW = _designResolution.x;
    float designH = _designResolution.y;

    float scaleX, scaleY;
    switch (_screenMatchMode)
    {
    case ScreenMatchMode::MatchWidth:
        scaleX = scaleY = screenW / designW;
        break;
    case ScreenMatchMode::MatchHeight:
        scaleX = scaleY = screenH / designH;
        break;
    case ScreenMatchMode::MatchFill:
        scaleX = screenW / designW;
        scaleY = screenH / designH;
        break;
    case ScreenMatchMode::MatchWidthOrHeight:
    default:
        scaleX = scaleY = std::min(screenW / designW, screenH / designH);
        break;
    }

    // Use the larger of the two scales to decide atlas resolution level
    float atlasScale = std::max(scaleX, scaleY);

    if (_displayObject)
        ((Node2D*)_displayObject)->set_scale(Vector2(scaleX, scaleY));

    if (atlasScale >= 3.5f)
        contentScaleLevel = 3; //x4
    else if (atlasScale >= 2.5f)
        contentScaleLevel = 2; //x3
    else if (atlasScale >= 1.5f)
        contentScaleLevel = 1; //x2
    else
        contentScaleLevel = 0;
}

void GRoot::setContentScaleFactor(int designWidth, int designHeight, int matchMode)
{
    _designResolution = Vector2((float)designWidth, (float)designHeight);
    _screenMatchMode = static_cast<ScreenMatchMode>(matchMode);
    _hasDesignResolution = true;
    applyContentScale();
    updateContentScaleLevel();
}

void GRoot::applyContentScale()
{
    if (!_hasDesignResolution || !_displayObject)
        return;

    Node2D* node = (Node2D*)_displayObject;

    if (!node->is_inside_tree())
    {
        // Not in tree yet — connect one-shot to retry when it enters
        node->connect("tree_entered", callable_mp(this, &GRoot::applyContentScale), Object::CONNECT_ONE_SHOT);
        return;
    }

    Viewport* viewport = node->get_viewport();
    if (!viewport)
        return;

    Rect2 visibleRect = viewport->get_visible_rect();
    float screenW = visibleRect.size.x;
    float screenH = visibleRect.size.y;
    if (screenW <= 0 || screenH <= 0)
        return;

    float designW = _designResolution.x;
    float designH = _designResolution.y;

    float scaleX, scaleY;
    float newW, newH;
    switch (_screenMatchMode)
    {
    case ScreenMatchMode::MatchWidth:
        scaleX = scaleY = screenW / designW;
        newW = designW;
        newH = screenH / scaleX;
        break;
    case ScreenMatchMode::MatchHeight:
        scaleX = scaleY = screenH / designH;
        newW = screenW / scaleX;
        newH = designH;
        break;
    case ScreenMatchMode::MatchFill:
        scaleX = screenW / designW;
        scaleY = screenH / designH;
        newW = screenW / scaleX;
        newH = screenH / scaleY;
        break;
    case ScreenMatchMode::MatchWidthOrHeight:
    default:
        scaleX = scaleY = std::min(screenW / designW, screenH / designH);
        newW = designW;
        newH = designH;
        break;
    }

    setSize(newW, newH);
    node->set_scale(Vector2(scaleX, scaleY));

    float offsetX = (screenW - newW * scaleX) * 0.5f;
    float offsetY = (screenH - newH * scaleY) * 0.5f;
    node->set_position(Vector2(offsetX, offsetY));
}

void GRoot::gd_showTooltips(const String& msg) { showTooltips(msg.utf8().get_data()); }
void GRoot::gd_playSound(const String& url, float volumeScale) { playSound(url.utf8().get_data(), volumeScale); }

NS_FGUI_END
