#ifndef __FAIRYGUIMACROS_H__
#define __FAIRYGUIMACROS_H__

// Define namespace macros BEFORE including headers that use them
#define NS_FGUI_BEGIN                     namespace fairygui {
#define NS_FGUI_END                       }
#define USING_NS_FGUI                     using namespace fairygui

#include "godot_types.h"
#include "FieldTypes.h"

// CALL_LATER - deferred calls. In Godot, use call_deferred on Object.
#define CALL_LATER_FUNC(__TYPE__,__FUNC__) \
    void __FUNC__()

#define CALL_LATER(__TYPE__,__FUNC__,...) \
    call_deferred(StringName(#__FUNC__))

#define CALL_LATER_CANCEL(__TYPE__,__FUNC__)

#endif
