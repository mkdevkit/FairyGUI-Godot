#ifndef __HTMLOBJECT_H__
#define __HTMLOBJECT_H__

// cocos2d.h removed - see godot_types.h
#include "FairyGUIMacros.h"
#include "GObjectPool.h"

NS_FGUI_BEGIN

class FUIRichText;
class HtmlElement;
class GObject;

class HtmlObject
{
public:
    static std::string buttonResource;
    static std::string inputResource;
    static std::string selectResource;
    static bool usePool;

    static GObjectPool& getObjectPool();

    HtmlObject();
    virtual ~HtmlObject();

    HtmlElement* getElement() const { return _element; }
    void setElement(HtmlElement* elem) { _element = elem; }
    GObject* getUI() const { return _ui.ptr(); }
    bool isHidden() const { return _hidden; }
    static std::vector<Ref<GObject>> loaderPool;

    virtual void create(FUIRichText* owner, HtmlElement* element);
    virtual void destroy();

protected:
    void createCommon();
    void createImage();
    void createButton();
    void createInput();
    void createSelect();

    HtmlElement* _element;
    FUIRichText* _owner;
    Ref<GObject> _ui;
    bool _hidden;
};

NS_FGUI_END

#endif
