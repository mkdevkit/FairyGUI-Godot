#ifndef __TRANSLATIONHELPER_H_
#define __TRANSLATIONHELPER_H_

#include "FairyGUIMacros.h"
// cocos2d.h removed - see godot_types.h

NS_FGUI_BEGIN

class PackageItem;

class TranslationHelper
{
public:
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> strings;

    static void loadFromXML(const char *xmlString, size_t nBytes);
    static void translateComponent(PackageItem* item);
};

NS_FGUI_END

#endif
