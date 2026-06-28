#include "DragDropManagerHelper.h"
#include "DragDropManager.h"

NS_FGUI_BEGIN

DragDropManagerHelper* DragDropManagerHelper::_inst = nullptr;

DragDropManagerHelper* DragDropManagerHelper::getInstance()
{
    if (_inst == nullptr)
    {
        _inst = memnew(DragDropManagerHelper);
        _inst->reference();
    }
    return _inst;
}

void DragDropManagerHelper::startDrag(const String& icon, const Variant& sourceData, int touchPointID)
{
    DragDropManager::getInstance()->startDrag(icon.utf8().get_data(), sourceData, touchPointID);
}

void DragDropManagerHelper::cancel()
{
    DragDropManager::getInstance()->cancel();
}

bool DragDropManagerHelper::isDragging()
{
    return DragDropManager::getInstance()->isDragging();
}

void DragDropManagerHelper::_bind_methods()
{
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getInstance"), &DragDropManagerHelper::getInstance);
    ClassDB::bind_method(D_METHOD("startDrag", "icon", "source_data", "touch_point_id"), &DragDropManagerHelper::startDrag, DEFVAL(Variant()), DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("cancel"), &DragDropManagerHelper::cancel);
    ClassDB::bind_method(D_METHOD("isDragging"), &DragDropManagerHelper::isDragging);
}

NS_FGUI_END
