#include "GButton.h"
#include "GLabel.h"
#include "GRoot.h"
#include "GTextField.h"
#include "PackageItem.h"
#include "UIConfig.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN
const std::string GButton::UP = "up";
const std::string GButton::DOWN = "down";
const std::string GButton::OVER = "over";
const std::string GButton::SELECTED_OVER = "selectedOver";
const std::string GButton::DISABLED = "disabled";
const std::string GButton::SELECTED_DISABLED = "selectedDisabled";

GButton::GButton() : _mode(ButtonMode::COMMON),
                     _titleObject(nullptr),
                     _iconObject(nullptr),
                     _selected(false),
                     _over(false),
                     _down(false),
                     _downEffect(0),
                     _downScaled(false),
                     _downEffectValue(0.8f),
                     _changeStateOnClick(true)
{
    _sound = UIConfig::buttonSound;
    _soundVolumeScale = UIConfig::buttonSoundVolumeScale;
}

GButton::~GButton()
{
}

void GButton::setTitle(const std::string& Variant)
{
    _title = Variant;
    if (_titleObject != nullptr)
        _titleObject->setText((_selected && _selectedTitle.length() > 0) ? _selectedTitle : _title);
    updateGear(6);
}

void GButton::setIcon(const std::string& Variant)
{
    _icon = Variant;
    if (_iconObject != nullptr)
        _iconObject->setIcon((_selected && _selectedIcon.length() > 0) ? _selectedIcon : _icon);
    updateGear(7);
}

void GButton::setSelectedTitle(const std::string& Variant)
{
    _selectedTitle = Variant;
    if (_titleObject != nullptr)
        _titleObject->setText((_selected && _selectedTitle.length() > 0) ? _selectedTitle : _title);
}

void GButton::setSelectedIcon(const std::string& Variant)
{
    _selectedIcon = Variant;
    if (_iconObject != nullptr)
        _iconObject->setIcon((_selected && _selectedIcon.length() > 0) ? _selectedIcon : _icon);
}

Color GButton::getTitleColor() const
{
    GTextField* tf = getTextField();
    if (tf)
        return tf->getColor();
    else
        return Color(0,0,0,1);
}

void GButton::setTitleColor(const Color& Variant)
{
    GTextField* tf = getTextField();
    if (tf)
        tf->setColor(Variant);
}

int GButton::getTitleFontSize() const
{
    GTextField* tf = getTextField();
    if (tf)
        return tf->getFontSize();
    else
        return 0;
}

void GButton::setTitleFontSize(int Variant)
{
    GTextField* tf = getTextField();
    if (tf)
        tf->setFontSize(Variant);
}

void GButton::setSelected(bool Variant)
{
    if (_mode == ButtonMode::COMMON)
        return;

    if (_selected != Variant)
    {
        _selected = Variant;
        setCurrentState();
        if (!_selectedTitle.empty() && _titleObject != nullptr)
            _titleObject->setText(_selected ? _selectedTitle : _title);
        if (!_selectedIcon.empty())
        {
            const std::string& str = _selected ? _selectedIcon : _icon;
            if (_iconObject != nullptr)
                _iconObject->setIcon(str);
        }
        if (_relatedController.is_valid() && getParent() != nullptr && !getParent()->_buildingDisplayList)
        {
            if (_selected)
            {
                _relatedController->setSelectedPageId(_relatedPageId);
                if (_relatedController->autoRadioGroupDepth)
                    getParent()->adjustRadioGroupDepth(this, _relatedController.ptr());
            }
            else if (_mode == ButtonMode::CHECK && _relatedController->getSelectedPageId().compare(_relatedPageId) == 0)
                _relatedController->setOppositePageId(_relatedPageId);
        }
    }
}

void GButton::setRelatedController(GController* c)
{
    _relatedController = c;
}

void GButton::setState(const std::string& value)
{
    if (_buttonController.is_valid())
        _buttonController->setSelectedPage(value);

    if (_downEffect == 1)
    {
        int cnt = this->numChildren();
        if (value == DOWN || value == SELECTED_OVER || value == SELECTED_DISABLED)
        {
            int c = _downEffectValue * 255;
            Variant color((c << 16) + (c << 8) + c);
            for (int i = 0; i < cnt; i++)
            {
                GObject* obj = this->getChildAt(i);
                if (dynamic_cast<GTextField*>(obj) == nullptr)
                    obj->setProp(ObjectPropID::Color, color);
            }
        }
        else
        {
            Variant color(0xFFFFFF);
            for (int i = 0; i < cnt; i++)
            {
                GObject* obj = this->getChildAt(i);
                if (dynamic_cast<GTextField*>(obj) == nullptr)
                    obj->setProp(ObjectPropID::Color, color);
            }
        }
    }
    else if (_downEffect == 2)
    {
        if (value == DOWN || value == SELECTED_OVER || value == SELECTED_DISABLED)
        {
            if (!_downScaled)
            {
                _downScaled = true;
                setScale(getScaleX() * _downEffectValue, getScaleY() * _downEffectValue);
            }
        }
        else
        {
            if (_downScaled)
            {
                _downScaled = false;
                setScale(getScaleX() / _downEffectValue, getScaleY() / _downEffectValue);
            }
        }
    }
}

void GButton::setCurrentState()
{
    if (isGrayed() && _buttonController.is_valid() && _buttonController->hasPage(DISABLED))
    {
        if (_selected)
            setState(SELECTED_DISABLED);
        else
            setState(DISABLED);
    }
    else
    {
        if (_selected)
            setState(_over ? SELECTED_OVER : DOWN);
        else
            setState(_over ? OVER : UP);
    }
}

GTextField* GButton::getTextField() const
{
    if (dynamic_cast<GTextField*>(_titleObject))
        return dynamic_cast<GTextField*>(_titleObject);
    else if (dynamic_cast<GLabel*>(_titleObject))
        return dynamic_cast<GLabel*>(_titleObject)->getTextField();
    else if (dynamic_cast<GButton*>(_titleObject))
        return dynamic_cast<GButton*>(_titleObject)->getTextField();
    else
        return nullptr;
}

Variant GButton::getProp(ObjectPropID propId)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        return Variant((unsigned int)ToolSet::colorToInt(getTitleColor()));
    case ObjectPropID::OutlineColor:
    {
        GTextField* tf = getTextField();
        if (tf != nullptr)
            return Variant((unsigned int)ToolSet::colorToInt(tf->getOutlineColor()));
        else
            return Variant();
    }
    case ObjectPropID::FontSize:
        return Variant(getTitleFontSize());
    case ObjectPropID::Selected:
        return Variant(isSelected());
    default:
        return GComponent::getProp(propId);
    }
}

void GButton::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        setTitleColor(ToolSet::intToColor((unsigned int)(int)value));
        break;
    case ObjectPropID::OutlineColor:
    {
        GTextField* tf = getTextField();
        if (tf != nullptr)
            tf->setOutlineColor(ToolSet::intToColor((unsigned int)(int)value));
        break;
    }
    case ObjectPropID::FontSize:
        setTitleFontSize((float)value);
        break;
    case ObjectPropID::Selected:
        setSelected((bool)value);
        break;
    default:
        GComponent::setProp(propId, value);
        break;
    }
}

void GButton::constructExtension(ByteBuffer* buffer)
{
    buffer->seek(0, 6);

    _mode = (ButtonMode)buffer->readByte();
    buffer->readS(_sound);
    _soundVolumeScale = buffer->readFloat();
    _downEffect = buffer->readByte();
    _downEffectValue = buffer->readFloat();
    if (_downEffect == 2)
        setPivot(0.5f, 0.5f, isPivotAsAnchor());

    _buttonController = Ref<GController>(getController("button"));
    _titleObject = getChild("title");
    _iconObject = getChild("icon");
    if (_titleObject != nullptr)
        _title = _titleObject->getText();
    if (_iconObject != nullptr)
        _icon = _iconObject->getIcon();

    if (_mode == ButtonMode::COMMON)
        setState(UP);

    addEventListener(UIEventType::RollOver, [this](EventContext* ctx) { onRollOver(ctx); });
    addEventListener(UIEventType::RollOut, [this](EventContext* ctx) { onRollOut(ctx); });
    addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { onTouchBegin(ctx); });
    addEventListener(UIEventType::TouchEnd, [this](EventContext* ctx) { onTouchEnd(ctx); });
    addEventListener(UIEventType::Click, [this](EventContext* ctx) { onClick(ctx); });
    addEventListener(UIEventType::Exit, [this](EventContext* ctx) { onExit(ctx); });
}

void GButton::setup_afterAdd(ByteBuffer* buffer, int beginPos)
{
    GComponent::setup_afterAdd(buffer, beginPos);

    if (!buffer->seek(beginPos, 6))
        return;

    if ((ObjectType)buffer->readByte() != _packageItem->objectType)
        return;

    const std::string* str;

    if ((str = buffer->readSP()))
        setTitle(*str);
    if ((str = buffer->readSP()))
        setSelectedTitle(*str);
    if ((str = buffer->readSP()))
        setIcon(*str);
    if ((str = buffer->readSP()))
        setSelectedIcon(*str);
    if (buffer->readBool())
        setTitleColor((Color)buffer->readColor());
    int iv = buffer->readInt();
    if (iv != 0)
        setTitleFontSize(iv);
    iv = buffer->readShort();
    if (iv >= 0)
        _relatedController = Ref<GController>(_parent->getControllerAt(iv));
    _relatedPageId = buffer->readS();

    buffer->readS(_sound);
    if (buffer->readBool())
        _soundVolumeScale = buffer->readFloat();

    setSelected(buffer->readBool());
}

void GButton::handleControllerChanged(GController* c)
{
    GObject::handleControllerChanged(c);

    if (_relatedController.ptr() == c)
        setSelected(_relatedPageId.compare(c->getSelectedPageId()) == 0);
}

void GButton::onRollOver(EventContext* context)
{
    if (_buttonController.is_null() || !_buttonController->hasPage(OVER))
        return;

    _over = true;
    if (_down)
        return;

    if (isGrayed() && _buttonController->hasPage(DISABLED))
        return;

    setState(_selected ? SELECTED_OVER : OVER);
}

void GButton::onRollOut(EventContext* context)
{
    if (_buttonController.is_null() || !_buttonController->hasPage(OVER))
        return;

    _over = false;
    if (_down)
        return;

    if (isGrayed() && _buttonController->hasPage(DISABLED))
        return;

    setState(_selected ? DOWN : UP);
}

void GButton::onTouchBegin(EventContext* context)
{
    if (context->getInput()->getButton() != MouseButton::LEFT)
        return;

    _down = true;
    context->captureTouch();

    if (_mode == ButtonMode::COMMON)
    {
        if (isGrayed() && _buttonController.is_valid() && _buttonController->hasPage(DISABLED))
            setState(SELECTED_DISABLED);
        else
            setState(DOWN);
    }
}

void GButton::onTouchEnd(EventContext* context)
{
    if (context->getInput()->getButton() != MouseButton::LEFT)
        return;

    if (_down)
    {
        _down = false;
        if (_mode == ButtonMode::COMMON)
        {
            if (isGrayed() && _buttonController.is_valid() && _buttonController->hasPage(DISABLED))
                setState(DISABLED);
            else if (_over)
                setState(OVER);
            else
                setState(UP);
        }
        else
        {
            if (!_over && _buttonController.is_valid() && (_buttonController->getSelectedPage() == OVER || _buttonController->getSelectedPage() == SELECTED_OVER))
            {
                setCurrentState();
            }
        }
    }
}

void GButton::onClick(EventContext* context)
{
    if (!_sound.empty())
        GRoot::getInstance()->playSound(_sound, _soundVolumeScale);

    if (_mode == ButtonMode::CHECK)
    {
        if (_changeStateOnClick)
        {
            setSelected(!_selected);
            dispatchEvent(UIEventType::Changed);
        }
    }
    else if (_mode == ButtonMode::RADIO)
    {
        if (_changeStateOnClick && !_selected)
        {
            setSelected(true);
            dispatchEvent(UIEventType::Changed);
        }
    }
    else
    {
        if (_relatedController.is_valid())
            _relatedController->setSelectedPageId(_relatedPageId);
    }
}

void GButton::onExit(EventContext* context)
{
    if (_over)
        onRollOut(context);
}

void GButton::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setTitle", "title"), &GButton::gd_setTitle);
    ClassDB::bind_method(D_METHOD("getTitle"), &GButton::gd_getTitle);

    ClassDB::bind_method(D_METHOD("setSelectedTitle", "title"), &GButton::gd_setSelectedTitle);
    ClassDB::bind_method(D_METHOD("getSelectedTitle"), &GButton::gd_getSelectedTitle);

    // Icon is bound in GObject

    ClassDB::bind_method(D_METHOD("setSelectedIcon", "icon"), &GButton::gd_setSelectedIcon);
    ClassDB::bind_method(D_METHOD("getSelectedIcon"), &GButton::gd_getSelectedIcon);

    ClassDB::bind_method(D_METHOD("setTitleColor", "color"), &GButton::setTitleColor);
    ClassDB::bind_method(D_METHOD("getTitleColor"), &GButton::getTitleColor);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "titleColor"), "setTitleColor", "getTitleColor");

    ClassDB::bind_method(D_METHOD("setTitleFontSize", "size"), &GButton::setTitleFontSize);
    ClassDB::bind_method(D_METHOD("getTitleFontSize"), &GButton::getTitleFontSize);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "titleFontSize"), "setTitleFontSize", "getTitleFontSize");

    ClassDB::bind_method(D_METHOD("setSelected", "value"), &GButton::setSelected);
    ClassDB::bind_method(D_METHOD("isSelected"), &GButton::isSelected);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "selected"), "setSelected", "isSelected");

    ClassDB::bind_method(D_METHOD("getRelatedController"), &GButton::getRelatedController);
    ClassDB::bind_method(D_METHOD("setRelatedController", "controller"), &GButton::setRelatedController);

    ClassDB::bind_method(D_METHOD("setChangeStateOnClick", "value"), &GButton::setChangeStateOnClick);
    ClassDB::bind_method(D_METHOD("isChangeStateOnClick"), &GButton::isChangeStateOnClick);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "changeStateOnClick"), "setChangeStateOnClick", "isChangeStateOnClick");
}

void GButton::gd_setTitle(const String& value) { setTitle(value.utf8().get_data()); }
String GButton::gd_getTitle() { return GObject::GObject::toGodotStr(getTitle()); }
void GButton::gd_setSelectedTitle(const String& value) { setSelectedTitle(value.utf8().get_data()); }
String GButton::gd_getSelectedTitle() const { return GObject::GObject::toGodotStr(getSelectedTitle()); }
void GButton::gd_setIcon(const String& value) { setIcon(value.utf8().get_data()); }
String GButton::gd_getIcon() const { return GObject::GObject::toGodotStr(getIcon()); }
void GButton::gd_setSelectedIcon(const String& value) { setSelectedIcon(value.utf8().get_data()); }
String GButton::gd_getSelectedIcon() const { return GObject::GObject::toGodotStr(getSelectedIcon()); }

NS_FGUI_END


