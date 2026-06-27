# FairyGUI-Godot

> 当前项目是正在开发中的实验项目，基于FairyGUI的cocos2dx版本，使用AI重构中。

FairyGUI 的 Godot 引擎运行时，以内置 C++ Module 形式集成。

## Project Structure

```
modules/fairygui/
├── config.py                           # 模块构建配置 (can_build, doc_classes)
├── SCsub                               # SCons 构建脚本
├── register_types.h                    # 模块注册头文件
├── register_types.cpp                  # 模块注册 (ClassDB 注册)
├── src/                                # 核心运行时
│   ├── FairyGUI.h                      # 总括包含头文件
│   ├── FairyGUIMacros.h                # 命名空间宏
│   ├── FieldTypes.h                    # 所有枚举 (ObjectType, FillMethod 等)
│   ├── godot_types.h                   # Godot 引擎类型映射
│   ├── Margin.h/Margin.cpp             # Margin 类型
│   ├── UIConfig.h/UIConfig.cpp         # 全局 UI 配置
│   ├── PackageItem.h/PackageItem.cpp   # 资源包项目
│   ├── UIPackage.h/UIPackage.cpp       # 资源包加载器 (读取 .fui 二进制)
│   ├── Controller.h/Controller.cpp     # GController (页面状态控制器)
│   ├── GObject.h/GObject.cpp           # 基础 GObject (继承 Node)
│   ├── GComponent.h/GComponent.cpp     # GComponent 容器
│   ├── UIObjectFactory.h/UIObjectFactory.cpp  # Widget 工厂
│   ├── GObjectPool.h/GObjectPool.cpp   # 对象池
│   ├── RelationItem.h/RelationItem.cpp  # 单一关系约束
│   ├── Relations.h/Relations.cpp       # 关系集合
│   ├── ScrollPane.h/ScrollPane.cpp     # ScrollPane (GComponent 滚动容器)
│   ├── Transition.h/Transition.cpp     # Transition 动画系统
│   ├── TranslationHelper.h/TranslationHelper.cpp  # 字符串翻译
│   ├── GRoot.h/GRoot.cpp               # GRoot (根画布)
│   ├── Window.h/Window.cpp             # GWindow (弹窗/模态窗口)
│   ├── GPopupMenu.h/GPopupMenu.cpp     # PopupMenu
│   ├── DragDropManager.h/DragDropManager.cpp  # DragDropManager
│   ├── GImage.h/GImage.cpp             # GImage 图片组件
│   ├── GGraph.h/GGraph.cpp             # GGraph 绘图组件
│   ├── GTextField.h/GTextField.cpp     # GTextField / GBasicTextField
│   ├── GRichTextField.h/GRichTextField.cpp   # GRichTextField 富文本组件
│   ├── GTextInput.h/GTextInput.cpp     # GTextInput 文本输入组件
│   ├── GButton.h/GButton.cpp           # GButton 按钮组件
│   ├── GLabel.h/GLabel.cpp             # GLabel 标签组件
│   ├── GComboBox.h/GComboBox.cpp       # GComboBox 下拉框组件
│   ├── GProgressBar.h/GProgressBar.cpp # GProgressBar 进度条组件
│   ├── GSlider.h/GSlider.cpp           # GSlider 滑块组件
│   ├── GScrollBar.h/GScrollBar.cpp     # GScrollBar 滚动条组件
│   ├── GList.h/GList.cpp               # GList 列表组件
│   ├── GTree.h/GTree.cpp               # GTree 树组件
│   ├── GTreeNode.h/GTreeNode.cpp       # GTreeNode 树节点
│   ├── GLoader.h/GLoader.cpp           # GLoader 加载器组件
│   ├── GLoader3D.h/GLoader3D.cpp       # GLoader3D 3D加载器 (Spine 运行时)
│   ├── GMovieClip.h/GMovieClip.cpp     # GMovieClip 帧动画组件
│   ├── GGroup.h/GGroup.cpp             # GGroup 组组件
│   ├── event/
│   │   ├── UIEventType.h               # 事件类型常量
│   │   ├── EventContext.h/EventContext.cpp    # 事件上下文
│   │   ├── UIEventDispatcher.h/UIEventDispatcher.cpp  # 事件分发系统
│   │   ├── InputEvent.h/InputEvent.cpp  # 输入事件数据
│   │   ├── InputProcessor.h/InputProcessor.cpp  # 输入事件处理器
│   │   └── HitTest.h/HitTest.cpp       # 点击测试接口 + 像素级点击测试
│   ├── display/
│   │   ├── BitmapFont.h/BitmapFont.cpp  # 位图字体渲染
│   │   ├── FUIContainer.h/FUIContainer.cpp  # 容器节点 (裁剪、模板)
│   │   ├── FUIInput.h/FUIInput.cpp     # 文本输入控件
│   │   ├── FUILabel.h/FUILabel.cpp     # 文本渲染节点
│   │   ├── FUIRichText.h/FUIRichText.cpp  # 富文本渲染节点
│   │   ├── FUISprite.h/FUISprite.cpp   # 精灵渲染节点
│   │   └── TextFormat.h/TextFormat.cpp  # 文本格式描述符
│   ├── gears/
│   │   ├── GearBase.h/GearBase.cpp     # Gear 基类
│   │   ├── GearAnimation.h/GearAnimation.cpp  # 动画齿轮
│   │   ├── GearColor.h/GearColor.cpp   # 颜色齿轮
│   │   ├── GearDisplay.h/GearDisplay.cpp  # 显示可见性齿轮
│   │   ├── GearDisplay2.h/GearDisplay2.cpp # Display2 齿轮
│   │   ├── GearFontSize.h/GearFontSize.cpp  # 字体大小齿轮
│   │   ├── GearIcon.h/GearIcon.cpp     # 图标齿轮
│   │   ├── GearLook.h/GearLook.cpp     # 外观齿轮
│   │   ├── GearSize.h/GearSize.cpp     # 尺寸齿轮
│   │   ├── GearText.h/GearText.cpp     # 文本齿轮
│   │   └── GearXY.h/GearXY.cpp         # 位置齿轮
│   ├── tween/
│   │   ├── EaseType.h                  # 缓动类型枚举
│   │   ├── EaseManager.h/EaseManager.cpp  # 缓动函数管理器
│   │   ├── GTween.h/GTween.cpp         # GTween 管理器 (静态)
│   │   ├── GTweener.h/GTweener.cpp     # 单个 Tweener
│   │   ├── GPath.h/GPath.cpp           # 路径动画
│   │   ├── TweenManager.h/TweenManager.cpp  # Tween 实例管理器
│   │   ├── TweenPropType.h/TweenPropType.cpp  # Tween 属性类型
│   │   └── TweenValue.h/TweenValue.cpp  # Tween 值包装
│   ├── controller_action/
│   │   ├── ControllerAction.h/ControllerAction.cpp  # 控制器动作基类
│   │   ├── ChangePageAction.h/ChangePageAction.cpp  # 页面切换动作
│   │   └── PlayTransitionAction.h/PlayTransitionAction.cpp  # 过渡播放动作
│   └── utils/
│       ├── ByteBuffer.h/ByteBuffer.cpp  # 二进制数据读取器
│       ├── ToolSet.h/ToolSet.cpp        # 工具函数
│       ├── UBBParser.h/UBBParser.cpp    # UBB 标记解析器
│       ├── WeakPtr.h/WeakPtr.cpp        # GObject 弱引用
│       └── html/
│           ├── HtmlElement.h/HtmlElement.cpp  # HTML 元素节点
│           ├── HtmlObject.h/HtmlObject.cpp    # HTML 渲染对象
│           └── HtmlParser.h/HtmlParser.cpp    # HTML 到富文本解析器
```

## 依赖

本模块依赖 **spine_godot** 模块，位于 `modules/spine_godot/`。

### Spine 运行时 (spine_godot)

位于 `modules/spine_godot/`，为 `GLoader3D` 提供 Spine 2D 骨骼动画支持。

```
modules/spine_godot/
├── config.py                           # 模块构建配置
├── SCsub                               # SCons 构建脚本
├── SpineSprite.h / .cpp                # Spine 精灵节点
├── SpineSkeleton.h / .cpp              # 骨骼包装
├── SpineAnimation.h / .cpp             # 动画包装
├── SpineAnimationState.h / .cpp        # 动画状态包装
├── SpineAtlasResource.h / .cpp         # Atlas 资源
├── SpineSkeletonDataResource.h / .cpp  # 骨骼数据资源
├── SpineSkeletonFileResource.h / .cpp  # 骨骼文件资源
├── SpineTrackEntry.h / .cpp            # 轨道条目包装
├── SpineCommon.h                       # 通用 spine 工具
├── SpineConstant.h                     # Spine 常量
├── spine-cpp/                          # Spine C++ 运行时 (来自 spine-runtimes)
│   ├── include/spine/                  # 运行时头文件
│   │   ├── Animation.h
│   │   ├── Skeleton.h
│   │   └── ...
│   └── src/spine/                      # 运行时源文件
│       ├── Animation.cpp
│       └── ...
└── ...
```

GLoader3D 通过以下方式引用 spine_godot 头文件：
```cpp
#include "SpineSprite.h"
#include "SpineSkeleton.h"
// 等等
```

## Key Architecture Decisions

| Cocos2dx Original | Godot Port |
|---|---|
| `cocos2d::Ref` | `Node` / `Object` |
| `cocos2d::Sprite` | `Sprite2D` / `FUISprite` |
| `cocos2d::Vec2` / `cocos2d::Size` | `Vector2` |
| `cocos2d::Color4F` / `Color3B` | `Color` |
| `cocos2d::Texture2D*` | `Ref<Texture2D>` |
| `cocos2d::Rect` | `Rect2` (别名 `Rect`) |
| `cocos2d::ui::Scale9Sprite` | 原生 `FUISprite` 九宫格 |
| `CREATE_FUNC` 宏 | `memnew` / Godot 内存管理 |
| `CC_SAFE_RETAIN` / `CC_SAFE_RELEASE` | Godot `Ref<>` 引用计数 |

## 构建

FairyGUI 作为 **Godot 内置模块**编译 — 无需独立构建步骤。

构建含 fairygui 模块的 Godot：

```sh
scons platform=windows target=editor dev_build=yes
or for debug
scons platform=windows vsproj=yes dev_build=yes arch=x86_64 vulkan=no opengl3=yes csharp=no
```

Godot 构建系统通过 `config.py` 自动发现模块。

`SCsub` 添加的包含路径：
- `src/` 及其所有子目录 (event, display, gears, tween, utils, utils/html, controller_action)
- `modules/spine_godot/` (Spine 封装头文件)
- `modules/spine_godot/spine-cpp/include` (Spine C++ 运行时头文件)

### Spine 运行时集成

Spine 支持由 `spine_godot` 模块 (`modules/spine_godot/`) 处理。

- `spine-cpp/` 包含来自 [EsotericSoftware/spine-runtimes](https://github.com/EsotericSoftware/spine-runtimes) 的上游 Spine C++ 运行时
- `spine_godot` 的 SCsub 同时编译 `spine-cpp/src/spine/*.cpp` 和自己的 `*.cpp` 文件
- Fairygui 的 `GLoader3D` 直接包含 spine_godot 头文件

> **修改说明：** `modules/spine_godot/SCsub` 第5行和第8行 — include 路径从
> `#../spine_godot/spine-cpp/include` 改为 `#modules/spine_godot/spine-cpp/include`
> 以适配新的模块目录布局。

## 使用

### 1. 初始化 GRoot 并加载 UI 包

FairyGUI 组件需要挂载在 `GRoot` 上。`GRoot` 是单例，通常在场景就绪时创建一次：

```gdscript
# 在主场景或 autoload 中初始化（只需调用一次）
# 直接 create 适合在场景已就绪后调用（如按钮回调）
GRoot.create(get_tree())

# 在 _ready() / _enter_tree() 中调用请使用 createDeferred，
# 它会自动推迟挂载以避免 "Parent node is busy setting up children" 错误
GRoot.createDeferred(get_tree())
```

UI 包资源来自 FairyGUI 编辑器导出的 `.fui` 文件。将这些文件放入 Godot 的 `res://` 目录即可直接使用，无需额外配置路径：

```
res://Ui/YourPackage.fui
res://Ui/YourPackage_atlas0.png
```

```gdscript
# 添加一个 FairyGUI 包（传入不带 .fui 后缀的路径）
UIPackage.addPackage("res://UI/YourPackage")

# 从包中创建组件，并挂载到 GRoot
var comp = UIPackage.createObject("YourPackage", "Main")
GRoot.getInstance().addChild(comp)

# 也可以通过 URL 直接创建
var btn = UIPackage.createObjectFromURL("ui://YourPackage/MyButton")
```

> **注意**：如果在 `GRoot.create()` 同一帧内立即调用 `addChild()`，可能遇到
> "Parent node is busy setting up children" 错误。
> 这种情况请使用 `GRoot.createDeferred(get_tree())`，它会在下一帧自动挂载。
> 如果是点击按钮、计时器等非初始化时机添加子节点，则直接 `addChild()` 即可。

> **注意**：如果从 FairyGUI 编辑器导出包时勾选了**分支**（branch）功能，组件名称会被加上分支 ID 前缀，例如 `7iys1/Menu`。此时 `createObject` 需要传入带前缀的名称。如果不需要分支功能，导出时取消勾选即可。

### 2. 字体配置

FairyGUI 支持两种字体类型：**BMFont**（FairyGUI 编辑器导出的位图字体）和 **TTF/系统** 字体。

#### 位图字体（FairyGUI 编辑器导出）

从 FairyGUI 编辑器导出的位图字体（`.fnt` 文件）通过资源包系统自动加载。字体名称遵循 `ui://PackageName/FontName` 的 URL 格式。

无需额外配置。

#### TTF 字体文件

使用前注册 TTF/OTF 字体文件：

```gdscript
# 按别名注册 TTF 字体文件
UIPackage.registerFont("myfont", "res://fonts/msyh.ttf")
UIPackage.setDefaultFont("myfont")
```

带文件扩展名（`.ttf` / `.otf`）的会通过 `FontFile::load_dynamic_font()` 加载字体文件。

#### 系统字体

按名称注册系统字体（不带文件扩展名）：

```gdscript
# 按各平台对应的字体名称注册
UIPackage.registerFont("SimHei", "SimHei")
UIPackage.registerFont("Arial", "Arial")
UIPackage.setDefaultFont("SimHei")
```

不带扩展名的字体名称通过 `SystemFont` 解析，查找系统已安装的字体。

#### 工作原理

在 FairyGUI 编辑器中，为文本对象设置字体名称（如 `"SimHei"` 或 `"myfont"`）。运行时：

1. `UIConfig::getRealFontName()` 将别名解析为实际文件路径或系统字体名
2. 如果解析后的名称以 `.ttf` / `.otf` 结尾 → `FontFile::load_dynamic_font()` 加载文件
3. 否则 → `SystemFont` 按名称匹配系统已安装字体

### 3. 常见 Widget 操作

```gdscript
# 获取/设置位置和大小
comp.setPosition(100, 200)
comp.setSize(300, 200, false)       # w, h, ignorePivot
comp.center()                        # 在父容器中居中
comp.makeFullScreen()

# 透明度与可见性
comp.setAlpha(0.5)
comp.setVisible(true)
comp.setGrayed(false)

# 缩放与旋转
comp.setScale(1.5, 1.5)
comp.setRotation(45.0)

# 轴心点
comp.setPivot(0.5, 0.5, true)       # x, y, asAnchor

# GComponent - 容器组件
comp.numChildren                     # 子节点数量
comp.getChildAt(0)                   # 按索引获取子节点
comp.getChild("btn_ok")              # 按名称查找子节点
comp.addChild(someObj)

# GButton
btn.title = "确定"
btn.selected = false
btn.enabled = true

# GTextField / GLabel
label.text = "Hello FairyGUI"
label.setFontSize(24)
label.setColor(Color.WHITE)

# GLoader - 加载远程图片/组件
loader.url = "ui://Package/ImageName"

# GList - 列表
list.itemRenderer = render_func
list.numItems = 10

# GProgressBar
bar.value = 50.0                     # 0-100
bar.max = 100.0

# GSlider
slider.value = 0.5
slider.max = 1.0
```

### 4. 事件监听

```gdscript
# 点击事件
btn.addClickListener(func(ctx):
    print("按钮被点击")
)

# 滚动事件
list.addEventListener(UIEventType.Scroll, func(ctx):
    print("列表滚动中")
)

# 其他常用事件类型
# fairygui.UIEventType.Click
# fairygui.UIEventType.Changed
# fairygui.UIEventType.TouchBegin / TouchEnd
# fairygui.UIEventType.RollOver / RollOut
# fairygui.UIEventType.Scroll / ScrollEnd
# fairygui.UIEventType.DragStart / DragEnd
# fairygui.UIEventType.PositionChange / SizeChange
# fairygui.UIEventType.GearStop
```

### 5. Controller（控制器）

```gdscript
# 获取组件上的控制器
var ctrl = comp.getController("button")
if ctrl:
    ctrl.selectedIndex = 1           # 切换页面
    print(ctrl.selectedPage)         # 当前页面名称
    print(ctrl.pageCount)            # 页面数量
    print(ctrl.previousIndex)        # 上一次的索引
```

### 6. 屏幕适配

```gdscript
# 在创建 GRoot 之后，设置设计分辨率和缩放模式
var root = GRoot.create(get_tree())

# 设置内容缩放（设计分辨率 1136x640，等比缩放）
root.setContentScaleFactor(1136, 640, GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT)

# 窗口大小变化时，需要调用此方法让 FairyGUI 重新计算缩放和布局
root.onWindowSizeChanged()
```

#### ScreenMatchMode 说明

| 模式 | scale 计算 | GRoot 尺寸 | 效果 |
|------|-----------|-----------|------|
| `MATCH_WIDTH_OR_HEIGHT` | `min(screenW/designW, screenH/designH)` | `(designW, designH)` | 等比缩放，黑边居中，内容始终可见 |
| `MATCH_WIDTH` | `screenW / designW` | `(designW, screenH/scale)` | 宽度固定，高度动态（屏幕越高内容越多） |
| `MATCH_HEIGHT` | `screenH / designH` | `(screenW/scale, designH)` | 高度固定，宽度动态（屏幕越宽内容越多） |
| `MATCH_FILL` | `scaleX=screenW/designW`<br>`scaleY=screenH/designH` | `(designW, designH)` | 非等比拉伸撑满屏幕，无黑边 |

#### GDScript 中监听窗口 resize

```gdscript
# 需要拿到 displayObject 才能监听窗口 resize 事件：
# （FairyGUI 内部 displayObject 的父节点链最终指向 Viewport/Window）

func _ready():
    var root = GRoot.create(get_tree())
    root.setContentScaleFactor(1136, 640, GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT)
    
    # 首次适配
    root.onWindowSizeChanged()
    
    # 监听窗口 resize——拿到 GRoot 的 displayObject，
    # 向上找到 Window 节点，连接 size_changed 信号
    var display_node = root.getDisplayObject()
    if display_node and display_node.is_inside_tree():
        var viewport = display_node.get_viewport()
        if viewport is Window:
            viewport.size_changed.connect(root.onWindowSizeChanged)
```

#### makeFullScreen 的注意事项

`makeFullScreen()` 内部会读取 `GRoot` 的尺寸来设置自身大小。如果在 `_ready()` 中直接调用，此时 `GRoot` 可能还未完成初始化或尺寸尚未设置。

```gdscript
# 初始化流程
var root = GRoot.create(get_tree())
root.setContentScaleFactor(1136, 640, GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT)

# 创建组件后延迟调用 makeFullScreen
var comp = UIPackage.createObject("UI", "MainPanel")
comp.center()
comp.makeFullScreen.call_deferred()  # 等 GRoot 初始化完成后再设置
root.addChild(comp)

# 窗口 resize 时，需要同时刷新 GRoot 适配和全屏组件
func _on_window_size_change() -> void:
    GRoot.getInstance().onWindowSizeChanged()
    main_panel.makeFullScreen()
```

### 7. 拖拽

```gdscript
obj.draggable = true
obj.setDragBounds(Rect2(0, 0, 500, 400))
```

### 7. Relation（关联布局）

```gdscript
# 使子节点与父节点保持相对布局
child.addRelation(parent, fairygui.RelationType.Width_Width)
child.addRelation(parent, fairygui.RelationType.Height_Height)
```

### 8. Transition（过渡动画）

通过 FairyGUI 编辑器设计过渡动画，运行时播放：

```gdscript
comp.getTransition("show").play()
comp.getTransition("hide").play(func():
    print("动画结束")
)
```

### 9. 已注册的 Godot 类

以下类已在 Godot 中注册（方法尚未暴露到 GDScript）：

| 类名 | 继承自 | 说明 |
|---|---|---|
| `fairygui.UIEventDispatcher` | `RefCounted` | 事件分发基类 |
| `fairygui.GController` | `UIEventDispatcher` | 控制器 |
| `fairygui.GObject` | `UIEventDispatcher` | 所有 UI 对象的基类 |
| `fairygui.GComponent` | `GObject` | 容器组件 |
| `fairygui.GImage` | `GObject` | 图片 |
| `fairygui.GGraph` | `GObject` | 图形绘制 |
| `fairygui.GTextField` | `GObject` | 文本基类（抽象） |
| `fairygui.GBasicTextField` | `GTextField` | 基础文本 |
| `fairygui.GRichTextField` | `GTextField` | 富文本 |
| `fairygui.GTextInput` | `GTextField` | 文本输入 |
| `fairygui.GButton` | `GComponent` | 按钮 |
| `fairygui.GLabel` | `GComponent` | 标签（按钮+标题） |
| `fairygui.GComboBox` | `GComponent` | 下拉框 |
| `fairygui.GProgressBar` | `GComponent` | 进度条 |
| `fairygui.GSlider` | `GComponent` | 滑块 |
| `fairygui.GScrollBar` | `GComponent` | 滚动条 |
| `fairygui.GWindow` | `GComponent` | 弹窗 |
| `fairygui.GList` | `GComponent` | 列表 |
| `fairygui.GTree` | `GList` | 树形控件 |
| `fairygui.GTreeNode` | `RefCounted` | 树节点 |
| `fairygui.GLoader` | `GObject` | 加载器 |
| `fairygui.GLoader3D` | `GObject` | Spine 动画 |
| `fairygui.GMovieClip` | `GObject` | 序列帧动画 |
| `fairygui.GGroup` | `GObject` | 编组 |
| `fairygui.GRoot` | `GComponent` | 根节点 |
| `fairygui.FUIContainer` | `Node2D` | 显示容器 |
| `fairygui.FUIInnerContainer` | `FUIContainer` | 内部容器 |
| `fairygui.FUIInput` | `Control` | 输入框控件 |
| `fairygui.FUILabel` | `Node2D` | 标签显示 |
| `fairygui.FUIRichText` | `Node2D` | 富文本显示 |
| `fairygui.FUISprite` | `Sprite2D` | 精灵显示 |
| `fairygui.UIPackage` | `RefCounted` | 包管理器 |
| `fairygui.GPopupMenu` | `RefCounted` | 弹出菜单 |
> **未注册：** `ScrollPane`/`Transition`（需要自定义构造函数）、`DrawNode`（内部类）。
