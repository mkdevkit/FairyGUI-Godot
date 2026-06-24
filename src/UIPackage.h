#ifndef __UIPACKAGE_H__
#define __UIPACKAGE_H__

#include "FairyGUIMacros.h"
#include "GObject.h"
#include "PackageItem.h"
#include "godot_types.h"

NS_FGUI_BEGIN

struct AtlasSprite;
class ByteBuffer;

class UIPackage : public RefCounted
{
    GDCLASS(UIPackage, RefCounted)

public:
    UIPackage();
    ~UIPackage();

    static void _bind_methods();

    static UIPackage* getById(const std::string& id);
    static UIPackage* getByName(const std::string& name);
    static UIPackage* addPackage(const std::string& descFilePath);
    static void removePackage(const std::string& packageIdOrName);
    static void removeAllPackages();
    static GObject* createObject(const std::string& pkgName, const std::string& resName);
    static GObject* createObjectFromURL(const std::string& url);
    static std::string getItemURL(const std::string& pkgName, const std::string& resName);
    static PackageItem* getItemByURL(const std::string& url);

    static UIPackage* gd_getById(const String& id);
    static UIPackage* gd_getByName(const String& name);
    static UIPackage* gd_addPackage(const String& descFilePath);
    static void gd_removePackage(const String& packageIdOrName);
    static GObject* gd_createObject(const String& pkgName, const String& resName);
    static GObject* gd_createObjectFromURL(const String& url);
    static String gd_getItemURL(const String& pkgName, const String& resName);

    static void gd_registerFont(const String& aliasName, const String& realName);
    static void gd_setDefaultFont(const String& fontName);

    static std::string normalizeURL(const std::string& url);
    static void* getItemAsset(const std::string& pkgName, const std::string& resName, PackageItemType type = PackageItemType::UNKNOWN);
    static void* getItemAssetByURL(const std::string& url, PackageItemType type = PackageItemType::UNKNOWN);
    static Ref<Texture2D> getEmptyTexture() { return _emptyTexture; }

    const std::string& getId() const { return _id; }

    String gd_getId() const;
    String gd_getName() const;
    const std::string& getName() const { return _name; }

    PackageItem* getItem(const std::string& itemId);
    PackageItem* getItemByName(const std::string& itemName);
    void* getItemAsset(PackageItem* item);

    static const std::string& getBranch() { return _branch; }
    static void setBranch(const std::string& value);
    static const std::string& getVar(const std::string& key);
    static void setVar(const std::string& key, const std::string& value);

    static int _constructing;
    static const std::string URL_PREFIX;

private:
    bool loadPackage(ByteBuffer* buffer);
    void loadAtlas(PackageItem* item);
    AtlasSprite* getSprite(const std::string& spriteId);
    void createSpriteTexture(AtlasSprite* sprite, ImageFrame& outFrame);
    void loadImage(PackageItem* item);
    void loadMovieClip(PackageItem* item);
    void loadFont(PackageItem* item);

    GObject* createObject(const std::string& resName);
    GObject* createObject(PackageItem* item);

private:
    std::string _id;
    std::string _name;
    std::string _assetPath;

    std::vector<PackageItem*> _items;
    std::unordered_map<std::string, PackageItem*> _itemsById;
    std::unordered_map<std::string, PackageItem*> _itemsByName;
    std::unordered_map<std::string, AtlasSprite*> _sprites;
    std::string _customId;
    std::vector<std::string> _stringTable;
    std::vector<std::unordered_map<std::string, std::string>> _dependencies;
    std::vector<std::string> _branches;
    int _branchIndex;

    static std::unordered_map<std::string, UIPackage*> _packageInstById;
    static std::unordered_map<std::string, UIPackage*> _packageInstByName;
    static std::vector<UIPackage*> _packageList;
    static std::unordered_map<std::string, std::string> _vars;
    static std::string _branch;

    static Ref<Texture2D> _emptyTexture;

    friend class PackageItem;
};

// Atlas sprite data (replaces cocos2d atlas sprite)
struct AtlasSprite
{
    PackageItem* atlas;
    Rect2 rect;
    Vector2 originalSize;
    Vector2 offset;
    bool rotated;
};

NS_FGUI_END

#endif
