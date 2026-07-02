#include "TranslationHelper.h"
#include "PackageItem.h"
#include "UIPackage.h"
#include "utils/ByteBuffer.h"
#include "core/io/xml_parser.h"

NS_FGUI_BEGIN

using namespace std;

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> TranslationHelper::strings;

static std::string intToStr(int v)
{
    return std::string(Variant(v).stringify().utf8().get_data());
}

void TranslationHelper::loadFromXML(const char* xmlString, size_t nBytes)
{
    strings.clear();

    if (xmlString == nullptr || nBytes == 0)
        return;

    PackedByteArray buffer;
    buffer.resize((int)nBytes);
    memcpy(buffer.ptrw(), xmlString, nBytes);

    Ref<XMLParser> parser;
    parser.instantiate();
    if (parser->open_buffer(buffer) != OK)
        return;

    while (true)
    {
        Error err = parser->read();
        if (err != OK)
            break;

        if (parser->get_node_type() != XMLParser::NODE_ELEMENT)
            continue;

        if (parser->get_node_name() != String("string"))
            continue;

        String nameAttr = parser->get_named_attribute_value("name");
        if (nameAttr.is_empty())
            continue;

        std::string key = nameAttr.utf8().get_data();
        std::string text;

        if (!parser->is_empty())
        {
            while (true)
            {
                Error inner = parser->read();
                if (inner != OK)
                    break;
                if (parser->get_node_type() == XMLParser::NODE_ELEMENT_END)
                    break;
                if (parser->get_node_type() == XMLParser::NODE_TEXT)
                    text += std::string(parser->get_node_data().utf8().get_data());
            }
        }

        size_t i = key.find('-');
        if (i == std::string::npos)
            continue;

        strings[key.substr(0, i)][key.substr(i + 1)] = text;
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
