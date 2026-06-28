#ifndef __UICONFIG_HELPER_H__
#define __UICONFIG_HELPER_H__

#include "FairyGUIMacros.h"

NS_FGUI_BEGIN

class UIConfigHelper : public RefCounted
{
    GDCLASS(UIConfigHelper, RefCounted)

public:
    static UIConfigHelper* getInstance();

    void setVerticalScrollBar(const String& value);
    String getVerticalScrollBar() const;
    void setHorizontalScrollBar(const String& value);
    String getHorizontalScrollBar() const;
    void setTooltipsWin(const String& value);
    String getTooltipsWin() const;
    void setGlobalModalWaiting(const String& value);
    String getGlobalModalWaiting() const;
    void setWindowModalWaiting(const String& value);
    String getWindowModalWaiting() const;
    void setPopupMenu(const String& value);
    String getPopupMenu() const;
    void setButtonSound(const String& value);
    String getButtonSound() const;
    void setModalLayerColor(const Color& value);
    Color getModalLayerColor() const;

    static void _bind_methods();

private:
    static UIConfigHelper* _inst;
};

NS_FGUI_END

#endif
