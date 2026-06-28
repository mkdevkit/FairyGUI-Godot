# FairyGUI-Godot

> This is an experimental project under active development, based on the FairyGUI cocos2d-x version and being refactored with AI assistance.

FairyGUI runtime for Godot Engine, built-in C++ module.

> [中文文档](README.zh.md)

## Project Structure

```
modules/fairygui/
├── config.py                           # Module build configuration (can_build, doc_classes)
├── SCsub                               # SCons build script
├── register_types.h                    # Module registration header
├── register_types.cpp                  # Module registration (ClassDB registration)
├── src/                                # Core runtime
│   ├── FairyGUI.h                      # Umbrella include header
│   ├── FairyGUIMacros.h                # Namespace macros
│   ├── FieldTypes.h                    # All enums (ObjectType, FillMethod, etc.)
│   ├── godot_types.h                   # Godot engine type mapping (replaces godot-cpp)
│   ├── Margin.h/Margin.cpp             # Margin type
│   ├── UIConfig.h/UIConfig.cpp         # Global UI configuration
│   ├── PackageItem.h/PackageItem.cpp   # Package resource item
│   ├── UIPackage.h/UIPackage.cpp       # Package loader (reads .fui binary)
│   ├── Controller.h/Controller.cpp     # GController (page-based state)
│   ├── GObject.h/GObject.cpp           # Base GObject (inherits Node)
│   ├── GComponent.h/GComponent.cpp     # GComponent container
│   ├── UIObjectFactory.h/UIObjectFactory.cpp  # Widget factory
│   ├── GObjectPool.h/GObjectPool.cpp   # Object pooling
│   ├── RelationItem.h/RelationItem.cpp  # Single relation constraint
│   ├── Relations.h/Relations.cpp       # Relation collection
│   ├── ScrollPane.h/ScrollPane.cpp     # ScrollPane for GComponent
│   ├── Transition.h/Transition.cpp     # Transition animation system
│   ├── TranslationHelper.h/TranslationHelper.cpp  # String translation
│   ├── GRoot.h/GRoot.cpp               # GRoot (root canvas)
│   ├── Window.h/Window.cpp             # GWindow (modal/popup window)
│   ├── GPopupMenu.h/GPopupMenu.cpp     # PopupMenu
│   ├── DragDropManager.h/DragDropManager.cpp  # DragDropManager
│   ├── GImage.h/GImage.cpp             # GImage widget
│   ├── GGraph.h/GGraph.cpp             # GGraph widget (drawing primitives)
│   ├── GTextField.h/GTextField.cpp     # GTextField / GBasicTextField
│   ├── GRichTextField.h/GRichTextField.cpp   # GRichTextField widget
│   ├── GTextInput.h/GTextInput.cpp     # GTextInput widget
│   ├── GButton.h/GButton.cpp           # GButton widget
│   ├── GLabel.h/GLabel.cpp             # GLabel widget
│   ├── GComboBox.h/GComboBox.cpp       # GComboBox widget
│   ├── GProgressBar.h/GProgressBar.cpp # GProgressBar widget
│   ├── GSlider.h/GSlider.cpp           # GSlider widget
│   ├── GScrollBar.h/GScrollBar.cpp     # GScrollBar widget
│   ├── GList.h/GList.cpp               # GList widget
│   ├── GTree.h/GTree.cpp               # GTree widget
│   ├── GTreeNode.h/GTreeNode.cpp       # GTreeNode
│   ├── GLoader.h/GLoader.cpp           # GLoader widget
│   ├── GLoader3D.h/GLoader3D.cpp       # GLoader3D widget (Spine runtime)
│   ├── GMovieClip.h/GMovieClip.cpp     # GMovieClip widget
│   ├── GGroup.h/GGroup.cpp             # GGroup widget
│   ├── event/
│   │   ├── UIEventType.h               # Event type constants
│   │   ├── EventContext.h/EventContext.cpp    # Event context
│   │   ├── UIEventDispatcher.h/UIEventDispatcher.cpp  # Event dispatch system
│   │   ├── InputEvent.h/InputEvent.cpp  # Input event data
│   │   ├── InputProcessor.h/InputProcessor.cpp  # Input event processor
│   │   └── HitTest.h/HitTest.cpp       # Hit test interface + pixel hit test
│   ├── display/
│   │   ├── BitmapFont.h/BitmapFont.cpp  # Bitmap font rendering
│   │   ├── FUIContainer.h/FUIContainer.cpp  # Container node (clipping, stencil)
│   │   ├── FUIInput.h/FUIInput.cpp     # Input text control
│   │   ├── FUILabel.h/FUILabel.cpp     # Label rendering node
│   │   ├── FUIRichText.h/FUIRichText.cpp  # Rich text rendering node
│   │   ├── FUISprite.h/FUISprite.cpp   # Sprite rendering node
│   │   └── TextFormat.h/TextFormat.cpp  # Text formatting descriptor
│   ├── gears/
│   │   ├── GearBase.h/GearBase.cpp     # Gear base class
│   │   ├── GearAnimation.h/GearAnimation.cpp  # Animation gear
│   │   ├── GearColor.h/GearColor.cpp   # Color gear
│   │   ├── GearDisplay.h/GearDisplay.cpp  # Display visibility gear
│   │   ├── GearDisplay2.h/GearDisplay2.cpp # Display2 gear
│   │   ├── GearFontSize.h/GearFontSize.cpp  # Font size gear
│   │   ├── GearIcon.h/GearIcon.cpp     # Icon gear
│   │   ├── GearLook.h/GearLook.cpp     # Look gear
│   │   ├── GearSize.h/GearSize.cpp     # Size gear
│   │   ├── GearText.h/GearText.cpp     # Text gear
│   │   └── GearXY.h/GearXY.cpp         # Position gear
│   ├── tween/
│   │   ├── EaseType.h                  # Easing type enum
│   │   ├── EaseManager.h/EaseManager.cpp  # Easing function manager
│   │   ├── GTween.h/GTween.cpp         # GTween manager (static)
│   │   ├── GTweener.h/GTweener.cpp     # Individual tweener
│   │   ├── GPath.h/GPath.cpp           # Path animation
│   │   ├── TweenManager.h/TweenManager.cpp  # Tween instance manager
│   │   ├── TweenPropType.h/TweenPropType.cpp  # Tween property types
│   │   └── TweenValue.h/TweenValue.cpp  # Tween value wrapper
│   ├── controller_action/
│   │   ├── ControllerAction.h/ControllerAction.cpp  # Controller action base
│   │   ├── ChangePageAction.h/ChangePageAction.cpp  # Page change action
│   │   └── PlayTransitionAction.h/PlayTransitionAction.cpp  # Transition play action
│   └── utils/
│       ├── ByteBuffer.h/ByteBuffer.cpp  # Binary data reader
│       ├── ToolSet.h/ToolSet.cpp        # Utility functions
│       ├── UBBParser.h/UBBParser.cpp    # UBB markup parser
│       ├── WeakPtr.h/WeakPtr.cpp        # Weak pointer for GObject
│       └── html/
│           ├── HtmlElement.h/HtmlElement.cpp  # HTML element node
│           ├── HtmlObject.h/HtmlObject.cpp    # HTML render object
│           └── HtmlParser.h/HtmlParser.cpp    # HTML to rich text parser
```

## Dependencies

This module depends on the **spine_godot** module, also located under `modules/`.

### Spine Runtime (spine_godot)

Located at `modules/spine_godot/`, this module provides Spine 2D skeletal animation support for `GLoader3D`.

```
modules/spine_godot/
├── config.py                           # Module build configuration
├── SCsub                               # SCons build script
├── SpineSprite.h / .cpp                # Spine sprite node
├── SpineSkeleton.h / .cpp              # Skeleton wrapper
├── SpineAnimation.h / .cpp             # Animation wrapper
├── SpineAnimationState.h / .cpp        # Animation state wrapper
├── SpineAtlasResource.h / .cpp         # Atlas resource
├── SpineSkeletonDataResource.h / .cpp  # Skeleton data resource
├── SpineSkeletonFileResource.h / .cpp  # Skeleton file resource
├── SpineTrackEntry.h / .cpp            # Track entry wrapper
├── SpineCommon.h                       # Common spine utilities
├── SpineConstant.h                     # Spine constants
├── spine-cpp/                          # Spine C++ runtime (from spine-runtimes)
│   ├── include/spine/                  # Runtime headers
│   │   ├── Animation.h
│   │   ├── Skeleton.h
│   │   └── ...
│   └── src/spine/                      # Runtime source files
│       ├── Animation.cpp
│       └── ...
└── ...
```

GLoader3D requires spine_godot headers via:
```cpp
#include "SpineSprite.h"
#include "SpineSkeleton.h"
// etc.
```

## Key Architecture Decisions

| Cocos2dx Original | Godot Port |
|---|---|
| `cocos2d::Ref` | `Node` / `Object` |
| `cocos2d::Sprite` | `Sprite2D` / `FUISprite` |
| `cocos2d::Vec2` / `cocos2d::Size` | `Vector2` |
| `cocos2d::Color4F` / `Color3B` | `Color` |
| `cocos2d::Texture2D*` | `Ref<Texture2D>` |
| `cocos2d::Rect` | `Rect2` (alias `Rect`) |
| `cocos2d::ui::Scale9Sprite` | Native `FUISprite` 9-slice |
| `CREATE_FUNC` macro | `memnew` / Godot memory management |
| `CC_SAFE_RETAIN` / `CC_SAFE_RELEASE` | Godot `Ref<>` reference counting |

## Build

FairyGUI is compiled as a **built-in Godot module** — no separate build step required.

Build Godot with the fairygui module:

```sh
scons platform=windows target=editor dev_build=yes
or for debug
scons platform=windows vsproj=yes dev_build=yes arch=x86_64 vulkan=no opengl3=yes csharp=no
```

The module is auto-detected by Godot's build system via `config.py`.

The `SCsub` adds include paths for:
- `src/` and all subdirectories (event, display, gears, tween, utils, utils/html, controller_action)
- `modules/spine_godot/` (Spine wrapper headers)
- `modules/spine_godot/spine-cpp/include` (Spine C++ runtime headers)

### Spine Runtime Integration

Spine support is handled by the `spine_godot` module (`modules/spine_godot/`).

- `spine-cpp/` contains the upstream Spine C++ runtime from [EsotericSoftware/spine-runtimes](https://github.com/EsotericSoftware/spine-runtimes)
- The `spine_godot` SCsub compiles both `spine-cpp/src/spine/*.cpp` and its own `*.cpp` files
- Fairygui's `GLoader3D` includes spine_godot headers directly

> **Modification:** `modules/spine_godot/SCsub` line 5 & 8 — include path changed from
> `#../spine_godot/spine-cpp/include` to `#modules/spine_godot/spine-cpp/include`
> to match the new module directory layout.

## Usage

### 1. Initialize GRoot and Load UI Packages

FairyGUI components must be attached to `GRoot`. `GRoot` is a singleton — create it once when your scene is ready:

```gdscript
# Initialize in a main scene or autoload (only once)
# Use create when the scene is already fully loaded (e.g. button callback)
GRoot.create(get_tree())

# During _ready() / _enter_tree() use createDeferred instead —
# it defers attachment to avoid "Parent node is busy setting up children"
GRoot.createDeferred(get_tree())
```

UI packages come from FairyGUI Editor exported `.fui` files. Place them under `res://` in your Godot project — no additional path configuration needed:

```
res://Ui/YourPackage.fui
res://Ui/YourPackage_atlas0.png
```

```gdscript
# Add a FairyGUI package (path without .fui extension)
UIPackage.addPackage("res://UI/YourPackage")

# Create a component and attach it to GRoot.
var comp = UIPackage.createObject("YourPackage", "Main")
GRoot.getInstance().addChild(comp)

# Also create directly by URL
var btn = UIPackage.createObjectFromURL("ui://YourPackage/MyButton")
```

> **Note**: If you call `addChild()` on the **same frame** as `GRoot.create()`,
> you may hit "Parent node is busy setting up children". In that case use
> `GRoot.getInstance().addChild.call_deferred(comp)`. When adding children
> from a button callback, timer, `_process()`, or any later frame, plain
> `addChild()` is fine.

> **Note**: If you enable **branch** export in FairyGUI Editor, component names will be prefixed with the branch ID, e.g. `7iys1/Menu`. In that case, `createObject` needs the full prefixed name. Disable branch in the editor if you don't need it.

### Scene Root Node Type & Mouse Input

FairyGUI receives input events through `FUIContainer`'s `_unhandled_input()` pipeline. For this to work, the scene's root node **must not** be a `Control` type.

`Control` nodes have `mouse_filter = MOUSE_FILTER_STOP` by default, which intercepts mouse events and prevents them from reaching `_unhandled_input`. As a result, FairyGUI's `InputProcessor → hitTest → bubbleEvent(Click)` chain will never trigger, and all button clicks will be silently swallowed.

**Fix:** Use `type="Node"` for the scene root node (or any `CanvasItem` that doesn't stop input):

```gdscript
# ❌ Control root - buttons won't respond to clicks
[node name="MainMenu" type="Control"]

# ✅ Node root - mouse events flow to FUIContainer normally
[node name="MainMenu" type="Node"]
```

This applies to all scenes that host FairyGUI content. All example scenes in this project use `type="Node"` for the root.

### 2. Font Configuration

FairyGUI supports two font types: **BMFont** (bitmap fonts from FairyGUI Editor) and **TTF/System** fonts.

#### BM Font (from FairyGUI Editor)

Bitmaps fonts exported from FairyGUI Editor (`.fnt` files) are loaded automatically through the package system. The font name follows the `ui://PackageName/FontName` URL format.

No additional configuration is required.

#### TTF Font Files

Register TTF/OTF font files before using them:

```gdscript
# Register a TTF font file by alias
UIPackage.registerFont("myfont", "res://fonts/msyh.ttf")
UIPackage.setDefaultFont("myfont")
```

The file extension (`.ttf` / `.otf`) triggers font file loading via `FontFile::load_dynamic_font()`.

#### System Fonts

Register system fonts by name (no file extension):

```gdscript
# Register system fonts by their platform-specific name
UIPackage.registerFont("SimHei", "SimHei")
UIPackage.registerFont("Arial", "Arial")
UIPackage.setDefaultFont("SimHei")
```

Font names without a file extension are resolved via `SystemFont`, which looks up the installed system fonts.

#### How It Works

In FairyGUI Editor, you set a font name on text objects (e.g., `"SimHei"` or `"myfont"`). At runtime:

1. `UIConfig::getRealFontName()` resolves the alias to the actual file path or system font name
2. If the resolved name ends with `.ttf` / `.otf` → `FontFile::load_dynamic_font()` loads the file
3. Otherwise → `SystemFont` matches the name against installed system fonts

### 3. Common Widget Operations

```gdscript
# Position and size
comp.setPosition(100, 200)
comp.setSize(300, 200, false)       # w, h, ignorePivot
comp.center()                        # center within parent
comp.makeFullScreen()

# Opacity and visibility
comp.setAlpha(0.5)
comp.setVisible(true)
comp.setGrayed(false)

# Scale and rotation
comp.setScale(1.5, 1.5)
comp.setRotation(45.0)

# Pivot point
comp.setPivot(0.5, 0.5, true)       # x, y, asAnchor

# GComponent - container
comp.numChildren                     # child count
comp.getChildAt(0)                   # get child by index
comp.getChild("btn_ok")              # find child by name
comp.addChild(someObj)

# GButton
btn.title = "OK"
btn.selected = false
btn.enabled = true

# GTextField / GLabel
label.text = "Hello FairyGUI"
label.setFontSize(24)
label.setColor(Color.WHITE)

# GLoader - load remote images/components
loader.url = "ui://Package/ImageName"

# GList
list.itemRenderer = render_func
list.numItems = 10

# GProgressBar
bar.value = 50.0                     # 0-100
bar.max = 100.0

# GSlider
slider.value = 0.5
slider.max = 1.0
```

### 4. Event Handling

```gdscript
# Click event
btn.addClickListener(func(ctx):
    print("Button clicked")
)

# Scroll event
list.addEventListener(fairygui.UIEventType.Scroll, func(ctx):
    print("List scrolling")
)

# Other common event types:
# fairygui.UIEventType.Click
# fairygui.UIEventType.Changed
# fairygui.UIEventType.TouchBegin / TouchEnd
# fairygui.UIEventType.RollOver / RollOut
# fairygui.UIEventType.Scroll / ScrollEnd
# fairygui.UIEventType.DragStart / DragEnd
# fairygui.UIEventType.PositionChange / SizeChange
# fairygui.UIEventType.GearStop
```

### 5. Controllers

```gdscript
# Get a controller from a component
var ctrl = comp.getController("button")
if ctrl:
    ctrl.selectedIndex = 1           # switch page
    print(ctrl.selectedPage)         # current page name
    print(ctrl.pageCount)            # page count
    print(ctrl.previousIndex)        # previous index
```

### 6. Drag & Drop

```gdscript
obj.draggable = true
obj.setDragBounds(Rect2(0, 0, 500, 400))
```

### 7. Relations (Layout Constraints)

```gdscript
# Keep child aligned relative to parent
child.addRelation(parent, fairygui.RelationType.Width_Width)
child.addRelation(parent, fairygui.RelationType.Height_Height)
```

### 8. Transitions

Design transitions in FairyGUI Editor, then play them at runtime:

```gdscript
comp.getTransition("show").play()
comp.getTransition("hide").play(func():
    print("Animation finished")
)
```

### 9. Registered Godot Classes

All classes below are registered in Godot (methods not yet exposed to GDScript):

| Class | Inherits From | Description |
|---|---|---|
| `fairygui.UIEventDispatcher` | `RefCounted` | Event dispatch base |
| `fairygui.GController` | `UIEventDispatcher` | Controller |
| `fairygui.GObject` | `UIEventDispatcher` | Base class for all UI objects |
| `fairygui.GComponent` | `GObject` | Container component |
| `fairygui.GImage` | `GObject` | Image |
| `fairygui.GGraph` | `GObject` | Shape drawing |
| `fairygui.GTextField` | `GObject` | Text base (abstract) |
| `fairygui.GBasicTextField` | `GTextField` | Basic text |
| `fairygui.GRichTextField` | `GTextField` | Rich text |
| `fairygui.GTextInput` | `GTextField` | Text input |
| `fairygui.GButton` | `GComponent` | Button |
| `fairygui.GLabel` | `GComponent` | Label (button + title) |
| `fairygui.GComboBox` | `GComponent` | Combo box |
| `fairygui.GProgressBar` | `GComponent` | Progress bar |
| `fairygui.GSlider` | `GComponent` | Slider |
| `fairygui.GScrollBar` | `GComponent` | Scroll bar |
| `fairygui.GWindow` | `GComponent` | Window |
| `fairygui.GList` | `GComponent` | List |
| `fairygui.GTree` | `GList` | Tree view |
| `fairygui.GTreeNode` | `RefCounted` | Tree node |
| `fairygui.GLoader` | `GObject` | Loader |
| `fairygui.GLoader3D` | `GObject` | Spine animation |
| `fairygui.GMovieClip` | `GObject` | Frame animation |
| `fairygui.GGroup` | `GObject` | Group |
| `fairygui.GRoot` | `GComponent` | Root node |
| `fairygui.FUIContainer` | `Node2D` | Display container |
| `fairygui.FUIInnerContainer` | `FUIContainer` | Inner container |
| `fairygui.FUIInput` | `Control` | Input field control |
| `fairygui.FUILabel` | `Node2D` | Label display |
| `fairygui.FUIRichText` | `Node2D` | Rich text display |
| `fairygui.FUISprite` | `Sprite2D` | Sprite display |
| `fairygui.UIPackage` | `RefCounted` | Package manager |
| `fairygui.GPopupMenu` | `RefCounted` | Popup menu |
> **Not registered:** `ScrollPane`/`Transition` (require custom constructors), `DrawNode` (internal class).
