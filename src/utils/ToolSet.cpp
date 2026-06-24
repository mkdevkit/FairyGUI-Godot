#include "utils/ToolSet.h"
#include "core/io/file_access.h"

NS_FGUI_BEGIN
using namespace std;

Color ToolSet::hexToColor(const char* str)
{
    ssize_t len = strlen(str);
    if (len < 7 || str[0] != '#')
        return Color(0, 0, 0, 1);

    char temp[3];
    memset(temp, 0, 3);

    if (len == 9)
    {
        return Color(strtol(strncpy(temp, str + 3, 2), NULL, 16) / 255.0f,
            strtol(strncpy(temp, str + 5, 2), NULL, 16) / 255.0f,
            strtol(strncpy(temp, str + 7, 2), NULL, 16) / 255.0f,
            strtol(strncpy(temp, str + 1, 2), NULL, 16) / 255.0f);
    }
    else
    {
        return Color(strtol(strncpy(temp, str + 1, 2), NULL, 16) / 255.0f,
            strtol(strncpy(temp, str + 3, 2), NULL, 16) / 255.0f,
            strtol(strncpy(temp, str + 5, 2), NULL, 16) / 255.0f,
            1.0f);
    }
}

Color ToolSet::intToColor(unsigned int rgb)
{
    return Color((float)((rgb >> 16) & 0xFF) / 255.0f, (float)((rgb >> 8) & 0xFF) / 255.0f, (float)(rgb & 0xFF) / 255.0f, 1.0f);
}

unsigned int ToolSet::colorToInt(const Color& color)
{
    return ((unsigned int)(color.r * 255) << 16) + ((unsigned int)(color.g * 255) << 8) + (unsigned int)(color.b * 255);
}

Rect ToolSet::intersection(const Rect& rect1, const Rect& rect2)
{
    if (rect1.size.x == 0 || rect1.size.y == 0 || rect2.size.x == 0 || rect2.size.y == 0)
        return Rect(0, 0, 0, 0);

    float left = rect1.position.x > rect2.position.x ? rect1.position.x : rect2.position.x;
    float right = rect1.position.x + rect1.size.x < rect2.position.x + rect2.size.x ? rect1.position.x + rect1.size.x : rect2.position.x + rect2.size.x;
    float top = rect1.position.y > rect2.position.y ? rect1.position.y : rect2.position.y;
    float bottom = rect1.position.y + rect1.size.y < rect2.position.y + rect2.size.y ? rect1.position.y + rect1.size.y : rect2.position.y + rect2.size.y;

    if (left > right || top > bottom)
        return Rect(0, 0, 0, 0);
    else
        return Rect(left, top, right - left, bottom - top);
}

int ToolSet::findInStringArray(const std::vector<std::string>& arr, const std::string& str)
{
    auto iter = std::find(arr.begin(), arr.end(), str);
    if (iter != arr.end())
        return (int)(iter - arr.begin());

    return -1;
}

bool ToolSet::isFileExist(const std::string& fileName)
{
    return FileAccess::exists(String(fileName.c_str()));
}

FastSplitter::FastSplitter() : data(nullptr), dataLength(-1), delimiter('\0')
{
}

void FastSplitter::start(const char* data, ssize_t dataLength, char delimiter)
{
    this->data = data;
    this->dataLength = dataLength;
    this->delimiter = delimiter;
    this->textLength = -1;
}

bool FastSplitter::next()
{
    if (dataLength < 0)
        return false;

    if (dataLength == 0)
    {
        dataLength = -1;
        textLength = 0;
        return true;
    }

    data += textLength + 1;
    char* found = (char*)memchr(data, (int)delimiter, dataLength);
    if (found)
        textLength = found - data;
    else
        textLength = dataLength;
    dataLength -= (textLength + 1);

    return true;
}

const char* FastSplitter::getText()
{
    if (textLength > 0)
        return data;
    else
        return nullptr;
}

ssize_t FastSplitter::getTextLength()
{
    return textLength;
}

void FastSplitter::getKeyValuePair(char* keyBuf, ssize_t keyBufSize, char* valueBuf, ssize_t valueBufSize)
{
    if (textLength == 0)
    {
        keyBuf[0] = '\0';
        valueBuf[0] = '\0';
    }
    else
    {
        char* found = (char*)memchr(data, (int)'=', textLength);
        if (found)
        {
            ssize_t len = std::min<ssize_t>(keyBufSize - 1, found - data);
            memcpy(keyBuf, data, len);
            keyBuf[len] = '\0';

            len = std::min<ssize_t>(valueBufSize - 1, textLength - (found - data) - 1);
            memcpy(valueBuf, found + 1, len);
            valueBuf[len] = '\0';
        }
        else
        {
            ssize_t len = std::min<ssize_t>(valueBufSize - 1, textLength);
            memcpy(keyBuf, data, len);
            keyBuf[len] = '\0';
            valueBuf[0] = '\0';
        }
    }
}

NS_FGUI_END
