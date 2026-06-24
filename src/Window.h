#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h
#include "GComponent.h"

NS_FGUI_BEGIN

class IUISource : public RefCounted
{
public:
    virtual const std::string& getFileName() = 0;
    virtual void setFileName(const std::string& value) = 0;
    virtual bool isLoaded() = 0;
    virtual void load(std::function<void()> callback) = 0;
};

class GWindow : public GComponent
{
    GDCLASS(GWindow, GComponent)

public:
    GWindow();
    virtual ~GWindow();

    FAIRYGUI_CREATE(GWindow)

    static void _bind_methods();

    void show();
    void hide();
    void hideImmediately();
    void toggleStatus();
    void bringToFront();
    bool isShowing() const { return _parent != nullptr; }
    bool isTop() const;
    bool isModal() const { return _modal; }
    void setModal(bool value) { _modal = value; }

    void showModalWait() { showModalWait(0); }
    void showModalWait(int requestingCmd);
    bool closeModalWait() { return closeModalWait(0); }
    bool closeModalWait(int requestingCmd);

    void gd_showModalWait(int requestingCmd) { showModalWait(requestingCmd); }
    bool gd_closeModalWait(int requestingCmd) { return closeModalWait(requestingCmd); }

    void initWindow();
    void addUISource(IUISource* uiSource);

    bool isBringToFrontOnClick() { return _bringToFontOnClick; }
    void setBringToFrontOnClick(bool value) { _bringToFontOnClick = value; }

    GComponent* getContentPane() const { return _contentPane; }
    void setContentPane(GComponent* value);
    GComponent* getFrame() const { return _frame; }

    GObject* getCloseButton() const { return _closeButton; }
    void setCloseButton(GObject* value);
    GObject* getDragArea() const { return _dragArea; }
    void setDragArea(GObject* value);
    GObject* getContentArea() const { return _contentArea; }
    void setContentArea(GObject* value) { _contentArea = value; }

    GObject* getModalWaitingPane() const { return _modalWaitPane; }

protected:
    virtual void handleInit() override;
    virtual void onInit() {};
    virtual void onShown() {};
    virtual void onHide() {};
    virtual void doShowAnimation();
    virtual void doHideAnimation();

    virtual void _enter_tree() override;
    virtual void _exit_tree() override;

    void closeEventHandler(EventContext*context);

    GComponent* _contentPane;

private:
    void layoutModalWaitPane();
    void onUILoadComplete();
    void _initWindow();

    void onTouchBegin(EventContext* context);
    void onDragStart(EventContext* context);

    int _requestingCmd;
    GComponent* _frame;
    GObject* _modalWaitPane;
    GObject* _closeButton;
    GObject* _dragArea;
    GObject* _contentArea;
    bool _modal;
    bool _bringToFontOnClick;
    std::vector<IUISource*> _uiSources;
    bool _inited;
    bool _loading;
};

NS_FGUI_END

#endif
