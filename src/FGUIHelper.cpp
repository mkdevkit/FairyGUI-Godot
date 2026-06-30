#include "FGUIHelper.h"
#include "GRoot.h"
#include "UIPackage.h"

NS_FGUI_BEGIN

FGUIHelper* FGUIHelper::_inst = nullptr;

FGUIHelper* FGUIHelper::getInstance()
{
    if (_inst == nullptr)
    {
        _inst = memnew(FGUIHelper);
        _inst->reference();
    }
    return _inst;
}

Object* FGUIHelper::getGRoot()
{
    return (Object*)GRoot::getInstance();
}

Object* FGUIHelper::createObject(const String& pkgName, const String& resName)
{
    return UIPackage::createObject(pkgName.utf8().get_data(), resName.utf8().get_data()).ptr();
}

void FGUIHelper::addPackage(const String& path)
{
    UIPackage::addPackage(path.utf8().get_data());
}

void FGUIHelper::_bind_methods()
{
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getInstance"), &FGUIHelper::getInstance);

    ClassDB::bind_method(D_METHOD("getGRoot"), &FGUIHelper::getGRoot);
    ClassDB::bind_method(D_METHOD("createObject", "pkg", "res"), &FGUIHelper::createObject);
    ClassDB::bind_method(D_METHOD("addPackage", "path"), &FGUIHelper::addPackage);
}

NS_FGUI_END
