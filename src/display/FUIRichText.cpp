#include "FUIRichText.h"
#include "utils/html/HtmlElement.h"
#include "utils/html/HtmlObject.h"
#include "FUILabel.h"
#include "UIPackage.h"
#include <locale>
#include <algorithm>
#include <sstream>
#include <cfloat>

NS_FGUI_BEGIN
using namespace std;

#if defined(WIN32) || defined(_WIN32)
#define strcasecmp _stricmp
#endif

static const int GUTTER_X = 2;
static const int GUTTER_Y = 2;

static int getPrevWord(const std::string& text, int idx)
{
    for (int i = idx - 1; i >= 0; --i)
    {
        if (!std::isalnum(text[i], std::locale()))
            return i;
    }
    return -1;
}

static bool isWhitespace(char c) {
    return std::isspace(c, std::locale());
}

static void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), isWhitespace));
}

static void rtrim(std::string& s) {
    s.erase(std::find_if_not(s.rbegin(), s.rend(), isWhitespace).base(), s.end());
}

static float stripTrailingWhitespace(const std::vector<Node*>& row) {
    if (!row.empty()) {
        if (auto label = dynamic_cast<FUILabel*>(row.back())) {
            float width = label->getTextWidth();
            auto str = label->getText();
            rtrim(str);
            if (label->getText() != str) {
                label->setText(str);
                return label->getTextWidth() - width;
            }
        }
    }
    return 0.0f;
}

HtmlObject* createHtmlObject(HtmlElement* element)
{
    if (element->type == HtmlElement::Type::IMAGE
        || element->type == HtmlElement::Type::INPUT
        || element->type == HtmlElement::Type::SELECT
        || element->type == HtmlElement::Type::OBJECT)
        return new HtmlObject();
    else
        return nullptr;
}

FUIRichText::FUIRichText() :
    _parser(nullptr),
    _dimensionsX(0),
    _dimensionsY(0),
    _overflow(0),
    _dirty(false)
{
    // can't init _parser here: creating RefCounted during GDREGISTER_CLASS triggers callback check
}

FUIRichText::~FUIRichText()
{
    if (_parser) delete _parser;
    for (auto& obj : _objects)
        delete obj;
    for (auto& elem : _elements)
        delete elem;
}

void FUIRichText::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setDimensions", "width", "height"), &FUIRichText::setDimensions);
    ClassDB::bind_method(D_METHOD("getDimensions"), &FUIRichText::getDimensions);

    ClassDB::bind_method(D_METHOD("setText", "text"), &FUIRichText::gd_setText);

    ClassDB::bind_method(D_METHOD("setOverflow", "overflow"), &FUIRichText::setOverflow);
    ClassDB::bind_method(D_METHOD("getOverflow"), &FUIRichText::getOverflow);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "overflow"), "setOverflow", "getOverflow");

    ClassDB::bind_method(D_METHOD("applyTextFormat"), &FUIRichText::applyTextFormat);

    ClassDB::bind_method(D_METHOD("getControl", "name"), &FUIRichText::gd_getControl);
    ClassDB::bind_method(D_METHOD("hitTestLink", "world_point"), &FUIRichText::gd_hitTestLink);
}

FUIRichText* FUIRichText::create()
{
    return memnew(FUIRichText);
}

void FUIRichText::_ready()
{
    if (_dirty)
        formatText();
}

const char* FUIRichText::hitTestLink(const Vector2& worldPoint)
{
    Vector2 localPt = to_local(worldPoint);
    // GODOT_TODO: test against child labels for link regions
    return nullptr;
}

void FUIRichText::setDimensions(float width, float height)
{
    _dimensionsX = width;
    _dimensionsY = height;
    _dirty = true;
}

void FUIRichText::setOverflow(int overflow)
{
    _overflow = overflow;
    _dirty = true;
}

void FUIRichText::setText(const std::string& value)
{
    _dirty = true;

    // Lazy init: can't create RefCounted in constructor during GDREGISTER_CLASS
    if (!_parser) {
        _parser = new HtmlParser();
        _objectFactory = createHtmlObject;
    }

    // Free old elements
    for (auto& elem : _elements)
        delete elem;
    _elements.clear();
    for (auto& obj : _objects)
        delete obj;
    _objects.clear();
    _controls.clear();

    // Remove all children
    for (int i = get_child_count() - 1; i >= 0; i--)
        get_child(i)->queue_free();

    if (value.empty())
        return;

    _parser->parse(value, _textFormat, _elements, _parseOptions);
    for (auto& elem : _elements)
    {
        HtmlObject* obj = _objectFactory ? _objectFactory(elem) : nullptr;
        if (obj)
        {
            obj->setElement(elem);
            _objects.push_back(obj);
            _controls.push_back(obj);
        }
        else
            _objects.push_back(nullptr);
    }

    formatText();
}

void FUIRichText::applyTextFormat()
{
    _dirty = true;
    formatText();
}

HtmlObject* FUIRichText::getControl(const std::string& name) const
{
    for (auto& ctrl : _controls)
    {
        if (ctrl->getElement()->getString("name") == name)
            return ctrl;
    }
    return nullptr;
}

void FUIRichText::formatText()
{
    for (int i = get_child_count() - 1; i >= 0; i--)
        get_child(i)->queue_free();

    if (_elements.empty())
        return;

    formarRenderers();
    _dirty = false;
}

void FUIRichText::formarRenderers()
{
    float textWidth = (_dimensionsX > 0) ? _dimensionsX - GUTTER_X * 2 : FLT_MAX;
    float rowX = GUTTER_X;
    float rowY = GUTTER_Y;
    float rowHeight = 0;
    std::vector<Node*> rowItems;

    for (size_t i = 0; i < _elements.size(); i++)
    {
        HtmlElement* element = _elements[i];
        HtmlObject* obj = _objects[i];

        if (element->type == HtmlElement::Type::TEXT)
        {
            const std::string& text = element->getString("text");
            if (!text.empty())
            {
                std::size_t startPos = 0;
                while (startPos < text.length())
                {
                    // Find end of segment (until newline or end of text)
                    std::size_t endPos = text.find('\n', startPos);
                    bool hasNewline = (endPos != std::string::npos);
                    std::string segment = hasNewline
                        ? text.substr(startPos, endPos - startPos)
                        : text.substr(startPos);

                    handleTextRenderer(element, segment);

                    if (hasNewline)
                    {
                        addNewLine();
                        startPos = endPos + 1;
                    }
                    else
                    {
                        startPos = text.length();
                    }
                }
            }
        }
        else if (element->type == HtmlElement::Type::BR)
        {
            addNewLine();
        }
        else if (element->type == HtmlElement::Type::IMAGE
            || element->type == HtmlElement::Type::INPUT
            || element->type == HtmlElement::Type::SELECT
            || element->type == HtmlElement::Type::OBJECT)
        {
            // GODOT_TODO: handle rich elements (images, inputs, etc.)
            if (obj)
                handleRichRenderer(element);
        }
    }
}

void FUIRichText::handleTextRenderer(HtmlElement* element, const std::string& text)
{
    FUILabel* label = (FUILabel*)(memnew(FUILabel));
    TextFormat* format = label->getTextFormat();
    format->setFormat(_textFormat);
    // GODOT_TODO: apply element styles (bold, italic, color, font, size, href)

    label->setText(text);
    add_child(label);
    label->set_owner(this);
}

void FUIRichText::handleRichRenderer(HtmlElement* element)
{
    // GODOT_TODO: handle rich elements
}

void FUIRichText::addNewLine()
{
    // GODOT_TODO: track line position for layout
}

int FUIRichText::findSplitPositionForWord(Node* label, const std::string& text)
{
    // GODOT_TODO: word wrapping
    return (int)text.length();
}

void FUIRichText::doHorizontalAlignment(const std::vector<Node*>& row, float rowWidth)
{
    // GODOT_TODO: horizontal alignment
}

void FUIRichText::gd_setText(const String& value) { setText(value.utf8().get_data()); }
Node* FUIRichText::gd_getControl(const String& name) { 
    HtmlObject* obj = getControl(name.utf8().get_data());
    return obj ? reinterpret_cast<Node*>(obj->getUI()) : nullptr; 
}
String FUIRichText::gd_hitTestLink(const Vector2& worldPoint) {
    const char* link = hitTestLink(worldPoint);
    return link ? String(link) : String();
}

NS_FGUI_END
