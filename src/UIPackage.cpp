#include "UIPackage.h"
#include "GObject.h"
#include "UIObjectFactory.h"
#include "UIConfig.h"
#include "display/BitmapFont.h"
#include "event/HitTest.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"
#include "core/io/image.h"
#include "core/io/image_loader.h"
#include "core/io/resource_uid.h"
#include "scene/resources/image_texture.h"
#include "core/io/file_access.h"

NS_FGUI_BEGIN

using namespace std;

const string UIPackage::URL_PREFIX = "ui://";
int UIPackage::_constructing = 0;
std::string UIPackage::_branch;
std::unordered_map<std::string, std::string> UIPackage::_vars;

std::unordered_map<std::string, UIPackage*> UIPackage::_packageInstById;
std::unordered_map<std::string, UIPackage*> UIPackage::_packageInstByName;
std::vector<UIPackage*> UIPackage::_packageList;

Ref<Texture2D> UIPackage::_emptyTexture;

UIPackage::UIPackage()
    : _branchIndex(-1)
{
}

UIPackage::~UIPackage()
{
    for (auto& it : _items)
        FGUI_DELETE(it);
    for (auto& it : _sprites)
        delete it.second;
}

void UIPackage::setBranch(const std::string& value)
{
    _branch = value;
    for (auto& it : _packageList)
    {
        if (it->_branches.size() > 0)
        {
            it->_branchIndex = ToolSet::findInStringArray(it->_branches, value);
        }
    }
}

const std::string& UIPackage::getVar(const std::string& key)
{
    auto it = _vars.find(key);
    if (it != _vars.end())
        return it->second;
    else
        return EMPTY_STRING;
}

void UIPackage::setVar(const std::string& key, const std::string& value)
{
    _vars[key] = value;
}

UIPackage* UIPackage::getById(const string& id)
{
    auto it = _packageInstById.find(id);
    if (it != _packageInstById.end())
        return it->second;
    return nullptr;
}

UIPackage* UIPackage::getByName(const string& name)
{
    auto it = _packageInstByName.find(name);
    if (it != _packageInstByName.end())
        return it->second;
    return nullptr;
}

UIPackage* UIPackage::addPackage(const string& assetPath)
{
    auto it = _packageInstById.find(assetPath);
    if (it != _packageInstById.end())
        return it->second;

    if (_emptyTexture.is_null())
    {
        // Create a 2x2 empty texture
        Ref<Image> emptyImg;
        emptyImg.instantiate();
        // Create a tiny white image
        PackedByteArray emptyData;
        emptyData.resize(16);
        for (int i = 0; i < 16; i++)
            emptyData.set(i, 0);
        emptyImg->set_data(2, 2, false, Image::FORMAT_RGBA8, emptyData);

        Ref<ImageTexture> emptyTex;
        emptyTex.instantiate();
        emptyTex->set_image(emptyImg);
        _emptyTexture = emptyTex;
    }

    // Read .fui file
    string fuiPath = assetPath + ".fui";
    Ref<FileAccess> file = FileAccess::open(String(fuiPath.c_str()), FileAccess::READ);
    if (file.is_null() || !file->is_open())
    {
        print_line("FairyGUI: cannot load package from '", assetPath.c_str(), "'");
        return nullptr;
    }

    PackedByteArray fileData = file->get_buffer(file->get_length());
    file.unref();

    char* p = new char[fileData.size()];
    memcpy(p, fileData.ptr(), fileData.size());
    ByteBuffer buffer(p, 0, (int)fileData.size(), true);

    Ref<UIPackage> pkg_ref = memnew(UIPackage);
    UIPackage* pkg = pkg_ref.ptr();
    pkg->_assetPath = assetPath;
    if (!pkg->loadPackage(&buffer))
    {
        // pkg_ref dtor cleans up (init_ref gave refcount=1 → dtor → 0 → freed)
        return nullptr;
    }

    _packageInstById[pkg->getId()] = pkg;
    _packageInstByName[pkg->getName()] = pkg;
    _packageInstById[assetPath] = pkg;
    _packageList.push_back(pkg);

    pkg->reference(); // keep alive for static maps (pkg_ref dtor: 2→1)
    return pkg;
}

void UIPackage::removePackage(const string& packageIdOrName)
{
    UIPackage* pkg = UIPackage::getByName(packageIdOrName);
    if (!pkg)
        pkg = getById(packageIdOrName);

    if (pkg)
    {
        auto it = std::find(_packageList.cbegin(), _packageList.cend(), pkg);
        if (it != _packageList.cend())
            _packageList.erase(it);

        _packageInstById.erase(pkg->getId());
        _packageInstById.erase(pkg->_assetPath);
        _packageInstByName.erase(pkg->getName());

        pkg->unreference(); // matches reference() in addPackage
    }
    else
        print_line("FairyGUI: invalid package name or id: ", packageIdOrName.c_str());
}

void UIPackage::removeAllPackages()
{
    for (auto& it : _packageList)
        it->unreference(); // matches reference() in addPackage

    _packageInstById.clear();
    _packageInstByName.clear();
    _packageList.clear();
}

GObject* UIPackage::createObject(const string& pkgName, const string& resName)
{
    UIPackage* pkg = UIPackage::getByName(pkgName);
    if (pkg)
        return pkg->createObject(resName);
    else
    {
        print_line("FairyGUI: package not found - ", pkgName.c_str());
        return nullptr;
    }
}

GObject* UIPackage::createObjectFromURL(const string& url)
{
    PackageItem* pi = UIPackage::getItemByURL(url);
    if (pi)
        return pi->owner->createObject(pi);
    else
    {
        print_line("FairyGUI: resource not found - ", url.c_str());
        return nullptr;
    }
}

string UIPackage::getItemURL(const string& pkgName, const string& resName)
{
    UIPackage* pkg = UIPackage::getByName(pkgName);
    if (pkg)
    {
        PackageItem* pi = pkg->getItemByName(resName);
        if (pi)
            return URL_PREFIX + pkg->getId() + pi->id;
    }
    return EMPTY_STRING;
}

PackageItem* UIPackage::getItemByURL(const string& url)
{
    if (url.size() == 0)
        return nullptr;

    ssize_t pos1 = url.find('/');
    if (pos1 == -1)
        return nullptr;

    ssize_t pos2 = url.find('/', pos1 + 2);
    if (pos2 == -1)
    {
        if (url.size() > 13)
        {
            string pkgId = url.substr(5, 8);
            UIPackage* pkg = getById(pkgId);
            if (pkg != nullptr)
            {
                string srcId = url.substr(13);
                return pkg->getItem(srcId);
            }
        }
    }
    else
    {
        string pkgName = url.substr(pos1 + 2, pos2 - pos1 - 2);
        UIPackage* pkg = getByName(pkgName);
        if (pkg != nullptr)
        {
            string srcName = url.substr(pos2 + 1);
            return pkg->getItemByName(srcName);
        }
    }

    return nullptr;
}

string UIPackage::normalizeURL(const string& url)
{
    if (url.size() == 0)
        return url;

    ssize_t pos1 = url.find('/');
    if (pos1 == -1)
        return url;

    ssize_t pos2 = url.find('/', pos1 + 2);
    if (pos2 == -1)
        return url;
    else
    {
        string pkgName = url.substr(pos1 + 2, pos2 - pos1 - 2);
        string srcName = url.substr(pos2 + 1);
        return getItemURL(pkgName, srcName);
    }
}

void* UIPackage::getItemAsset(const std::string& pkgName, const std::string& resName, PackageItemType type)
{
    UIPackage* pkg = UIPackage::getByName(pkgName);
    if (pkg)
    {
        PackageItem* pi = pkg->getItemByName(resName);
        if (pi)
        {
            if (type != PackageItemType::UNKNOWN && pi->type != type)
                return nullptr;
            else
                return pkg->getItemAsset(pi);
        }
    }
    return nullptr;
}

void* UIPackage::getItemAssetByURL(const std::string& url, PackageItemType type)
{
    PackageItem* pi = UIPackage::getItemByURL(url);
    if (pi)
    {
        if (type != PackageItemType::UNKNOWN && pi->type != type)
            return nullptr;
        else
            return pi->owner->getItemAsset(pi);
    }
    else
        return nullptr;
}

PackageItem* UIPackage::getItem(const string& itemId)
{
    auto it = _itemsById.find(itemId);
    if (it != _itemsById.end())
        return it->second;
    return nullptr;
}

PackageItem* UIPackage::getItemByName(const string& itemName)
{
    auto it = _itemsByName.find(itemName);
    if (it != _itemsByName.end())
        return it->second;
    return nullptr;
}

GObject* UIPackage::createObject(const string& resName)
{
    PackageItem* pi = getItemByName(resName);
    if (!pi)
    {
        print_line("FairyGUI: resource not found - ", resName.c_str(), " in ", _name.c_str());
        return nullptr;
    }

    return createObject(pi);
}

GObject* UIPackage::createObject(PackageItem* item)
{
    GObject* g = UIObjectFactory::newObject(item);
    if (g == nullptr)
        return nullptr;

    _constructing++;
    g->constructFromResource();
    _constructing--;
    return g;
}

bool UIPackage::loadPackage(ByteBuffer* buffer)
{
    if (buffer->readUint() != 0x46475549)
    {
        print_line("FairyGUI: old package format found in '", _assetPath.c_str(), "'");
        return false;
    }

    buffer->version = buffer->readInt();
    bool ver2 = buffer->version >= 2;
    buffer->readBool(); //compressed
    _id = buffer->readString();
    _name = buffer->readString();
    buffer->skip(20);
    int indexTablePos = buffer->getPos();
    int cnt;

    buffer->seek(indexTablePos, 4);

    cnt = buffer->readInt();
    _stringTable.resize(cnt);
    for (int i = 0; i < cnt; i++)
        _stringTable[i] = buffer->readString();
    buffer->setStringTable(&_stringTable);

    buffer->seek(indexTablePos, 0);
    cnt = buffer->readShort();
    for (int i = 0; i < cnt; i++)
    {
        std::unordered_map<std::string, std::string> info;
        info["id"] = buffer->readS();
        info["name"] = buffer->readS();

        _dependencies.push_back(info);
    }

    bool branchIncluded = false;
    if (ver2)
    {
        cnt = buffer->readShort();
        if (cnt > 0)
        {
            buffer->readSArray(_branches, cnt);
            if (_branch.size() > 0)
                _branchIndex = ToolSet::findInStringArray(_branches, _branch);
        }

        branchIncluded = cnt > 0;
    }

    buffer->seek(indexTablePos, 1);

    PackageItem* pi;
    string path = _assetPath;
    size_t pos = path.find('/');
    string shortPath = pos == -1 ? EMPTY_STRING : path.substr(0, pos + 1);
    path += "_";

    cnt = buffer->readShort();
    for (int i = 0; i < cnt; i++)
    {
        int nextPos = buffer->readInt();
        nextPos += buffer->getPos();

        pi = new PackageItem();
        pi->owner = this;
        pi->type = (PackageItemType)buffer->readByte();
        pi->id = buffer->readS();
        pi->name = buffer->readS();
        buffer->skip(2); //path
        pi->file = buffer->readS();
        buffer->readBool(); //exported
        pi->width = buffer->readInt();
        pi->height = buffer->readInt();

        switch (pi->type)
        {
        case PackageItemType::IMAGE:
        {
            pi->objectType = ObjectType::IMAGE;
            int scaleOption = buffer->readByte();
            if (scaleOption == 1)
            {
                pi->scale9Grid = Rect2(
                    (float)buffer->readInt(),
                    (float)buffer->readInt(),
                    (float)buffer->readInt(),
                    (float)buffer->readInt()
                );
                pi->tileGridIndice = buffer->readInt();
                pi->hasScale9Grid = true;
            }
            else if (scaleOption == 2)
                pi->scaleByTile = true;

            buffer->readBool(); //smoothing
            break;
        }

        case PackageItemType::MOVIECLIP:
        {
            buffer->readBool(); //smoothing
            pi->objectType = ObjectType::MOVIECLIP;
            pi->rawData = buffer->readBuffer();
            break;
        }

        case PackageItemType::FONT:
        {
            pi->rawData = buffer->readBuffer();
            break;
        }

        case PackageItemType::COMPONENT:
        {
            int extension = buffer->readByte();
            if (extension > 0)
                pi->objectType = (ObjectType)extension;
            else
                pi->objectType = ObjectType::COMPONENT;
            pi->rawData = buffer->readBuffer();

            UIObjectFactory::resolvePackageItemExtension(pi);
            break;
        }

        case PackageItemType::ATLAS:
        case PackageItemType::SOUND:
        case PackageItemType::MISC:
        {
            pi->file = path + pi->file;
            break;
        }

        case PackageItemType::SPINE:
        case PackageItemType::DRAGONBONES:
        {
            pi->file = shortPath + pi->file;
            pi->skeletonAnchor = Vector2(buffer->readFloat(), buffer->readFloat());
            pi->hasSkeletonAnchor = true;
            break;
        }

        default:
            break;
        }

        if (ver2)
        {
            std::string str = buffer->readS(); //branch
            if (!str.empty())
                pi->name = str + "/" + pi->name;

            int branchCnt = buffer->readUbyte();
            if (branchCnt > 0)
            {
                if (branchIncluded)
                {
                    pi->branches = new std::vector<std::string>();
                    buffer->readSArray(*pi->branches, branchCnt);
                }
                else
                    _itemsById[buffer->readS()] = pi;
            }

            int highResCnt = buffer->readUbyte();
            if (highResCnt > 0)
            {
                pi->highResolution = new std::vector<std::string>();
                buffer->readSArray(*pi->highResolution, highResCnt);
            }
        }

        _items.push_back(pi);
        _itemsById[pi->id] = pi;
        if (!pi->name.empty())
            _itemsByName[pi->name] = pi;

        buffer->setPos(nextPos);
    }

    buffer->seek(indexTablePos, 2);

    cnt = buffer->readShort();
    for (int i = 0; i < cnt; i++)
    {
        int nextPos = buffer->readUshort();
        nextPos += buffer->getPos();

        const string& itemId = buffer->readS();
        pi = _itemsById[buffer->readS()];

        AtlasSprite* sprite = new AtlasSprite();
        sprite->atlas = pi;
        sprite->rect = Rect2(
            (float)buffer->readInt(),
            (float)buffer->readInt(),
            (float)buffer->readInt(),
            (float)buffer->readInt()
        );
        sprite->rotated = buffer->readBool();
        if (ver2 && buffer->readBool())
        {
            sprite->offset = Vector2((float)buffer->readInt(), (float)buffer->readInt());
            sprite->originalSize = Vector2((float)buffer->readInt(), (float)buffer->readInt());
        }
        else
        {
            sprite->offset = Vector2();
            sprite->originalSize = sprite->rect.size;
        }
        _sprites[itemId] = sprite;

        buffer->setPos(nextPos);
    }

    if (buffer->seek(indexTablePos, 3))
    {
        cnt = buffer->readShort();
        for (int i = 0; i < cnt; i++)
        {
            int nextPos = buffer->readInt();
            nextPos += buffer->getPos();

            auto it = _itemsById.find(buffer->readS());
            if (it != _itemsById.end())
            {
                pi = it->second;
                if (pi->type == PackageItemType::IMAGE)
                {
                    pi->pixelHitTestData = new PixelHitTestData();
                    pi->pixelHitTestData->load(buffer);
                }
            }

            buffer->setPos(nextPos);
        }
    }

    return true;
}

void* UIPackage::getItemAsset(PackageItem* item)
{
    switch (item->type)
    {
    case PackageItemType::IMAGE:
        if (item->imageFrame.texture.is_null())
            loadImage(item);
        return (void*)&item->imageFrame;

    case PackageItemType::ATLAS:
        if (item->texture.is_null())
            loadAtlas(item);
        return item->texture.ptr();

    case PackageItemType::FONT:
        if (item->bitmapFont == nullptr)
            loadFont(item);
        return item->bitmapFont;

    case PackageItemType::MOVIECLIP:
        if (item->movieclip == nullptr)
            loadMovieClip(item);
        return item->movieclip;

    default:
        return nullptr;
    }
}

void UIPackage::loadAtlas(PackageItem* item)
{
    Ref<Image> image;
    image.instantiate();
    Error err = image->load(GObject::toGodotStr(item->file));
    if (err != Error::OK)
    {
        item->texture = _emptyTexture;
        print_line("FairyGUI: texture '", item->file.c_str(), "' not found in ", _name.c_str());
        return;
    }

    // Handle separate alpha texture for ETC1 format
    string alphaFilePath;
    string ext;
    size_t dotPos = item->file.find_last_of('.');
    if (dotPos != -1)
    {
        ext = item->file.substr(dotPos);
        alphaFilePath = item->file.substr(0, dotPos) + "!a" + ext;
    }
    else
    {
        alphaFilePath = item->file + "!a";
    }

    if (ToolSet::isFileExist(alphaFilePath))
    {
        Ref<Image> alphaImg;
        alphaImg.instantiate();
        if (ImageLoader::load_image(GObject::toGodotStr(alphaFilePath), alphaImg) == Error::OK)
        {
            // Combine RGB from main image, alpha from alpha image
            // Get pixel data from both images
            int width = image->get_width();
            int height = image->get_height();
            PackedByteArray mainData = image->get_data();
            PackedByteArray alphaData = alphaImg->get_data();

            int alphaW = alphaImg->get_width();
            int alphaH = alphaImg->get_height();

            // Copy alpha channel from alpha image to main image
            for (int y = 0; y < height && y < alphaH; y++)
            {
                for (int x = 0; x < width && x < alphaW; x++)
                {
                    int mainIdx = (y * width + x) * 4;
                    int alphaIdx = (y * alphaW + x) * 4;

                    if (mainIdx + 3 < mainData.size() && alphaIdx < alphaData.size())
                    {
                        // Use red channel of alpha image as the alpha of main image
                        mainData.set(mainIdx + 3, alphaData[alphaIdx]);
                    }
                }
            }

            image->set_data(width, height, false, Image::FORMAT_RGBA8, mainData);
        }
    }

    Ref<ImageTexture> tex;
    tex.instantiate();
    tex->set_image(image);
    item->texture = tex;
}

AtlasSprite* UIPackage::getSprite(const std::string& spriteId)
{
    auto it = _sprites.find(spriteId);
    if (it != _sprites.end())
        return it->second;
    else
        return nullptr;
}

void UIPackage::createSpriteTexture(AtlasSprite* sprite, ImageFrame& outFrame)
{
    getItemAsset(sprite->atlas);

    outFrame.texture = sprite->atlas->texture;
    outFrame.region = sprite->rect;
    outFrame.rotated = sprite->rotated;
    outFrame.offset = Vector2(
        sprite->offset.x - (sprite->originalSize.x - sprite->rect.size.x) / 2,
        -(sprite->offset.y - (sprite->originalSize.y - sprite->rect.size.y) / 2)
    );
    outFrame.originalSize = sprite->originalSize;
    outFrame.originalSizeInPixels = sprite->rect.size;
}

void UIPackage::loadImage(PackageItem* item)
{
    AtlasSprite* sprite = getSprite(item->id);
    if (sprite != nullptr)
    {
        item->imageFrame = ImageFrame();
        createSpriteTexture(sprite, item->imageFrame);
        item->texture = item->imageFrame.texture;
    }
    else
    {
        // Create empty image frame using empty texture
        item->imageFrame = ImageFrame();
        item->imageFrame.texture = _emptyTexture;
        item->imageFrame.region = Rect2(0, 0, 2, 2);
        item->imageFrame.originalSize = Vector2(2, 2);
        item->imageFrame.originalSizeInPixels = Vector2(2, 2);
        item->texture = _emptyTexture;
    }

    if (item->scaleByTile)
    {
        // In Godot, texture repeat is set on the CanvasItem/Sprite2D level
        // This is handled by FUISprite when rendering with scaleByTile
    }
}

void UIPackage::loadMovieClip(PackageItem* item)
{
    item->movieclip = new MovieClipData();

    ByteBuffer* buffer = item->rawData;

    buffer->seek(0, 0);

    float interval = buffer->readInt() / 1000.0f;
    item->swing = buffer->readBool();
    item->repeatDelay = buffer->readInt() / 1000.0f;

    buffer->seek(0, 1);

    int frameCount = buffer->readShort();
    item->movieclip->frames.resize(frameCount);

    Vector2 mcSizeInPixels = Vector2((float)item->width, (float)item->height);
    Vector2 mcSize = mcSizeInPixels;

    AtlasSprite* sprite;

    for (int i = 0; i < frameCount; i++)
    {
        int nextPos = buffer->readUshort();
        nextPos += buffer->getPos();

        Rect2 rect;
        rect.position.x = (float)buffer->readInt();
        rect.position.y = (float)buffer->readInt();
        rect.size.x = (float)buffer->readInt();
        rect.size.y = (float)buffer->readInt();
        float addDelay = buffer->readInt() / 1000.0f;
        const string& spriteId = buffer->readS();

        MovieClipFrameData& frameData = item->movieclip->frames[i];
        frameData.addDelay = addDelay;

        if (!spriteId.empty() && (sprite = getSprite(spriteId)) != nullptr)
        {
            getItemAsset(sprite->atlas);

            frameData.imageData.texture = sprite->atlas->texture;
            frameData.imageData.region = sprite->rect;
            frameData.imageData.rotated = sprite->rotated;
            frameData.imageData.offset = Vector2(
                rect.position.x - (mcSize.x - rect.size.x) / 2,
                -(rect.position.y - (mcSize.y - rect.size.y) / 2)
            );
            frameData.imageData.originalSize = mcSize;
            frameData.imageData.originalSizeInPixels = mcSizeInPixels;
        }
        else
        {
            frameData.imageData.texture = _emptyTexture;
            frameData.imageData.region = Rect2(0, 0, 2, 2);
            frameData.imageData.offset = Vector2(
                rect.position.x - (mcSize.x - rect.size.x) / 2,
                -(rect.position.y - (mcSize.y - rect.size.y) / 2)
            );
            frameData.imageData.originalSize = mcSize;
            frameData.imageData.originalSizeInPixels = mcSizeInPixels;
        }

        buffer->setPos(nextPos);
    }

    item->movieclip->interval = interval;
    item->movieclip->repeatDelay = item->repeatDelay;
    item->movieclip->swing = item->swing;

    delete buffer;
    item->rawData = nullptr;
}

void UIPackage::loadFont(PackageItem* item)
{
    item->bitmapFont = BitmapFont::create();
    FontAtlas* fontAtlas = new FontAtlas(*item->bitmapFont);
    item->bitmapFont->_fontAtlas = fontAtlas;

    ByteBuffer* buffer = item->rawData;

    buffer->seek(0, 0);

    bool ttf = buffer->readBool();
    item->bitmapFont->_canTint = buffer->readBool();
    item->bitmapFont->_resizable = buffer->readBool();
    buffer->readBool(); //hasChannel
    int fontSize = buffer->readInt();
    int xadvance = buffer->readInt();
    int lineHeight = buffer->readInt();

    Ref<Texture2D> mainTexture;
    AtlasSprite* mainSprite = nullptr;

    if (ttf && (mainSprite = getSprite(item->id)) != nullptr)
    {
        getItemAsset(mainSprite->atlas);
        mainTexture = mainSprite->atlas->texture;
    }

    buffer->seek(0, 1);

    FontLetterDefinition def;
    int bx = 0, by = 0;
    int bw = 0, bh = 0;
    PackageItem* charImg = nullptr;

    int cnt = buffer->readInt();
    for (int i = 0; i < cnt; i++)
    {
        int nextPos = buffer->readUshort();
        nextPos += buffer->getPos();

        memset(&def, 0, sizeof(def));

        unsigned short ch = buffer->readUshort();
        const string& img = buffer->readS();
        bx = buffer->readInt();
        by = buffer->readInt();
        def.offsetX = (float)buffer->readInt();
        def.offsetY = (float)buffer->readInt();
        bw = buffer->readInt();
        bh = buffer->readInt();
        def.xAdvance = (float)buffer->readInt();
        buffer->readByte(); //channel

        if (ttf)
        {
            Rect2 tempRect = Rect2(
                (float)(bx + mainSprite->rect.position.x),
                (float)(by + mainSprite->rect.position.y),
                (float)bw,
                (float)bh
            );
            def.U = tempRect.position.x;
            def.V = tempRect.position.y;
            def.width = tempRect.size.x;
            def.height = tempRect.size.y;
            def.validDefinition = true;
        }
        else
        {
            charImg = getItem(img);
            if (charImg)
            {
                charImg = charImg->getBranch();
                bw = charImg->width;
                bh = charImg->height;

                AtlasSprite* atlasSprite = getSprite(img);
                if (atlasSprite != nullptr)
                {
                    def.offsetX += atlasSprite->offset.x;
                    def.offsetY += atlasSprite->offset.y;
                }

                charImg = charImg->getHighResolution();
                getItemAsset(charImg);

                if (charImg->imageFrame.texture.is_valid())
                {
                    Rect2 tempRect = charImg->imageFrame.region;
                    def.U = tempRect.position.x;
                    def.V = tempRect.position.y;
                    def.width = tempRect.size.x;
                    def.height = tempRect.size.y;
                    if (mainTexture.is_null())
                        mainTexture = charImg->imageFrame.texture;
                    def.validDefinition = true;
                }

                if (def.xAdvance == 0)
                {
                    if (xadvance == 0)
                        def.xAdvance = def.offsetX + bw;
                    else
                        def.xAdvance = (float)xadvance;
                }

                if (fontSize == 0)
                    fontSize = bh;
                lineHeight = std::max(fontSize, lineHeight);
            }
        }

        fontAtlas->addLetterDefinition(ch, def);
        buffer->setPos(nextPos);
    }

    if (mainTexture.is_valid())
        fontAtlas->addTexture(mainTexture, 0);
    fontAtlas->setLineHeight((float)lineHeight);
    item->bitmapFont->_originalFontSize = (float)fontSize;
    item->bitmapFont->_fontSize = fontSize;

    delete buffer;
    item->rawData = nullptr;
}

void UIPackage::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getId"), &UIPackage::gd_getId);
    ClassDB::bind_method(D_METHOD("getName"), &UIPackage::gd_getName);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("getById", "id"), &UIPackage::gd_getById);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getByName", "name"), &UIPackage::gd_getByName);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("addPackage", "asset_path"), &UIPackage::gd_addPackage);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("removePackage", "package_id_or_name"), &UIPackage::gd_removePackage);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("removeAllPackages"), &UIPackage::removeAllPackages);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("createObject", "pkg_name", "res_name"), &UIPackage::gd_createObject);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("createObjectFromURL", "url"), &UIPackage::gd_createObjectFromURL);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("getItemURL", "pkg_name", "res_name"), &UIPackage::gd_getItemURL);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("registerFont", "alias_name", "real_name"), &UIPackage::gd_registerFont);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("setDefaultFont", "font_name"), &UIPackage::gd_setDefaultFont);
}

String UIPackage::gd_getId() const { return String(getId().c_str()); }
String UIPackage::gd_getName() const { return String(getName().c_str()); }
UIPackage* UIPackage::gd_getById(const String& id) { return getById(id.utf8().get_data()); }
UIPackage* UIPackage::gd_getByName(const String& name) { return getByName(name.utf8().get_data()); }
UIPackage* UIPackage::gd_addPackage(const String& descFilePath) { return addPackage(descFilePath.utf8().get_data()); }
void UIPackage::gd_removePackage(const String& packageIdOrName) { removePackage(packageIdOrName.utf8().get_data()); }
GObject* UIPackage::gd_createObject(const String& pkgName, const String& resName) { return createObject(pkgName.utf8().get_data(), resName.utf8().get_data()); }
GObject* UIPackage::gd_createObjectFromURL(const String& url) { return createObjectFromURL(url.utf8().get_data()); }
String UIPackage::gd_getItemURL(const String& pkgName, const String& resName) { return String(getItemURL(pkgName.utf8().get_data(), resName.utf8().get_data()).c_str()); }

void UIPackage::gd_registerFont(const String& aliasName, const String& realName) { UIConfig::registerFont(aliasName.utf8().get_data(), realName.utf8().get_data()); }
void UIPackage::gd_setDefaultFont(const String& fontName) { UIConfig::defaultFont = fontName.utf8().get_data(); }

NS_FGUI_END
