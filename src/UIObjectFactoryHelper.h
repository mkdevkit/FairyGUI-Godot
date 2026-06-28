#ifndef __UIOBJECTFACTORY_HELPER_H__
#define __UIOBJECTFACTORY_HELPER_H__

#include "FairyGUIMacros.h"
#include "GObject.h"

NS_FGUI_BEGIN

class UIObjectFactoryHelper : public RefCounted
{
    GDCLASS(UIObjectFactoryHelper, RefCounted)

public:
    static UIObjectFactoryHelper* getInstance();

    void setPackageItemExtension(const String& url, const Callable& creator);

    static void _bind_methods();

private:
    static UIObjectFactoryHelper* _inst;
};

NS_FGUI_END

#endif
