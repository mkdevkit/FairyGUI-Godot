#ifndef __FGUI_GD_HELPER_H__
#define __FGUI_GD_HELPER_H__

#include "FairyGUIMacros.h"
#include "GObject.h"

NS_FGUI_BEGIN

class FGUIHelper : public RefCounted
{
    GDCLASS(FGUIHelper, RefCounted)

public:
    static FGUIHelper* getInstance();

    Object* getGRoot();
    Object* createObject(const String& pkgName, const String& resName);
    void addPackage(const String& path);

    static void _bind_methods();

private:
    static FGUIHelper* _inst;
};

NS_FGUI_END

#endif
