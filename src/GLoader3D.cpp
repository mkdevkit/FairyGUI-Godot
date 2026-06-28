#include "GLoader3D.h"
#include "PackageItem.h"
#include "UIPackage.h"
#include "display/FUISprite.h"
#include "utils/ToolSet.h"

#ifndef SPINE_GODOT_DISABLED
#include "SpineSprite.h"
#include "SpineSkeleton.h"
#include "SpineAnimationState.h"
#include "SpineSkeletonDataResource.h"
#include "SpineSkeletonFileResource.h"
#include "SpineAtlasResource.h"
#include "SpineTrackEntry.h"
#endif

#include "core/io/file_access.h"

NS_FGUI_BEGIN

GLoader3D::GLoader3D()
    : _align(AlignType::CENTER)
    , _verticalAlign(VertAlignType::CENTER)
    , _fillMethod(FillMethod::None)
    , _fillOrigin(0)
    , _fillAmount(0)
    , _fillClockwise(false)
    , _shrinkOnly(false)
    , _autoPlay(false)
    , _playing(false)
    , _loop(false)
    , _updatingLayout(false)
    , _contentItem(nullptr)
    , _spineSprite(nullptr)
{
}

GLoader3D::~GLoader3D()
{
}

void GLoader3D::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setURL", "url"), &GLoader3D::gd_setURL);
    ClassDB::bind_method(D_METHOD("getURL"), &GLoader3D::gd_getURL);

    // Icon is bound in GObject

    ClassDB::bind_method(D_METHOD("setAlign", "align"), &GLoader3D::gd_setAlign);
    ClassDB::bind_method(D_METHOD("getAlign"), &GLoader3D::gd_getAlign);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "align"), "setAlign", "getAlign");

    ClassDB::bind_method(D_METHOD("setVerticalAlign", "align"), &GLoader3D::gd_setVerticalAlign);
    ClassDB::bind_method(D_METHOD("getVerticalAlign"), &GLoader3D::gd_getVerticalAlign);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "verticalAlign"), "setVerticalAlign", "getVerticalAlign");

    ClassDB::bind_method(D_METHOD("setFillMethod", "method"), &GLoader3D::gd_setFillMethod);
    ClassDB::bind_method(D_METHOD("getFillMethod"), &GLoader3D::gd_getFillMethod);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fillMethod"), "setFillMethod", "getFillMethod");

    ClassDB::bind_method(D_METHOD("setFillOrigin", "origin"), &GLoader3D::setFillOrigin);
    ClassDB::bind_method(D_METHOD("getFillOrigin"), &GLoader3D::getFillOrigin);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fillOrigin"), "setFillOrigin", "getFillOrigin");

    ClassDB::bind_method(D_METHOD("setFillAmount", "amount"), &GLoader3D::setFillAmount);
    ClassDB::bind_method(D_METHOD("getFillAmount"), &GLoader3D::getFillAmount);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fillAmount", PROPERTY_HINT_RANGE, "0,1,0.01"), "setFillAmount", "getFillAmount");

    ClassDB::bind_method(D_METHOD("setFillClockwise", "value"), &GLoader3D::setFillClockwise);
    ClassDB::bind_method(D_METHOD("isFillClockwise"), &GLoader3D::isFillClockwise);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fillClockwise"), "setFillClockwise", "isFillClockwise");

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
    ClassDB::bind_method(D_METHOD("setAutoPlay", "value"), &GLoader3D::setAutoPlay);

    ClassDB::bind_method(D_METHOD("play", "name"), &GLoader3D::gd_play);
    ClassDB::bind_method(D_METHOD("stop"), &GLoader3D::stop);
    ClassDB::bind_method(D_METHOD("dispose"), &GLoader3D::dispose);

    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "None", static_cast<GDExtensionInt>(FillMethod::None));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Horizontal", static_cast<GDExtensionInt>(FillMethod::Horizontal));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Vertical", static_cast<GDExtensionInt>(FillMethod::Vertical));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Radial90", static_cast<GDExtensionInt>(FillMethod::Radial90));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Radial180", static_cast<GDExtensionInt>(FillMethod::Radial180));
    ClassDB::bind_integer_constant(get_class_static(), "FillMethod", "Radial360", static_cast<GDExtensionInt>(FillMethod::Radial360));
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

void GLoader3D::setIcon(const std::string& value)
{
    _icon = value;
}

void GLoader3D::setAlign(AlignType value)
{
    if (_align != value)
    {
        _align = value;
        updateSkeletonScale();
    }
}

void GLoader3D::setVerticalAlign(VertAlignType value)
{
    if (_verticalAlign != value)
    {
        _verticalAlign = value;
        updateSkeletonScale();
    }
}

void GLoader3D::setFillMethod(FillMethod value)
{
    _fillMethod = value;
}

void GLoader3D::setFillOrigin(int value)
{
    _fillOrigin = value;
}

void GLoader3D::setFillAmount(float value)
{
    _fillAmount = value;
}

void GLoader3D::setFillClockwise(bool value)
{
    _fillClockwise = value;
}

void GLoader3D::setShrinkOnly(bool value)
{
    _shrinkOnly = value;
}

void GLoader3D::setAutoPlay(bool value)
{
    if (_autoPlay != value)
    {
        _autoPlay = value;
        if (_autoPlay && _spineSprite)
            play();
    }
}

void GLoader3D::setAnimationName(const std::string& value)
{
    _animationName = value;
}

void GLoader3D::setSkinName(const std::string& value)
{
    _skinName = value;
}

void GLoader3D::setLoop(bool value)
{
    _loop = value;
}

void GLoader3D::play(const std::string& name)
{
#ifndef SPINE_GODOT_DISABLED
    if (!_spineSprite)
        return;

    Ref<SpineAnimationState> state = _spineSprite->get_animation_state();
    if (!state.is_valid())
        return;

    std::string animName = name.empty() ? _animationName : name;
    if (animName.empty())
    {
        state->clear_tracks();
    }
    else
    {
        state->set_animation(animName.c_str(), _loop, 0);
    }

    _playing = true;
#endif
}

void GLoader3D::stop()
{
#ifndef SPINE_GODOT_DISABLED
    if (!_spineSprite)
        return;

    Ref<SpineAnimationState> state = _spineSprite->get_animation_state();
    if (state.is_valid())
        state->clear_tracks();
#endif

    _playing = false;
}

void GLoader3D::handleSizeChanged()
{
    GObject::handleSizeChanged();

    if (_updatingLayout)
        return;

    updateSkeletonScale();
}

void GLoader3D::handleGrayedChanged()
{
    // Grayed effect for spine content
}

void GLoader3D::_ready()
{
    GObject::_ready();

    if (!_url.empty())
        loadContent();
    else if (!_icon.empty())
        loadFromPackage();
}

void GLoader3D::_process(double delta)
{
    GObject::_process(delta);

#ifndef SPINE_GODOT_DISABLED
    if (_spineSprite)
        updateSpine();
#endif
}

void GLoader3D::loadFromPackage()
{
    clearContent();

    if (_icon.empty())
        return;

    _contentItem = UIPackage::getItemByURL(_icon);
    if (_contentItem != nullptr)
    {
        _contentItem = _contentItem->getBranch();
        loadContent();
    }
    else
    {
        print_line("FairyGUI: resource not found: ", _icon.c_str());
    }
}

void GLoader3D::loadContent()
{
    clearContent();

    if (_url.empty() && _icon.empty())
        return;

    if (!_url.empty())
    {
        _contentItem = UIPackage::getItemByURL(_url);
        if (_contentItem == nullptr)
        {
            print_line("FairyGUI: resource not found: ", _url.c_str());
            return;
        }
    }

    if (_contentItem == nullptr)
    {
        print_line("FairyGUI: spine load failed, no content item");
        return;
    }

#ifdef SPINE_GODOT_DISABLED
    print_line("FairyGUI: spine support is disabled");
    return;
#else
    // Load spine skeleton from package
    std::string skelFile = _contentItem->file;

    // Determine atlas file path
    std::string atlasFile;
    size_t dotPos = skelFile.find_last_of('.');
    if (dotPos != std::string::npos)
        atlasFile = skelFile.substr(0, dotPos) + ".atlas";
    else
        atlasFile = skelFile + ".atlas";

    // Create SpineSkeletonFileResource and load from file
    Ref<SpineSkeletonFileResource> skeletonFileResource;
    skeletonFileResource.instantiate();
    Error skelErr = skeletonFileResource->load_from_file(skelFile.c_str());
    if (skelErr != Error::OK)
    {
        print_line("FairyGUI: failed to load skeleton file: ", skelFile.c_str());
        return;
    }

    // Create SpineAtlasResource and load from atlas file
    Ref<SpineAtlasResource> atlasResource;
    atlasResource.instantiate();
    Error atlasErr = atlasResource->load_from_atlas_file(atlasFile.c_str());
    if (atlasErr != Error::OK)
    {
        print_line("FairyGUI: cannot load atlas file: ", atlasFile.c_str());
        return;
    }

    // Create SpineSkeletonDataResource
    Ref<SpineSkeletonDataResource> skeletonDataResource;
    skeletonDataResource.instantiate();
    skeletonDataResource->set_atlas_res(atlasResource);
    skeletonDataResource->set_skeleton_file_res(skeletonFileResource);

    // Create SpineSprite as child
    _spineSprite = memnew(SpineSprite);
    addChild(_spineSprite);
    _spineSprite->set_skeleton_data_res(skeletonDataResource);

    // Set skin
    if (!_skinName.empty())
    {
        Ref<SpineSkeleton> skeleton = _spineSprite->get_skeleton();
        if (skeleton.is_valid())
        {
            skeleton->set_skin_by_name(_skinName.c_str());
            skeleton->set_slots_to_setup_pose();
        }
    }

    updateSkeletonScale();

    // Handle skeleton anchor if present
    if (_contentItem->hasSkeletonAnchor)
    {
        Vector2 anchor = _contentItem->skeletonAnchor;
        _spineSprite->set_position(Vector2(anchor.x, -anchor.y));
    }

    // Auto-play
    if (_autoPlay)
        play();
#endif
}

void GLoader3D::clearContent()
{
    stop();

#ifndef SPINE_GODOT_DISABLED
    if (_spineSprite)
    {
        _spineSprite->get_parent()->remove_child(_spineSprite);
        memdelete(_spineSprite);
        _spineSprite = nullptr;
    }
#endif

    _playing = false;
    _contentItem = nullptr;
}

void GLoader3D::updateSkeletonScale()
{
#ifndef SPINE_GODOT_DISABLED
    if (!_spineSprite)
        return;

    float contentWidth = 0;
    float contentHeight = 0;

    if (_contentItem != nullptr)
    {
        contentWidth = (float)_contentItem->width;
        contentHeight = (float)_contentItem->height;
    }

    if (_shrinkOnly)
    {
        // Only shrink, don't scale up
        // Default spine skeleton scale is 1
        _spineSprite->set_scale(Vector2(1, 1));
        return;
    }

    if (contentWidth == 0 || contentHeight == 0)
        return;

    float viewWidth = _size.width;
    float viewHeight = _size.height;

    if (viewWidth == 0 || viewHeight == 0)
        return;

    float scaleX = viewWidth / contentWidth;
    float scaleY = viewHeight / contentHeight;

    if (_fillMethod == FillMethod::None)
    {
        // Use the smaller scale to fit the content
        float scale = MIN(scaleX, scaleY);

        // Apply alignment
        float offsetX = 0;
        float offsetY = 0;

        float actualWidth = contentWidth * scale;
        float actualHeight = contentHeight * scale;

        if (_align == AlignType::CENTER)
            offsetX = (viewWidth - actualWidth) / 2;
        else if (_align == AlignType::RIGHT)
            offsetX = viewWidth - actualWidth;

        if (_verticalAlign == VertAlignType::CENTER)
            offsetY = (viewHeight - actualHeight) / 2;
        else if (_verticalAlign == VertAlignType::BOTTOM)
            offsetY = viewHeight - actualHeight;

        _spineSprite->set_scale(Vector2(scale, scale));
        _spineSprite->set_position(Vector2(offsetX, offsetY));
    }
    else
    {
        // Stretch to fill
        _spineSprite->set_scale(Vector2(scaleX, scaleY));
        _spineSprite->set_position(Vector2());
    }
#endif
}

void GLoader3D::updateSpine()
{
#ifndef SPINE_GODOT_DISABLED
    if (!_spineSprite || !_playing)
        return;

    _spineSprite->update_skeleton(0); // delta is handled by _process
#endif
}

void GLoader3D::gd_setURL(const String& value) { setURL(value.utf8().get_data()); }
String GLoader3D::gd_getURL() const { return String(getURL().c_str()); }
void GLoader3D::gd_setIcon(const String& value) { setIcon(value.utf8().get_data()); }
String GLoader3D::gd_getIcon() const { return String(getIcon().c_str()); }
void GLoader3D::gd_setAnimationName(const String& value) { setAnimationName(value.utf8().get_data()); }
String GLoader3D::gd_getPlayingAnimationName() const { return String(getPlayingAnimationName().c_str()); }
void GLoader3D::gd_setSkinName(const String& value) { setSkinName(value.utf8().get_data()); }
String GLoader3D::gd_getSkinName() const { return String(getSkinName().c_str()); }
void GLoader3D::gd_play(const String& name) { play(name.utf8().get_data()); }

void GLoader3D::gd_setAlign(int value) { setAlign(static_cast<AlignType>(value)); }
int GLoader3D::gd_getAlign() const { return static_cast<int>(getAlign()); }
void GLoader3D::gd_setVerticalAlign(int value) { setVerticalAlign(static_cast<VertAlignType>(value)); }
int GLoader3D::gd_getVerticalAlign() const { return static_cast<int>(getVerticalAlign()); }
void GLoader3D::gd_setFillMethod(int value) { setFillMethod(static_cast<FillMethod>(value)); }
int GLoader3D::gd_getFillMethod() const { return static_cast<int>(getFillMethod()); }

NS_FGUI_END
