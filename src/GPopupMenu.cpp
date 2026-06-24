#include "GPopupMenu.h"
#include "GRoot.h"
#include "UIPackage.h"
#include "GList.h"
#include "GButton.h"
#include "UIConfig.h"

NS_FGUI_BEGIN
GPopupMenu* GPopupMenu::create(const std::string & resourceURL)
{
    Ref<GPopupMenu> ref = memnew(GPopupMenu);
    GPopupMenu* pRet = ref.ptr();
    if (pRet->init(resourceURL))
    {
        pRet->reference(); // keep alive after ref dtor (2→1)
        return pRet;
    }
    // ref dtor cleans up (1→0→freed)
    return nullptr;
}

GPopupMenu::GPopupMenu() :
    _contentPane(nullptr),
    _list(nullptr)
{
}

GPopupMenu::~GPopupMenu()
{
    // // CC_SAFE_RELEASE removed - _contentPane managed by Godot ref counting;
}

bool GPopupMenu::init(const std::string & resourceURL)
{
    std::string url = resourceURL;
    if (url.empty())
    {
        url = UIConfig::popupMenu;
        if (url.empty())
        {
            // CCLOGWARN("FairyGUI: UIConfig.popupMenu not defined");
            return false;
        }
    }

    _contentPane = UIPackage::createObjectFromURL(url)->as<GComponent>();
    _contentPane->addEventListener(UIEventType::Enter, [this](EventContext* ctx) { GPopupMenu::onEnter(ctx); });

    _list = _contentPane->getChild("list")->as<GList>();
    _list->removeChildrenToPool();

    _list->addRelation(_contentPane, RelationType::Width);
    _list->removeRelation(_contentPane, RelationType::Height);
    _contentPane->addRelation(_list, RelationType::Height);

    _list->addEventListener(UIEventType::ClickItem, [this](EventContext* ctx) { GPopupMenu::onClickItem(ctx); });

    return true;
}

GButton * GPopupMenu::addItem(const std::string & caption, EventCallback callback)
{
    GButton* item = _list->addItemFromPool()->as<GButton>();
    item->setTitle(caption);
    item->setGrayed(false);
    GController* c = item->getController("checked");
    if (c != nullptr)
        c->setSelectedIndex(0);
    item->removeEventListener(UIEventType::ClickMenu);
    if (callback)
        item->addEventListener(UIEventType::ClickMenu, callback);

    return item;
}

GButton * GPopupMenu::addItemAt(const std::string & caption, int index, EventCallback callback)
{
    GButton* item = _list->getFromPool(_list->getDefaultItem())->as<GButton>();
    _list->addChildAt(item, index);

    item->setTitle(caption);
    item->setGrayed(false);
    GController* c = item->getController("checked");
    if (c != nullptr)
        c->setSelectedIndex(0);
    item->removeEventListener(UIEventType::ClickMenu);
    if (callback)
        item->addEventListener(UIEventType::ClickMenu, callback);

    return item;
}

void GPopupMenu::addSeperator()
{
    if (UIConfig::popupMenu_seperator.empty())
    {
        // CCLOGWARN("FairyGUI: UIConfig.popupMenu_seperator not defined");
        return;
    }

    _list->addItemFromPool(UIConfig::popupMenu_seperator);
}

const std::string & GPopupMenu::getItemName(int index) const
{
    GButton* item = _list->getChildAt(index)->as<GButton>();
    return item->name;
}

void GPopupMenu::setItemText(const std::string & name, const std::string & caption)
{
    GButton* item = _list->getChild(name)->as<GButton>();
    item->setTitle(caption);
}

void GPopupMenu::setItemVisible(const std::string & name, bool visible)
{
    GButton* item = _list->getChild(name)->as<GButton>();
    if (((CanvasItem*)item->displayObject())->is_visible() != visible)
    {
        ((CanvasItem*)item->displayObject())->set_visible(visible);
        _list->setBoundsChangedFlag();
    }
}

void GPopupMenu::setItemGrayed(const std::string & name, bool grayed)
{
    GButton* item = _list->getChild(name)->as<GButton>();
    item->setGrayed(grayed);
}

void GPopupMenu::setItemCheckable(const std::string & name, bool checkable)
{
    GButton* item = _list->getChild(name)->as<GButton>();
    GController* c = item->getController("checked");
    if (c != nullptr)
    {
        if (checkable)
        {
            if (c->getSelectedIndex() == 0)
                c->setSelectedIndex(1);
        }
        else
            c->setSelectedIndex(0);
    }
}

void GPopupMenu::setItemChecked(const std::string & name, bool check)
{
    GButton* item = _list->getChild(name)->as<GButton>();
    GController* c = item->getController("checked");
    if (c != nullptr)
        c->setSelectedIndex(check ? 2 : 1);
}

bool GPopupMenu::isItemChecked(const std::string & name) const
{
    GButton* item = _list->getChild(name)->as<GButton>();
    GController* c = item->getController("checked");
    if (c != nullptr)
        return c->getSelectedIndex() == 2;
    else
        return false;
}

bool GPopupMenu::removeItem(const std::string & name)
{
    GObject* item = _list->getChild(name);
    if (item != nullptr)
    {
        int index = _list->getChildIndex(item);
        _list->removeChildToPoolAt(index);
        item->removeEventListener(UIEventType::ClickMenu);

        return true;
    }
    else
        return false;
}

void GPopupMenu::clearItems()
{
    int cnt = _list->numChildren();
    for (int i = 0; i < cnt; i++)
        _list->getChildAt(i)->removeEventListener(UIEventType::ClickMenu);
    _list->removeChildrenToPool();
}

int GPopupMenu::getItemCount() const
{
    return _list->numChildren();
}

void GPopupMenu::show(GObject * target, PopupDirection dir)
{
    GRoot* r = target != nullptr ? target->getRoot() : GRoot::getInstance();
    r->showPopup(_contentPane, dynamic_cast<GRoot*>(target) ? nullptr : target, dir);
}

void GPopupMenu::onClickItem(EventContext * context)
{
    GButton* item = ((GObject*)context->getData())->as<GButton>();
    if (item == nullptr)
        return;

    if (item->isGrayed())
    {
        _list->setSelectedIndex(-1);
        return;
    }

    GController* c = item->getController("checked");
    if (c != nullptr && c->getSelectedIndex() != 0)
    {
        if (c->getSelectedIndex() == 1)
            c->setSelectedIndex(2);
        else
            c->setSelectedIndex(1);
    }

    GRoot* r = (GRoot*)_contentPane->getParent();
    r->hidePopup(_contentPane);

    item->dispatchEvent(UIEventType::ClickMenu, context->getData());
}

void GPopupMenu::onEnter(EventContext * context)
{
    _list->setSelectedIndex(-1);
    _list->resizeToFit(INT_MAX, 10);
}

void GPopupMenu::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("addSeperator"), &GPopupMenu::addSeperator);
    ClassDB::bind_method(D_METHOD("clearItems"), &GPopupMenu::clearItems);
    ClassDB::bind_method(D_METHOD("getItemCount"), &GPopupMenu::getItemCount);
    ClassDB::bind_method(D_METHOD("getList"), &GPopupMenu::getList);
    ClassDB::bind_method(D_METHOD("getContentPane"), &GPopupMenu::getContentPane);

    ClassDB::bind_method(D_METHOD("getItemName", "index"), &GPopupMenu::gd_getItemName);
    ClassDB::bind_method(D_METHOD("setItemText", "name", "caption"), &GPopupMenu::gd_setItemText);
    ClassDB::bind_method(D_METHOD("setItemVisible", "name", "visible"), &GPopupMenu::gd_setItemVisible);
    ClassDB::bind_method(D_METHOD("removeItem", "name"), &GPopupMenu::gd_removeItem);

    ClassDB::bind_method(D_METHOD("setItemGrayed", "name", "grayed"), &GPopupMenu::gd_setItemGrayed);
    ClassDB::bind_method(D_METHOD("setItemCheckable", "name", "checkable"), &GPopupMenu::gd_setItemCheckable);
    ClassDB::bind_method(D_METHOD("setItemChecked", "name", "check"), &GPopupMenu::gd_setItemChecked);
    ClassDB::bind_method(D_METHOD("isItemChecked", "name"), &GPopupMenu::gd_isItemChecked);
    ClassDB::bind_method(D_METHOD("show"), static_cast<void(GPopupMenu::*)()>(&GPopupMenu::show));
    ClassDB::bind_method(D_METHOD("showMenuAt", "target", "dir"), &GPopupMenu::gd_showMenuAt);
}

void GPopupMenu::gd_showMenuAt(GObject* target, int dir) { show(target, static_cast<PopupDirection>(dir)); }

String GPopupMenu::gd_getItemName(int index) const { return String(getItemName(index).c_str()); }
void GPopupMenu::gd_setItemText(const String& name, const String& caption) { setItemText(name.utf8().get_data(), caption.utf8().get_data()); }
void GPopupMenu::gd_setItemVisible(const String& name, bool visible) { setItemVisible(name.utf8().get_data(), visible); }
bool GPopupMenu::gd_removeItem(const String& name) { return removeItem(name.utf8().get_data()); }
void GPopupMenu::gd_setItemGrayed(const String& name, bool grayed) { setItemGrayed(name.utf8().get_data(), grayed); }
void GPopupMenu::gd_setItemCheckable(const String& name, bool checkable) { setItemCheckable(name.utf8().get_data(), checkable); }
void GPopupMenu::gd_setItemChecked(const String& name, bool check) { setItemChecked(name.utf8().get_data(), check); }
bool GPopupMenu::gd_isItemChecked(const String& name) { return isItemChecked(name.utf8().get_data()); }

NS_FGUI_END
