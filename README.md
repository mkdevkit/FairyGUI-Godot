# FairyGUI-Godot

> This project is a refactor of the FairyGUI cocos2d-x runtime for Godot. It supports C#, GDScript, JavaScript/TypeScript, or any other scripting language Godot supports.
> Currently supports Godot 4.5–4.6.

> **中文文档：[README.zh.md](README.zh.md)**

[JavaScript demo (online)](https://fairygui.demo.store.loongdusk.com/js/)  
[GDScript demo (online)](https://fairygui.demo.store.loongdusk.com/gd/)

FairyGUI runtime for Godot Engine, integrated as a built-in C++ module.

You can also build from [https://github.com/mkdevkit/godot](https://github.com/mkdevkit/godot).

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
│   ├── godot_types.h                   # Godot engine type mapping
│   ├── Margin.h/Margin.cpp             # Margin type
│   ├── UIConfig.h/UIConfig.cpp         # Global UI configuration
│   ├── PackageItem.h/PackageItem.cpp   # Package resource item
│   ├── UIPackage.h/UIPackage.cpp       # Package loader (reads .fui binary)
│   ├── Controller.h/Controller.cpp     # GController (page state controller)
│   ├── GObject.h/GObject.cpp           # Base GObject (inherits Node)
│   ├── GComponent.h/GComponent.cpp     # GComponent container
│   ├── UIObjectFactory.h/UIObjectFactory.cpp  # Widget factory
│   ├── GObjectPool.h/GObjectPool.cpp   # Object pool
│   ├── RelationItem.h/RelationItem.cpp  # Single relation constraint
│   ├── Relations.h/Relations.cpp       # Relation collection
│   ├── ScrollPane.h/ScrollPane.cpp     # ScrollPane (GComponent scroll container)
│   ├── Transition.h/Transition.cpp     # Transition animation system
│   ├── TranslationHelper.h/TranslationHelper.cpp  # String translation
│   ├── GRoot.h/GRoot.cpp               # GRoot (root canvas)
│   ├── Window.h/Window.cpp             # GWindow (popup / modal window)
│   ├── GPopupMenu.h/GPopupMenu.cpp     # PopupMenu
│   ├── DragDropManager.h/DragDropManager.cpp  # DragDropManager
│   ├── GImage.h/GImage.cpp             # GImage widget
│   ├── GGraph.h/GGraph.cpp             # GGraph drawing widget
│   ├── GTextField.h/GTextField.cpp     # GTextField / GBasicTextField
│   ├── GRichTextField.h/GRichTextField.cpp   # GRichTextField rich text widget
│   ├── GTextInput.h/GTextInput.cpp     # GTextInput text input widget
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
│   ├── GLoader3D.h/GLoader3D.cpp       # GLoader3D (Spine runtime)
│   ├── GMovieClip.h/GMovieClip.cpp     # GMovieClip frame animation widget
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
│   │   ├── FUIInput.h/FUIInput.cpp     # Text input control
│   │   ├── FUILabel.h/FUILabel.cpp     # Label rendering node
│   │   ├── FUIRichText.h/FUIRichText.cpp  # Rich text rendering node
│   │   ├── FUISprite.h/FUISprite.cpp   # Sprite rendering node
│   │   └── TextFormat.h/TextFormat.cpp  # Text format descriptor
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

This module **optionally** depends on the **spine_godot** module (`modules/spine_godot/`) for Spine skeletal animation in `GLoader3D`. When `spine_godot` is not enabled, fairygui still builds; `SPINE_GODOT_DISABLED` is defined and `GLoader3D` skips Spine loading at runtime.

### Spine Runtime (spine_godot)

Located at `modules/spine_godot/`, provides Spine 2D skeletal animation support for `GLoader3D`.

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

`GLoader3D` includes spine_godot headers as follows:

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

Run from the Godot source root:

**With Spine support** (recommended if you use `GLoader3D`):

```sh
scons module_fairygui_enabled=yes module_spine_godot_enabled=yes -j8
```

**Without Spine** (fairygui only; `GLoader3D` Spine loading is disabled):

```sh
scons module_fairygui_enabled=yes -j8
```

You can combine these flags with other SCons options, for example:

```sh
scons platform=windows target=editor dev_build=yes module_fairygui_enabled=yes module_spine_godot_enabled=yes -j8
```

Or generate a Visual Studio project (debug):

```sh
scons platform=windows vsproj=yes dev_build=yes arch=x86_64 vulkan=no opengl3=yes csharp=no module_fairygui_enabled=yes module_spine_godot_enabled=yes
```

Godot's build system auto-detects the module via `config.py`.

`SCsub` adds include paths for:

- `src/` and all subdirectories (event, display, gears, tween, utils, utils/html, controller_action)
- `modules/spine_godot/` and `modules/spine_godot/spine-cpp/include` (only when `module_spine_godot_enabled=yes`)

### Spine Runtime Integration

The Spine runtime has been partially modified. You can clone [https://github.com/mkdevkit/spine_godot](https://github.com/mkdevkit/spine_godot).

When `module_spine_godot_enabled=yes`, Spine support is provided by the `spine_godot` module (`modules/spine_godot/`).

- `spine-cpp/` contains the upstream Spine C++ runtime from [EsotericSoftware/spine-runtimes](https://github.com/EsotericSoftware/spine-runtimes)
- The `spine_godot` SCsub compiles both `spine-cpp/src/spine/*.cpp` and its own `*.cpp` files
- Fairygui's `GLoader3D` includes spine_godot headers directly

> **Modification note:** In `modules/spine_godot/SCsub` lines 5 and 8, the include path was changed from
> `#../spine_godot/spine-cpp/include` to `#modules/spine_godot/spine-cpp/include`
> to match the new module directory layout.

### JavaScript development with GodotJS (optional; TypeScript demo included)

GodotJS requires a patch. Clone [https://github.com/mkdevkit/GodotJS](https://github.com/mkdevkit/GodotJS).

## GodotJS Patch

When running the TypeScript demos (`examples/ts/`) with **GodotJS**, both items below are required.

### 1. Register the FairyGUI base class as `GuiObject` (required)

GodotJS exposes engine `Object` in JavaScript as **`GObject`** (to avoid clashing with the JS built-in `Object`; see `GodotJS/internal/jsb_naming_util.cpp`).

If the FairyGUI base were still registered as ClassDB `GObject`, it would collide with that JS name and break inheritance / type lookup (e.g. `GComponent.addChild is not a function`).

This module therefore registers the UI base as **`GuiObject`**, with `using GObject = GuiObject` in C++ for Cocos-source compatibility. **This rename is separate from the GodotJS patch below; you need both.**

### 2. Fix `jsb_godot_module_loader` assertion (required for dev builds)

GodotJS lazily loads ClassDB types via the `godot` module Proxy. For renamed engine types (`Object` → `GObject`), `NativeClassInfo::name` stores the **ClassDB name** while JS requests the **display name**. In dev builds the assertion below compares the wrong names and crashes when loading `GObject` or binding FairyGUI types:

**File:** `modules/GodotJS/bridge/jsb_godot_module_loader.cpp`

```diff
             if (const NativeClassInfoPtr class_info = env->expose_godot_object_class(ClassDB::classes.getptr(original_name)))
             {
-                jsb_check(class_info->name == p_type_name);
+                // class_info->name is ClassDB original name (e.g. "Object"), not JS display name (e.g. "GObject").
+                jsb_check(class_info->name == original_name);
                 jsb_check(!class_info->clazz.IsEmpty());
                 info.GetReturnValue().Set(class_info->clazz.Get(isolate));
                 return;
             }
```

Do **not** change lines 97–99 (`expose_class` path for types like `Signal`) — there `class_info->name` matches the JS display name.

Rebuild Godot (including GodotJS) after applying the patch. Then `FGUIEventContext.getData()` can return `GuiObject` to JS normally; `getItemText()` remains available when you only need ClickItem text.

> **Note:** Eager-loading `godot.GObject` at startup in `fgui-globals.ts` was tried and still hits this assertion in dev builds; it cannot replace this patch.

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

UI packages come from FairyGUI Editor exported `.fui` files. Place them under `res://` in your Godot project:

```
res://Ui/YourPackage.fui
res://Ui/YourPackage_atlas0.png
```

```gdscript
# Add a FairyGUI package (path without .fui extension)
UIPackage.addPackage("res://UI/YourPackage")

# Create a component and attach it to GRoot
var comp = UIPackage.createObject("YourPackage", "Main")
GRoot.getInstance().addChild(comp)

# Also create directly by URL
var btn = UIPackage.createObjectFromURL("ui://YourPackage/MyButton")
```

> **Note**: If you call `addChild()` on the **same frame** as `GRoot.create()`,
> you may hit "Parent node is busy setting up children".
> Use `GRoot.createDeferred(get_tree())` in that case — it attaches on the next frame.
> When adding children from a button callback, timer, or any later frame, plain `addChild()` is fine.

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

Bitmap fonts exported from FairyGUI Editor (`.fnt` files) are loaded automatically through the package system. The font name follows the `ui://PackageName/FontName` URL format.

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

Font names without a file extension are resolved via `SystemFont`, which looks up installed system fonts.

#### How It Works

In FairyGUI Editor, you set a font name on text objects (e.g., `"SimHei"` or `"myfont"`). At runtime:

1. `UIConfig::getRealFontName()` resolves the alias to the actual file path or system font name
2. If the resolved name ends with `.ttf` / `.otf` → `FontFile::load_dynamic_font()` loads the file
3. Otherwise → `SystemFont` matches the name against installed system fonts

#### ScrollBar

Horizontal and vertical scroll bars used by `ScrollPane` are **separate UI components**. You must register their URLs via `UIConfig`, and the corresponding package must be loaded with `UIPackage.addPackage`. The `scrollBars` entry in editor `Common.json` does **not** apply automatically at runtime.

- The demo registers scroll bars in `examples/*/Scripts/DemoSceneBase` via `_registerDefaultScrollBars()`: load the **Basics** package and set `horizontalScrollBar` / `verticalScrollBar`.
- `scrollBar="auto"` (editor: show on scroll): matches Cocos — visible on desktop hover / drag / inertia scroll; mouse wheel does not trigger display.
- GList **item pool** only recycles list items; it is unrelated to ScrollBar components.

For full details (including “no scroll bar until Basics demo was opened” behavior), see [`examples/README.md`](examples/README.md#scrollbar).

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
list.addEventListener(UIEventType.Scroll, func(ctx):
    print("List scrolling")
)

# Other common event types
# UIEventType.Click
# UIEventType.Changed
# UIEventType.TouchBegin / TouchEnd
# UIEventType.RollOver / RollOut
# UIEventType.Scroll / ScrollEnd
# UIEventType.DragStart / DragEnd
# UIEventType.PositionChange / SizeChange
# UIEventType.GearStop
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

### 6. Screen Adaptation

```gdscript
# After creating GRoot, set design resolution and scale mode
var root = GRoot.create(get_tree())

# Content scale (design resolution 1136x640, proportional scaling)
root.setContentScaleFactor(1136, 640, GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT)

# Call when the window size changes so FairyGUI recalculates scale and layout
root.onWindowSizeChanged()
```

#### ScreenMatchMode

| Mode | Scale | GRoot size | Effect |
|------|-------|------------|--------|
| `MATCH_WIDTH_OR_HEIGHT` | `min(screenW/designW, screenH/designH)` | `(designW, designH)` | Uniform scale, letterboxed, content always visible |
| `MATCH_WIDTH` | `screenW / designW` | `(designW, screenH/scale)` | Fixed width, dynamic height |
| `MATCH_HEIGHT` | `screenH / designH` | `(screenW/scale, designH)` | Fixed height, dynamic width |
| `MATCH_FILL` | `scaleX=screenW/designW`, `scaleY=screenH/designH` | `(designW, designH)` | Non-uniform stretch to fill screen |

#### Listening for window resize in GDScript

```gdscript
func _ready():
    var root = GRoot.create(get_tree())
    root.setContentScaleFactor(1136, 640, GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT)

    # Initial layout
    root.onWindowSizeChanged()

    # Listen for resize — get GRoot's displayObject, find the Window node, connect size_changed
    var display_node = root.getDisplayObject()
    if display_node and display_node.is_inside_tree():
        var viewport = display_node.get_viewport()
        if viewport is Window:
            viewport.size_changed.connect(root.onWindowSizeChanged)
```

#### Notes on makeFullScreen

`makeFullScreen()` reads `GRoot` size to set its own size. Calling it directly in `_ready()` may run before `GRoot` is fully initialized.

```gdscript
var root = GRoot.create(get_tree())
root.setContentScaleFactor(1136, 640, GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT)

var comp = UIPackage.createObject("UI", "MainPanel")
comp.center()
comp.makeFullScreen.call_deferred()  # wait until GRoot is ready
root.addChild(comp)

func _on_window_size_change() -> void:
    GRoot.getInstance().onWindowSizeChanged()
    main_panel.makeFullScreen()
```

### 7. Drag & Drop

```gdscript
obj.draggable = true
obj.setDragBounds(Rect2(0, 0, 500, 400))
```

### 8. Relations (Layout Constraints)

```gdscript
# Keep child aligned relative to parent
child.addRelation(parent, RelationType.Width_Width)
child.addRelation(parent, RelationType.Height_Height)
```

### 9. Transitions

Design transitions in FairyGUI Editor, then play them at runtime:

```gdscript
comp.getTransition("show").play()
comp.getTransition("hide").play(func():
    print("Animation finished")
)
```

### 10. Registered Godot Classes

The following classes are registered in Godot (methods not yet fully exposed to GDScript):

| Class | Inherits From | Description |
|---|---|---|
| `fairygui.UIEventDispatcher` | `RefCounted` | Event dispatch base |
| `fairygui.GController` | `UIEventDispatcher` | Controller |
| `fairygui.GuiObject` | `UIEventDispatcher` | Base class for all UI objects |
| `fairygui.GComponent` | `GuiObject` | Container component |
| `fairygui.GImage` | `GuiObject` | Image |
| `fairygui.GGraph` | `GuiObject` | Shape drawing |
| `fairygui.GTextField` | `GuiObject` | Text base (abstract) |
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
| `fairygui.GLoader` | `GuiObject` | Loader |
| `fairygui.GLoader3D` | `GuiObject` | Spine animation |
| `fairygui.GMovieClip` | `GuiObject` | Frame animation |
| `fairygui.GGroup` | `GuiObject` | Group |
| `fairygui.GRoot` | `GComponent` | Root node |
| `fairygui.FUIContainer` | `Node2D` | Display container |
| `fairygui.FUIInnerContainer` | `FUIContainer` | Inner container |
| `fairygui.FUIInput` | `Control` | Input field control |
| `fairygui.FUILabel` | `Node2D` | Label display |
| `fairygui.FUIRichText` | `Node2D` | Rich text display |
| `fairygui.FUISprite` | `Sprite2D` | Sprite display |
| `fairygui.UIPackage` | `RefCounted` | Package manager |
| `fairygui.GPopupMenu` | `RefCounted` | Popup menu |

> **Not registered:** `ScrollPane` / `Transition` (require custom constructors), `DrawNode` (internal class).

### Export notes

When using filtered export, include these non-resource files/folders:

`*.fui`, `Resources/UI/*`, `*.ttf`, `Resources/fonts/*`

Exclude these files/folders:

`gen/godot/*`, `*.cs`

## Demo Running Guide

### TypeScript

Build Godot with the GodotJS module enabled.

In the `examples/` directory, compile TypeScript:

```sh
npx -p typescript tsc
```

Set the main scene to `res://ts/Scenes/MainMenu.tscn`.

### C#

Build Godot with Mono enabled:

```sh
scons module_mono_enabled=yes module_spine_godot_enabled=yes
```

Generate Mono glue code:

```sh
.\bin\godot.windows.editor.dev.x86_64.mono.exe --headless --generate-mono-glue modules/mono/glue
```

After glue generation, build managed assemblies:

```sh
python modules/mono/build_scripts/build_assemblies.py --godot-output-dir ./bin --push-nupkgs-local ./nupkgs
```

Set the `GODOT_BIN` environment variable to your Godot build output directory (`bin/`).

If you don't have the NuGet feed configured, add a local source (once):

```sh
dotnet nuget add source D:\Source\godot\nupkgs --name GodotLocal
```

Set the main scene to `res://csharp/Scenes/MainMenu.tscn`, then run from the editor.

See also [`examples/csharp/README.md`](examples/csharp/README.md).

### GDScript

Set the main scene to `res://gd/Scenes/MainMenu.tscn`.

### Web

```sh
scons platform=web target=template_release optimize=size_extra lto=full disable_3d=yes disable_advanced_gui=yes module_mono_enabled=no module_xr_enabled=no module_webxr_enabled=no module_multiplayer_enabled=no module_text_server_adv_enabled=no module_text_server_fb_enabled=yes module_bmp_enabled=no module_dds_enabled=no module_hdr_enabled=no module_ktx_enabled=no module_tga_enabled=no disable_audio_speech=yes module_spine_godot_enabled=yes
```

For broader mobile browser compatibility, add `threads=no`.

Set the main scene to `res://ts/Scenes/MainMenu.tscn` or `res://gd/Scenes/MainMenu.tscn` for JavaScript or GDScript demos. Godot does not currently support exporting C# Web projects.

See [`examples/README.md`](examples/README.md#web-export) for export filters, COOP/COEP, fonts, and related notes.

### WeChat Mini Game

**Disable wasm-eh:** when building export templates, edit `platform/web/detect.py` and change both `'wasm'` entries to `'emscripten'`:

```python
env.Append(CCFLAGS=["-sSUPPORT_LONGJMP='emscripten'"])
env.Append(LINKFLAGS=["-sSUPPORT_LONGJMP='emscripten'"])
```

**Disable `javascript_eval`:** add `javascript_eval=no` to your SCons command.

WeChat does not allow `eval()`. If you develop with JavaScript (GodotJS), also add `use_quickjs_ng=yes` — this embeds a JS VM in the WASM build instead of using the host JS environment. WeChat runs WASM in high-performance mode, so performance is generally acceptable (similar to using Lua for WeChat mini games in Unity).

```sh
scons platform=web target=template_release optimize=size_extra lto=full disable_3d=yes disable_advanced_gui=yes module_mono_enabled=no module_xr_enabled=no module_webxr_enabled=no module_multiplayer_enabled=no module_text_server_adv_enabled=no module_text_server_fb_enabled=yes module_bmp_enabled=no module_dds_enabled=no module_hdr_enabled=no module_ktx_enabled=no module_tga_enabled=no disable_audio_speech=yes module_spine_godot_enabled=yes threads=no javascript_eval=no use_quickjs_ng=yes
```

Export your project as **Web** using the templates built with the command above, then convert the exported output into a WeChat mini game project with [godot-minigame](https://github.com/mkdevkit/godot-minigame).
