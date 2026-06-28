#ifndef __DRAGDROPMANAGER_HELPER_H__
#define __DRAGDROPMANAGER_HELPER_H__

#include "FairyGUIMacros.h"

NS_FGUI_BEGIN

class DragDropManagerHelper : public RefCounted
{
    GDCLASS(DragDropManagerHelper, RefCounted)

public:
    static DragDropManagerHelper* getInstance();

    void startDrag(const String& icon, const Variant& sourceData, int touchPointID);
    void cancel();
    bool isDragging();

    static void _bind_methods();

private:
    static DragDropManagerHelper* _inst;
};

NS_FGUI_END

#endif
