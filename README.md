# FairyGUI-Godot

FairyGUI runtime for Godot Engine, built-in C++ module (no godot-cpp dependency).

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

## Key Architecture Decisions

| Cocos2dx Original | Godot Port |
|---|---|
| `cocos2d::Ref` | `godot::Node` / `godot::Object` |
| `cocos2d::Sprite` / `cocos2d::DrawNode` | `RenderNode` (godot::Node2D with `_draw()`) |
| `cocos2d::Vec2` / `cocos2d::Size` | `godot::Vector2` |
| `cocos2d::Color4F` / `Color3B` | `godot::Color` |
| `cocos2d::Texture2D*` | `godot::Ref<godot::Texture2D>` |
| `cocos2d::Rect` | `godot::Rect2` |
| `cocos2d::ui::Scale9Sprite` | Native `draw_texture_rect_region` in RenderNode |
| `CREATE_FUNC` macro | `memnew` / Godot memory management |
| `CC_SAFE_RETAIN` / `CC_SAFE_RELEASE` | Godot `Ref<>` reference counting |

## Build Instructions

### Prerequisites

- Python 3.x + SCons (`pip install scons`)
- Godot Engine 4.x
- godot-cpp (clone into `./godot-cpp/`)
- MinGW-w64 (for `use_mingw=yes`, optional)

### Build Commands

| Command | Description |
|---|---|
| `scons` | Build with default toolchain (auto-detect platform/arch) |
| `scons use_mingw=yes` | Build with MinGW-w64 (static link, `D:\GNU\mingw64\bin` by default) |
| `scons vsproj=yes` | Generate VS solution only (no compilation): `build/fairygui_godot.sln` |
| `scons debug=yes` | MSVC debug build: `/Zi /Od /DEBUG` (generates .pdb) |

The build delegates standard godot-cpp parameters (`p=<platform>`, `arch=<arch>`, `dev_build=yes`, `target=<debug/release>`) to `godot-cpp/SConstruct`.

#### MinGW Custom Path

Set the `MINGW_BIN` environment variable to override the default MinGW path:

```powershell
$env:MINGW_BIN = "C:\msys64\mingw64\bin"
scons use_mingw=yes
```

### Output

Built library goes to `build/libfairygui_godot.dll` (Windows) or `build/libfairygui_godot.so` (Linux).

Copy `fairygui_godot.gdextension` + the built library into your Godot project, adjusting paths in `.gdextension` as needed.

#### Godot 4.5+ Compatibility

When upgrading godot-cpp to match newer Godot versions, some source changes may be needed. Known changes:

- `SpineAtlasResource.cpp`: `JSON*` → `Ref<JSON>` API update
- `GLoader3D.cpp`: `#include "spine_godot/SpineTrackEntry.h"` for `Ref<T>` destructor
- Package file format changes may require re-exporting `.fui` from FairyGUI Editor

### Spine Runtime Integration

FairyGUI-Godot supports Spine 2D skeletal animations via GLoader3D. The spine runtime is sourced from [EsotericSoftware/spine-runtimes](https://github.com/EsotericSoftware/spine-runtimes) and compiled as a static library linked into the final GDExtension binary.

**Modifications made to the upstream spine-godot source:**

| File | Change |
|---|---|
| `spine-runtimes/spine-godot/SConscript` | **New file** — Receives fairygui's build environment, compiles `spine-cpp` + `spine_godot` as a static library |
| `spine-runtimes/spine-godot/spine_godot/SpineAtlasResource.cpp` | `JSON*` → `Ref<JSON>` adaptation for newer godot-cpp API |

View the above changes in the `diff/` directory.
See `spine-godot-SConscript.diff` and `spine-godot-SpineAtlasResource.diff` in the project root for the exact diffs.

## Usage

### 1. Integrate into Godot Project

Copy the following into your Godot project root:

```
your_project/
├── fairygui_godot.gdextension
└── bin/
    └── libfairygui_godot.windows.debug.x86_64.dll  (platform-specific)
```

Ensure paths in `.gdextension` match your actual build output.

### 2. Initialize GRoot and Load UI Packages

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
| `fairygui.PopupMenu` | `RefCounted` | Popup menu |

> **Not registered:** `Window` (conflicts with `godot::Window`), `ScrollPane`/`Transition` (require custom constructors), `DrawNode` (internal class).
