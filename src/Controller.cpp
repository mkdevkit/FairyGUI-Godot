#include "Controller.h"
#include "GComponent.h"
#include "controller_action/ControllerAction.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN
GController::GController() : changing(false),
                             autoRadioGroupDepth(false),
                             _parent(nullptr),
                             _selectedIndex(-1),
                             _previousIndex(-1)
{
}

GController::~GController()
{
    for (auto& it : _actions)
        delete it;
}

void GController::setSelectedIndex(int value, bool triggerEvent)
{
    if (_selectedIndex != value)
    {
        // CCASSERT(value < (int)_pageIds.size(), "Invalid selected index");

        changing = true;

        _previousIndex = _selectedIndex;
        _selectedIndex = value;
        _parent->applyController(this);

        if (triggerEvent)
            dispatchEvent(UIEventType::Changed);

        changing = false;
    }
}

const std::string& GController::getSelectedPage() const
{
    if (_selectedIndex == -1)
        return "";
    else
        return _pageNames[_selectedIndex];
}

void GController::setSelectedPage(const std::string& value, bool triggerEvent)
{
    int i = ToolSet::findInStringArray(_pageNames, value);
    if (i == -1)
        i = 0;
    setSelectedIndex(i, triggerEvent);
}

const std::string& GController::getSelectedPageId() const
{
    if (_selectedIndex == -1)
        return "";
    else
        return _pageIds[_selectedIndex];
}

void GController::setSelectedPageId(const std::string& value, bool triggerEvent)
{
    int i = ToolSet::findInStringArray(_pageIds, value);
    if (i != -1)
        setSelectedIndex(i, triggerEvent);
}

const std::string& GController::getPreviousPage() const
{
    if (_previousIndex == -1)
        return "";
    else
        return _pageNames[_previousIndex];
}

const std::string& GController::getPreviousPageId() const
{
    if (_previousIndex == -1)
        return "";
    else
        return _pageIds[_previousIndex];
}

int GController::getPageCount() const
{
    return (int)_pageIds.size();
}

bool GController::hasPage(const std::string& aName) const
{
    return ToolSet::findInStringArray(_pageNames, aName) != -1;
}

int GController::getPageIndexById(const std::string& value) const
{
    return ToolSet::findInStringArray(_pageIds, value);
}

const std::string& GController::getPageNameById(const std::string& value) const
{
    int i = ToolSet::findInStringArray(_pageIds, value);
    if (i != -1)
        return _pageNames[i];
    else
        return "";
}

const std::string& GController::getPageId(int index) const
{
    return _pageIds[index];
}

void GController::setOppositePageId(const std::string& value)
{
    int i = ToolSet::findInStringArray(_pageIds, value);
    if (i > 0)
        setSelectedIndex(0);
    else if (_pageIds.size() > 1)
        setSelectedIndex(1);
}

void GController::runActions()
{
    if (_actions.empty())
        return;

    for (auto& it : _actions)
        it->run(this, getPreviousPageId(), getSelectedPageId());
}

void GController::setup(ByteBuffer* buffer)
{
    int beginPos = buffer->getPos();
    buffer->seek(beginPos, 0);

    name = buffer->readS();
    autoRadioGroupDepth = buffer->readBool();

    buffer->seek(beginPos, 1);

    int cnt = buffer->readShort();
    _pageIds.resize(cnt);
    _pageNames.resize(cnt);
    for (int i = 0; i < cnt; i++)
    {
        _pageIds[i].assign(buffer->readS());
        _pageNames[i].assign(buffer->readS());
    }

    int homePageIndex = 0;
    if (buffer->version >= 2)
    {
        int homePageType = buffer->readByte();
        switch (homePageType)
        {
        case 1:
            homePageIndex = buffer->readShort();
            break;

        case 2:
            homePageIndex = ToolSet::findInStringArray(_pageNames, UIPackage::getBranch());
            if (homePageIndex == -1)
                homePageIndex = 0;
            break;

        case 3:
            homePageIndex = ToolSet::findInStringArray(_pageNames, UIPackage::getVar(buffer->readS()));
            if (homePageIndex == -1)
                homePageIndex = 0;
            break;
        }
    }

    buffer->seek(beginPos, 2);

    cnt = buffer->readShort();
    if (cnt > 0)
    {
        for (int i = 0; i < cnt; i++)
        {
            int nextPos = buffer->readUshort();
            nextPos += buffer->getPos();

            ControllerAction* action = ControllerAction::createAction(buffer->readByte());
            action->setup(buffer);
            _actions.push_back(action);

            buffer->setPos(nextPos);
        }
    }

    if (_parent != nullptr && _pageIds.size() > 0)
        _selectedIndex = homePageIndex;
    else
        _selectedIndex = -1;
}

void GController::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setSelectedIndex", "index"), &GController::gd_setSelectedIndex);
    ClassDB::bind_method(D_METHOD("getSelectedIndex"), &GController::getSelectedIndex);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "selectedIndex"), "setSelectedIndex", "getSelectedIndex");

    ClassDB::bind_method(D_METHOD("getPageCount"), &GController::getPageCount);
    ClassDB::bind_method(D_METHOD("getPreviousPage"), &GController::gd_getPreviousPage);
    ClassDB::bind_method(D_METHOD("getPreviousPageId"), &GController::gd_getPreviousPageId);

    ClassDB::bind_method(D_METHOD("setSelectedPage", "value"), &GController::gd_setSelectedPage);
    ClassDB::bind_method(D_METHOD("getSelectedPage"), &GController::gd_getSelectedPage);
    ClassDB::bind_method(D_METHOD("setSelectedPageId", "value"), &GController::gd_setSelectedPageId);
    ClassDB::bind_method(D_METHOD("getSelectedPageId"), &GController::gd_getSelectedPageId);

    ClassDB::bind_method(D_METHOD("getParent"), &GController::getParent);
    ClassDB::bind_method(D_METHOD("setParent", "parent"), &GController::setParent);
}

void GController::gd_setSelectedPage(const String& value) { setSelectedPage(value.utf8().get_data()); }
String GController::gd_getSelectedPage() const { return String(getSelectedPage().c_str()); }
void GController::gd_setSelectedPageId(const String& value) { setSelectedPageId(value.utf8().get_data()); }
String GController::gd_getSelectedPageId() const { return String(getSelectedPageId().c_str()); }
String GController::gd_getPreviousPage() const { return String(getPreviousPage().c_str()); }
String GController::gd_getPreviousPageId() const { return String(getPreviousPageId().c_str()); }

NS_FGUI_END
