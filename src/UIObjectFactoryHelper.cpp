#include "UIObjectFactoryHelper.h"
#include "UIObjectFactory.h"

NS_FGUI_BEGIN

UIObjectFactoryHelper* UIObjectFactoryHelper::_inst = nullptr;

UIObjectFactoryHelper* UIObjectFactoryHelper::getInstance()
{
    if (_inst == nullptr)
    {
        _inst = memnew(UIObjectFactoryHelper);
        _inst->reference();
    }
    return _inst;
}

void UIObjectFactoryHelper::setPackageItemExtension(const String& url, const Callable& creator)
{
    UIObjectFactory::setPackageItemExtension(url.utf8().get_data(), [creator]() -> GComponent* {
        Object* obj = creator.call();
        GComponent* comp = Object::cast_to<GComponent>(obj);
        return comp;
    });
}

void UIObjectFactoryHelper::_bind_methods()
{
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getInstance"), &UIObjectFactoryHelper::getInstance);
    ClassDB::bind_method(D_METHOD("setPackageItemExtension", "url", "creator"), &UIObjectFactoryHelper::setPackageItemExtension);
}

NS_FGUI_END
