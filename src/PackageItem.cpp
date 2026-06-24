#include "PackageItem.h"
#include "GRoot.h"
#include "UIPackage.h"
#include "display/BitmapFont.h"
#include "utils/ByteBuffer.h"

NS_FGUI_BEGIN

PackageItem::PackageItem()
    : owner(nullptr)
    , objectType(ObjectType::COMPONENT)
    , width(0)
    , height(0)
    , rawData(nullptr)
    , scaleByTile(false)
    , tileGridIndice(0)
    , hasScale9Grid(false)
    , pixelHitTestData(nullptr)
    , movieclip(nullptr)
    , repeatDelay(0)
    , swing(false)
    , extensionCreator(nullptr)
    , translated(false)
    , bitmapFont(nullptr)
    , branches(nullptr)
    , highResolution(nullptr)
    , hasSkeletonAnchor(false)
{
}

PackageItem::~PackageItem()
{
    if (rawData)
    {
        delete rawData;
        rawData = nullptr;
    }
    if (bitmapFont)
        bitmapFont->releaseAtlas();
    bitmapFont = nullptr;

    if (movieclip)
    {
        delete movieclip;
        movieclip = nullptr;
    }

    if (pixelHitTestData)
    {
        delete pixelHitTestData;
        pixelHitTestData = nullptr;
    }

    if (branches)
    {
        delete branches;
        branches = nullptr;
    }
    if (highResolution)
    {
        delete highResolution;
        highResolution = nullptr;
    }
}

void PackageItem::load()
{
    owner->getItemAsset(this);
}

PackageItem* PackageItem::getBranch()
{
    if (branches != nullptr && owner->_branchIndex != -1)
    {
        std::string itemId = (*branches)[owner->_branchIndex];
        if (!itemId.empty())
            return owner->getItem(itemId);
    }

    return this;
}

PackageItem* PackageItem::getHighResolution()
{
    if (highResolution != nullptr && GRoot::contentScaleLevel > 0)
    {
        std::string itemId = (*highResolution)[GRoot::contentScaleLevel - 1];
        if (!itemId.empty())
            return owner->getItem(itemId);
    }

    return this;
}

NS_FGUI_END
