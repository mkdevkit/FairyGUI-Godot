#include "TranslationHelper.h"
#include "PackageItem.h"
#include "UIPackage.h"
#include "utils/ByteBuffer.h"

NS_FGUI_BEGIN

using namespace std;

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> TranslationHelper::strings;

static std::string intToStr(int v)
{
    return std::string(Variant(v).stringify().utf8().get_data());
}

static void skipWhitespace(const char*& p, const char* end)
{
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
        ++p;
}

static std::string extractAttrValue(const char*& p, const char* end, const char* attrName)
{
    std::string result;
    int nameLen = (int)strlen(attrName);
    while (p < end)
    {
        skipWhitespace(p, end);
        if (p >= end || *p == '>' || *p == '/' || *p == '?')
            break;
        if (strncmp(p, attrName, nameLen) == 0 && (p[nameLen] == '=' || p[nameLen] == ' '))
        {
            p += nameLen;
            skipWhitespace(p, end);
            if (p < end && *p == '=')
            {
                ++p;
                skipWhitespace(p, end);
                if (p < end && (*p == '"' || *p == '\''))
                {
                    char quote = *p++;
                    while (p < end && *p != quote)
                        result += *p++;
                    if (p < end) ++p;
                    return result;
                }
            }
        }
        ++p;
    }
    return result;
}

void TranslationHelper::loadFromXML(const char* xmlString, size_t nBytes)
{
    strings.clear();

    const char* p = xmlString;
    const char* end = xmlString + nBytes;

    while (p < end)
    {
        // Find start of <string> element
        const char* tagStart = strstr(p, "<string");
        if (!tagStart || tagStart >= end)
            break;

        p = tagStart + 7; // skip "<string"

        // Extract name attribute
        std::string key = extractAttrValue(p, end, "name");
        if (key.empty())
        {
            // Couldn't find name, try to find next element
            p = strstr(p, "<string");
            if (!p) break;
            p += 7;
            continue;
        }

        // Find closing '>' of the opening tag
        const char* dataStart = (const char*)memchr(p, '>', end - p);
        if (!dataStart) break;
        dataStart++; // skip past '>'

        // Find closing tag
        const char* closingTag = strstr(dataStart, "</string>");
        if (!closingTag) break;

        std::string text(dataStart, closingTag - dataStart);

        size_t i = key.find("-");
        if (i == std::string::npos)
        {
            p = closingTag + 9; // skip "</string>"
            continue;
        }

        std::string key2 = key.substr(0, i);
        std::string key3 = key.substr(i + 1);
        std::unordered_map<std::string, std::string>& col = strings[key2];
        col[key3] = text;

        p = closingTag + 9; // skip "</string>"
    }
}

void TranslationHelper::translateComponent(PackageItem* item)
{
    if (strings.empty())
        return;

    auto col = strings.find(item->owner->getId() + item->id);
    if (col == strings.end())
        return;

    std::unordered_map<std::string, std::string>& strings = col->second;

    ByteBuffer* buffer = item->rawData;

    buffer->seek(0, 2);

    int childCount = buffer->readShort();
    for (int i = 0; i < childCount; i++)
    {
        int dataLen = buffer->readUshort();
        int curPos = buffer->getPos();

        buffer->seek(curPos, 0);

        ObjectType baseType = (ObjectType)buffer->readByte();
        ObjectType type = baseType;

        buffer->skip(4);
        const string& elementId = buffer->readS();

        if (type == ObjectType::COMPONENT)
        {
            if (buffer->seek(curPos, 6))
                type = (ObjectType)buffer->readByte();
        }

        buffer->seek(curPos, 1);

        auto it = strings.find(elementId + "-tips");
        if (it != strings.end())
            buffer->writeS(it->second);

        buffer->seek(curPos, 2);

        int gearCnt = buffer->readShort();
        for (int j = 0; j < gearCnt; j++)
        {
            int nextPos = buffer->readUshort();
            nextPos += buffer->getPos();

            if (buffer->readByte() == 6) //gearText
            {
                buffer->skip(2); //controller
                int valueCnt = buffer->readShort();
                for (int k = 0; k < valueCnt; k++)
                {
                    const string& page = buffer->readS();
                    if (!page.empty())
                    {
                        if ((it = strings.find(elementId + "-texts_" + intToStr(k))) != strings.end())
                            buffer->writeS(it->second);
                        else
                            buffer->skip(2);
                    }
                }

                if (buffer->readBool() && (it = strings.find(elementId + "-texts_def")) != strings.end())
                    buffer->writeS(it->second);
            }

            buffer->setPos(nextPos);
        }

        if (baseType == ObjectType::COMPONENT && buffer->version >= 2)
        {
            buffer->seek(curPos, 4);

            buffer->skip(2); //pageController

            buffer->skip(4 * buffer->readShort());

            int cpCount = buffer->readShort();
            for (int k = 0; k < cpCount; k++)
            {
                std::string target = buffer->readS();
                int propertyId = buffer->readShort();
                if (propertyId == 0 && (it = strings.find(elementId + "-cp-" + target)) != strings.end())
                    buffer->writeS(it->second);
                else
                    buffer->skip(2);
            }
        }

        switch (type)
        {
        case ObjectType::TEXT:
        case ObjectType::RICHTEXT:
        case ObjectType::INPUTTEXT:
        {
            if ((it = strings.find(elementId)) != strings.end())
            {
                buffer->seek(curPos, 6);
                buffer->writeS(it->second);
            }
            if ((it = strings.find(elementId + "-prompt")) != strings.end())
            {
                buffer->seek(curPos, 4);
                buffer->writeS(it->second);
            }
            break;
        }

        case ObjectType::LIST:
        {
            buffer->seek(curPos, 8);
            buffer->skip(2);
            int itemCount = buffer->readShort();
            for (int j = 0; j < itemCount; j++)
            {
                int nextPos = buffer->readUshort();
                nextPos += buffer->getPos();

                buffer->skip(2); //url
                if (type == ObjectType::TREE)
                    buffer->skip(2);

                //title
                if ((it = strings.find(elementId + "-" + intToStr(j))) != strings.end())
                    buffer->writeS(it->second);
                else
                    buffer->skip(2);

                //selected title
                if ((it = strings.find(elementId + "-" + intToStr(j) + "-0")) != strings.end())
                    buffer->writeS(it->second);
                else
                    buffer->skip(2);

                if (buffer->version >= 2)
                {
                    buffer->skip(6);
                    buffer->skip(buffer->readShort() * 4); //controllers

                    int cpCount = buffer->readShort();
                    for (int k = 0; k < cpCount; k++)
                    {
                        std::string target = buffer->readS();
                        int propertyId = buffer->readShort();
                        if (propertyId == 0 && (it = strings.find(elementId + "-" + intToStr(j) + "-" + target)) != strings.end())
                            buffer->writeS(it->second);
                        else
                            buffer->skip(2);
                    }
                }

                buffer->setPos(nextPos);
            }
            break;
        }

        case ObjectType::LABEL:
        {
            if (buffer->seek(curPos, 6) && (ObjectType)buffer->readByte() == type)
            {
                if ((it = strings.find(elementId)) != strings.end())
                    buffer->writeS(it->second);
                else
                    buffer->skip(2);

                buffer->skip(2);
                if (buffer->readBool())
                    buffer->skip(4);
                buffer->skip(4);
                if (buffer->readBool() && (it = strings.find(elementId + "-prompt")) != strings.end())
                    buffer->writeS(it->second);
            }
            break;
        }

        case ObjectType::BUTTON:
        {
            if (buffer->seek(curPos, 6) && (ObjectType)buffer->readByte() == type)
            {
                if ((it = strings.find(elementId)) != strings.end())
                    buffer->writeS(it->second);
                else
                    buffer->skip(2);
                if ((it = strings.find(elementId + "-0")) != strings.end())
                    buffer->writeS(it->second);
            }
            break;
        }

        case ObjectType::COMBOBOX:
        {
            if (buffer->seek(curPos, 6) && (ObjectType)buffer->readByte() == type)
            {
                int itemCount = buffer->readShort();
                for (int j = 0; j < itemCount; j++)
                {
                    int nextPos = buffer->readUshort();
                    nextPos += buffer->getPos();

                    if ((it = strings.find(elementId + "-" + intToStr(j))) != strings.end())
                        buffer->writeS(it->second);

                    buffer->setPos(nextPos);
                }

                if ((it = strings.find(elementId)) != strings.end())
                    buffer->writeS(it->second);
            }

            break;
        }
        default:
            break;
        }

        buffer->setPos(curPos + dataLen);
    }
}

NS_FGUI_END
