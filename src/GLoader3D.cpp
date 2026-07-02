#include "GLoader3D.h"
#include "PackageItem.h"
#include "UIPackage.h"
#include "display/FUIContainer.h"
#include "utils/ByteBuffer.h"
#include "utils/ToolSet.h"

#include "scene/2d/sprite_2d.h"
#include "scene/gui/control.h"

#ifndef SPINE_GODOT_DISABLED
#include "SpineSprite.h"
#include "SpineSkeleton.h"
#include "SpineAnimationState.h"
#include "SpineSkeletonDataResource.h"
#include "SpineSkeletonFileResource.h"
#include "SpineAtlasResource.h"
#include "SpineTrackEntry.h"
#endif

NS_FGUI_BEGIN

static Vector2 get_node_content_size(Node* node)
{
    if (!node)
        return Vector2();

    if (Control* ctrl = Object::cast_to<Control>(node))
        return ctrl->get_rect().size;
    if (Sprite2D* sprite = Object::cast_to<Sprite2D>(node))
        return sprite->get_rect().size;

    return Vector2();
}

GLoader3D::GLoader3D()
    : _align(AlignType::LEFT)
    , _verticalAlign(VertAlignType::TOP)
    , _autoSize(false)
    , _fill(LoaderFillType::NONE)
    , _shrinkOnly(false)
    , _updatingLayout(false)
    , _contentItem(nullptr)
    , _playing(true)
    , _frame(0)
    , _loop(false)
    , _color(Color(1, 1, 1, 1))
    , _container(nullptr)
    , _content(nullptr)
#ifndef SPINE_GODOT_DISABLED
    , _spineSprite(nullptr)
#endif
{
    _touchable = false;
}

GLoader3D::~GLoader3D()
{
}

void GLoader3D::handleInit()
{
    FUIContainer* root = FUIContainer::create();
    root->gOwner = this;
    _displayObject = root;

    _container = FUIContainer::create();
    _displayObject->add_child(_container);
}

void GLoader3D::dispose()
{
    clearContent();
    GObject::dispose();
}

void GLoader3D::setURL(const std::string& value)
{
    if (_url.compare(value) == 0)
        return;

    _url = value;
    loadContent();
    updateGear(7);
}

void GLoader3D::setAlign(AlignType value)
{
    if (_align != value)
    {
        _align = value;
        updateLayout();
    }
}

void GLoader3D::setVerticalAlign(VertAlignType value)
{
    if (_verticalAlign != value)
    {
        _verticalAlign = value;
        updateLayout();
    }
}

void GLoader3D::setAutoSize(bool value)
{
    if (_autoSize != value)
    {
        _autoSize = value;
        updateLayout();
    }
}

void GLoader3D::setFill(LoaderFillType value)
{
    if (_fill != value)
    {
        _fill = value;
        updateLayout();
    }
}

void GLoader3D::setShrinkOnly(bool value)
{
    if (_shrinkOnly != value)
    {
        _shrinkOnly = value;
        updateLayout();
    }
}

Color GLoader3D::getColor() const
{
    return _color;
}

void GLoader3D::setColor(const Color& value)
{
    _color = value;
#ifndef SPINE_GODOT_DISABLED
    if (_spineSprite)
    {
        Color c = value;
        if (_finalGrayed)
            c = Color(c.r * 0.5f, c.g * 0.5f, c.b * 0.5f, c.a);
        _spineSprite->set_self_modulate(c);
    }
#endif
}

void GLoader3D::handleGrayedChanged()
{
    GObject::handleGrayedChanged();
    setColor(_color);
}

void GLoader3D::setPlaying(bool value)
{
    if (_playing != value)
    {
        _playing = value;
        updateGear(5);
        onChangeSpine();
    }
}

void GLoader3D::setFrame(int value)
{
    if (_frame != value)
    {
        _frame = value;
        updateGear(5);
        onChangeSpine();
    }
}

void GLoader3D::setAnimationName(const std::string& value)
{
    _animationName = value;
    onChange();
}

void GLoader3D::setSkinName(const std::string& value)
{
    _skinName = value;
    onChange();
}

void GLoader3D::setLoop(bool value)
{
    _loop = value;
    onChange();
}

void GLoader3D::play(const std::string& animName)
{
#ifndef SPINE_GODOT_DISABLED
    if (!_spineSprite)
        return;

    Ref<SpineAnimationState> state = _spineSprite->get_animation_state();
    if (!state.is_valid())
        return;

    std::string animation = animName.empty() ? _animationName : animName;
    if (animation.empty())
        state->clear_track(0);
    else
        state->set_animation(animation.c_str(), _loop, 0);

    _playing = true;
    onChangeSpine();
#endif
}

void GLoader3D::stop()
{
#ifndef SPINE_GODOT_DISABLED
    if (_spineSprite)
    {
        Ref<SpineAnimationState> state = _spineSprite->get_animation_state();
        if (state.is_valid())
            state->clear_track(0);
    }
#endif
    _playing = false;
}

Node* GLoader3D::getContentNode() const
{
    if (_content)
        return _content;
#ifndef SPINE_GODOT_DISABLED
    if (_spineSprite)
        return _spineSprite;
#endif
    return nullptr;
}

Node* GLoader3D::getContent() const
{
    return getContentNode();
}

void GLoader3D::setContent(Node* value)
{
    setURL("");

    clearContent();
    _content = value;
    if (_content)
    {
        _container->add_child(_content);
        if (sourceSize.width == 0 || sourceSize.height == 0)
        {
            Vector2 contentSize = get_node_content_size(_content);
            if (contentSize.x > 0 && contentSize.y > 0)
            {
                sourceSize.width = contentSize.x;
                sourceSize.height = contentSize.y;
            }
        }
    }
    updateLayout();
}

void GLoader3D::onChange()
{
    onChangeSpine();
}

void GLoader3D::onChangeSpine()
{
#ifndef SPINE_GODOT_DISABLED
    if (!_spineSprite)
        return;

    Ref<SpineAnimationState> state = _spineSprite->get_animation_state();
    if (!state.is_valid())
        return;

    if (!_animationName.empty())
    {
        Ref<SpineTrackEntry> entry = state->set_animation(_animationName.c_str(), _loop, 0);
        if (entry.is_valid())
        {
            if (_playing)
                entry->set_time_scale(1);
            else
            {
                entry->set_time_scale(0);
                float start = entry->get_animation_start();
                float end = entry->get_animation_end();
                float duration = end - start;
                float t = start + duration * (_frame / 100.0f);
                entry->set_track_time(t);
            }
        }
    }
    else
    {
        state->clear_track(0);
    }

    if (!_skinName.empty())
    {
        Ref<SpineSkeleton> skeleton = _spineSprite->get_skeleton();
        if (skeleton.is_valid())
        {
            skeleton->set_skin_by_name(_skinName.c_str());
            skeleton->set_slots_to_setup_pose();
        }
    }
#endif
}

void GLoader3D::loadContent()
{
    clearContent();

    if (_url.empty())
        return;

    if (_url.compare(0, 5, "ui://") == 0)
        loadFromPackage();
}

void GLoader3D::loadFromPackage()
{
    _contentItem = UIPackage::getItemByURL(_url);
    if (_contentItem == nullptr)
    {
        setErrorState();
        return;
    }

    _contentItem = _contentItem->getBranch();
    sourceSize.width = _contentItem->width;
    sourceSize.height = _contentItem->height;
    _contentItem = _contentItem->getHighResolution();
    _contentItem->load();

    if (_contentItem->type == PackageItemType::SPINE)
    {
#ifndef SPINE_GODOT_DISABLED
        if (!loadSpineContent())
        {
            setErrorState();
            return;
        }
        clearErrorState();
        onChangeSpine();
        updateLayout();
#else
        print_line("FairyGUI: spine support is disabled (enable module_spine_godot)");
        setErrorState();
#endif
    }
    else
    {
        if (_autoSize)
            setSize(_contentItem->width, _contentItem->height);
        setErrorState();
    }
}

#ifndef SPINE_GODOT_DISABLED
bool GLoader3D::loadSpineContent()
{
    std::string skelFile = _contentItem->file;

    std::string atlasFile;
    size_t dotPos = skelFile.find_last_of('.');
    if (dotPos != std::string::npos)
        atlasFile = skelFile.substr(0, dotPos) + ".atlas";
    else
        atlasFile = skelFile + ".atlas";

    if (!ToolSet::isFileExist(atlasFile))
    {
        if (dotPos != std::string::npos)
            atlasFile = skelFile.substr(0, dotPos) + ".atlas.txt";
        else
            atlasFile = skelFile + ".atlas.txt";
    }

    Ref<SpineSkeletonFileResource> skeletonFileResource;
    skeletonFileResource.instantiate();
    if (skeletonFileResource->load_from_file(skelFile.c_str()) != Error::OK)
    {
        print_line("FairyGUI: failed to load skeleton file: ", skelFile.c_str());
        return false;
    }

    Ref<SpineAtlasResource> atlasResource;
    atlasResource.instantiate();
    if (atlasResource->load_from_atlas_file(atlasFile.c_str()) != Error::OK)
    {
        print_line("FairyGUI: cannot load atlas file: ", atlasFile.c_str());
        return false;
    }

    Ref<SpineSkeletonDataResource> skeletonDataResource;
    skeletonDataResource.instantiate();
    skeletonDataResource->set_atlas_res(atlasResource);
    skeletonDataResource->set_skeleton_file_res(skeletonFileResource);

    _spineSprite = memnew(SpineSprite);
    _container->add_child(_spineSprite);
    _spineSprite->set_skeleton_data_res(skeletonDataResource);

    if (_contentItem->hasSkeletonAnchor)
        _spineSprite->set_position(_contentItem->skeletonAnchor);

    setColor(_color);

    return true;
}
#endif

void GLoader3D::clearContent()
{
    stop();

#ifndef SPINE_GODOT_DISABLED
    if (_spineSprite)
    {
        _container->remove_child(_spineSprite);
        memdelete(_spineSprite);
        _spineSprite = nullptr;
    }
#endif

    if (_content)
    {
        _container->remove_child(_content);
        _content = nullptr;
    }

    _contentItem = nullptr;
}

void GLoader3D::applyPivotOffset()
{
    if (!_updatingLayout)
        updateLayout();
}

void GLoader3D::updateLayout()
{
    Node* contentNode = getContentNode();
    if (!contentNode)
    {
        if (_autoSize)
        {
            _updatingLayout = true;
            setSize(50, 30);
            _updatingLayout = false;
        }
        return;
    }

    Vector2 contentSize = sourceSize;

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
            _container->set_scale(Vector2(1, 1));
            _container->set_position(Vector2());
            return;
        }
    }

    float sx = 1, sy = 1;
    if (_fill != LoaderFillType::NONE && sourceSize.width != 0 && sourceSize.height != 0)
    {
        sx = _size.width / sourceSize.width;
        sy = _size.height / sourceSize.height;

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
            contentSize.width = floor(sourceSize.width * sx);
            contentSize.height = floor(sourceSize.height * sy);
        }
    }

    _container->set_scale(Vector2(sx, sy));

    float nx;
    float ny;
    if (_align == AlignType::CENTER)
        nx = floor((_size.width - contentSize.width) / 2);
    else if (_align == AlignType::RIGHT)
        nx = floor(_size.width - contentSize.width);
    else
        nx = 0;

    if (_verticalAlign == VertAlignType::CENTER)
        ny = floor((_size.height - contentSize.height) / 2);
    else if (_verticalAlign == VertAlignType::BOTTOM)
        ny = _size.height - contentSize.height;
    else
        ny = 0;

    _container->set_position(Vector2(nx, ny));
}

void GLoader3D::setErrorState()
{
}

void GLoader3D::clearErrorState()
{
}

void GLoader3D::handleSizeChanged()
{
    GObject::handleSizeChanged();

    if (!_updatingLayout)
        updateLayout();
}

void GLoader3D::_enter_tree()
{
    GObject::_enter_tree();

    if (!_updatingLayout)
        updateLayout();
}

Variant GLoader3D::getProp(ObjectPropID propId)
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
        return Variant(1);
    default:
        return GObject::getProp(propId);
    }
}

void GLoader3D::setProp(ObjectPropID propId, const Variant& value)
{
    switch (propId)
    {
    case ObjectPropID::Color:
        setColor(ToolSet::intToColor(value.operator int()));
        break;
    case ObjectPropID::Playing:
        setPlaying(value);
        break;
    case ObjectPropID::Frame:
        setFrame(value);
        break;
    case ObjectPropID::TimeScale:
    case ObjectPropID::DeltaTime:
        break;
    default:
        GObject::setProp(propId, value);
        break;
    }
}

void GLoader3D::setup_beforeAdd(ByteBuffer* buffer, int beginPos)
{
    GObject::setup_beforeAdd(buffer, beginPos);

    buffer->seek(beginPos, 5);

    _url = buffer->readS();
    _align = (AlignType)buffer->readByte();
    _verticalAlign = (VertAlignType)buffer->readByte();
    _fill = (LoaderFillType)buffer->readByte();
    _shrinkOnly = buffer->readBool();
    _autoSize = buffer->readBool();
    _animationName = buffer->readS();
    _skinName = buffer->readS();
    _playing = buffer->readBool();
    _frame = buffer->readInt();
    _loop = buffer->readBool();

    if (buffer->readBool())
        setColor(buffer->readColor());

    if (!_url.empty())
        loadContent();
}

GObject* GLoader3D::hitTest(const Vector2& worldPoint, const Camera2D* camera)
{
    if (!_touchable || !_displayObject || !((CanvasItem*)_displayObject)->is_visible() || !_displayObject->get_parent())
        return nullptr;

    Rect2 rect(Vector2(), _size);
    if (rect.has_point(globalToLocal(worldPoint)))
        return this;
    return nullptr;
}

void GLoader3D::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setURL", "url"), &GLoader3D::gd_setURL);
    ClassDB::bind_method(D_METHOD("getURL"), &GLoader3D::gd_getURL);

    ClassDB::bind_method(D_METHOD("setAlign", "align"), &GLoader3D::gd_setAlign);
    ClassDB::bind_method(D_METHOD("getAlign"), &GLoader3D::gd_getAlign);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "align"), "setAlign", "getAlign");

    ClassDB::bind_method(D_METHOD("setVerticalAlign", "align"), &GLoader3D::gd_setVerticalAlign);
    ClassDB::bind_method(D_METHOD("getVerticalAlign"), &GLoader3D::gd_getVerticalAlign);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "verticalAlign"), "setVerticalAlign", "getVerticalAlign");

    ClassDB::bind_method(D_METHOD("setAutoSize", "value"), &GLoader3D::setAutoSize);
    ClassDB::bind_method(D_METHOD("getAutoSize"), &GLoader3D::getAutoSize);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autoSize"), "setAutoSize", "getAutoSize");

    ClassDB::bind_method(D_METHOD("setFillType", "fill"), &GLoader3D::gd_setFillType);
    ClassDB::bind_method(D_METHOD("getFillType"), &GLoader3D::gd_getFillType);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fill"), "setFillType", "getFillType");

    ClassDB::bind_method(D_METHOD("setShrinkOnly", "value"), &GLoader3D::setShrinkOnly);
    ClassDB::bind_method(D_METHOD("isShrinkOnly"), &GLoader3D::isShrinkOnly);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "shrinkOnly"), "setShrinkOnly", "isShrinkOnly");

    ClassDB::bind_method(D_METHOD("setAnimationName", "name"), &GLoader3D::gd_setAnimationName);
    ClassDB::bind_method(D_METHOD("getPlayingAnimationName"), &GLoader3D::gd_getPlayingAnimationName);

    ClassDB::bind_method(D_METHOD("setSkinName", "name"), &GLoader3D::gd_setSkinName);
    ClassDB::bind_method(D_METHOD("getSkinName"), &GLoader3D::gd_getSkinName);

    ClassDB::bind_method(D_METHOD("isPlaying"), &GLoader3D::isPlaying);
    ClassDB::bind_method(D_METHOD("isLoop"), &GLoader3D::isLoop);
    ClassDB::bind_method(D_METHOD("setLoop", "value"), &GLoader3D::setLoop);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "setLoop", "isLoop");

    ClassDB::bind_method(D_METHOD("setPlaying", "value"), &GLoader3D::setPlaying);
    ClassDB::bind_method(D_METHOD("setFrame", "frame"), &GLoader3D::setFrame);
    ClassDB::bind_method(D_METHOD("getFrame"), &GLoader3D::getFrame);

    ClassDB::bind_method(D_METHOD("play", "name"), &GLoader3D::gd_play);
    ClassDB::bind_method(D_METHOD("stop"), &GLoader3D::stop);
    ClassDB::bind_method(D_METHOD("dispose"), &GLoader3D::dispose);
    ClassDB::bind_method(D_METHOD("getContent"), &GLoader3D::getContent);
    ClassDB::bind_method(D_METHOD("setContent", "node"), &GLoader3D::setContent);

    ClassDB::bind_method(D_METHOD("setColor", "color"), &GLoader3D::setColor);
    ClassDB::bind_method(D_METHOD("getColor"), &GLoader3D::getColor);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "setColor", "getColor");
}

void GLoader3D::gd_setURL(const String& value) { setURL(value.utf8().get_data()); }
String GLoader3D::gd_getURL() const { return String(getURL().c_str()); }
void GLoader3D::gd_setAnimationName(const String& value) { setAnimationName(value.utf8().get_data()); }
String GLoader3D::gd_getPlayingAnimationName() const { return String(getAnimationName().c_str()); }
void GLoader3D::gd_setSkinName(const String& value) { setSkinName(value.utf8().get_data()); }
String GLoader3D::gd_getSkinName() const { return String(getSkinName().c_str()); }
void GLoader3D::gd_play(const String& animName) { play(animName.utf8().get_data()); }
void GLoader3D::gd_setAlign(int value) { setAlign(static_cast<AlignType>(value)); }
int GLoader3D::gd_getAlign() const { return static_cast<int>(getAlign()); }
void GLoader3D::gd_setVerticalAlign(int value) { setVerticalAlign(static_cast<VertAlignType>(value)); }
int GLoader3D::gd_getVerticalAlign() const { return static_cast<int>(getVerticalAlign()); }
void GLoader3D::gd_setFillType(int value) { setFill(static_cast<LoaderFillType>(value)); }
int GLoader3D::gd_getFillType() const { return static_cast<int>(getFill()); }

NS_FGUI_END
