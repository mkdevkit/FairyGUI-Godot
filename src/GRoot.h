#ifndef __GROOT_H__
#define __GROOT_H__

#include "FairyGUIMacros.h"
#include "GComponent.h"
#include "GGraph.h"
#include "Window.h"
#include "godot_types.h"
#include "event/InputProcessor.h"
#include "scene/main/scene_tree.h"

NS_FGUI_BEGIN

class WeakPtr;

class GRoot : public GComponent
{
    GDCLASS(GRoot, GComponent)

public:
    GRoot();
    virtual ~GRoot();

    static GRoot* create(SceneTree* tree, int zOrder = 1000);
    static GRoot* createDeferred(SceneTree* tree, int zOrder = 1000);
    static GRoot* getInstance() { return _inst; }
    static void cleanup();

    static void _bind_methods();

    void showWindow(GWindow* win);
    void hideWindow(GWindow* win);
    void hideWindowImmediately(GWindow* win);
    void bringToFront(GWindow* win);
    void showModalWait();
    void closeModalWait();
    void closeAllExceptModals();
    void closeAllWindows();
    GWindow* getTopWindow();

    GObject* getModalWaitingPane();
    GGraph* getModalLayer();
    bool hasModalWindow();
    bool isModalWaiting();

    // gd_ wrappers for Godot binding
    void gd_showWindow(GWindow* win) { showWindow(win); }
    void gd_hideWindow(GWindow* win) { hideWindow(win); }
    void gd_hideWindowImmediately(GWindow* win) { hideWindowImmediately(win); }
    void gd_bringToFront(GWindow* win) { bringToFront(win); }
    GWindow* gd_getTopWindow() { return getTopWindow(); }
    void gd_closeAllWindows() { closeAllWindows(); }
    bool gd_hasModalWindow() { return hasModalWindow(); }
    void gd_hidePopup() { hidePopup(); }
    GObject* gd_getModalWaitingPane() { return getModalWaitingPane(); }
    GObject* gd_getTouchTarget() { return getTouchTarget(); }
    Vector2 gd_getTouchPosition() { return getTouchPosition(0); }
    void gd_showTooltipsWin(GObject* tooltipWin) { showTooltipsWin(tooltipWin); }
    void gd_showPopup(GObject* popup, GObject* target, int dir) { showPopup(popup, target, (PopupDirection)dir); }
    void gd_showPopupSimple(GObject* popup) { showPopup(popup, nullptr, PopupDirection::AUTO); }

    InputProcessor* getInputProcessor() const { return _inputProcessor; }
    Vector2 getTouchPosition(int touchId);
    GObject* getTouchTarget();

    Vector2 worldToRoot(const Vector2& pt);
    Vector2 rootToWorld(const Vector2& pt);

    void showPopup(GObject* popup);
    void showPopup(GObject* popup, GObject* target, PopupDirection dir);
    void togglePopup(GObject* popup);
    void togglePopup(GObject* popup, GObject* target, PopupDirection dir);
    void hidePopup();
    void hidePopup(GObject* popup);
    bool hasAnyPopup();
    Vector2 getPoupPosition(GObject* popup, GObject* target, PopupDirection dir);

    void showTooltips(const std::string& msg);
    void gd_showTooltips(const String& msg);
    void showTooltipsWin(GObject* tooltipWin);
    void hideTooltips();

    void playSound(const std::string& url, float volumeScale = 1);
    void gd_playSound(const String& url, float volumeScale = 1.0f);
    bool isSoundEnabled() const { return _soundEnabled; }
    void setSoundEnabled(bool value);
    float getSoundVolumeScale() const { return _soundVolumeScale; }
    void setSoundVolumeScale(float value);

    static int contentScaleLevel;

    void setContentScaleFactor(int designWidth, int designHeight, int matchMode);
    const Vector2& getDesignResolution() const { return _designResolution; }
    ScreenMatchMode getScreenMatchMode() const { return _screenMatchMode; }

    void onWindowSizeChanged();
    Node* getDisplayObject() const { return _displayObject; }

protected:
    virtual void handlePositionChanged() override;
    virtual void handleSizeChanged() override;
    virtual void _enter_tree() override;
    virtual void _exit_tree() override;

private:
    bool initWithParent(Node* parent, int zOrder);
    void onInitWithParent(Node* parent, int zOrder, bool deferAdd = false);
    void createModalLayer();
    void adjustModalLayer();
    void closePopup(GObject* target);
    void checkPopups();
    void onTouchEvent(int eventType);
    void updateContentScaleLevel();
    void applyContentScale();
    Node* gd_getDisplayObject();

    CALL_LATER_FUNC(GRoot, doShowTooltipsWin);

    InputProcessor* _inputProcessor;

    GGraph* _modalLayer;
    Ref<GObject> _modalWaitPane;
    std::vector<WeakPtr> _popupStack;
    std::vector<WeakPtr> _justClosedPopups;
    GObject* _tooltipWin;
    GObject* _defaultTooltipWin;

    static bool _soundEnabled;
    static float _soundVolumeScale;

    Vector2 _designResolution;
    ScreenMatchMode _screenMatchMode;
    bool _hasDesignResolution;
    bool _viewportSizeConnected;

    static GRoot* _inst;
};

NS_FGUI_END

#endif
