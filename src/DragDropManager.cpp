#include "DragDropManager.h"
#include "UIObjectFactory.h"
#include "GRoot.h"

NS_FGUI_BEGIN
DragDropManager* DragDropManager::_inst = nullptr;

DragDropManager::DragDropManager()
{
    Ref<GObject> obj = UIObjectFactory::newObject(ObjectType::LOADER);
    _agent = Ref<GLoader>(Object::cast_to<GLoader>(obj.ptr()));
    _agent->setTouchable(false);
    _agent->setDraggable(true);
    _agent->setSize(100, 100);
    _agent->setPivot(0.5f, 0.5f, true);
    _agent->setAlign(AlignType::CENTER);
    _agent->setVerticalAlign(VertAlignType::CENTER);
    _agent->setSortingOrder(INT_MAX);
    _agent->addEventListener(UIEventType::DragEnd, [this](EventContext* ctx) { onDragEnd(ctx); });
}

DragDropManager::~DragDropManager()
{
    // // CC_SAFE_RELEASE removed - _agent managed by Godot ref counting;
}

DragDropManager* DragDropManager::getInstance()
{
    if (_inst == nullptr)
        _inst = new DragDropManager();

    return _inst;
}

void DragDropManager::startDrag(const std::string& icon, const Variant& sourceData, int touchPointID)
{
    if (_agent->getParent() != nullptr)
        return;

    _sourceData = sourceData;
    _agent->setURL(icon);
    GRoot::getInstance()->addChild(Ref<GObject>(_agent.ptr()));
    Vector2 pt = GRoot::getInstance()->globalToLocal(GRoot::getInstance()->getTouchPosition(touchPointID));
    _agent->setPosition(pt.x, pt.y);
    _agent->startDrag(touchPointID);
}

void DragDropManager::cancel()
{
    if (_agent->getParent() != nullptr)
    {
        _agent->stopDrag();
        GRoot::getInstance()->removeChild(_agent.ptr());
        _sourceData = Variant();
    }
}

void DragDropManager::onDragEnd(EventContext * context)
{
    if (_agent->getParent() == nullptr) //cancelled
        return;

    GRoot::getInstance()->removeChild(_agent.ptr());

    GObject* obj = GRoot::getInstance()->getTouchTarget();
    while (obj != nullptr)
    {
        if (dynamic_cast<GComponent*>(obj))
        {
            if (obj->hasEventListener(UIEventType::Drop))
            {
                //obj->requestFocus();
                obj->dispatchEvent(UIEventType::Drop, nullptr, _sourceData);
                return;
            }
        }

        obj = obj->getParent();
    }
}

NS_FGUI_END