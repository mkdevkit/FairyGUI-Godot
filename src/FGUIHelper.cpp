#include "FGUIHelper.h"
#include "GRoot.h"
#include "UIPackage.h"
#include "TranslationHelper.h"
#include "utils/html/HtmlObject.h"
#include "core/io/file_access.h"

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

void FGUIHelper::loadTranslation(const String& xmlPath)
{
    Ref<FileAccess> file = FileAccess::open(xmlPath, FileAccess::READ);
    if (file.is_null())
        return;
    PackedByteArray bytes = file->get_buffer(file->get_length());
    if (bytes.size() > 0)
        TranslationHelper::loadFromXML((const char*)bytes.ptr(), bytes.size());
}

void FGUIHelper::loadTranslationFromXML(const String& xml)
{
    PackedByteArray bytes = xml.to_utf8_buffer();
    if (bytes.size() > 0)
        TranslationHelper::loadFromXML((const char*)bytes.ptr(), bytes.size());
}

void FGUIHelper::setHtmlButtonResource(const String& url)
{
    HtmlObject::buttonResource = url.utf8().get_data();
}

void FGUIHelper::setHtmlInputResource(const String& url)
{
    HtmlObject::inputResource = url.utf8().get_data();
}

void FGUIHelper::setHtmlSelectResource(const String& url)
{
    HtmlObject::selectResource = url.utf8().get_data();
}

void FGUIHelper::_bind_methods()
{
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getInstance"), &FGUIHelper::getInstance);

    ClassDB::bind_method(D_METHOD("getGRoot"), &FGUIHelper::getGRoot);
    ClassDB::bind_method(D_METHOD("createObject", "pkg", "res"), &FGUIHelper::createObject);
    ClassDB::bind_method(D_METHOD("addPackage", "path"), &FGUIHelper::addPackage);
    ClassDB::bind_method(D_METHOD("loadTranslation", "xml_path"), &FGUIHelper::loadTranslation);
    ClassDB::bind_method(D_METHOD("loadTranslationFromXML", "xml"), &FGUIHelper::loadTranslationFromXML);
    ClassDB::bind_method(D_METHOD("setHtmlButtonResource", "url"), &FGUIHelper::setHtmlButtonResource);
    ClassDB::bind_method(D_METHOD("setHtmlInputResource", "url"), &FGUIHelper::setHtmlInputResource);
    ClassDB::bind_method(D_METHOD("setHtmlSelectResource", "url"), &FGUIHelper::setHtmlSelectResource);
}

NS_FGUI_END
