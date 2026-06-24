#ifndef __HTMLPARSER_H__
#define __HTMLPARSER_H__

// cocos2d.h removed - see godot_types.h
#include "FairyGUIMacros.h"
#include "display/TextFormat.h"

NS_FGUI_BEGIN

class HtmlElement;

class HtmlParseOptions
{
public:
    static bool defaultLinkUnderline;
    static Color defaultLinkColor;

    HtmlParseOptions();

    bool linkUnderline;
    Color linkColor;
};

class HtmlParser
{
public:
    static HtmlParser defaultParser;
    static HtmlParser& getDefaultParser();

    HtmlParser();
    virtual ~HtmlParser();

    void parse(const std::string& source, const TextFormat& format, std::vector<HtmlElement*>& elements, const HtmlParseOptions& parseOptions);

    void startElement(void *ctx, const char *name, const char **atts);
    void endElement(void *ctx, const char *name);
    void textHandler(void *ctx, const char *s, size_t len);

private:
    Dictionary parseAttrs(const char ** attrs);
    int attributeInt(const  Dictionary& vm, const std::string& key, int defaultValue);

    void pushTextFormat();
    void popTextFormat();
    void addNewLine(bool check);
    void finishTextBlock();

    std::vector<HtmlElement*>* _elements;
    std::vector<TextFormat> _textFormatStack;
    std::vector<HtmlElement*> _linkStack;
    HtmlElement* _currentSelect;
    TextFormat _format;
    size_t _textFormatStackTop;
    int _skipText;
    bool _ignoreWhiteSpace;
    std::string _textBlock;
    HtmlParseOptions _parseOptions;
};

NS_FGUI_END

#endif
