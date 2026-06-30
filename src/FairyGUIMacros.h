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

// FAIRYGUI_CREATE - returns Ref<TYPE>. memnew sets refcount=1,
// Ref<TYPE>(pRet) calls reference() once (refcount=2). When the
// returned Ref<> is stored (e.g. in _children), the refcount reflects
// all active references. Objects are auto-freed when the last Ref<>
// goes out of scope.
#define FAIRYGUI_CREATE(TYPE) \
    static Ref<TYPE> create() { \
        TYPE* pRet = memnew(TYPE); \
        if (pRet->init()) \
            return Ref<TYPE>(pRet); \
        memdelete(pRet); \
        return Ref<TYPE>(); \
    }

#endif
