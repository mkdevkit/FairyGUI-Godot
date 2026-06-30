#ifndef __DRAGDROPMANAGER_H__
#define __DRAGDROPMANAGER_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h
#include "GLoader.h"

NS_FGUI_BEGIN

class DragDropManager
{
public:
    DragDropManager();
    ~DragDropManager();

    static DragDropManager* getInstance();

    GLoader* getAgent() const { return _agent.ptr(); }
    bool isDragging() const { return _agent->getParent() != nullptr; }
    void startDrag(const std::string& icon, const Variant& sourceData = Variant(), int touchPointID = -1);
    void cancel();

private:
    void onDragEnd(EventContext* context);

    static DragDropManager* _inst;

    Ref<GLoader> _agent;
    Variant _sourceData;
};

NS_FGUI_END

#endif
