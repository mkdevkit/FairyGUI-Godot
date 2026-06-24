#include "GTextInput.h"
#include "UIPackage.h"
#include "GRoot.h"
#include "utils/ByteBuffer.h"
#include "utils/UBBParser.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN
GTextInput::GTextInput()
{
}

GTextInput::~GTextInput()
{
}

void GTextInput::_bind_methods()
{
    // set_text/get_text inherited from GTextField

    ClassDB::bind_method(D_METHOD("setSingleLine", "value"), &GTextInput::setSingleLine);
    ClassDB::bind_method(D_METHOD("isSingleLine"), &GTextInput::isSingleLine);
    // ADD_PROPERTY(PropertyInfo(Variant::BOOL, "singleLine"), "setSingleLine", "isSingleLine");

    ClassDB::bind_method(D_METHOD("setPrompt", "value"), &GTextInput::gd_setPrompt);
    ClassDB::bind_method(D_METHOD("setPassword", "value"), &GTextInput::setPassword);
    ClassDB::bind_method(D_METHOD("setKeyboardType", "value"), &GTextInput::setKeyboardType);
    ClassDB::bind_method(D_METHOD("setMaxLength", "value"), &GTextInput::setMaxLength);
}

void GTextInput::handleInit()
{
    _input = FUIInput::create();
    _displayObject = _input;

    addEventListener(UIEventType::TouchEnd, [this](EventContext*) {
        _input->openKeyboard();
    });
}

bool GTextInput::isSingleLine() const
{
    return _input->isSingleLine();
}

void GTextInput::setSingleLine(bool value)
{
    _input->setSingleLine(value);
}

void GTextInput::applyTextFormat()
{
    _input->applyTextFormat();
}

void GTextInput::setPrompt(const std::string& value)
{
    if (value.empty())
        _input->setPlaceHolder("");
    else
    {
        UBBParser* parser = UBBParser::getInstance();
        _input->setPlaceHolder(parser->parse(value.c_str(), true));
        if (!parser->lastColor.empty())
            _input->setPlaceholderColor(ToolSet::hexToColor(parser->lastColor.c_str()));
        if (!parser->lastFontSize.empty())
            _input->setPlaceholderFontSize(std::atoi(parser->lastFontSize.c_str()));
    }
}

void GTextInput::setPassword(bool value)
{
    _input->setPassword(value);
}

void GTextInput::setKeyboardType(int value)
{
    _input->setKeyboardType(value);
}

void GTextInput::setMaxLength(int value)
{
    _input->setMaxLength(value);
}

void GTextInput::setRestrict(const std::string& value)
{
    _input->setInputRestrict(value);
}

void GTextInput::handleSizeChanged()
{
    _input->set_size(Size2(_size.width, _size.height));
}

void GTextInput::setup_beforeAdd(ByteBuffer* buffer, int beginPos)
{
    GTextField::setup_beforeAdd(buffer, beginPos);

    buffer->seek(beginPos, 4);

    const std::string* str;
    if ((str = buffer->readSP()))
        setPrompt(*str);

    if ((str = buffer->readSP()))
        setRestrict(*str);

    int iv = buffer->readInt();
    if (iv != 0)
        setMaxLength(iv);
    iv = buffer->readInt();
    if (iv != 0)
        setKeyboardType(iv);
    if (buffer->readBool())
        setPassword(true);
}

void GTextInput::setTextFieldText()
{
    if (_templateVars != nullptr)
        _input->setText(parseTemplate(_text.c_str()));
    else
        _input->setText(_text);
}

void GTextInput::editBoxReturn()
{
    // text submitted callback - will be handled by Godot signals in future
}

void GTextInput::editBoxTextChanged(const std::string& text)
{
    _text.clear();
    _text.append(_input->getText());
}

GTextInput* GTextInput::create() { Ref<GTextInput> ref = memnew(GTextInput); auto* o = ref.ptr(); o->reference(); return o; }

NS_FGUI_END
