#include "GObject.h"
#include "GGroup.h"
#include "GList.h"
#include "GRoot.h"
#include "UIConfig.h"
#include "UIPackage.h"
#include "display/FUISprite.h"
#include "gears/GearDisplay.h"
#include "gears/GearDisplay2.h"
#include "utils/ByteBuffer.h"
#include "utils/WeakPtr.h"
#include <sstream>
#include <cfloat>

NS_FGUI_BEGIN
GObject* GObject::_draggingObject = nullptr;

static Vector2 sGlobalDragStart;
static Rect sGlobalRect;
static bool sUpdateInDragging;

GObject::GObject() : _scale{1, 1},
                     _sizePercentInGroup(0.0f),
                     _pivotAsAnchor(false),
                     _alpha(1.0f),
                     _rotation(0.0f),
                     _visible(true),
                     _internalVisible(true),
                     _handlingController(false),
                     _touchable(true),
                     _grayed(false),
                     _finalGrayed(false),
                     _draggable(false),
                     _dragBounds(nullptr),
                     _dragTesting(false),
                     _sortingOrder(0),
                     _focusable(false),
                     _pixelSnapping(false),
                     _group(nullptr),
                     _parent(nullptr),
                     _displayObject(nullptr),
                     _sizeImplType(0),
                     _underConstruct(false),
                     _gearLocked(false),
                     _packageItem(nullptr),
                     _data(nullptr),
                     _touchDisabled(false),
                     _alignToBL(false),
                     _weakPtrRef(0)
{
    static uint64_t _gInstanceCounter = 1;
    _uid = _gInstanceCounter++;
    std::stringstream ss;
    ss << _uid;
    id = ss.str();
    _relations = new Relations(this);

    for (int i = 0; i < 10; i++)
        _gears[i] = nullptr;
}

GObject::~GObject()
{
    removeFromParent();

    if (_displayObject)
    {
        _displayObject->queue_free();
        // // CC_SAFE_RELEASE removed - _displayObject managed by Godot ref counting;
    }
    for (int i = 0; i < 10; i++)
        FGUI_DELETE(_gears[i]);
    FGUI_DELETE(_relations);
    FGUI_DELETE(_dragBounds);

    if (_weakPtrRef > 0)
        WeakPtr::markDisposed(this);
}

bool GObject::init()
{
    handleInit();

    if (_displayObject != nullptr)
    {
        ((Node2D*)_displayObject)->set_position(_size * Vector2(0, -1)); // GODOT_ADAPT: anchor->offset
        // _displayObject->setOnEnterCallback([this]() { GObject::onEnter(); });
        // _displayObject->setOnExitCallback([this]() { GObject::onExit(); });
    }
    return true;
}

void GObject::setX(float value)
{
    setPosition(value, _position.y);
}

void GObject::setY(float value)
{
    setPosition(_position.x, value);
}

void GObject::setPosition(float xv, float yv)
{
    if (_position.x != xv || _position.y != yv)
    {
        float dx = xv - _position.x;
        float dy = yv - _position.y;
        _position.x = xv;
        _position.y = yv;

        handlePositionChanged();

        GGroup* g = dynamic_cast<GGroup*>(this);
        if (g != nullptr)
            g->moveChildren(dx, dy);

        updateGear(1);

        if (_parent != nullptr && dynamic_cast<GList*>(_parent) == nullptr)
        {
            _parent->setBoundsChangedFlag();
            if (_group != nullptr)
                _group->setBoundsChangedFlag(true);

            dispatchEvent(UIEventType::PositionChange);
        }

        if (_draggingObject == this && !sUpdateInDragging)
            sGlobalRect = localToGlobal(Rect(Vector2(), _size));
    }
}

float GObject::getXMin() const
{
    return _pivotAsAnchor ? (_position.x - _size.width * _pivot.x) : _position.x;
}

void GObject::setXMin(float value)
{
    if (_pivotAsAnchor)
        setPosition(value + _size.width * _pivot.x, _position.y);
    else
        setPosition(value, _position.y);
}

float GObject::getYMin() const
{
    return _pivotAsAnchor ? (_position.y - _size.height * _pivot.y) : _position.y;
}

void GObject::setYMin(float value)
{
    if (_pivotAsAnchor)
        setPosition(_position.x, value + _size.height * _pivot.y);
    else
        setPosition(_position.x, value);
}

void GObject::setPixelSnapping(bool value)
{
    if (_pixelSnapping != value)
    {
        _pixelSnapping = value;
        handlePositionChanged();
    }
}

void GObject::setSize(float wv, float hv, bool ignorePivot /*= false*/)
{
    if (_rawSize.width != wv || _rawSize.height != hv)
    {
        _rawSize.width = wv;
        _rawSize.height = hv;
        if (wv < minSize.width)
            wv = minSize.width;
        else if (maxSize.width > 0 && wv > maxSize.width)
            wv = maxSize.width;
        if (hv < minSize.height)
            hv = minSize.height;
        else if (maxSize.height > 0 && hv > maxSize.height)
            hv = maxSize.height;
        float dWidth = wv - _size.width;
        float dHeight = hv - _size.height;
        _size.width = wv;
        _size.height = hv;

        handleSizeChanged();

        if (_pivot.x != 0 || _pivot.y != 0)
        {
            if (!_pivotAsAnchor)
            {
                if (!ignorePivot)
                    setPosition(_position.x - _pivot.x * dWidth, _position.y - _pivot.y * dHeight);
                else
                    handlePositionChanged();
            }
            else
                handlePositionChanged();
        }
        else
            handlePositionChanged();

        GGroup* g = dynamic_cast<GGroup*>(this);
        if (g != nullptr)
            g->resizeChildren(dWidth, dHeight);

        updateGear(2);

        if (_parent != nullptr)
        {
            _relations->onOwnerSizeChanged(dWidth, dHeight, _pivotAsAnchor || !ignorePivot);
            _parent->setBoundsChangedFlag();
            if (_group != nullptr)
                _group->setBoundsChangedFlag();
        }

        dispatchEvent(UIEventType::SizeChange);
    }
}

void GObject::setSizeDirectly(float wv, float hv)
{
    _rawSize.width = wv;
    _rawSize.height = hv;
    if (wv < 0)
        wv = 0;
    if (hv < 0)
        hv = 0;
    _size.width = wv;
    _size.height = hv;
}

void GObject::center(bool restraint /*= false*/)
{
    GComponent* r;
    if (_parent != nullptr)
        r = _parent;
    else
        r = GRoot::getInstance();

    setPosition((int)((r->_size.width - _size.width) / 2), (int)((r->_size.height - _size.height) / 2));
    if (restraint)
    {
        addRelation(r, RelationType::Center_Center);
        addRelation(r, RelationType::Middle_Middle);
    }
}

void GObject::makeFullScreen()
{
    setSize(GRoot::getInstance()->getWidth(), GRoot::getInstance()->getHeight());
}

void GObject::setPivot(float xv, float yv, bool asAnchor)
{
    if (_pivot.x != xv || _pivot.y != yv || _pivotAsAnchor != asAnchor)
    {
        _pivot = Vector2(xv, yv);
        _pivotAsAnchor = asAnchor;
        if (_displayObject != nullptr)
            ((Node2D*)_displayObject)->set_position(Vector2(_pivot.x, 1 - _pivot.y) * _size * Vector2(1,-1)); // GODOT_ADAPT: anchor->offset
        handlePositionChanged(); 
    }
}

void GObject::setScale(float xv, float yv)
{
    if (_scale.x != xv || _scale.y != yv)
    {
        _scale.x = xv;
        _scale.y = yv;
        handleScaleChanged();

        updateGear(2);
    }
}

void GObject::setSkewX(float value)
{
    _skewX = value;
}

void GObject::setSkewY(float value)
{
    _skewY = value;
}

void GObject::setRotation(float value)
{
    if (_rotation != value)
    {
        _rotation = value;
        ((Node2D*)_displayObject)->set_rotation(_rotation);
        updateGear(3);
    }
}

void GObject::setAlpha(float value)
{
    if (_alpha != value)
    {
        _alpha = value;
        handleAlphaChanged();
        updateGear(3);
    }
}

void GObject::setGrayed(bool value)
{
    if (_grayed != value || _finalGrayed != value)
    {
        _grayed = value;
        handleGrayedChanged();
        updateGear(3);
    }
}

void GObject::setVisible(bool value)
{
    if (_visible != value)
    {
        _visible = value;
        handleVisibleChanged();
        if (_parent != nullptr)
            _parent->setBoundsChangedFlag();
        if (_group != nullptr && _group->isExcludeInvisibles())
            _group->setBoundsChangedFlag();
    }
}

bool GObject::internalVisible() const
{
    return _internalVisible && (_group == nullptr || _group->internalVisible());
}

bool GObject::internalVisible2() const
{
    return _visible && (_group == nullptr || _group->internalVisible2());
}

bool GObject::internalVisible3() const
{
    return _visible && _internalVisible;
}

void GObject::setTouchable(bool value)
{
    _touchable = value;
}

void GObject::setSortingOrder(int value)
{
    if (value < 0)
        value = 0;
    if (_sortingOrder != value)
    {
        int old = _sortingOrder;
        _sortingOrder = value;
        if (_parent != nullptr)
            _parent->childSortingOrderChanged(this, old, _sortingOrder);
    }
}

void GObject::setGroup(GGroup* value)
{
    if (_group != value)
    {
        if (_group != nullptr)
            _group->setBoundsChangedFlag();
        _group = value;
        if (_group != nullptr)
            _group->setBoundsChangedFlag();
        handleVisibleChanged();
        if (_parent)
            _parent->childStateChanged(this);
    }
}

const std::string& GObject::getText() const
{
    static const std::string empty;
    return empty;
}

void GObject::setText(const std::string& text)
{
}

const std::string& GObject::getIcon() const
{
    static const std::string empty;
    return empty;
}

void GObject::setIcon(const std::string& text)
{
}

void GObject::setTooltips(const std::string& value)
{
    _tooltips = value;
    if (!_tooltips.empty())
    {
        addEventListener(UIEventType::RollOver, [this](EventContext* ctx) { GObject::onRollOver(ctx); }, EventTag(this));
        addEventListener(UIEventType::RollOut, [this](EventContext* ctx) { GObject::onRollOut(ctx); }, EventTag(this));
    }
}

void GObject::onRollOver(EventContext* context)
{
    getRoot()->showTooltips(_tooltips);
}

void GObject::onRollOut(EventContext* context)
{
    getRoot()->hideTooltips();
}

void GObject::setDraggable(bool value)
{
    if (_draggable != value)
    {
        _draggable = value;
        initDrag();
    }
}

void GObject::setDragBounds(const Rect2& value)
{
    if (_dragBounds == nullptr)
        _dragBounds = new Rect();
    *_dragBounds = value;
}

void GObject::startDrag(int touchId)
{
    dragBegin(touchId);
}

void GObject::stopDrag()
{
    dragEnd();
}

std::string GObject::getResourceURL() const
{
    if (_packageItem != nullptr)
        return "ui://" + _packageItem->owner->getId() + _packageItem->id;
    else
        return "";
}

Vector2 GObject::localToGlobal(const Vector2& pt)
{
    Vector2 pt2 = pt;
    if (_pivotAsAnchor)
    {
        pt2.x += _size.width * _pivot.x;
        pt2.y += _size.height * _pivot.y;
    }
    pt2.y = _size.height - pt2.y;
    pt2 = ((Node2D*)_displayObject)->to_global(pt2);
    return GRoot::getInstance()->worldToRoot(pt2);
}

Rect2 GObject::localToGlobal(const Rect2& rect)
{
    Rect ret;
    Vector2 v = localToGlobal(rect.position);
    ret.position.x = v.x;
    ret.position.y = v.y;
    v = localToGlobal(Vector2(rect.position.x + rect.size.x, rect.position.y + rect.size.y));
    ret.size.x = v.x - ret.position.x;
    ret.size.y = v.y - ret.position.y;
    return ret;
}

Vector2 GObject::globalToLocal(const Vector2& pt)
{
    Vector2 pt2 = GRoot::getInstance()->rootToWorld(pt);
    pt2 = ((Node2D*)_displayObject)->to_local(pt2);
    pt2.y = _size.height - pt2.y;
    if (_pivotAsAnchor)
    {
        pt2.x -= _size.width * _pivot.x;
        pt2.y -= _size.height * _pivot.y;
    }
    return pt2;
}

Rect2 GObject::globalToLocal(const Rect2& rect)
{
    Rect ret;
    Vector2 v = globalToLocal(rect.position);
    ret.position.x = v.x;
    ret.position.y = v.y;
    v = globalToLocal(Vector2(rect.position.x + rect.size.x, rect.position.y + rect.size.y));
    ret.size.x = v.x - ret.position.x;
    ret.size.y = v.y - ret.position.y;
    return ret;
}

Rect2 GObject::transformRect(const Rect2& rect, GObject* targetSpace)
{
    if (targetSpace == this)
        return rect;

    if (targetSpace == _parent) // optimization
    {
        return Rect((_position.x + rect.position.x) * _scale.x,
                    (_position.y + rect.position.y) * _scale.y,
                    rect.size.x * _scale.x,
                    rect.size.y * _scale.y);
    }
    else
    {
        float result[4]{FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};

        transformRectPoint(rect.position, result, targetSpace);
        transformRectPoint(Vector2(rect.position.x + rect.size.x, rect.position.y), result, targetSpace);
        transformRectPoint(Vector2(rect.position.x, rect.position.y + rect.size.y), result, targetSpace);
        transformRectPoint(Vector2(rect.position.x + rect.size.x, rect.position.y + rect.size.y), result, targetSpace);

        return Rect(result[0], result[1], result[2] - result[0], result[3] - result[1]);
    }
}

void GObject::transformRectPoint(const Vector2& pt, float rect[], GObject* targetSpace)
{
    Vector2 v = localToGlobal(pt);
    if (targetSpace != nullptr)
        v = targetSpace->globalToLocal(v);

    if (rect[0] > v.x)
        rect[0] = v.x;
    if (rect[2] < v.x)
        rect[2] = v.x;
    if (rect[1] > v.y)
        rect[1] = v.y;
    if (rect[3] < v.y)
        rect[3] = v.y;
}

void GObject::addRelation(GObject* target, RelationType relationType, bool usePercent)
{
    _relations->add(target, relationType, usePercent);
}

void GObject::removeRelation(GObject* target, RelationType relationType)
{
    _relations->remove(target, relationType);
}

GearBase* GObject::getGear(int index)
{
    GearBase* gear = _gears[index];
    if (gear == nullptr)
    {
        gear = GearBase::create(this, index);
        _gears[index] = gear;
    }
    return gear;
}

void GObject::updateGear(int index)
{
    if (_underConstruct || _gearLocked)
        return;

    GearBase* gear = _gears[index];
    if (gear != nullptr && gear->getController() != nullptr)
        gear->updateState();
}

bool GObject::checkGearController(int index, GController* c)
{
    return _gears[index] != nullptr && _gears[index]->getController() == c;
}

void GObject::updateGearFromRelations(int index, float dx, float dy)
{
    if (_gears[index] != nullptr)
        _gears[index]->updateFromRelations(dx, dy);
}

uint32_t GObject::addDisplayLock()
{
    GearDisplay* gearDisplay = (GearDisplay*)_gears[0];
    if (gearDisplay != nullptr && gearDisplay->getController() != nullptr)
    {
        uint32_t ret = gearDisplay->addLock();
        checkGearDisplay();

        return ret;
    }
    else
        return 0;
}

void GObject::releaseDisplayLock(uint32_t token)
{
    GearDisplay* gearDisplay = (GearDisplay*)_gears[0];
    if (gearDisplay != nullptr && gearDisplay->getController() != nullptr)
    {
        gearDisplay->releaseLock(token);
        checkGearDisplay();
    }
}

void GObject::checkGearDisplay()
{
    if (_handlingController)
        return;

    bool connected = _gears[0] == nullptr || ((GearDisplay*)_gears[0])->isConnected();
    if (_gears[8] != nullptr)
        connected = dynamic_cast<GearDisplay2*>(_gears[8])->evaluate(connected);

    if (connected != _internalVisible)
    {
        _internalVisible = connected;
        if (_parent != nullptr)
            _parent->childStateChanged(this);
        if (_group != nullptr && _group->isExcludeInvisibles())
            _group->setBoundsChangedFlag();
    }
}

bool GObject::onStage() const
{
    return _displayObject->get_tree() != nullptr;
}

GObject* GObject::findParent() const
{
    if (_parent != nullptr)
        return _parent;

    Node* pn = _displayObject->get_parent();
    if (pn == nullptr)
        return nullptr;

    while (pn != nullptr)
    {
        FUIContainer* fc = dynamic_cast<FUIContainer*>(pn);
        if (fc != nullptr && fc->gOwner)
            return fc->gOwner;

        pn = pn->get_parent();
    }

    return nullptr;
}

GRoot* GObject::getRoot() const
{
    GObject* p = (GObject*)this;
    while (p->_parent != nullptr)
        p = p->_parent;

    GRoot* root = dynamic_cast<GRoot*>(p);
    if (root != nullptr)
        return root;
    else
        return GRoot::getInstance();
}

void GObject::removeFromParent()
{
    if (_parent != nullptr)
        _parent->removeChild(this);
}

Variant GObject::getProp(ObjectPropID propId)
{
    switch (propId)
    {
    case ObjectPropID::Text:
        return Variant(getText().c_str());
    case ObjectPropID::Icon:
        return Variant(getIcon().c_str());
    default:
        return Variant();
    }
}

void GObject::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Text:
        return setText(std::string((const char*)value.stringify().utf8().ptr()));
    case ObjectPropID::Icon:
        return setIcon(std::string((const char*)value.stringify().utf8().ptr()));
    default:
        break;
    }
}

void GObject::constructFromResource()
{
}

GObject* GObject::hitTest(const Vector2& worldPoint, const Camera2D* camera)
{
    if (_touchDisabled || !_touchable || !((CanvasItem*)_displayObject)->is_visible() || !_displayObject->get_parent())
        return nullptr;

    Rect rect;
    rect.size = _size;
    //if (isScreenPointInRect(worldPoint, camera, _displayObject->getWorldToNodeTransform(), rect, nullptr))
    if (rect.has_point(((Node2D*)_displayObject)->to_local(worldPoint)))
        return this;
    else
        return nullptr;
}

void GObject::handleInit()
{
    _displayObject = memnew(FUISprite);
}

void GObject::_enter_tree()
{
    dispatchEvent(UIEventType::Enter);
}

void GObject::_exit_tree()
{
    dispatchEvent(UIEventType::Exit);
}

void GObject::_ready()
{
}

void GObject::_process(double delta)
{
}

void GObject::dispose()
{
}

void GObject::addChild(Node* node)
{
    if (_displayObject)
        _displayObject->add_child(node);
}

void GObject::removeChild(Node* node)
{
    if (_displayObject)
        _displayObject->remove_child(node);
}

void GObject::handlePositionChanged()
{
    if (_displayObject)
    {
        Vector2 pt = _position;
        pt.y = -pt.y;
        if (!_pivotAsAnchor)
        {
            pt.x += _size.width * _pivot.x;
            pt.y -= _size.height * _pivot.y;
        }
        if (_alignToBL)
        {
            if (_displayObject->get_parent())
                pt.y += 0; // GODOT_ADAPT: getContentSize.height (TODO)
            else if (_parent != nullptr)
                pt.y += _parent->getSize().height; // GODOT_ADAPT: getContentSize.height
        }
        if (_pixelSnapping)
        {
            pt.x = (int)pt.x;
            pt.y = (int)pt.y;
        }
        ((Node2D*)_displayObject)->set_position(pt);
    }
}

void GObject::handleSizeChanged()
{
    if (_displayObject)
    {
        if (_sizeImplType == 0 || sourceSize.width == 0 || sourceSize.height == 0)
            ((Node2D*)_displayObject)->set_scale(Vector2(1, 1)); // GODOT_ADAPT: setContentSize _size (TODO)
        else
            ((Node2D*)_displayObject)->set_scale(Vector2(_scale.x * _size.width / sourceSize.width, _scale.y * _size.height / sourceSize.height));
    }
}

void GObject::handleScaleChanged()
{
    if (_sizeImplType == 0 || sourceSize.width == 0 || sourceSize.height == 0)
        ((Node2D*)_displayObject)->set_scale(Vector2(_scale.x, _scale.y));
    else
        ((Node2D*)_displayObject)->set_scale(Vector2(_scale.x * _size.width / sourceSize.width, _scale.y * _size.height / sourceSize.height));
}

void GObject::handleAlphaChanged()
{
    ((CanvasItem*)_displayObject)->set_self_modulate(Color(1, 1, 1, _alpha));
}

void GObject::handleGrayedChanged()
{
    _finalGrayed = (_parent && _parent->_finalGrayed) || _grayed;
}

void GObject::handleVisibleChanged()
{
    ((CanvasItem*)_displayObject)->set_visible(internalVisible2());
}

void GObject::handleControllerChanged(GController* c)
{
    _handlingController = true;
    for (int i = 0; i < 10; i++)
    {
        GearBase* gear = _gears[i];
        if (gear != nullptr && gear->getController() == c)
            gear->apply();
    }
    _handlingController = false;

    checkGearDisplay();
}

void GObject::setup_beforeAdd(ByteBuffer* buffer, int beginPos)
{
    buffer->seek(beginPos, 0);
    buffer->skip(5);

    id = buffer->readS();
    name = buffer->readS();
    float f1 = buffer->readInt();
    float f2 = buffer->readInt();
    setPosition(f1, f2);

    if (buffer->readBool())
    {
        initSize.width = buffer->readInt();
        initSize.height = buffer->readInt();
        setSize(initSize.width, initSize.height, true);
    }

    if (buffer->readBool())
    {
        minSize.width = buffer->readInt();
        maxSize.width = buffer->readInt();
        minSize.height = buffer->readInt();
        maxSize.height = buffer->readInt();
    }

    if (buffer->readBool())
    {
        f1 = buffer->readFloat();
        f2 = buffer->readFloat();
        setScale(f1, f2);
    }

    if (buffer->readBool())
    {
        f1 = buffer->readFloat();
        f2 = buffer->readFloat();
        setSkewX(f1);
        setSkewY(f2);
    }

    if (buffer->readBool())
    {
        f1 = buffer->readFloat();
        f2 = buffer->readFloat();
        setPivot(f1, f2, buffer->readBool());
    }

    f1 = buffer->readFloat();
    if (f1 != 1)
        setAlpha(f1);

    f1 = buffer->readFloat();
    if (f1 != 0)
        setRotation(f1);

    if (!buffer->readBool())
        setVisible(false);
    if (!buffer->readBool())
        setTouchable(false);
    if (buffer->readBool())
        setGrayed(true);
    buffer->readByte(); //blendMode
    buffer->readByte(); //filter

    const std::string& str = buffer->readS();
    if (!str.empty())
        _customData = Variant(str.c_str());
}

void GObject::setup_afterAdd(ByteBuffer* buffer, int beginPos)
{
    buffer->seek(beginPos, 1);

    const std::string& str = buffer->readS();
    if (!str.empty())
        setTooltips(str);

    int groupId = buffer->readShort();
    if (groupId >= 0)
        _group = dynamic_cast<GGroup*>(_parent->getChildAt(groupId));

    buffer->seek(beginPos, 2);

    int cnt = buffer->readShort();
    for (int i = 0; i < cnt; i++)
    {
        int nextPos = buffer->readUshort();
        nextPos += buffer->getPos();

        GearBase* gear = getGear(buffer->readByte());
        gear->setup(buffer);

        buffer->setPos(nextPos);
    }
}

void GObject::initDrag()
{
    if (_draggable)
    {
        addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { GObject::onTouchBegin(ctx); }, EventTag(this));
        addEventListener(UIEventType::TouchMove, [this](EventContext* ctx) { GObject::onTouchMove(ctx); }, EventTag(this));
        addEventListener(UIEventType::TouchEnd, [this](EventContext* ctx) { GObject::onTouchEnd(ctx); }, EventTag(this));
    }
    else
    {
        removeEventListener(UIEventType::TouchBegin, EventTag(this));
        removeEventListener(UIEventType::TouchBegin, EventTag(this));
        removeEventListener(UIEventType::TouchMove, EventTag(this));
        removeEventListener(UIEventType::TouchEnd, EventTag(this));
    }
}

void GObject::dragBegin(int touchId)
{
    if (_draggingObject != nullptr)
    {
        GObject* tmp = _draggingObject;
        _draggingObject->stopDrag();
        _draggingObject = nullptr;
        tmp->dispatchEvent(UIEventType::DragEnd);
    }

    sGlobalDragStart = GRoot::getInstance()->getTouchPosition(touchId);
    sGlobalRect = localToGlobal(Rect(Vector2(), _size));

    _draggingObject = this;
    _dragTesting = true;
    GRoot::getInstance()->getInputProcessor()->addTouchMonitor(touchId, this);

    addEventListener(UIEventType::TouchMove, [this](EventContext* ctx) { GObject::onTouchMove(ctx); }, EventTag(this));
    addEventListener(UIEventType::TouchEnd, [this](EventContext* ctx) { GObject::onTouchEnd(ctx); }, EventTag(this));
}

void GObject::dragEnd()
{
    if (_draggingObject == this) {
        _draggingObject = nullptr;
    }
}

void GObject::onTouchBegin(EventContext* context)
{
    _dragTouchStartPos = context->getInput()->getPosition();
    _dragTesting = true;
    context->captureTouch();
}

void GObject::onTouchMove(EventContext* context)
{
    InputEvent* evt = context->getInput();

    if (_draggingObject != this && _draggable && _dragTesting)
    {
        int sensitivity;
#ifdef CC_PLATFORM_PC
        sensitivity = UIConfig::clickDragSensitivity;
#else
        sensitivity = UIConfig::touchDragSensitivity;
#endif
        if (std::abs(_dragTouchStartPos.x - evt->getPosition().x) < sensitivity && std::abs(_dragTouchStartPos.y - evt->getPosition().y) < sensitivity)
            return;

        _dragTesting = false;
        if (!dispatchEvent(UIEventType::DragStart))
            dragBegin(evt->getTouchId());
    }

    if (_draggingObject == this)
    {
        float xx = evt->getPosition().x - sGlobalDragStart.x + sGlobalRect.position.x;
        float yy = evt->getPosition().y - sGlobalDragStart.y + sGlobalRect.position.y;

        if (_dragBounds != nullptr)
        {
            Rect rect = GRoot::getInstance()->localToGlobal(*_dragBounds);
            if (xx < rect.position.x)
                xx = rect.position.x;
            else if (xx + sGlobalRect.size.x > rect.position.x + rect.size.x)
            {
                xx = rect.position.x + rect.size.x - sGlobalRect.size.x;
                if (xx < rect.position.x)
                    xx = rect.position.x;
            }

            if (yy < rect.position.y)
                yy = rect.position.y;
            else if (yy + sGlobalRect.size.y > rect.position.y + rect.size.y)
            {
                yy = rect.position.y + rect.size.y - sGlobalRect.size.y;
                if (yy < rect.position.y)
                    yy = rect.position.y;
            }
        }

        Vector2 pt = _parent->globalToLocal(Vector2(xx, yy));

        sUpdateInDragging = true;
        setPosition(round(pt.x), round(pt.y));
        sUpdateInDragging = false;

        dispatchEvent(UIEventType::DragMove);
    }
}

void GObject::onTouchEnd(EventContext* context)
{
    if (_draggingObject == this)
    {
        _draggingObject = nullptr;
        dispatchEvent(UIEventType::DragEnd);
    }
}

GObject* GObject::create() { Ref<GObject> ref = memnew(GObject); auto* o = ref.ptr(); o->reference(); return o; }

void GObject::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setX", "value"), &GObject::setX);
    ClassDB::bind_method(D_METHOD("getX"), &GObject::getX);

    ClassDB::bind_method(D_METHOD("setY", "value"), &GObject::setY);
    ClassDB::bind_method(D_METHOD("getY"), &GObject::getY);

    ClassDB::bind_method(D_METHOD("setPosition", "x", "y"), &GObject::setPosition);
    ClassDB::bind_method(D_METHOD("getPosition"), &GObject::getPosition);

    ClassDB::bind_method(D_METHOD("setWidth", "value"), &GObject::setWidth);
    ClassDB::bind_method(D_METHOD("getWidth"), &GObject::getWidth);

    ClassDB::bind_method(D_METHOD("setHeight", "value"), &GObject::setHeight);
    ClassDB::bind_method(D_METHOD("getHeight"), &GObject::getHeight);

    ClassDB::bind_method(D_METHOD("setSize", "width", "height", "ignore_pivot"), &GObject::setSize, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("getSize"), &GObject::getSize);

    ClassDB::bind_method(D_METHOD("setPixelSnapping", "value"), &GObject::setPixelSnapping);
    ClassDB::bind_method(D_METHOD("isPixelSnapping"), &GObject::isPixelSnapping);

    ClassDB::bind_method(D_METHOD("setScaleX", "value"), &GObject::setScaleX);
    ClassDB::bind_method(D_METHOD("getScaleX"), &GObject::getScaleX);
    ClassDB::bind_method(D_METHOD("setScaleY", "value"), &GObject::setScaleY);
    ClassDB::bind_method(D_METHOD("getScaleY"), &GObject::getScaleY);

    ClassDB::bind_method(D_METHOD("setSkewX", "value"), &GObject::setSkewX);
    ClassDB::bind_method(D_METHOD("getSkewX"), &GObject::getSkewX);
    ClassDB::bind_method(D_METHOD("setSkewY", "value"), &GObject::setSkewY);
    ClassDB::bind_method(D_METHOD("getSkewY"), &GObject::getSkewY);

    ClassDB::bind_method(D_METHOD("setRotation", "value"), &GObject::setRotation);
    ClassDB::bind_method(D_METHOD("getRotation"), &GObject::getRotation);

    ClassDB::bind_method(D_METHOD("setAlpha", "value"), &GObject::setAlpha);
    ClassDB::bind_method(D_METHOD("getAlpha"), &GObject::getAlpha);

    ClassDB::bind_method(D_METHOD("setGrayed", "value"), &GObject::setGrayed);
    ClassDB::bind_method(D_METHOD("isGrayed"), &GObject::isGrayed);

    ClassDB::bind_method(D_METHOD("setVisible", "value"), &GObject::setVisible);
    ClassDB::bind_method(D_METHOD("isVisible"), &GObject::isVisible);

    ClassDB::bind_method(D_METHOD("setTouchable", "value"), &GObject::setTouchable);
    ClassDB::bind_method(D_METHOD("isTouchable"), &GObject::isTouchable);

    ClassDB::bind_method(D_METHOD("setSortingOrder", "value"), &GObject::setSortingOrder);
    ClassDB::bind_method(D_METHOD("getSortingOrder"), &GObject::getSortingOrder);

    ClassDB::bind_method(D_METHOD("center"), &GObject::center, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("makeFullScreen"), &GObject::makeFullScreen);

    ClassDB::bind_method(D_METHOD("setText", "text"), &GObject::gd_setText);
    ClassDB::bind_method(D_METHOD("getText"), &GObject::gd_getText);

    ClassDB::bind_method(D_METHOD("setTooltips", "text"), &GObject::gd_setTooltips);
    ClassDB::bind_method(D_METHOD("getTooltips"), &GObject::gd_getTooltips);

    ClassDB::bind_method(D_METHOD("setDraggable", "value"), &GObject::setDraggable);
    ClassDB::bind_method(D_METHOD("isDraggable"), &GObject::isDraggable);

    ClassDB::bind_method(D_METHOD("startDrag", "touch_id"), &GObject::startDrag, DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("stopDrag"), &GObject::stopDrag);

    ClassDB::bind_method(D_METHOD("getResourceURL"), &GObject::gd_getResourceURL);

    // get_parent returns GComponent* which can't be bound directly
    ClassDB::bind_method(D_METHOD("removeFromParent"), &GObject::removeFromParent);
    ClassDB::bind_method(D_METHOD("onStage"), &GObject::onStage);

    ClassDB::bind_method(D_METHOD("localToGlobal", "pt"), &GObject::gd_localToGlobal);
    ClassDB::bind_method(D_METHOD("globalToLocal", "pt"), &GObject::gd_globalToLocal);

    ClassDB::bind_method(D_METHOD("addChild", "node"), &GObject::addChild);
    ClassDB::bind_method(D_METHOD("removeChild", "node"), &GObject::removeChild);
}

void GObject::gd_setText(const String& text) { setText(text.utf8().get_data()); }
String GObject::gd_getText() const { return String(getText().c_str()); }
void GObject::gd_setTooltips(const String& value) { setTooltips(value.utf8().get_data()); }
String GObject::gd_getTooltips() const { return String(getTooltips().c_str()); }
String GObject::gd_getResourceURL() const { return String(getResourceURL().c_str()); }

NS_FGUI_END
