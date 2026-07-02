#include "window.h"
#include "GRoot.h"
#include "UIPackage.h"
#include "UIConfig.h"

NS_FGUI_BEGIN
GWindow::GWindow() :
    _requestingCmd(0),
    _frame(nullptr),
    _contentPane(nullptr),
    _modalWaitPane(nullptr),
    _closeButton(nullptr),
    _dragArea(nullptr),
    _contentArea(nullptr),
    _modal(false),
    _inited(false),
    _loading(false)
{
    _bringToFontOnClick = UIConfig::bringWindowToFrontOnClick;
}

GWindow::~GWindow()
{
    // // CC_SAFE_RELEASE removed - _contentPane managed by Godot ref counting;
    // CC_SAFE_RELEASE removed - _frame managed by Godot ref counting;
    // CC_SAFE_RELEASE removed - _closeButton managed by Godot ref counting;
    // CC_SAFE_RELEASE removed - _dragArea managed by Godot ref counting;
    // CC_SAFE_RELEASE removed - _modalWaitPane managed by Godot ref counting;
}

void GWindow::_bind_methods()
{
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create"), &GWindow::gd_create);

    ClassDB::bind_method(D_METHOD("show"), &GWindow::show);
    ClassDB::bind_method(D_METHOD("hide"), &GWindow::hide);
    ClassDB::bind_method(D_METHOD("hideImmediately"), &GWindow::hideImmediately);
    ClassDB::bind_method(D_METHOD("toggleStatus"), &GWindow::toggleStatus);
    ClassDB::bind_method(D_METHOD("bringToFront"), &GWindow::bringToFront);

    ClassDB::bind_method(D_METHOD("isShowing"), &GWindow::isShowing);
    ClassDB::bind_method(D_METHOD("isTop"), &GWindow::isTop);

    ClassDB::bind_method(D_METHOD("setModal", "value"), &GWindow::setModal);
    ClassDB::bind_method(D_METHOD("isModal"), &GWindow::isModal);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "modal"), "setModal", "isModal");

    ClassDB::bind_method(D_METHOD("showModalWait", "requesting_cmd"), &GWindow::gd_showModalWait, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("closeModalWait", "requesting_cmd"), &GWindow::gd_closeModalWait, DEFVAL(0));

    ClassDB::bind_method(D_METHOD("initWindow"), &GWindow::initWindow);

    ClassDB::bind_method(D_METHOD("setBringToFrontOnClick", "value"), &GWindow::setBringToFrontOnClick);
    ClassDB::bind_method(D_METHOD("isBringToFrontOnClick"), &GWindow::isBringToFrontOnClick);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bringToFrontOnClick"), "setBringToFrontOnClick", "isBringToFrontOnClick");

    ClassDB::bind_method(D_METHOD("setContentPane", "pane"), &GWindow::setContentPane);
    ClassDB::bind_method(D_METHOD("getContentPane"), &GWindow::getContentPane);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "contentPane", PROPERTY_HINT_NODE_TYPE, "GComponent"), "setContentPane", "getContentPane");

    ClassDB::bind_method(D_METHOD("getFrame"), &GWindow::getFrame);

    ClassDB::bind_method(D_METHOD("setCloseButton", "button"), &GWindow::setCloseButton);
    ClassDB::bind_method(D_METHOD("getCloseButton"), &GWindow::getCloseButton);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "closeButton", PROPERTY_HINT_NODE_TYPE, "GObject"), "setCloseButton", "getCloseButton");

    ClassDB::bind_method(D_METHOD("setDragArea", "area"), &GWindow::setDragArea);
    ClassDB::bind_method(D_METHOD("getDragArea"), &GWindow::getDragArea);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "dragArea", PROPERTY_HINT_NODE_TYPE, "GObject"), "setDragArea", "getDragArea");

    ClassDB::bind_method(D_METHOD("setContentArea", "area"), &GWindow::setContentArea);
    ClassDB::bind_method(D_METHOD("getContentArea"), &GWindow::getContentArea);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "contentArea", PROPERTY_HINT_NODE_TYPE, "GObject"), "setContentArea", "getContentArea");

    ClassDB::bind_method(D_METHOD("getModalWaitingPane"), &GWindow::getModalWaitingPane);

    // GDScript virtual method hooks
    ClassDB::bind_method(D_METHOD("setOnInitCallback", "callback"), &GWindow::setOnInitCallback);
    ClassDB::bind_method(D_METHOD("getOnInitCallback"), &GWindow::getOnInitCallback);
    ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "onInitCallback"), "setOnInitCallback", "getOnInitCallback");

    ClassDB::bind_method(D_METHOD("setOnShownCallback", "callback"), &GWindow::setOnShownCallback);
    ClassDB::bind_method(D_METHOD("getOnShownCallback"), &GWindow::getOnShownCallback);
    ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "onShownCallback"), "setOnShownCallback", "getOnShownCallback");

    ClassDB::bind_method(D_METHOD("setOnHideCallback", "callback"), &GWindow::setOnHideCallback);
    ClassDB::bind_method(D_METHOD("getOnHideCallback"), &GWindow::getOnHideCallback);
    ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "onHideCallback"), "setOnHideCallback", "getOnHideCallback");

    ClassDB::bind_method(D_METHOD("setDoShowAnimationCallback", "callback"), &GWindow::setDoShowAnimationCallback);
    ClassDB::bind_method(D_METHOD("getDoShowAnimationCallback"), &GWindow::getDoShowAnimationCallback);
    ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "doShowAnimationCallback"), "setDoShowAnimationCallback", "getDoShowAnimationCallback");

    ClassDB::bind_method(D_METHOD("setDoHideAnimationCallback", "callback"), &GWindow::setDoHideAnimationCallback);
    ClassDB::bind_method(D_METHOD("getDoHideAnimationCallback"), &GWindow::getDoHideAnimationCallback);
    ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "doHideAnimationCallback"), "setDoHideAnimationCallback", "getDoHideAnimationCallback");
}

void GWindow::handleInit()
{
    GComponent::handleInit();

    addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { GWindow::onTouchBegin(ctx); });
}

void GWindow::setContentPane(GComponent* value)
{
    if (_contentPane != value)
    {
        if (_contentPane != nullptr)
        {
            removeChild(_contentPane);
            _contentPaneRef = Ref<GComponent>();
        }
        _contentPane = value;
        if (_contentPane != nullptr)
        {
            _contentPaneRef = Ref<GComponent>(_contentPane);
            addChild(Ref<GObject>(_contentPaneRef));
            setSize(_contentPane->getWidth(), _contentPane->getHeight());
            _contentPane->addRelation(this, RelationType::Size);
            _frame = dynamic_cast<GComponent*>(_contentPane->getChild("frame"));
            if (_frame != nullptr)
            {
                setCloseButton(_frame->getChild("closeButton"));
                setDragArea(_frame->getChild("dragArea"));
                setContentArea(_frame->getChild("contentArea"));
            }
        }
        else
            _frame = nullptr;
    }
}

GWindow* GWindow::gd_create()
{
    Ref<GWindow> ref = GWindow::create();
    return ref.is_valid() ? ref.ptr() : nullptr;
}

void GWindow::setCloseButton(GObject * value)
{
    if (_closeButton != nullptr)
    {
        _closeButton->removeClickListener(EventTag(this));
        }
    _closeButton = value;
    if (_closeButton != nullptr)
    {
        _closeButton->addClickListener([this](EventContext* ctx) { GWindow::closeEventHandler(ctx); }, EventTag(this));
    }
}

void GWindow::setDragArea(GObject * value)
{
    if (_dragArea != value)
    {
        if (_dragArea != nullptr)
        {
            _dragArea->setDraggable(false);
            _dragArea->removeEventListener(UIEventType::DragStart, EventTag(this));
        }

        _dragArea = value;
        if (_dragArea != nullptr)
        {
            if (dynamic_cast<GGraph*>(_dragArea) && ((GGraph*)_dragArea)->isEmpty())
                ((GGraph*)_dragArea)->drawRect(_dragArea->getWidth(), _dragArea->getHeight(), 0, Color(0, 0, 0, 0), Color(0, 0, 0, 0));
            _dragArea->setDraggable(true);
            _dragArea->addEventListener(UIEventType::DragStart, [this](EventContext* ctx) { GWindow::onDragStart(ctx); }, EventTag(this));
        }
    }
}

void GWindow::show()
{
    if (!_displayObject)
        init();
    GRoot::getInstance()->showWindow(this);
}

void GWindow::hide()
{
    if (isShowing())
        doHideAnimation();
}

void GWindow::hideImmediately()
{
    GRoot::getInstance()->hideWindowImmediately(this);
}

void GWindow::toggleStatus()
{
    if (isTop())
        hide();
    else
        show();
}

void GWindow::bringToFront()
{
    GRoot::getInstance()->bringToFront(this);
}

bool GWindow::isTop() const
{
    return _parent != nullptr && _parent->getChildIndex(this) == _parent->numChildren() - 1;
}

void GWindow::showModalWait(int requestingCmd)
{
    if (requestingCmd != 0)
        _requestingCmd = requestingCmd;

    if (!UIConfig::windowModalWaiting.empty())
    {
        if (_modalWaitPane == nullptr)
        {
            _modalWaitPane = UIPackage::createObjectFromURL(UIConfig::windowModalWaiting).ptr();
            _modalWaitPane;
        }

        layoutModalWaitPane();

        addChild(Ref<GObject>(_modalWaitPane));
    }
}

void GWindow::layoutModalWaitPane()
{
    if (_contentArea != nullptr)
    {
        Vector2 pt = _frame->localToGlobal(Vector2());
        pt = globalToLocal(pt);
        _modalWaitPane->setPosition((int)pt.x + _contentArea->getX(), (int)pt.y + _contentArea->getY());
        _modalWaitPane->setSize(_contentArea->getWidth(), _contentArea->getHeight());
    }
    else
        _modalWaitPane->setSize(_size.width, _size.height);
}

bool GWindow::closeModalWait(int requestingCmd)
{
    if (requestingCmd != 0)
    {
        if (_requestingCmd != requestingCmd)
            return false;
    }
    _requestingCmd = 0;

    if (_modalWaitPane != nullptr && _modalWaitPane->getParent() != nullptr)
        removeChild(_modalWaitPane);

    return true;
}

void GWindow::initWindow()
{
    if (!_displayObject)
        init();

    if (_inited || _loading)
        return;

    if (!_uiSources.empty())
    {
        _loading = false;
        int cnt = (int)_uiSources.size();
        for (int i = 0; i < cnt; i++)
        {
            IUISource* lib = _uiSources.at(i);
            if (!lib->isLoaded())
            {
                lib->load([this]() { onUILoadComplete(); });
                _loading = true;
            }
        }

        if (!_loading)
            _initWindow();
    }
    else
        _initWindow();
}

void GWindow::_initWindow()
{
    _inited = true;
    if (_onInitCallback.is_valid())
        _onInitCallback.call();
    onInit();

    if (isShowing())
        doShowAnimation();
}

void GWindow::addUISource(IUISource * uiSource)
{
    _uiSources.push_back(uiSource);
}

void GWindow::doShowAnimation()
{
    if (_doShowAnimationCallback.is_valid())
        _doShowAnimationCallback.call();
    else
        onShown();
    if (_onShownCallback.is_valid())
        _onShownCallback.call();
}

void GWindow::doHideAnimation()
{
    if (_doHideAnimationCallback.is_valid())
        _doHideAnimationCallback.call();
    else
        hideImmediately();
}

void GWindow::closeEventHandler(EventContext * context)
{
    hide();
}

void GWindow::onUILoadComplete()
{
    int cnt = (int)_uiSources.size();
    for (int i = 0; i < cnt; i++)
    {
        IUISource* lib = _uiSources.at(i);
        if (!lib->isLoaded())
            return;
    }

    _loading = false;
    _initWindow();
}

void GWindow::_enter_tree()
{
    GComponent::_enter_tree();

    if (!_inited)
        initWindow();
    else
        doShowAnimation();
}

void GWindow::_exit_tree()
{
    GComponent::_exit_tree();

    closeModalWait();
    if (_onHideCallback.is_valid())
        _onHideCallback.call();
    onHide();
}

void GWindow::onTouchBegin(EventContext * context)
{
    if (isShowing() && _bringToFontOnClick)
    {
        bringToFront();
    }
}

void GWindow::onDragStart(EventContext * context)
{
    context->preventDefault();

    startDrag(context->getInput()->getTouchId());
}


NS_FGUI_END
