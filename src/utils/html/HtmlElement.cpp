#include "HtmlElement.h"
#include "HtmlObject.h"

NS_FGUI_BEGIN
using namespace std;

HtmlElement::HtmlElement(Type type) :
    type(type),
    link(nullptr),
    obj(nullptr),
    space(0)
{
}

int HtmlElement::getInt(const std::string& attrName, int defValue)
{
    if (attrs.is_empty())
        return defValue;

    Variant var = attrs[String(attrName.c_str())];
    if (var.get_type() != Variant::NIL)
        return (int)var;
    else
        return defValue;
}

std::string HtmlElement::getString(const std::string& attrName, const std::string& defValue)
{
    if (attrs.is_empty())
        return defValue;

    Variant var = attrs[String(attrName.c_str())];
    if (var.get_type() != Variant::NIL)
        return std::string((const char*)var.stringify().utf8().ptr());
    else
        return defValue;
}

Array HtmlElement::getArray(const std::string& attrName)
{
    if (!attrs.is_empty())
    {
        Variant var = attrs[String(attrName.c_str())];
        if (var.get_type() == Variant::ARRAY)
            return var;
    }

    Array arr;
    attrs[String(attrName.c_str())] = arr;
    return arr;
}

NS_FGUI_END
