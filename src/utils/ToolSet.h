#ifndef __TOOLSET_H__
#define __TOOLSET_H__

#include "FairyGUI.h"

#if defined(_MSC_VER)
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

NS_FGUI_BEGIN

class ToolSet
{
public:
    static Color hexToColor(const char* str);
    static Color intToColor(unsigned int rgb);
    static unsigned int colorToInt(const Color& color);

    static Rect2 intersection(const Rect2& rect1, const Rect2& rect2);
    static int findInStringArray(const std::vector<std::string>& arr, const std::string& str);

    static bool isFileExist(const std::string& fileName);

    // Replaces Cocos CC_PLATFORM_PC for input/scroll sensitivity.
    static bool isDesktopInput();
};

class FastSplitter
{
public:
    FastSplitter();
    void start(const char* data, ssize_t dataLength, char delimiter);
    bool next();
    const char* getText();
    ssize_t getTextLength();
    void getKeyValuePair(char* keyBuf, ssize_t keyBufSize, char* valueBuf, ssize_t valueBufSize);

private:
    const char* data;
    ssize_t dataLength;
    ssize_t textLength;
    char delimiter;
};

NS_FGUI_END

#endif
