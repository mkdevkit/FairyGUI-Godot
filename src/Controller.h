#ifndef __GCONTROLLER_H__
#define __GCONTROLLER_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h
#include "event/UIEventDispatcher.h"

NS_FGUI_BEGIN

class GComponent;
class ControllerAction;
class ByteBuffer;

class GController : public UIEventDispatcher
{
    GDCLASS(GController, UIEventDispatcher)

public:
    GController();
    virtual ~GController();

    static void _bind_methods();

    GComponent* getParent() const { return _parent; }
    void setParent(GComponent* value) { _parent = value; }

    int getSelectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(int value, bool triggerEvent = true);
    void gd_setSelectedIndex(int value) { setSelectedIndex(value); }

    const std::string& getSelectedPage() const;
    void setSelectedPage(const std::string& value, bool triggerEvent = true);

    const std::string& getSelectedPageId() const;
    void setSelectedPageId(const std::string& value, bool triggerEvent = true);

    void gd_setSelectedPage(const String& value);
    String gd_getSelectedPage() const;
    void gd_setSelectedPageId(const String& value);
    String gd_getSelectedPageId() const;

    int getPrevisousIndex() const { return _previousIndex; }
    const std::string& getPreviousPage() const;
    const std::string& getPreviousPageId() const;

    String gd_getPreviousPage() const;
    String gd_getPreviousPageId() const;

    int getPageCount() const;
    bool hasPage(const std::string& aName) const;
    int getPageIndexById(const std::string& value) const;
    const std::string& getPageNameById(const std::string& value) const;
    const std::string& getPageId(int index) const;
    void setOppositePageId(const std::string& value);
    void runActions();

    void setup(ByteBuffer* buffer);

    std::string name;
    bool changing;
    bool autoRadioGroupDepth;

private:
    GComponent* _parent;
    int _selectedIndex;
    int _previousIndex;
    std::vector<std::string> _pageIds;
    std::vector<std::string> _pageNames;
    std::vector<ControllerAction*> _actions;
};

NS_FGUI_END

#endif
