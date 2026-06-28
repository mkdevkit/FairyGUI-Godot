#include "UIConfigHelper.h"
#include "UIConfig.h"

NS_FGUI_BEGIN

UIConfigHelper* UIConfigHelper::_inst = nullptr;

UIConfigHelper* UIConfigHelper::getInstance()
{
    if (_inst == nullptr)
    {
        _inst = memnew(UIConfigHelper);
        _inst->reference();
    }
    return _inst;
}

void UIConfigHelper::setVerticalScrollBar(const String& value) { UIConfig::verticalScrollBar = value.utf8().get_data(); }
String UIConfigHelper::getVerticalScrollBar() const { return UIConfig::verticalScrollBar.c_str(); }
void UIConfigHelper::setHorizontalScrollBar(const String& value) { UIConfig::horizontalScrollBar = value.utf8().get_data(); }
String UIConfigHelper::getHorizontalScrollBar() const { return UIConfig::horizontalScrollBar.c_str(); }
void UIConfigHelper::setTooltipsWin(const String& value) { UIConfig::tooltipsWin = value.utf8().get_data(); }
String UIConfigHelper::getTooltipsWin() const { return UIConfig::tooltipsWin.c_str(); }
void UIConfigHelper::setGlobalModalWaiting(const String& value) { UIConfig::globalModalWaiting = value.utf8().get_data(); }
String UIConfigHelper::getGlobalModalWaiting() const { return UIConfig::globalModalWaiting.c_str(); }
void UIConfigHelper::setWindowModalWaiting(const String& value) { UIConfig::windowModalWaiting = value.utf8().get_data(); }
String UIConfigHelper::getWindowModalWaiting() const { return UIConfig::windowModalWaiting.c_str(); }
void UIConfigHelper::setPopupMenu(const String& value) { UIConfig::popupMenu = value.utf8().get_data(); }
String UIConfigHelper::getPopupMenu() const { return UIConfig::popupMenu.c_str(); }
void UIConfigHelper::setButtonSound(const String& value) { UIConfig::buttonSound = value.utf8().get_data(); }
String UIConfigHelper::getButtonSound() const { return UIConfig::buttonSound.c_str(); }
void UIConfigHelper::setModalLayerColor(const Color& value) { UIConfig::modalLayerColor = value; }
Color UIConfigHelper::getModalLayerColor() const { return UIConfig::modalLayerColor; }

void UIConfigHelper::_bind_methods()
{
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getInstance"), &UIConfigHelper::getInstance);

    ClassDB::bind_method(D_METHOD("setVerticalScrollBar", "value"), &UIConfigHelper::setVerticalScrollBar);
    ClassDB::bind_method(D_METHOD("getVerticalScrollBar"), &UIConfigHelper::getVerticalScrollBar);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "verticalScrollBar"), "setVerticalScrollBar", "getVerticalScrollBar");

    ClassDB::bind_method(D_METHOD("setHorizontalScrollBar", "value"), &UIConfigHelper::setHorizontalScrollBar);
    ClassDB::bind_method(D_METHOD("getHorizontalScrollBar"), &UIConfigHelper::getHorizontalScrollBar);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "horizontalScrollBar"), "setHorizontalScrollBar", "getHorizontalScrollBar");

    ClassDB::bind_method(D_METHOD("setTooltipsWin", "value"), &UIConfigHelper::setTooltipsWin);
    ClassDB::bind_method(D_METHOD("getTooltipsWin"), &UIConfigHelper::getTooltipsWin);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "tooltipsWin"), "setTooltipsWin", "getTooltipsWin");

    ClassDB::bind_method(D_METHOD("setGlobalModalWaiting", "value"), &UIConfigHelper::setGlobalModalWaiting);
    ClassDB::bind_method(D_METHOD("getGlobalModalWaiting"), &UIConfigHelper::getGlobalModalWaiting);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "globalModalWaiting"), "setGlobalModalWaiting", "getGlobalModalWaiting");

    ClassDB::bind_method(D_METHOD("setWindowModalWaiting", "value"), &UIConfigHelper::setWindowModalWaiting);
    ClassDB::bind_method(D_METHOD("getWindowModalWaiting"), &UIConfigHelper::getWindowModalWaiting);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "windowModalWaiting"), "setWindowModalWaiting", "getWindowModalWaiting");

    ClassDB::bind_method(D_METHOD("setPopupMenu", "value"), &UIConfigHelper::setPopupMenu);
    ClassDB::bind_method(D_METHOD("getPopupMenu"), &UIConfigHelper::getPopupMenu);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "popupMenu"), "setPopupMenu", "getPopupMenu");

    ClassDB::bind_method(D_METHOD("setButtonSound", "value"), &UIConfigHelper::setButtonSound);
    ClassDB::bind_method(D_METHOD("getButtonSound"), &UIConfigHelper::getButtonSound);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "buttonSound"), "setButtonSound", "getButtonSound");

    ClassDB::bind_method(D_METHOD("setModalLayerColor", "value"), &UIConfigHelper::setModalLayerColor);
    ClassDB::bind_method(D_METHOD("getModalLayerColor"), &UIConfigHelper::getModalLayerColor);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "modalLayerColor"), "setModalLayerColor", "getModalLayerColor");
}

NS_FGUI_END
