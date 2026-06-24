#include "GearFontSize.h"
#include "GObject.h"
#include "utils/ByteBuffer.h"

NS_FGUI_BEGIN
GearFontSize::GearFontSize(GObject* owner) : GearBase(owner)
{
}

GearFontSize::~GearFontSize()
{
}

void GearFontSize::init()
{
    _default = _owner->getProp(ObjectPropID::FontSize).operator int64_t();
    _storage.clear();
}

void GearFontSize::addStatus(const std::string& pageId, ByteBuffer* buffer)
{
    if (pageId.length() == 0)
        _default = buffer->readInt();
    else
        _storage[pageId] = buffer->readInt();
}

void GearFontSize::apply()
{
    _owner->_gearLocked = true;

    auto it = _storage.find(_controller->getSelectedPageId());
    if (it != _storage.end())
        _owner->setProp(ObjectPropID::FontSize, Variant(it->second));
    else
        _owner->setProp(ObjectPropID::FontSize, Variant(_default));

    _owner->_gearLocked = false;
}

void GearFontSize::updateState()
{
    _storage[_controller->getSelectedPageId()] = _owner->getProp(ObjectPropID::FontSize).operator int64_t();
}

NS_FGUI_END