#ifndef __UIOBJECTFACTORY_H__
#define __UIOBJECTFACTORY_H__

// cocos2d.h removed - see godot_types.h
#include "FairyGUIMacros.h"
#include "GComponent.h"
#include "PackageItem.h"
#include "GLoader.h"

NS_FGUI_BEGIN

class UIObjectFactory
{
public:
    typedef std::function<Ref<GComponent>()> GComponentCreator;
    typedef std::function<Ref<GLoader>()> GLoaderCreator;

    static void setPackageItemExtension(const std::string& url, GComponentCreator creator);
    static Ref<GObject> newObject(PackageItem* pi);
    static Ref<GObject> newObject(ObjectType type);

    static void setLoaderExtension(GLoaderCreator creator);

private:
    static void resolvePackageItemExtension(PackageItem* pi);

    static std::unordered_map<std::string, GComponentCreator> _packageItemExtensions;
    static GLoaderCreator _loaderCreator;

    friend class UIPackage;
};

NS_FGUI_END

#endif
