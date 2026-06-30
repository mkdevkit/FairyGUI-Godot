#include "HtmlObject.h"
#include "HtmlElement.h"
#include "display/FUIRichText.h"
#include "utils/ToolSet.h"
#include "UIPackage.h"

NS_FGUI_BEGIN
using namespace std;

std::string HtmlObject::buttonResource;
std::string HtmlObject::inputResource;
std::string HtmlObject::selectResource;
bool HtmlObject::usePool = true;

// GObjectPool HtmlObject::objectPool;  // moved to getObjectPool() to avoid static init crash
GObjectPool& HtmlObject::getObjectPool() {
    static GObjectPool pool;
    return pool;
}
std::vector<Ref<GObject>> HtmlObject::loaderPool;

HtmlObject::HtmlObject() :_ui(), _hidden(false)
{
}

HtmlObject::~HtmlObject()
{
    if (_ui.is_valid())
    {
        destroy();

        if (usePool)
        {
            if (!_ui->getResourceURL().empty())
                getObjectPool().returnObject(_ui.ptr());
            else if (dynamic_cast<GLoader*>(_ui.ptr()) != nullptr)
                loaderPool.push_back(_ui);
        }
    }

    // CC_SAFE_RELEASE(_ui)
}

void HtmlObject::create(FUIRichText* owner, HtmlElement* element)
{
    _owner = owner;
    _element = element;

    switch (element->type)
    {
    case HtmlElement::Type::IMAGE:
        createImage();
        break;

    case HtmlElement::Type::INPUT:
    {
        string type = element->getString("type");
        transform(type.begin(), type.end(), type.begin(), ::tolower);
        if (type == "button" || type == "submit")
            createButton();
        else
            createInput();
        break;
    }

    case HtmlElement::Type::SELECT:
        createSelect();
        break;

    case HtmlElement::Type::OBJECT:
        createCommon();
        break;
    }
}

void HtmlObject::destroy()
{
    switch (_element->type)
    {
    case HtmlElement::Type::IMAGE:
        ((GLoader*)_ui.ptr())->setURL("");
        break;
    }
}

void HtmlObject::createCommon()
{
    string src = _element->getString("src");
    if (!src.empty())
        _ui = getObjectPool().getObject(src);
    else
        _ui = GComponent::create();

    int width = _element->getInt("width", _ui->sourceSize.width);
    int height = _element->getInt("height", _ui->sourceSize.height);

    _ui->setSize(width, height);
    _ui->setText(_element->getString("title"));
    _ui->setIcon(_element->getString("icon"));
}

void HtmlObject::createImage()
{
    int width = 0;
    int height = 0;
    string src = _element->getString("src");
    if (!src.empty()) {
        PackageItem* pi = UIPackage::getItemByURL(src);
        if (pi)
        {
            width = pi->width;
            height = pi->height;
        }
    }

    width = _element->getInt("width", width);
    height = _element->getInt("height", height);

    GLoader* loader;
    if (!loaderPool.empty())
    {
        loader = (GLoader*)loaderPool.back().ptr();
        loaderPool.pop_back();
    }
    else
    {
        _ui = GLoader::create();
        loader = Object::cast_to<GLoader>(_ui.ptr());
    }

    _ui = loader;

    loader->setSize(width, height);
    loader->setFill(LoaderFillType::SCALE_FREE);
    loader->setURL(src);
}

void HtmlObject::createButton()
{
    if (!buttonResource.empty())
        _ui = getObjectPool().getObject(buttonResource);
    else
    {
        _ui = GComponent::create();
        // CCLOGWARN("Set HtmlObject.buttonResource first");
    }

    int width = _element->getInt("width", _ui->sourceSize.width);
    int height = _element->getInt("height", _ui->sourceSize.height);

    _ui->setSize(width, height);
    _ui->setText(_element->getString("value"));

    GButton *button = dynamic_cast<GButton*>(_ui.ptr());
    if (button != nullptr)
    {
        button->setSelected(_element->getString("checked") == "true");
    }
}

void HtmlObject::createInput()
{
    if (!inputResource.empty())
        _ui = getObjectPool().getObject(inputResource);
    else
    {
        _ui = GComponent::create();
        // CCLOGWARN("Set HtmlObject.inputResource first");
    }

    string type = _element->getString("type");
    transform(type.begin(), type.end(), type.begin(), ::tolower);
    _hidden = type == "hidden";

    int width = _element->getInt("width");
    int height = _element->getInt("height");

    if (width == 0)
    {
        width = _element->space;
        // GODOT_ADAPT: _owner->getContentSize() replaced with default size
        float ownerWidth = 300.0f;
        if (width > (int)(ownerWidth / 2) || width < 100)
            width = (int)(ownerWidth / 2);
    }

    if (height == 0)
        height = _element->format.fontSize + 10;

    _ui->setSize(width, height);
    _ui->setText(_element->getString("value"));

    GLabel *label = dynamic_cast<GLabel*>(_ui.ptr());
    if (label != nullptr)
    {
        GTextInput* input = dynamic_cast<GTextInput*>(label->getTextField());
        if (input != nullptr)
        {
            // GODOT_ADAPT: GTextInput not yet ported
            // input->setPassword(type == "password");
        }
    }
}

void HtmlObject::createSelect()
{
    if (!selectResource.empty())
        _ui = getObjectPool().getObject(selectResource);
    else
    {
        _ui = GComponent::create();
        // CCLOGWARN("Set HtmlObject.selectResource first");
    }

    int width = _element->getInt("width", _ui->sourceSize.width);
    int height = _element->getInt("height", _ui->sourceSize.height);

    _ui->setSize(width, height);

    GComboBox* comboBox = dynamic_cast<GComboBox*>(_ui.ptr());
    if (comboBox != nullptr)
    {
        auto items = _element->getArray("items");
        auto values = _element->getArray("values");
        comboBox->getItems().clear();
        comboBox->getValues().clear();
        for (auto it : items)
            comboBox->getItems().push_back(std::string((const char*)it.stringify().utf8().ptr()));
        for (auto it : values)
            comboBox->getValues().push_back(std::string((const char*)it.stringify().utf8().ptr()));
        comboBox->setValue(_element->getString("value"));
        comboBox->refresh();
    }
}

NS_FGUI_END
