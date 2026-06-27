#include "GLoader.h"
#include "GComponent.h"
#include "GObject.h"
#include "GMovieClip.h"
#include "PackageItem.h"
#include "UIPackage.h"
#include "display/FUISprite.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"
#include "core/io/resource_loader.h"

#include "core/io/image.h"
#include "core/io/image_loader.h"
#include "core/io/resource_uid.h"
#include "scene/resources/image_texture.h"

NS_FGUI_BEGIN

GLoader::GLoader()
    : _autoSize(false),
    _align(AlignType::LEFT),
    _verticalAlign(VertAlignType::TOP),
    _fill(LoaderFillType::NONE),
    _shrinkOnly(false),
    _updatingLayout(false),
    _contentItem(nullptr),
    _contentStatus(0),
    _content(nullptr),
    _content2(nullptr),
    _playAction(nullptr),
    _playing(true),
    _frame(0)
{
}

GLoader::~GLoader()
{
    FGUI_DELETE(_playAction);
    FGUI_DELETE(_content);
    FGUI_DELETE(_content2);
}

void GLoader::handleInit()
{
    _content = FUISprite::create();
    _displayObject = _content;

}

void GLoader::setURL(const std::string& value)
{
    if (_url.compare(value) == 0)
        return;

    _url = value;
    loadContent();
    updateGear(7);
}

void GLoader::setAlign(AlignType value)
{
    if (_align != value)
    {
        _align = value;
        updateLayout();
    }
}

void GLoader::setVerticalAlign(VertAlignType value)
{
    if (_verticalAlign != value)
    {
        _verticalAlign = value;
        updateLayout();
    }
}

void GLoader::setAutoSize(bool value)
{
    if (_autoSize != value)
    {
        _autoSize = value;
        updateLayout();
    }
}

void GLoader::setFill(LoaderFillType value)
{
    if (_fill != value)
    {
        _fill = value;
        updateLayout();
    }
}

void GLoader::setShrinkOnly(bool value)
{
    if (_shrinkOnly != value)
    {
        _shrinkOnly = value;
        updateLayout();
    }
}

Vector2 GLoader::getContentSize()
{
    return _content->get_rect().size;
}

Color GLoader::getColor() const
{
    if (!_content) return Color(1, 1, 1, 1);
    return _content->getColor();
}

void GLoader::setColor(const Color& value)
{
    _content->setColor(value);
}

void GLoader::setPlaying(bool value)
{
    if (_playing != value)
    {
        _playing = value;
        updateGear(5);
    }
}

int GLoader::getFrame() const
{
    return _frame;
}

void GLoader::setFrame(int value)
{
    if (_frame != value)
    {
        _frame = value;
        if (_playAction)
            _playAction->setFrame(value);
        updateGear(5);
    }
}

FillMethod GLoader::getFillMethod() const
{
    return (FillMethod)0;
}

void GLoader::setFillMethod(FillMethod value)
{
    // stub
}

FillOrigin GLoader::getFillOrigin() const
{
    return (FillOrigin)0;
}

void GLoader::setFillOrigin(FillOrigin value)
{
    // stub
}

bool GLoader::isFillClockwise() const
{
    return false;
}

void GLoader::setFillClockwise(bool value)
{
    // stub
}

float GLoader::getFillAmount() const
{
    return 0;
}

void GLoader::setFillAmount(float value)
{
    // stub
}

void GLoader::loadContent()
{
    clearContent();

    if (_url.length() == 0)
        return;

    if (_url.compare(0, 5, "ui://") == 0)
        loadFromPackage();
    else
    {
        _contentStatus = 3;
        loadExternal();
    }
}

void GLoader::loadFromPackage()
{
    _contentItem = UIPackage::getItemByURL(_url);

    if (_contentItem != nullptr)
    {
        _contentItem = _contentItem->getBranch();
        _sourceSize.width = (float)_contentItem->width;
        _sourceSize.height = (float)_contentItem->height;
        _contentItem = _contentItem->getHighResolution();
        _contentItem->load();

        if (_contentItem->type == PackageItemType::IMAGE)
        {
            _contentStatus = 1;
            _content->setTexture(_contentItem->texture);
            if (_contentItem->hasScale9Grid)
                _content->setScale9Grid(_contentItem->scale9Grid);
            updateLayout();
        }
        else if (_contentItem->type == PackageItemType::MOVIECLIP)
        {
            _contentStatus = 2;
            if (_playAction == nullptr)
            {
                _playAction = ActionMovieClip::create(_contentItem->movieclip, _contentItem->repeatDelay);
            }
            else
                _playAction->setAnimation(_contentItem->movieclip, _contentItem->repeatDelay);
            if (_playing)
            {
                // stub: runAction
            }
            else
                _playAction->setFrame(_frame);

            updateLayout();
        }
        else if (_contentItem->type == PackageItemType::COMPONENT)
        {
            GObject* obj = UIPackage::createObjectFromURL(_url);
            if (obj == nullptr)
                setErrorState();
            else if (dynamic_cast<GComponent*>(obj) == nullptr)
            {
                setErrorState();
            }
            else
            {
                _content2 = obj->as<GComponent>();
                _content2->addEventListener(UIEventType::SizeChange, [this](EventContext*) {
                    if (!_updatingLayout)
                        updateLayout();
                });
                _displayObject->add_child(_content2->displayObject());
                updateLayout();
            }
        }
        else
        {
            if (_autoSize)
                setSize((float)_contentItem->width, (float)_contentItem->height);

            setErrorState();
        }
    }
    else
        setErrorState();
}

void GLoader::loadExternal()
{
    String path = ResourceUID::ensure_path(GObject::toGodotStr(_url));
    Ref<Texture2D> tex2d = ResourceLoader::load(path);
    if (tex2d.is_valid())
    {
        ImageFrame* sf = new ImageFrame();
        sf->texture = tex2d;
        sf->region = Rect2(Vector2(), tex2d->get_size());
        onExternalLoadSuccess(sf);
    }
    else
        onExternalLoadFailed();
}

void GLoader::freeExternal(ImageFrame* spriteFrame)
{
    FGUI_DELETE(spriteFrame);
}

void GLoader::onExternalLoadSuccess(ImageFrame* spriteFrame)
{
    _contentStatus = 4;
    _content->setTexture(spriteFrame->texture);
    _sourceSize = spriteFrame->region.size;
    updateLayout();
}

void GLoader::onExternalLoadFailed()
{
    setErrorState();
}

void GLoader::clearContent()
{
    clearErrorState();

    if (_contentStatus == 2)
    {
        _playAction->setAnimation(nullptr);
    }

    if (_content2 != nullptr)
    {
        _displayObject->remove_child(_content2->displayObject());
        _content2 = nullptr;
    }
    _content->clearContent();

    _contentItem = nullptr;
    _contentStatus = 0;
}

void GLoader::updateLayout()
{
    if (_content2 == nullptr && _contentStatus == 0)
    {
        if (_autoSize)
        {
            _updatingLayout = true;
            setSize(50, 30);
            _updatingLayout = false;
        }
        return;
    }

    Vector2 contentSize = _sourceSize;

    if (_autoSize)
    {
        _updatingLayout = true;
        if (contentSize.width == 0)
            contentSize.width = 50;
        if (contentSize.height == 0)
            contentSize.height = 30;
        setSize(contentSize.width, contentSize.height);
        _updatingLayout = false;

        if (_size == contentSize)
        {
            if (_content2 != nullptr)
            {
                ((Node2D*)_content2->displayObject())->set_scale(Vector2(1, 1));
                ((Node2D*)_content2->displayObject())->set_position(Vector2(0, _size.height));
            }
            else
            {
                _content->set_scale(Vector2(1, 1));
                _content->set_position(Vector2(0, 0));
            }
            return;
        }
    }

    float sx = 1, sy = 1;
    if (_fill != LoaderFillType::NONE && _sourceSize.width != 0 && _sourceSize.height != 0)
    {
        sx = _size.width / _sourceSize.width;
        sy = _size.height / _sourceSize.height;

        if (sx != 1 || sy != 1)
        {
            if (_fill == LoaderFillType::SCALE_MATCH_HEIGHT)
                sx = sy;
            else if (_fill == LoaderFillType::SCALE_MATCH_WIDTH)
                sy = sx;
            else if (_fill == LoaderFillType::SCALE)
            {
                if (sx > sy)
                    sx = sy;
                else
                    sy = sx;
            }
            else if (_fill == LoaderFillType::SCALE_NO_BORDER)
            {
                if (sx > sy)
                    sy = sx;
                else
                    sx = sy;
            }

            if (_shrinkOnly)
            {
                if (sx > 1)
                    sx = 1;
                if (sy > 1)
                    sy = 1;
            }
            contentSize.width = floor(_sourceSize.width * sx);
            contentSize.height = floor(_sourceSize.height * sy);
        }
    }

    if (_content2 != nullptr)
    {
        ((Node2D*)_content2->displayObject())->set_scale(Vector2(sx, sy));
    }
    else
    {
        if (_contentItem != nullptr)
        {
            if (_contentItem->hasScale9Grid)
            {
                _content->set_scale(Vector2(1, 1));
                _content->set_content_size(contentSize);
            }
            else if (_contentItem->scaleByTile)
            {
                _content->set_scale(Vector2(1, 1));
                _content->set_content_size(_sourceSize);
            }
            else
            {
                _content->set_content_size(_sourceSize);
                _content->set_scale(Vector2(sx, sy));
            }
        }
        else
        {
            _content->set_content_size(_sourceSize);
            _content->set_scale(Vector2(sx, sy));
        }
    }

    float nx;
    float ny;
    if (_align == AlignType::CENTER)
        nx = floor((_size.width - contentSize.width) / 2);
    else if (_align == AlignType::RIGHT)
        nx = floor(_size.width - contentSize.width);
    else
        nx = 0;

    if (_content2 != nullptr)
    {
        if (_verticalAlign == VertAlignType::CENTER)
            ny = floor(-contentSize.height - (_size.height - contentSize.height) / 2);
        else if (_verticalAlign == VertAlignType::BOTTOM)
            ny = -contentSize.height;
        else
            ny = -_size.height;

        ((Node2D*)_content2->displayObject())->set_position(Vector2(nx, ny));
    }
    else
    {
        if (_verticalAlign == VertAlignType::CENTER)
            ny = floor((_size.height - contentSize.height) / 2);
        else if (_verticalAlign == VertAlignType::BOTTOM)
            ny = 0;
        else
            ny = _size.height - contentSize.height;

        _content->set_position(Vector2(nx, ny));
    }
}

void GLoader::setErrorState()
{
}

void GLoader::clearErrorState()
{
}

void GLoader::handleSizeChanged()
{
    GObject::handleSizeChanged();

    if (!_updatingLayout)
        updateLayout();
}

void GLoader::handleGrayedChanged()
{
    GObject::handleGrayedChanged();

    _content->setGrayed(_finalGrayed);
    if (_content2 != nullptr)
        _content2->setGrayed(_finalGrayed);
}

Variant GLoader::getProp(ObjectPropID propId)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        return Variant((int)ToolSet::colorToInt(getColor()));
    case ObjectPropID::Playing:
        return Variant(isPlaying());
    case ObjectPropID::Frame:
        return Variant(getFrame());
    case ObjectPropID::TimeScale:
        if (_playAction)
            return Variant(_playAction->getTimeScale());
        else
            return Variant(1.0f);
    default:
        return GObject::getProp(propId);
    }
}

void GLoader::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        setColor(ToolSet::intToColor((unsigned int)(int)value));
        break;
    case ObjectPropID::Playing:
        setPlaying((bool)value);
        break;
    case ObjectPropID::Frame:
        setFrame((int)value);
        break;
    case ObjectPropID::TimeScale:
        if (_playAction)
            _playAction->setTimeScale((float)value);
        break;
    case ObjectPropID::DeltaTime:
        if (_playAction)
            _playAction->advance((float)value);
        break;
    default:
        GObject::setProp(propId, value);
        break;
    }
}

void GLoader::setup_beforeAdd(ByteBuffer* buffer, int beginPos)
{
    GObject::setup_beforeAdd(buffer, beginPos);

    buffer->seek(beginPos, 5);

    _url = buffer->readS();
    _align = (AlignType)buffer->readByte();
    _verticalAlign = (VertAlignType)buffer->readByte();
    _fill = (LoaderFillType)buffer->readByte();
    _shrinkOnly = buffer->readBool();
    _autoSize = buffer->readBool();
    buffer->readBool(); //_showErrorSign
    _playing = buffer->readBool();
    _frame = buffer->readInt();

    if (buffer->readBool())
        setColor(buffer->readColor());
    int fillMethod = buffer->readByte();
    if (fillMethod != 0)
    {
        buffer->readByte();
        buffer->readBool();
        buffer->readFloat();
    }

    if (_url.length() > 0)
        loadContent();
}

GObject* GLoader::hitTest(const Vector2 & worldPoint, const Camera2D * camera)
{
    if (!_touchable || !((CanvasItem*)_displayObject)->is_visible() || !_displayObject->get_parent())
        return nullptr;

    if (_content2 != nullptr)
    {
        GObject* obj = _content2->hitTest(worldPoint, camera);
        if (obj != nullptr)
            return obj;
    }

    Rect2 rect(Vector2(), _size);
    if (rect.has_point(((CanvasItem*)_displayObject)->get_global_transform_with_canvas().affine_inverse().xform(worldPoint)))
        return this;
    else
        return nullptr;
}

void GLoader::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setURL", "url"), &GLoader::gd_setURL);
    ClassDB::bind_method(D_METHOD("getURL"), &GLoader::gd_getURL);

    ClassDB::bind_method(D_METHOD("setAlign", "align"), &GLoader::gd_setAlign);
    ClassDB::bind_method(D_METHOD("getAlign"), &GLoader::gd_getAlign);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "align"), "setAlign", "getAlign");

    ClassDB::bind_method(D_METHOD("setVerticalAlign", "align"), &GLoader::gd_setVerticalAlign);
    ClassDB::bind_method(D_METHOD("getVerticalAlign"), &GLoader::gd_getVerticalAlign);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "verticalAlign"), "setVerticalAlign", "getVerticalAlign");

    ClassDB::bind_method(D_METHOD("setAutoSize", "value"), &GLoader::setAutoSize);
    ClassDB::bind_method(D_METHOD("getAutoSize"), &GLoader::getAutoSize);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autoSize"), "setAutoSize", "getAutoSize");

    ClassDB::bind_method(D_METHOD("setFillType", "fill"), &GLoader::gd_setFillType);
    ClassDB::bind_method(D_METHOD("getFillType"), &GLoader::gd_getFillType);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fill"), "setFillType", "getFillType");

    ClassDB::bind_method(D_METHOD("setShrinkOnly", "value"), &GLoader::setShrinkOnly);
    ClassDB::bind_method(D_METHOD("isShrinkOnly"), &GLoader::isShrinkOnly);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "shrinkOnly"), "setShrinkOnly", "isShrinkOnly");

    ClassDB::bind_method(D_METHOD("setPlaying", "value"), &GLoader::setPlaying);
    ClassDB::bind_method(D_METHOD("isPlaying"), &GLoader::isPlaying);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playing"), "setPlaying", "isPlaying");

    ClassDB::bind_method(D_METHOD("setFrame", "frame"), &GLoader::setFrame);
    ClassDB::bind_method(D_METHOD("getFrame"), &GLoader::getFrame);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "frame"), "setFrame", "getFrame");

    ClassDB::bind_method(D_METHOD("setFillMethod", "method"), &GLoader::gd_setFillMethod);
    ClassDB::bind_method(D_METHOD("getFillMethod"), &GLoader::gd_getFillMethod);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fillMethod"), "setFillMethod", "getFillMethod");

    ClassDB::bind_method(D_METHOD("setFillOrigin", "origin"), &GLoader::gd_setFillOrigin);
    ClassDB::bind_method(D_METHOD("getFillOrigin"), &GLoader::gd_getFillOrigin);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fillOrigin"), "setFillOrigin", "getFillOrigin");

    ClassDB::bind_method(D_METHOD("setFillAmount", "amount"), &GLoader::setFillAmount);
    ClassDB::bind_method(D_METHOD("getFillAmount"), &GLoader::getFillAmount);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fillAmount", PROPERTY_HINT_RANGE, "0,1,0.01"), "setFillAmount", "getFillAmount");

    ClassDB::bind_method(D_METHOD("getComponent"), &GLoader::getComponent);

    ClassDB::bind_method(D_METHOD("setColor", "color"), &GLoader::setColor);
    ClassDB::bind_method(D_METHOD("getColor"), &GLoader::getColor);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "imageColor"), "setColor", "getColor");
}

void GLoader::gd_setURL(const String& value) { setURL(value.utf8().get_data()); }
String GLoader::gd_getURL() const { return String(getURL().c_str()); }

NS_FGUI_END


