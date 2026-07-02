#ifndef __POPUPMENU_H__
#define __POPUPMENU_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h
#include "event/UIEventDispatcher.h"
#include "GComponent.h"

NS_FGUI_BEGIN

class GObject;
class GComponent;
class GButton;
class GList;

class GPopupMenu : public RefCounted
{
    GDCLASS(GPopupMenu, RefCounted)

public:
    static Ref<GPopupMenu> create(const std::string& resourceURL);
    static Ref<GPopupMenu> create() { return create(""); }
    static GPopupMenu* gd_create();

    GPopupMenu();
    virtual ~GPopupMenu();

    static void _bind_methods();

    GButton* addItem(const std::string& caption, EventCallback callback = nullptr);
    GButton* addItemAt(const std::string& caption, int index, EventCallback callback = nullptr);
    void addSeperator();
    const std::string& getItemName(int index) const;
    void setItemText(const std::string& name, const std::string& caption);
    void setItemVisible(const std::string& name, bool visible);
    void setItemGrayed(const std::string& name, bool grayed);
    void setItemCheckable(const std::string& name, bool checkable);
    void setItemChecked(const std::string& name, bool check);
    bool isItemChecked(const std::string& name) const;
    bool removeItem(const std::string& name);
    void clearItems();
    int getItemCount() const;

    // gd_ wrappers for GDScript binding
    String gd_getItemName(int index) const;
    void gd_setItemText(const String& name, const String& caption);
    void gd_setItemVisible(const String& name, bool visible);
    void gd_setItemGrayed(const String& name, bool grayed);
    void gd_setItemCheckable(const String& name, bool checkable);
    void gd_setItemChecked(const String& name, bool check);
    bool gd_isItemChecked(const String& name);
    bool gd_removeItem(const String& name);
    void gd_showMenuAt(GObject* target, int dir);
    GButton* gd_addItem(const String& caption);
    GComponent* getContentPane() const { return _contentPane; }
    GList* getList() const { return _list; }
    void show() { show(nullptr, PopupDirection::AUTO); }
    void show(GObject* target, PopupDirection dir);

protected:
    bool init(const std::string& resourceURL);

    Ref<GComponent> _contentPaneRef;
    GComponent* _contentPane;
    GList* _list;

private:
    void onClickItem(EventContext* context);
    void onEnter(EventContext* context);
};

NS_FGUI_END

#endif
