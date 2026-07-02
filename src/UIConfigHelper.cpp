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

void UIConfigHelper::setDefaultFont(const String& value) { UIConfig::defaultFont = value.utf8().get_data(); }
String UIConfigHelper::getDefaultFont() const { return UIConfig::defaultFont.c_str(); }
void UIConfigHelper::setButtonSound(const String& value) { UIConfig::buttonSound = value.utf8().get_data(); }
String UIConfigHelper::getButtonSound() const { return UIConfig::buttonSound.c_str(); }
void UIConfigHelper::setButtonSoundVolumeScale(float value) { UIConfig::buttonSoundVolumeScale = value; }
float UIConfigHelper::getButtonSoundVolumeScale() const { return UIConfig::buttonSoundVolumeScale; }
void UIConfigHelper::setDefaultScrollStep(int value) { UIConfig::defaultScrollStep = value; }
int UIConfigHelper::getDefaultScrollStep() const { return UIConfig::defaultScrollStep; }
void UIConfigHelper::setDefaultScrollDecelerationRate(float value) { UIConfig::defaultScrollDecelerationRate = value; }
float UIConfigHelper::getDefaultScrollDecelerationRate() const { return UIConfig::defaultScrollDecelerationRate; }
void UIConfigHelper::setDefaultScrollTouchEffect(bool value) { UIConfig::defaultScrollTouchEffect = value; }
bool UIConfigHelper::getDefaultScrollTouchEffect() const { return UIConfig::defaultScrollTouchEffect; }
void UIConfigHelper::setDefaultScrollBounceEffect(bool value) { UIConfig::defaultScrollBounceEffect = value; }
bool UIConfigHelper::getDefaultScrollBounceEffect() const { return UIConfig::defaultScrollBounceEffect; }
void UIConfigHelper::setDefaultScrollBarDisplay(int value) { UIConfig::defaultScrollBarDisplay = (ScrollBarDisplayType)value; }
int UIConfigHelper::getDefaultScrollBarDisplay() const { return (int)UIConfig::defaultScrollBarDisplay; }
void UIConfigHelper::setVerticalScrollBar(const String& value) { UIConfig::verticalScrollBar = value.utf8().get_data(); }
String UIConfigHelper::getVerticalScrollBar() const { return UIConfig::verticalScrollBar.c_str(); }
void UIConfigHelper::setHorizontalScrollBar(const String& value) { UIConfig::horizontalScrollBar = value.utf8().get_data(); }
String UIConfigHelper::getHorizontalScrollBar() const { return UIConfig::horizontalScrollBar.c_str(); }
void UIConfigHelper::setTouchDragSensitivity(int value) { UIConfig::touchDragSensitivity = value; }
int UIConfigHelper::getTouchDragSensitivity() const { return UIConfig::touchDragSensitivity; }
void UIConfigHelper::setClickDragSensitivity(int value) { UIConfig::clickDragSensitivity = value; }
int UIConfigHelper::getClickDragSensitivity() const { return UIConfig::clickDragSensitivity; }
void UIConfigHelper::setTouchScrollSensitivity(int value) { UIConfig::touchScrollSensitivity = value; }
int UIConfigHelper::getTouchScrollSensitivity() const { return UIConfig::touchScrollSensitivity; }
void UIConfigHelper::setDefaultComboBoxVisibleItemCount(int value) { UIConfig::defaultComboBoxVisibleItemCount = value; }
int UIConfigHelper::getDefaultComboBoxVisibleItemCount() const { return UIConfig::defaultComboBoxVisibleItemCount; }
void UIConfigHelper::setGlobalModalWaiting(const String& value) { UIConfig::globalModalWaiting = value.utf8().get_data(); }
String UIConfigHelper::getGlobalModalWaiting() const { return UIConfig::globalModalWaiting.c_str(); }
void UIConfigHelper::setModalLayerColor(const Color& value) { UIConfig::modalLayerColor = value; }
Color UIConfigHelper::getModalLayerColor() const { return UIConfig::modalLayerColor; }
void UIConfigHelper::setTooltipsWin(const String& value) { UIConfig::tooltipsWin = value.utf8().get_data(); }
String UIConfigHelper::getTooltipsWin() const { return UIConfig::tooltipsWin.c_str(); }
void UIConfigHelper::setBringWindowToFrontOnClick(bool value) { UIConfig::bringWindowToFrontOnClick = value; }
bool UIConfigHelper::getBringWindowToFrontOnClick() const { return UIConfig::bringWindowToFrontOnClick; }
void UIConfigHelper::setWindowModalWaiting(const String& value) { UIConfig::windowModalWaiting = value.utf8().get_data(); }
String UIConfigHelper::getWindowModalWaiting() const { return UIConfig::windowModalWaiting.c_str(); }
void UIConfigHelper::setPopupMenu(const String& value) { UIConfig::popupMenu = value.utf8().get_data(); }
String UIConfigHelper::getPopupMenu() const { return UIConfig::popupMenu.c_str(); }
void UIConfigHelper::setPopupMenuSeperator(const String& value) { UIConfig::popupMenu_seperator = value.utf8().get_data(); }
String UIConfigHelper::getPopupMenuSeperator() const { return UIConfig::popupMenu_seperator.c_str(); }

void UIConfigHelper::registerFont(const String& aliasName, const String& realName)
{
    UIConfig::registerFont(aliasName.utf8().get_data(), realName.utf8().get_data());
}

String UIConfigHelper::getRealFontName(const String& aliasName) const
{
    return UIConfig::getRealFontName(aliasName.utf8().get_data()).c_str();
}

void UIConfigHelper::_bind_methods()
{
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getInstance"), &UIConfigHelper::getInstance);

    ClassDB::bind_method(D_METHOD("setDefaultFont", "value"), &UIConfigHelper::setDefaultFont);
    ClassDB::bind_method(D_METHOD("getDefaultFont"), &UIConfigHelper::getDefaultFont);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "defaultFont"), "setDefaultFont", "getDefaultFont");

    ClassDB::bind_method(D_METHOD("setButtonSound", "value"), &UIConfigHelper::setButtonSound);
    ClassDB::bind_method(D_METHOD("getButtonSound"), &UIConfigHelper::getButtonSound);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "buttonSound"), "setButtonSound", "getButtonSound");

    ClassDB::bind_method(D_METHOD("setButtonSoundVolumeScale", "value"), &UIConfigHelper::setButtonSoundVolumeScale);
    ClassDB::bind_method(D_METHOD("getButtonSoundVolumeScale"), &UIConfigHelper::getButtonSoundVolumeScale);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "buttonSoundVolumeScale"), "setButtonSoundVolumeScale", "getButtonSoundVolumeScale");

    ClassDB::bind_method(D_METHOD("setDefaultScrollStep", "value"), &UIConfigHelper::setDefaultScrollStep);
    ClassDB::bind_method(D_METHOD("getDefaultScrollStep"), &UIConfigHelper::getDefaultScrollStep);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "defaultScrollStep"), "setDefaultScrollStep", "getDefaultScrollStep");

    ClassDB::bind_method(D_METHOD("setDefaultScrollDecelerationRate", "value"), &UIConfigHelper::setDefaultScrollDecelerationRate);
    ClassDB::bind_method(D_METHOD("getDefaultScrollDecelerationRate"), &UIConfigHelper::getDefaultScrollDecelerationRate);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "defaultScrollDecelerationRate"), "setDefaultScrollDecelerationRate", "getDefaultScrollDecelerationRate");

    ClassDB::bind_method(D_METHOD("setDefaultScrollTouchEffect", "value"), &UIConfigHelper::setDefaultScrollTouchEffect);
    ClassDB::bind_method(D_METHOD("getDefaultScrollTouchEffect"), &UIConfigHelper::getDefaultScrollTouchEffect);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "defaultScrollTouchEffect"), "setDefaultScrollTouchEffect", "getDefaultScrollTouchEffect");

    ClassDB::bind_method(D_METHOD("setDefaultScrollBounceEffect", "value"), &UIConfigHelper::setDefaultScrollBounceEffect);
    ClassDB::bind_method(D_METHOD("getDefaultScrollBounceEffect"), &UIConfigHelper::getDefaultScrollBounceEffect);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "defaultScrollBounceEffect"), "setDefaultScrollBounceEffect", "getDefaultScrollBounceEffect");

    ClassDB::bind_method(D_METHOD("setDefaultScrollBarDisplay", "value"), &UIConfigHelper::setDefaultScrollBarDisplay);
    ClassDB::bind_method(D_METHOD("getDefaultScrollBarDisplay"), &UIConfigHelper::getDefaultScrollBarDisplay);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "defaultScrollBarDisplay"), "setDefaultScrollBarDisplay", "getDefaultScrollBarDisplay");

    ClassDB::bind_method(D_METHOD("setVerticalScrollBar", "value"), &UIConfigHelper::setVerticalScrollBar);
    ClassDB::bind_method(D_METHOD("getVerticalScrollBar"), &UIConfigHelper::getVerticalScrollBar);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "verticalScrollBar"), "setVerticalScrollBar", "getVerticalScrollBar");

    ClassDB::bind_method(D_METHOD("setHorizontalScrollBar", "value"), &UIConfigHelper::setHorizontalScrollBar);
    ClassDB::bind_method(D_METHOD("getHorizontalScrollBar"), &UIConfigHelper::getHorizontalScrollBar);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "horizontalScrollBar"), "setHorizontalScrollBar", "getHorizontalScrollBar");

    ClassDB::bind_method(D_METHOD("setTouchDragSensitivity", "value"), &UIConfigHelper::setTouchDragSensitivity);
    ClassDB::bind_method(D_METHOD("getTouchDragSensitivity"), &UIConfigHelper::getTouchDragSensitivity);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "touchDragSensitivity"), "setTouchDragSensitivity", "getTouchDragSensitivity");

    ClassDB::bind_method(D_METHOD("setClickDragSensitivity", "value"), &UIConfigHelper::setClickDragSensitivity);
    ClassDB::bind_method(D_METHOD("getClickDragSensitivity"), &UIConfigHelper::getClickDragSensitivity);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "clickDragSensitivity"), "setClickDragSensitivity", "getClickDragSensitivity");

    ClassDB::bind_method(D_METHOD("setTouchScrollSensitivity", "value"), &UIConfigHelper::setTouchScrollSensitivity);
    ClassDB::bind_method(D_METHOD("getTouchScrollSensitivity"), &UIConfigHelper::getTouchScrollSensitivity);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "touchScrollSensitivity"), "setTouchScrollSensitivity", "getTouchScrollSensitivity");

    ClassDB::bind_method(D_METHOD("setDefaultComboBoxVisibleItemCount", "value"), &UIConfigHelper::setDefaultComboBoxVisibleItemCount);
    ClassDB::bind_method(D_METHOD("getDefaultComboBoxVisibleItemCount"), &UIConfigHelper::getDefaultComboBoxVisibleItemCount);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "defaultComboBoxVisibleItemCount"), "setDefaultComboBoxVisibleItemCount", "getDefaultComboBoxVisibleItemCount");

    ClassDB::bind_method(D_METHOD("setGlobalModalWaiting", "value"), &UIConfigHelper::setGlobalModalWaiting);
    ClassDB::bind_method(D_METHOD("getGlobalModalWaiting"), &UIConfigHelper::getGlobalModalWaiting);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "globalModalWaiting"), "setGlobalModalWaiting", "getGlobalModalWaiting");

    ClassDB::bind_method(D_METHOD("setModalLayerColor", "value"), &UIConfigHelper::setModalLayerColor);
    ClassDB::bind_method(D_METHOD("getModalLayerColor"), &UIConfigHelper::getModalLayerColor);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "modalLayerColor"), "setModalLayerColor", "getModalLayerColor");

    ClassDB::bind_method(D_METHOD("setTooltipsWin", "value"), &UIConfigHelper::setTooltipsWin);
    ClassDB::bind_method(D_METHOD("getTooltipsWin"), &UIConfigHelper::getTooltipsWin);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "tooltipsWin"), "setTooltipsWin", "getTooltipsWin");

    ClassDB::bind_method(D_METHOD("setBringWindowToFrontOnClick", "value"), &UIConfigHelper::setBringWindowToFrontOnClick);
    ClassDB::bind_method(D_METHOD("getBringWindowToFrontOnClick"), &UIConfigHelper::getBringWindowToFrontOnClick);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bringWindowToFrontOnClick"), "setBringWindowToFrontOnClick", "getBringWindowToFrontOnClick");

    ClassDB::bind_method(D_METHOD("setWindowModalWaiting", "value"), &UIConfigHelper::setWindowModalWaiting);
    ClassDB::bind_method(D_METHOD("getWindowModalWaiting"), &UIConfigHelper::getWindowModalWaiting);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "windowModalWaiting"), "setWindowModalWaiting", "getWindowModalWaiting");

    ClassDB::bind_method(D_METHOD("setPopupMenu", "value"), &UIConfigHelper::setPopupMenu);
    ClassDB::bind_method(D_METHOD("getPopupMenu"), &UIConfigHelper::getPopupMenu);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "popupMenu"), "setPopupMenu", "getPopupMenu");

    ClassDB::bind_method(D_METHOD("setPopupMenuSeperator", "value"), &UIConfigHelper::setPopupMenuSeperator);
    ClassDB::bind_method(D_METHOD("getPopupMenuSeperator"), &UIConfigHelper::getPopupMenuSeperator);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "popupMenuSeperator"), "setPopupMenuSeperator", "getPopupMenuSeperator");

    ClassDB::bind_method(D_METHOD("registerFont", "aliasName", "realName"), &UIConfigHelper::registerFont);
    ClassDB::bind_method(D_METHOD("getRealFontName", "aliasName"), &UIConfigHelper::getRealFontName);
}

NS_FGUI_END
