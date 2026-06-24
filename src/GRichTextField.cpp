#include "GRichTextField.h"
#include "utils/UBBParser.h"
#include "utils/ByteBuffer.h"
#include "utils/html/HtmlObject.h"

NS_FGUI_BEGIN
GRichTextField::GRichTextField() :
    _richText(nullptr),
    _updatingSize(false)
{
}

GRichTextField::~GRichTextField()
{
}

void GRichTextField::handleInit()
{
    _richText = FUIRichText::create();
    _displayObject = _richText;

}

void GRichTextField::applyTextFormat()
{
    _richText->applyTextFormat();
    updateGear(4);
    if (!_underConstruct)
        updateSize();
}

void GRichTextField::setAutoSize(AutoSizeType value)
{
    _autoSize = value;
    switch (value)
    {
    case AutoSizeType::NONE:
        _richText->setOverflow(1);
        break;
    case AutoSizeType::BOTH:
        _richText->setOverflow(0);
        break;
    case AutoSizeType::HEIGHT:
        _richText->setOverflow(3);
        break;
    case AutoSizeType::SHRINK:
        _richText->setOverflow(2);
        break;
    }

    _richText->setDimensions(_size.width, _size.height);
    if (!_underConstruct)
        updateSize();
}

void GRichTextField::setSingleLine(bool value)
{
}

void GRichTextField::setTextFieldText()
{
    if (_ubbEnabled)
    {
        std::string parsedText = UBBParser::getInstance()->parse(_text.c_str());
        if (_templateVars != nullptr)
            parsedText = parseTemplate(parsedText.c_str());
        _richText->setText(parsedText);
    }
    else
    {
        if (_templateVars != nullptr)
            _richText->setText(parseTemplate(_text.c_str()));
        else
            _richText->setText(_text);
    }
}

void GRichTextField::updateSize()
{
    if (_updatingSize)
        return;

    _updatingSize = true;

    Vector2 sz = _richText->get_content_size();
    if (_autoSize == AutoSizeType::BOTH)
        setSize(sz.x, sz.y);
    else if (_autoSize == AutoSizeType::HEIGHT)
        setHeight(sz.y);

    _updatingSize = false;
}

HtmlObject* GRichTextField::getControl(const std::string& name) const
{
    return _richText->getControl(name);
}

void GRichTextField::handleSizeChanged()
{
    if (_updatingSize)
        return;

    if (_autoSize != AutoSizeType::BOTH)
    {
        _richText->setDimensions(_size.width, _size.height);

        if (_autoSize == AutoSizeType::HEIGHT)
        {
            if (!_text.empty())
                setSizeDirectly(_size.width, _richText->get_content_size().height);
        }
    }
}

GObject* GRichTextField::hitTest(const Vector2& worldPoint, const Camera2D* camera)
{
    const std::vector<HtmlObject*>& objs = _richText->getControls();
    for (auto &obj : objs)
    {
        GObject* target = obj->getUI()->hitTest(worldPoint, camera);
        if (target)
            return target;
    }

    return GTextField::hitTest(worldPoint, camera);
}

void GRichTextField::_bind_methods()
{
    // set_text/get_text inherited from GTextField
}

NS_FGUI_END


