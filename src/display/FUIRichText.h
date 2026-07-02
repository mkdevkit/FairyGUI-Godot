#ifndef __FUIRICHTEXT_H__
#define __FUIRICHTEXT_H__

#include "FairyGUIMacros.h"
#include "TextFormat.h"
#include "utils/html/HtmlParser.h"

NS_FGUI_BEGIN

class HtmlElement;
class HtmlObject;

class FUIRichText : public Node2D
{
    GDCLASS(FUIRichText, Node2D)

public:
    FUIRichText();
    virtual ~FUIRichText();

    static FUIRichText* create();

    void setDimensions(float width, float height);
    Vector2 getDimensions() const { return Vector2(_dimensionsX, _dimensionsY); }
    virtual Vector2 get_content_size() const;

    void setText(const std::string& value);

    void gd_setText(const String& value);
    Node* gd_getControl(const String& name);
    String gd_hitTestLink(const Vector2& worldPoint);

    TextFormat* getTextFormat() { return &_textFormat; }
    void applyTextFormat();

    int getOverflow() const { return _overflow; }
    void setOverflow(int overflow);

    void setObjectFactory(const std::function<HtmlObject*(HtmlElement*)>& value) { _objectFactory = value; }
    HtmlParseOptions& parseOptions() { return _parseOptions; }

    const std::vector<HtmlObject*>& getControls() const { return _controls; }
    HtmlObject* getControl(const std::string& name) const;

    const char* hitTestLink(const Vector2& worldPoint);

    static void _bind_methods();
    void _ready();

protected:

private:
    void formatText();
    void formarRenderers();
    void handleTextRenderer(HtmlElement* element, const std::string& text);
    void handleRichRenderer(HtmlElement* element, HtmlObject* obj);
    void addNewLine();
    int findSplitPositionForWord(Node* label, const std::string& text);
    void doHorizontalAlignment(const std::vector<Node*>& row, float rowWidth);

    std::vector<HtmlElement*> _elements;
    std::vector<HtmlObject*> _objects;
    std::vector<HtmlObject*> _controls;

    HtmlParser* _parser;
    std::function<HtmlObject*(HtmlElement*)> _objectFactory;
    HtmlParseOptions _parseOptions;

    TextFormat _textFormat;
    float _dimensionsX;
    float _dimensionsY;
    float _contentWidth;
    float _contentHeight;
    int _overflow;
    bool _dirty;

    std::vector<std::vector<Node*>> _renderers;
    float _leftSpaceWidth;
    float _textRectWidth;
    int _numLines;
};

NS_FGUI_END

#endif
