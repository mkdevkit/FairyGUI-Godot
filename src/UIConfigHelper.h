#ifndef __UICONFIG_HELPER_H__
#define __UICONFIG_HELPER_H__

#include "FairyGUIMacros.h"
#include "FieldTypes.h"

NS_FGUI_BEGIN

class UIConfigHelper : public RefCounted
{
    GDCLASS(UIConfigHelper, RefCounted)

public:
    static UIConfigHelper* getInstance();

    void setDefaultFont(const String& value);
    String getDefaultFont() const;
    void setButtonSound(const String& value);
    String getButtonSound() const;
    void setButtonSoundVolumeScale(float value);
    float getButtonSoundVolumeScale() const;
    void setDefaultScrollStep(int value);
    int getDefaultScrollStep() const;
    void setDefaultScrollDecelerationRate(float value);
    float getDefaultScrollDecelerationRate() const;
    void setDefaultScrollTouchEffect(bool value);
    bool getDefaultScrollTouchEffect() const;
    void setDefaultScrollBounceEffect(bool value);
    bool getDefaultScrollBounceEffect() const;
    void setDefaultScrollBarDisplay(int value);
    int getDefaultScrollBarDisplay() const;
    void setVerticalScrollBar(const String& value);
    String getVerticalScrollBar() const;
    void setHorizontalScrollBar(const String& value);
    String getHorizontalScrollBar() const;
    void setTouchDragSensitivity(int value);
    int getTouchDragSensitivity() const;
    void setClickDragSensitivity(int value);
    int getClickDragSensitivity() const;
    void setTouchScrollSensitivity(int value);
    int getTouchScrollSensitivity() const;
    void setDefaultComboBoxVisibleItemCount(int value);
    int getDefaultComboBoxVisibleItemCount() const;
    void setGlobalModalWaiting(const String& value);
    String getGlobalModalWaiting() const;
    void setModalLayerColor(const Color& value);
    Color getModalLayerColor() const;
    void setTooltipsWin(const String& value);
    String getTooltipsWin() const;
    void setBringWindowToFrontOnClick(bool value);
    bool getBringWindowToFrontOnClick() const;
    void setWindowModalWaiting(const String& value);
    String getWindowModalWaiting() const;
    void setPopupMenu(const String& value);
    String getPopupMenu() const;
    void setPopupMenuSeperator(const String& value);
    String getPopupMenuSeperator() const;

    void registerFont(const String& aliasName, const String& realName);
    String getRealFontName(const String& aliasName) const;

    static void _bind_methods();

private:
    static UIConfigHelper* _inst;
};

NS_FGUI_END

#endif
