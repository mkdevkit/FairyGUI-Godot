#include "FUIRichText.h"
#include "utils/html/HtmlElement.h"
#include "utils/html/HtmlObject.h"
#include "FUILabel.h"
#include "GObject.h"
#include "UIPackage.h"
#include "godot_types.h"
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

static bool isWhitespace(char c) {
    return std::isspace(c, std::locale());
}

static void rtrim(std::string& s) {
    s.erase(std::find_if_not(s.rbegin(), s.rend(), isWhitespace).base(), s.end());
}

static int getPrevWord(const std::string& text, int idx)
{
    for (int i = idx - 1; i >= 0; --i)
    {
        if (!std::isalnum((unsigned char)text[i], std::locale()))
            return i;
    }
    return -1;
}

static bool isWrappable(const std::string& text)
{
    for (size_t i = 0, size = text.length(); i < size; ++i)
    {
        if (!std::isalnum((unsigned char)text[i], std::locale()))
            return true;
    }
    return false;
}

static float getPaddingAmount(int alignment, float leftOver) {
    switch (alignment) {
    case 1: // center
        return leftOver / 2.f;
    case 2: // right
        return leftOver;
    default:
        return 0.f;
    }
}

static float stripTrailingWhitespace(const std::vector<Node*>& row) {
    if (!row.empty()) {
        if (auto label = Object::cast_to<FUILabel>(row.back())) {
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

static std::string getSubStringOfUTF8String(const std::string& str, size_t start, size_t length)
{
    String s = String::utf8(str.c_str());
    if (start >= (size_t)s.length())
        return "";
    return s.substr(start, length).utf8().get_data();
}

static void setNodeElement(Node* node, HtmlElement* element)
{
    node->set_meta("_html_element", Variant((int64_t)(uintptr_t)element));
}

static HtmlElement* getNodeElement(Node* node)
{
    Variant v = node->get_meta("_html_element", Variant());
    if (v.get_type() == Variant::INT || v.get_type() == Variant::FLOAT)
        return (HtmlElement*)(uintptr_t)(int64_t)v;
    return nullptr;
}

static Vector2 getNodePosition(Node* node)
{
    if (Node2D* n2d = Object::cast_to<Node2D>(node))
        return n2d->get_position();
    if (Control* ctrl = Object::cast_to<Control>(node))
        return ctrl->get_position();
    return Vector2();
}

static void setNodePosition(Node* node, const Vector2& pos)
{
    if (Node2D* n2d = Object::cast_to<Node2D>(node))
        n2d->set_position(pos);
    else if (Control* ctrl = Object::cast_to<Control>(node))
        ctrl->set_position(pos);
}

static void offsetNodePosition(Node* node, const Vector2& offset)
{
    setNodePosition(node, getNodePosition(node) + offset);
}

static Vector2 getNodeSize(Node* node)
{
    if (FUILabel* label = Object::cast_to<FUILabel>(node))
        return Vector2(label->getTextWidth(), label->getTextHeight());
    HtmlElement* element = getNodeElement(node);
    if (element && element->obj)
        return Vector2(element->obj->getUI()->getWidth(), element->obj->getUI()->getHeight());
    if (Control* ctrl = Object::cast_to<Control>(node))
        return ctrl->get_size();
    return Vector2();
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
    _contentWidth(0),
    _contentHeight(0),
    _overflow(1),
    _dirty(false),
    _leftSpaceWidth(0),
    _textRectWidth(0),
    _numLines(0)
{
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

    ClassDB::bind_method(D_METHOD("isAnchorTextUnderline"), &FUIRichText::isAnchorTextUnderline);
    ClassDB::bind_method(D_METHOD("setAnchorTextUnderline", "enable"), &FUIRichText::setAnchorTextUnderline);
    ClassDB::bind_method(D_METHOD("getAnchorFontColor"), &FUIRichText::getAnchorFontColor);
    ClassDB::bind_method(D_METHOD("setAnchorFontColor", "color"), &FUIRichText::setAnchorFontColor);
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

Vector2 FUIRichText::get_content_size() const
{
    if (_dirty)
        const_cast<FUIRichText*>(this)->formatText();
    return Vector2(_contentWidth, _contentHeight);
}

const char* FUIRichText::hitTestLink(const Vector2& worldPoint)
{
    Vector2 localPt = to_local(worldPoint);
    for (int i = 0; i < get_child_count(); i++)
    {
        Node* child = get_child(i);
        HtmlElement* element = getNodeElement(child);
        if (!element || !element->link)
            continue;

        Vector2 sz = getNodeSize(child);
        Rect2 rect(Vector2(0, 0), sz);
        if (rect.has_point(localPt - getNodePosition(child)))
            return element->link->text.c_str();
    }
    return nullptr;
}

void FUIRichText::setDimensions(float width, float height)
{
    if ((_numLines > 1 && width != _dimensionsX) || width < _contentWidth)
        _dirty = true;
    _dimensionsX = width;
    _dimensionsY = height;
}

void FUIRichText::setOverflow(int overflow)
{
    if (_overflow != overflow)
    {
        _overflow = overflow;
        _dirty = true;
    }
}

void FUIRichText::setAnchorTextUnderline(bool enable)
{
    if (_parseOptions.linkUnderline != enable)
    {
        _parseOptions.linkUnderline = enable;
        _dirty = true;
        formatText();
    }
}

void FUIRichText::setAnchorFontColor(const Color& color)
{
    if (_parseOptions.linkColor != color)
    {
        _parseOptions.linkColor = color;
        _dirty = true;
        formatText();
    }
}

void FUIRichText::setText(const std::string& value)
{
    _text = value;
    _dirty = true;

    if (!_parser) {
        _parser = new HtmlParser();
        _objectFactory = createHtmlObject;
    }

    for (auto& elem : _elements)
        delete elem;
    _elements.clear();
    for (auto& obj : _objects)
        delete obj;
    _objects.clear();
    _controls.clear();
    _numLines = 0;

    for (int i = get_child_count() - 1; i >= 0; i--)
        get_child(i)->queue_free();

    if (value.empty())
    {
        _contentWidth = 0;
        _contentHeight = 0;
        _dirty = false;
        return;
    }

    _parser->parse(_text, _textFormat, _elements, _parseOptions);
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
    // Match Cocos: re-parse HTML/UBB so inline styles pick up the updated base format.
    if (!_text.empty())
        setText(_text);
    else
    {
        _dirty = true;
        formatText();
    }
}

HtmlObject* FUIRichText::getControl(const std::string& name) const
{
    if (_dirty)
        const_cast<FUIRichText*>(this)->formatText();

    for (auto& ctrl : _controls)
    {
        if (ctrl->getElement()->getString("name") == name)
            return ctrl;
    }
    return nullptr;
}

void FUIRichText::formatText()
{
    if (!_dirty)
        return;

    _dirty = false;

    for (int i = get_child_count() - 1; i >= 0; i--)
        get_child(i)->queue_free();

    _renderers.clear();

    if (_elements.empty())
    {
        _contentWidth = 0;
        _contentHeight = 0;
        return;
    }

    if (_overflow == 0)
        _textRectWidth = FLT_MAX;
    else
        _textRectWidth = _dimensionsX - GUTTER_X * 2;

    int size = (int)_elements.size();
    addNewLine();
    for (int i = 0; i < size; ++i)
    {
        HtmlElement* element = _elements[i];
        switch (element->type)
        {
        case HtmlElement::Type::TEXT:
        {
            const std::string& text = element->text;
            size_t startPos = 0;
            bool first = true;
            while (startPos < text.length())
            {
                size_t endPos = text.find('\n', startPos);
                bool hasNewline = (endPos != std::string::npos);
                std::string segment = hasNewline
                    ? text.substr(startPos, endPos - startPos)
                    : text.substr(startPos);

                if (!first && hasNewline)
                    addNewLine();
                if (!segment.empty())
                    handleTextRenderer(element, segment);
                first = false;

                if (hasNewline)
                    startPos = endPos + 1;
                else
                    startPos = text.length();
            }
            break;
        }
        case HtmlElement::Type::BR:
            addNewLine();
            break;
        default:
            element->space = (int)floor(_leftSpaceWidth);
            handleRichRenderer(element, _objects[i]);
            break;
        }
    }

    formarRenderers();
}

void FUIRichText::addNewLine()
{
    _leftSpaceWidth = _textRectWidth;
    _renderers.emplace_back();
    _numLines++;
}

void FUIRichText::handleTextRenderer(HtmlElement* element, const std::string& text)
{
    FUILabel* textRenderer = memnew(FUILabel);
    textRenderer->getTextFormat()->setFormat(element->format);
    textRenderer->applyTextFormat();
    textRenderer->setUnderlineColor(_parseOptions.linkColor);
    textRenderer->setText(text);
    setNodeElement(textRenderer, element);

    float textRendererWidth = textRenderer->getTextWidth();
    _leftSpaceWidth -= textRendererWidth;
    if (_leftSpaceWidth >= 0)
    {
        _renderers.back().push_back(textRenderer);
        return;
    }

    int leftLength = findSplitPositionForWord(textRenderer, text);
    if (leftLength == 0)
        leftLength = 1;
    std::string leftWords = getSubStringOfUTF8String(text, 0, leftLength);
    int rightStart = leftLength;
    if (rightStart < (int)text.length() && std::isspace((unsigned char)text[rightStart], std::locale()))
        rightStart++;
    std::string cutWords = getSubStringOfUTF8String(text, rightStart, text.length() - rightStart);

    if (leftLength > 0)
    {
        FUILabel* leftRenderer = memnew(FUILabel);
        leftRenderer->getTextFormat()->setFormat(element->format);
        leftRenderer->applyTextFormat();
        leftRenderer->setUnderlineColor(_parseOptions.linkColor);
        leftRenderer->setText(leftWords);
        setNodeElement(leftRenderer, element);
        _renderers.back().push_back(leftRenderer);
    }

    textRenderer->queue_free();

    if (!cutWords.empty())
    {
        addNewLine();
        handleTextRenderer(element, cutWords);
    }
}

int FUIRichText::findSplitPositionForWord(Node* label, const std::string& text)
{
    FUILabel* flabel = Object::cast_to<FUILabel>(label);
    if (!flabel)
        return (int)text.length();

    float originalLeftSpaceWidth = _leftSpaceWidth + flabel->getTextWidth();
    bool startingNewLine = (_textRectWidth == originalLeftSpaceWidth);
    if (!isWrappable(text))
    {
        if (startingNewLine)
            return (int)text.length();
        return 0;
    }

    for (int idx = (int)text.size() - 1; idx >= 0; )
    {
        int newidx = getPrevWord(text, idx);
        if (newidx >= 0)
        {
            idx = newidx;
            auto leftStr = getSubStringOfUTF8String(text, 0, idx);
            flabel->setText(leftStr);
            if (flabel->getTextWidth() <= originalLeftSpaceWidth)
                return idx;
        }
        else
        {
            if (startingNewLine)
                return idx;
            return 0;
        }
    }

    flabel->setText(text);
    return (int)text.size();
}

void FUIRichText::handleRichRenderer(HtmlElement* element, HtmlObject* obj)
{
    if (obj == nullptr)
        return;

    element->obj = obj;
    obj->create(this, element);

    if (obj->isHidden())
        return;

    Node* display = obj->getUI()->displayObject();
    setNodeElement(display, element);

    float width = obj->getUI()->getWidth();
    _leftSpaceWidth -= (width + 4);
    if (_leftSpaceWidth < 0.0f)
    {
        addNewLine();
        _leftSpaceWidth -= (width + 4);
    }

    _renderers.back().push_back(display);
}

void FUIRichText::formarRenderers()
{
    float rowY = (float)GUTTER_Y;
    float textWidth = 0;
    float textHeight = 0;

    for (auto& row : _renderers)
    {
        if (rowY != GUTTER_Y)
            rowY += _textFormat.lineSpacing - 3;

        float nextPosX = (float)GUTTER_X;
        float lineHeight = 0.0f;
        float lineTextHeight = 0.0f;
        for (auto& node : row)
        {
            Vector2 sz = getNodeSize(node);
            lineHeight = MAX(sz.y, lineHeight);
            HtmlElement* element = getNodeElement(node);
            if (!element || element->obj == nullptr)
                lineTextHeight = MAX(sz.y, lineTextHeight);
        }

        for (auto& node : row)
        {
            HtmlElement* element = getNodeElement(node);
            Vector2 sz = getNodeSize(node);
            if (element && element->obj != nullptr)
            {
                nextPosX += 2;
                int adjustment = (int)floor((lineHeight - sz.y) / 2);
                setNodePosition(node, Vector2(nextPosX, rowY + adjustment));
                nextPosX += sz.x + 2;
            }
            else
            {
                int adjustment = (int)floor((lineHeight - lineTextHeight) / 2);
                setNodePosition(node, Vector2(nextPosX, rowY + adjustment));
                nextPosX += sz.x;
            }
            if (node->get_parent() != this)
                add_child(node);
        }

        rowY += lineHeight;
        nextPosX += GUTTER_X;
        if (nextPosX > textWidth)
            textWidth = nextPosX;
        if (_overflow != 0)
            doHorizontalAlignment(row, nextPosX);
    }

    if (textWidth == GUTTER_X + GUTTER_X)
        textWidth = 0;
    else if (_numLines > 1 || (_textFormat.align != 0 && _overflow != 0))
        textWidth = MAX(_dimensionsX, textWidth);

    if (rowY != GUTTER_Y)
        textHeight = rowY + GUTTER_Y;
    else
        textHeight = 0;

    _contentWidth = textWidth;
    _contentHeight = textHeight;

    float oldDimensionsHeight = _dimensionsY;
    if (_overflow == 0)
    {
        _dimensionsX = _contentWidth;
        _dimensionsY = _contentHeight;
    }
    else if (_overflow == 3)
        _dimensionsY = _contentHeight;

    float delta = _contentHeight - oldDimensionsHeight;
    if (_textFormat.verticalAlign == 1)
        delta -= floor((_dimensionsY - textHeight) * 0.5f);
    else if (_textFormat.verticalAlign == 2)
        delta -= _dimensionsY - textHeight;

    if (delta != 0)
    {
        for (int i = 0; i < get_child_count(); i++)
        {
            Node* node = get_child(i);
            offsetNodePosition(node, Vector2(0, delta));
        }
    }

    _renderers.clear();
}

void FUIRichText::doHorizontalAlignment(const std::vector<Node*>& row, float rowWidth)
{
    if (_textFormat.align != 0)
    {
        const float diff = stripTrailingWhitespace(row);
        const float leftOver = _dimensionsX - (rowWidth + diff);
        const float leftPadding = getPaddingAmount(_textFormat.align, leftOver);
        for (auto& node : row)
            offsetNodePosition(node, Vector2(leftPadding, 0));
    }
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
