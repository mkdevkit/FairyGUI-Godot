#include "GComboBox.h"
#include "GRoot.h"
#include "PackageItem.h"
#include "UIConfig.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN
GComboBox::GComboBox()
    : _dropdown(nullptr),
      _titleObject(nullptr),
      _iconObject(nullptr),
      _list(nullptr),
      _selectionController(nullptr),
      _itemsUpdated(true),
      _selectedIndex(-1),
      popupDirection(PopupDirection::AUTO)
{
    visibleItemCount = UIConfig::defaultComboBoxVisibleItemCount;
}

GComboBox::~GComboBox()
{
    // // CC_SAFE_RELEASE removed - _dropdown managed by Godot ref counting;
}

const std::string& GComboBox::getTitle() const
{
    if (_titleObject != nullptr)
        return _titleObject->getText();
    else
        return "";
}

void GComboBox::setTitle(const std::string& value)
{
    if (_titleObject != nullptr)
        _titleObject->setText(value);
    updateGear(6);
}

const Color GComboBox::getTitleColor() const
{
    GTextField* tf = getTextField();
    if (tf)
        return tf->getColor();
    else
        return Color(0,0,0,1);
}

void GComboBox::setTitleColor(const Color& value)
{
    GTextField* tf = getTextField();
    if (tf)
        tf->setColor(value);
}

int GComboBox::getTitleFontSize() const
{
    GTextField* tf = getTextField();
    if (tf)
        return tf->getFontSize();
    else
        return 0;
}

void GComboBox::setTitleFontSize(int value)
{
    GTextField* tf = getTextField();
    if (tf)
        tf->setFontSize(value);
}

const std::string& GComboBox::getIcon() const
{
    if (_iconObject != nullptr)
        return _iconObject->getIcon();
    else
        return "";
}

void GComboBox::setIcon(const std::string& value)
{
    if (_iconObject != nullptr)
        _iconObject->setIcon(value);
    updateGear(7);
}

const std::string& GComboBox::getValue() const
{
    if (_selectedIndex >= 0 && _selectedIndex < (int)_values.size())
        return _values[_selectedIndex];
    else
        return "";
}

void GComboBox::setValue(const std::string& value)
{
    setSelectedIndex(ToolSet::findInStringArray(_values, value));
}

void GComboBox::setSelectedIndex(int value)
{
    if (_selectedIndex == value)
        return;

    _selectedIndex = value;
    if (_selectedIndex >= 0 && _selectedIndex < (int)_items.size())
    {
        setText(_items[_selectedIndex]);
        if (!_icons.empty() && _selectedIndex != -1 && _selectedIndex < (int)_icons.size())
            setIcon(_icons[_selectedIndex]);
    }
    else
    {
        setTitle("");
        if (!_icons.empty())
            setIcon("");
    }

    updateSelectionController();
}

void GComboBox::refresh()
{
    if (!_items.empty())
    {
        if (_selectedIndex >= (int)_items.size())
            _selectedIndex = (int)_items.size() - 1;
        else if (_selectedIndex == -1)
            _selectedIndex = 0;
        setTitle(_items[_selectedIndex]);
    }
    else
    {
        setTitle("");
        _selectedIndex = -1;
    }

    if (!_icons.empty())
    {
        if (_selectedIndex != -1 && _selectedIndex < (int)_icons.size())
            setIcon(_icons[_selectedIndex]);
        else
            setIcon("");
    }

    _itemsUpdated = true;
}

void GComboBox::setState(const std::string& value)
{
    if (_buttonController != nullptr)
        _buttonController->setSelectedPage(value);
}

void GComboBox::setCurrentState()
{
    if (isGrayed() && _buttonController != nullptr && _buttonController->hasPage(GButton::DISABLED))
        setState(GButton::DISABLED);
    else if (_dropdown != nullptr && _dropdown->getParent() != nullptr)
        setState(GButton::DOWN);
    else
        setState(_over ? GButton::OVER : GButton::UP);
}

void GComboBox::updateSelectionController()
{
    if (_selectionController != nullptr && !_selectionController->changing && _selectedIndex < _selectionController->getPageCount())
    {
        GController* c = _selectionController;
        _selectionController = nullptr;
        c->setSelectedIndex(_selectedIndex);
        _selectionController = c;
    }
}

void GComboBox::updateDropdownList()
{
    if (_itemsUpdated)
    {
        _itemsUpdated = false;
        renderDropdownList();
        _list->resizeToFit(visibleItemCount);
    }
}

void GComboBox::showDropdown()
{
    updateDropdownList();
    if (_list->getSelectionMode() == ListSelectionMode::SINGLE)
        _list->setSelectedIndex(-1);
    _dropdown->setWidth(_size.width);
    _list->ensureBoundsCorrect();

    GRoot::getInstance()->togglePopup(_dropdown, this, popupDirection);
    if (_dropdown->getParent() != nullptr)
        setState(GButton::DOWN);
}

void GComboBox::renderDropdownList()
{
    _list->removeChildrenToPool();
    size_t cnt = _items.size();
    for (size_t i = 0; i < cnt; i++)
    {
        GObject* item = _list->addItemFromPool();
        item->setText(_items[i]);
        item->setIcon((!_icons.empty() && i < _icons.size()) ? _icons[i] : "");
        item->name = i < _values.size() ? _values[i] : "";
    }
}

void GComboBox::handleControllerChanged(GController* c)
{
    GComponent::handleControllerChanged(c);

    if (_selectionController == c)
        setSelectedIndex(c->getSelectedIndex());
}

void GComboBox::handleGrayedChanged()
{
    if (_buttonController != nullptr && _buttonController->hasPage(GButton::DISABLED))
    {
        if (isGrayed())
            setState(GButton::DISABLED);
        else
            setState(GButton::UP);
    }
    else
        GComponent::handleGrayedChanged();
}

GTextField* GComboBox::getTextField() const
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

Variant GComboBox::getProp(ObjectPropID propId)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        return Variant(ToolSet::colorToInt(getTitleColor()));
    case ObjectPropID::OutlineColor:
    {
        GTextField* tf = getTextField();
        if (tf != nullptr)
            return Variant(ToolSet::colorToInt(tf->getOutlineColor()));
        else
            return Variant();
    }
    case ObjectPropID::FontSize:
        return Variant(getTitleFontSize());
    default:
        return GComponent::getProp(propId);
    }
}

void GComboBox::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        setTitleColor(ToolSet::intToColor(value.operator int()));
        break;
    case ObjectPropID::OutlineColor:
    {
        GTextField* tf = getTextField();
        if (tf != nullptr)
            tf->setOutlineColor(ToolSet::intToColor(value.operator int()));
        break;
    }
    case ObjectPropID::FontSize:
        setTitleFontSize(value.operator int64_t());
        break;
    default:
        GComponent::setProp(propId, value);
        break;
    }
}

void GComboBox::constructExtension(ByteBuffer* buffer)
{
    buffer->seek(0, 6);

    _buttonController = getController("button");
    _titleObject = getChild("title");
    _iconObject = getChild("icon");

    const std::string& dropdown = buffer->readS();
    if (!dropdown.empty())
    {
        _dropdown = dynamic_cast<GComponent*>(UIPackage::createObjectFromURL(dropdown));
        // CCASSERT(_dropdown != nullptr, "FairyGUI: should be a component.")

        _list = dynamic_cast<GList*>(_dropdown->getChild("list"));
        if (_list == nullptr)
            return;

        _list->addEventListener(UIEventType::ClickItem, [this](EventContext* ctx) { onClickItem(ctx); });

        _list->addRelation(_dropdown, RelationType::Width);
        _list->removeRelation(_dropdown, RelationType::Height);

        _dropdown->addRelation(_list, RelationType::Height);
        _dropdown->removeRelation(_list, RelationType::Width);

        _dropdown->addEventListener(UIEventType::Exit, [this](EventContext* ctx) { onPopupWinClosed(ctx); });
    }

    addEventListener(UIEventType::RollOver, [this](EventContext* ctx) { onRollover(ctx); });
    addEventListener(UIEventType::RollOut, [this](EventContext* ctx) { onRollout(ctx); });
    addEventListener(UIEventType::TouchBegin, [this](EventContext* ctx) { onTouchBegin(ctx); });
    addEventListener(UIEventType::TouchEnd, [this](EventContext* ctx) { onTouchEnd(ctx); });
}

void GComboBox::setup_afterAdd(ByteBuffer* buffer, int beginPos)
{
    GComponent::setup_afterAdd(buffer, beginPos);

    if (!buffer->seek(beginPos, 6))
        return;

    if ((ObjectType)buffer->readByte() != _packageItem->objectType)
        return;

    const std::string* str;
    bool hasIcon = false;
    int itemCount = buffer->readShort();
    for (int i = 0; i < itemCount; i++)
    {
        int nextPos = buffer->readShort();
        nextPos += buffer->getPos();

        _items.push_back(buffer->readS());
        _values.push_back(buffer->readS());
        if ((str = buffer->readSP()))
        {
            if (!hasIcon)
            {
                for (int j = 0; j < (int)_items.size() - 1; j++)
                    _icons.push_back("");
            }
            _icons.push_back(*str);
        }

        buffer->setPos(nextPos);
    }

    if ((str = buffer->readSP()))
    {
        setTitle(*str);
        _selectedIndex = ToolSet::findInStringArray(_items, *str);
    }
    else if (!_items.empty())
    {
        _selectedIndex = 0;
        setTitle(_items[0]);
    }
    else
        _selectedIndex = -1;

    if ((str = buffer->readSP()))
        setIcon(*str);

    if (buffer->readBool())
        setTitleColor((Color)buffer->readColor());
    int iv = buffer->readInt();
    if (iv > 0)
        visibleItemCount = iv;
    popupDirection = (PopupDirection)buffer->readByte();

    iv = buffer->readShort();
    if (iv >= 0)
        _selectionController = _parent->getControllerAt(iv);
}

void GComboBox::onClickItem(EventContext* context)
{
    if (dynamic_cast<GRoot*>(_dropdown->getParent()))
        ((GRoot*)_dropdown->getParent())->hidePopup(_dropdown);
    _selectedIndex = INT_MIN;
    setSelectedIndex(_list->getChildIndex((GObject*)context->getData()));

    dispatchEvent(UIEventType::Changed);
}

void GComboBox::onRollover(EventContext* context)
{
    _over = true;
    if (_down || (_dropdown != nullptr && _dropdown->getParent() != nullptr))
        return;

    setCurrentState();
}

void GComboBox::onRollout(EventContext* context)
{
    _over = false;
    if (_down || (_dropdown != nullptr && _dropdown->getParent() != nullptr))
        return;

    setCurrentState();
}

void GComboBox::onTouchBegin(EventContext* context)
{
    if (context->getInput()->getButton() != MouseButton::LEFT)
        return;

    if (dynamic_cast<GTextInput*>(context->getInput()->getTarget()))
        return;

    _down = true;

    if (_dropdown != nullptr)
        showDropdown();

    context->captureTouch();
}

void GComboBox::onTouchEnd(EventContext* context)
{
    if (context->getInput()->getButton() != MouseButton::LEFT)
        return;

    if (_down)
    {
        _down = false;
        if (_dropdown != nullptr && _dropdown->getParent() != nullptr)
            setCurrentState();
    }
}

void GComboBox::onPopupWinClosed(EventContext* context)
{
    setCurrentState();
}

void GComboBox::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setTitle", "title"), &GComboBox::gd_setTitle);
    ClassDB::bind_method(D_METHOD("getTitle"), &GComboBox::gd_getTitle);

    ClassDB::bind_method(D_METHOD("setTitleColor", "color"), &GComboBox::setTitleColor);
    ClassDB::bind_method(D_METHOD("getTitleColor"), &GComboBox::gd_getTitleColor);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "titleColor"), "setTitleColor", "getTitleColor");

    ClassDB::bind_method(D_METHOD("setTitleFontSize", "size"), &GComboBox::setTitleFontSize);
    ClassDB::bind_method(D_METHOD("getTitleFontSize"), &GComboBox::getTitleFontSize);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "titleFontSize"), "setTitleFontSize", "getTitleFontSize");

    ClassDB::bind_method(D_METHOD("setIcon", "icon"), &GComboBox::gd_setIcon);
    ClassDB::bind_method(D_METHOD("getIcon"), &GComboBox::gd_getIcon);

    ClassDB::bind_method(D_METHOD("setValue", "value"), &GComboBox::gd_setValue);
    ClassDB::bind_method(D_METHOD("getValue"), &GComboBox::gd_getValue);

    ClassDB::bind_method(D_METHOD("setSelectedIndex", "index"), &GComboBox::setSelectedIndex);
    ClassDB::bind_method(D_METHOD("getSelectedIndex"), &GComboBox::getSelectedIndex);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "selectedIndex"), "setSelectedIndex", "getSelectedIndex");

    ClassDB::bind_method(D_METHOD("getDropdown"), &GComboBox::getDropdown);
    ClassDB::bind_method(D_METHOD("refresh"), &GComboBox::refresh);
}

void GComboBox::gd_setTitle(const String& value) { setTitle(value.utf8().get_data()); }
String GComboBox::gd_getTitle() const { return GObject::toGodotStr(getTitle()); }
void GComboBox::gd_setIcon(const String& value) { setIcon(value.utf8().get_data()); }
String GComboBox::gd_getIcon() const { return GObject::toGodotStr(getIcon()); }
void GComboBox::gd_setValue(const String& value) { setValue(value.utf8().get_data()); }
String GComboBox::gd_getValue() const { return GObject::toGodotStr(getValue()); }

NS_FGUI_END


