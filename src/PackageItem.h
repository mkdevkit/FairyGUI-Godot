#ifndef __PACKAGEITEM_H__
#define __PACKAGEITEM_H__

#include "FairyGUIMacros.h"
#include "godot_types.h"

NS_FGUI_BEGIN

class UIPackage;
class UIObjectFactory;
class GComponent;
class BitmapFont;
class PixelHitTestData;
class ByteBuffer;

// Stores image frame data (replaces cocos2d::SpriteFrame)
struct ImageFrame
{
    Ref<Texture2D> texture;
    Rect2 region;           // region in texture (pixels)
    bool rotated;
    Vector2 offset;
    Vector2 originalSize;   // in points
    Vector2 originalSizeInPixels;

    ImageFrame()
        : region()
        , rotated(false)
        , offset()
        , originalSize()
        , originalSizeInPixels()
    {
    }

    ~ImageFrame() {}
};

// Stores a single movieclip frame
struct MovieClipFrameData
{
    ImageFrame imageData;
    float addDelay;

    MovieClipFrameData() : imageData(), addDelay(0) {}
};

// Stores movieclip animation data (replaces cocos2d::Animation)
struct MovieClipData
{
    std::vector<MovieClipFrameData> frames;
    float interval;
    float repeatDelay;
    bool swing;

    MovieClipData()
        : interval(0)
        , repeatDelay(0)
        , swing(false)
    {
    }
};

class PackageItem
{
public:
    PackageItem();
    virtual ~PackageItem();

    void load();
    PackageItem* getBranch();
    PackageItem* getHighResolution();

public:
    UIPackage* owner;

    PackageItemType type;
    ObjectType objectType;
    std::string id;
    std::string name;
    int width;
    int height;
    std::string file;
    ByteBuffer* rawData;
    std::vector<std::string>* branches;
    std::vector<std::string>* highResolution;

    //atlas
    Ref<Texture2D> texture;

    //image
    Rect2 scale9Grid;
    bool scaleByTile;
    int tileGridIndice;
    bool hasScale9Grid;     // whether scale9Grid is valid
    ImageFrame imageFrame;  // replaces SpriteFrame
    PixelHitTestData* pixelHitTestData;

    //movieclip
    MovieClipData* movieclip; // replaces Animation*
    float repeatDelay;       // kept for GMovieClip (original field)
    bool swing;              // kept for GMovieClip (original field)

    //component
    std::function<GComponent*()> extensionCreator;
    bool translated;

    //font
    BitmapFont* bitmapFont;

    //skeleton
    Vector2 skeletonAnchor;
    bool hasSkeletonAnchor;  // whether skeletonAnchor is valid
};

NS_FGUI_END

#endif
