# FairyGUI for Godot — GDScript 接口使用文档

> 面向后续 AI 与开发者。内容以 `modules/fairygui/src` 的 ClassDB 绑定和 `modules/fairygui/examples/gd` 为准。
> 适用：Godot 4.5+ + FairyGUI 内置模块。

## 0. 最重要的事实（先读）

1. **FairyGUI 的 GDScript 方法名是 camelCase**（`addPackage`、`setPosition`、`numChildren`），不是 Godot 常见的 snake_case。不要臆造 `add_package`、`get_child` 之类的方法。
2. **事件常量全大写**：`UIEventDispatcher.CLICK`、`UIEventDispatcher.CLICKITEM`、`GuiObject.RIGHT_RIGHT`、`GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT`。
3. **`numChildren()` 是方法，不是属性**；`viewWidth` 是属性（也可 `getViewWidth()`）。
4. **`GButton.title` 不存在**，用 `setTitle()/getTitle()`；`GTextField.text` 用 `setText()/getText()`。
5. **`icon`、`selected`、`selectedIndex`、`value`、`contentPane`、`modal`** 等是注册过的属性，可直接读写，也可调用对应 setter。
6. **场景根节点必须是 `Node`，不能是 `Control`**。`Control` 默认 `mouse_filter = STOP`，会截断输入，FairyGUI 收不到点击。
7. **`GWindow` 用 `extends GWindow` 继承**，在 `_init()` 中调用 `setupDisplay()` 并设置回调。
8. **`addEventListener` 的回调可以带 0 个或 1 个参数**（1 个参数为 `FGUIEventContext`）；`addClickListener` 回调通常无参。
9. **切换场景前调用 `GRoot.cleanup()`**，并在下一场景 `_ready` 重新 `GRoot.create()`。
10. **UI 包路径不带 `.fui` 扩展名**。

## 1. 初始化

### 1.1 创建 GRoot

```gdscript
func _ready() -> void:
    # _ready / _enter_tree 中用 createDeferred，避免 "Parent node is busy setting up children"
    GRoot.createDeferred(get_tree())
    # 也可以（节点已就绪时）：
    # GRoot.create(get_tree())
```

静态方法：

| 方法 | 说明 |
|---|---|
| `GRoot.create(tree: SceneTree, z_order: int = 1000) -> GRoot` | 创建并挂载 |
| `GRoot.createDeferred(tree, z_order = 1000) -> GRoot` | 延迟一帧挂载 |
| `GRoot.getInstance() -> GRoot` | 单例 |
| `GRoot.cleanup()` | 卸载显示层 |

### 1.2 加载 UI 包并创建对象

```gdscript
func _ready() -> void:
    GRoot.createDeferred(get_tree())

func _make_ui() -> void:
    UIPackage.addPackage("res://Resources/UI/MainMenu")

    var view = UIPackage.createObject("MainMenu", "Main")
    GRoot.getInstance().addChild(view)

    # 按 URL 创建
    var btn = UIPackage.createObjectFromURL("ui://MainMenu/MyButton")
```

| 静态方法 | 说明 |
|---|---|
| `UIPackage.addPackage(asset_path) -> UIPackage` | 加载 `.fui`（不带扩展名） |
| `UIPackage.createObject(pkg_name, res_name) -> Object` | 创建组件 |
| `UIPackage.createObjectFromURL(url) -> Object` | 按 `ui://` 创建 |
| `UIPackage.getById(id)` / `UIPackage.getByName(name)` | 查找包 |
| `UIPackage.removePackage(id_or_name)` / `UIPackage.removeAllPackages()` | 卸载 |
| `UIPackage.getItemURL(pkg_name, res_name)` | 生成 URL |
| `UIPackage.registerFont(alias, real)` | 注册字体 |
| `UIPackage.setDefaultFont(name)` | 默认字体 |

若编辑器开启 branch 导出，组件名会带分支前缀（如 `7iys1/Menu`），`createObject` 需使用完整名称。

### 1.3 字体

```gdscript
var ui := UIConfigHelper.getInstance()
ui.registerFont("default", "res://Resources/fonts/NotoSansSC-Regular.ttf")
ui.registerFont("msyh", "res://fonts/msyh.ttf")   # .ttf/.otf -> 文件字体
ui.registerFont("SimHei", "SimHei")               # 无扩展名 -> 系统字体
ui.setDefaultFont("default")
# 或：ui.defaultFont = "default"
```

BMFont（编辑器导出的 `.fnt`）随包自动加载。

### 1.4 滚动条（必须显式注册）

```gdscript
UIPackage.addPackage("res://Resources/UI/Basics")
var ui := UIConfigHelper.getInstance()
ui.verticalScrollBar = "ui://Basics/ScrollBar_VT"
ui.horizontalScrollBar = "ui://Basics/ScrollBar_HZ"
# 或 ui.setVerticalScrollBar(...) / ui.setHorizontalScrollBar(...)
```

必须在创建任何 `overflow="scroll"` 的组件之前完成。

### 1.5 屏幕适配

```gdscript
var root := GRoot.create(get_tree())
root.setContentScaleFactor(1136, 640, GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT)
root.onWindowSizeChanged()

# 监听窗口尺寸变化
var display := root.getDisplayObject()
if display != null and display.is_inside_tree():
    var viewport := display.get_viewport()
    if viewport is Window:
        viewport.size_changed.connect(root.onWindowSizeChanged)
```

| 模式 | 值 | 说明 |
|---|---:|---|
| `GRoot.ScreenMatchMode.MATCH_WIDTH_OR_HEIGHT` | 0 | 等比缩放，留黑边 |
| `GRoot.ScreenMatchMode.MATCH_WIDTH` | 1 | 固定宽度、动态高度 |
| `GRoot.ScreenMatchMode.MATCH_HEIGHT` | 2 | 固定高度、动态宽度 |
| `GRoot.ScreenMatchMode.MATCH_FILL` | 3 | 非等比拉伸铺满 |

## 2. 事件

### 2.1 事件常量（`UIEventDispatcher`）

| 含义 | 值 | GDScript 常量 |
|---|---:|---|
| Enter | 0 | `UIEventDispatcher.ENTER` |
| Exit | 1 | `UIEventDispatcher.EXIT` |
| Changed | 2 | `UIEventDispatcher.CHANGED` |
| Submit | 3 | `UIEventDispatcher.SUBMIT` |
| TouchBegin | 10 | `UIEventDispatcher.TOUCHBEGIN` |
| TouchMove | 11 | `UIEventDispatcher.TOUCHMOVE` |
| TouchEnd | 12 | `UIEventDispatcher.TOUCHEND` |
| Click | 13 | `UIEventDispatcher.CLICK` |
| RollOver | 14 | `UIEventDispatcher.ROLLOVER` |
| RollOut | 15 | `UIEventDispatcher.ROLLOUT` |
| MouseWheel | 16 | `UIEventDispatcher.MOUSEWHEEL` |
| RightClick | 17 | `UIEventDispatcher.RIGHTCLICK` |
| MiddleClick | 18 | `UIEventDispatcher.MIDDLECLICK` |
| KeyDown | 30 | `UIEventDispatcher.KEYDOWN` |
| KeyUp | 31 | `UIEventDispatcher.KEYUP` |
| Scroll | 40 | `UIEventDispatcher.SCROLL` |
| ScrollEnd | 41 | `UIEventDispatcher.SCROLLEND` |
| PullDownRelease | 42 | `UIEventDispatcher.PULLDOWNRELEASE` |
| PullUpRelease | 43 | `UIEventDispatcher.PULLUPRELEASE` |
| PositionChange | 20 | `UIEventDispatcher.POSITIONCHANGE` |
| SizeChange | 21 | `UIEventDispatcher.SIZECHANGE` |
| ClickItem | 50 | `UIEventDispatcher.CLICKITEM` |
| ClickLink | 51 | `UIEventDispatcher.CLICKLINK` |
| ClickMenu | 52 | `UIEventDispatcher.CLICKMENU` |
| RightClickItem | 53 | `UIEventDispatcher.RIGHTCLICKITEM` |
| DragStart | 60 | `UIEventDispatcher.DRAGSTART` |
| DragMove | 61 | `UIEventDispatcher.DRAGMOVE` |
| DragEnd | 62 | `UIEventDispatcher.DRAGEND` |
| Drop | 63 | `UIEventDispatcher.DROP` |
| GearStop | 70 | `UIEventDispatcher.GEARSTOP` |

### 2.2 添加/移除监听

```gdscript
# 点击（无参回调）
btn.addClickListener(_on_click)
btn.addClickListener(func(): print("click"))

# 任意事件，带上下文
list.addEventListener(UIEventDispatcher.CLICKITEM, _on_click_item)

func _on_click_item(ctx = null) -> void:
    var item: Object = null
    if ctx != null:
        item = ctx.getData()           # 常见为 GuiObject
        # 或 var text: String = ctx.getItemText()
    if item == null:
        item = GRoot.getInstance().getTouchTarget()
    if item == null:
        return
    var node = item.getTreeNode()

# 移除该事件类型的全部监听
list.removeEventListener(UIEventDispatcher.CLICKITEM)
```

`FGUIEventContext` 方法：

| 方法 | 说明 |
|---|---|
| `preventDefault()` | 阻止默认行为 |
| `getData()` | 事件数据（常为 `GuiObject`） |
| `getItemText()` | 列表项文本 |
| `getTouchId()` | 触点 id |

> 回调既可以写 0 参方法，也可以写 1 参方法接收 `FGUIEventContext`；绑定层会在参数过多时回退到 0 参调用。

### 2.3 拖拽

```gdscript
obj.setDraggable(true)
obj.setDragBounds(Rect2(0, 0, 500, 400))

obj.addEventListener(UIEventDispatcher.DRAGSTART, func(ctx):
    ctx.preventDefault()
    DragDropManagerHelper.getInstance().startDrag(
        obj.getIcon(),
        obj.getIcon(),
        ctx.getTouchId(),
    )
)

target.addEventListener(UIEventDispatcher.DROP, func(ctx):
    target.setIcon(str(ctx.getData()))
)
```

## 3. 类层次

```
UIEventDispatcher (RefCounted)
├─ GController
└─ GuiObject
   ├─ GComponent
   │  ├─ GRoot
   │  ├─ GButton / GLabel / GComboBox / GProgressBar / GSlider / GScrollBar / GWindow
   │  └─ GList ── GTree
   ├─ GImage / GGraph / GGroup / GLoader / GLoader3D / GMovieClip
   └─ GTextField (abstract) ── GBasicTextField / GRichTextField / GTextInput
```

`GTreeNode` 继承 `RefCounted`；`ScrollPane`、`Transition`、`GTweener`、`GTweenHelper`、`UIPackage`、`GController`、`UIEventDispatcher`、`FGUIEventContext` 也是 `RefCounted`。

## 4. GuiObject（所有 UI 对象基类）

### 4.1 变换与外观

| 方法/属性 | 说明 |
|---|---|
| `getX()` / `setX(v)` | X 坐标 |
| `getY()` / `setY(v)` | Y 坐标 |
| `getPosition() -> Vector2` / `setPosition(x, y)` | 位置 |
| `getWidth()` / `setWidth(v)` / `getHeight()` / `setHeight(v)` | 宽高 |
| `getSize() -> Vector2` / `setSize(w, h, ignore_pivot = false)` | 尺寸 |
| `getInitSize() -> Vector2` | 初始尺寸 |
| `setPivot(x, y, as_anchor = false)` / `getPivot()` | 轴心 |
| `setScale(x, y)` / `getScale()` / `setScaleX` / `getScaleY` | 缩放 |
| `setRotation(v)` / `getRotation()` | 旋转 |
| `setAlpha(v)` / `getAlpha()` | 透明度 |
| `setGrayed(v)` / `isGrayed()` | 灰度 |
| `setVisible(v)` / `isVisible()` | 可见 |
| `setTouchable(v)` / `isTouchable()` | 可触摸 |
| `setSortingOrder(v)` / `getSortingOrder()` | 显示层级 |
| `center(restraint = false)` | 父容器居中 |
| `makeFullScreen()` | 铺满 GRoot |
| `setPixelSnapping(v)` / `isPixelSnapping()` | 像素对齐 |
| `setSkewX(v)` / `getSkewX()` / `setSkewY(v)` / `getSkewY()` | 斜切 |

### 4.2 文本/图标/名称

| 方法/属性 | 说明 |
|---|---|
| `setText(text)` / `getText()` | 文本 |
| `setName(name)` / `getName()` | 名称 |
| `icon`（属性）/ `setIcon(v)` / `getIcon()` | 图标 URL |
| `setTooltips(text)` / `getTooltips()` | 悬浮提示 |
| `getResourceURL()` | 资源 URL |

> `GButton.title`、`GTextField.text` **不是**注册属性，请用 `setTitle()` / `setText()`。

### 4.3 层级与关系

| 方法/属性 | 说明 |
|---|---|
| `getParent() -> GComponent` | 父组件 |
| `removeFromParent()` | 从父移除 |
| `onStage() -> bool` | 是否在舞台 |
| `group`（属性）/ `getGroup()` / `setGroup(g)` | 所属 GGroup |
| `addRelation(target, relation_type, use_percent = false)` | 添加布局关系 |
| `localToGlobal(pt)` / `globalToLocal(pt)` | 坐标转换 |
| `transformRect(rect, target_space) -> Rect2` | 矩形坐标变换 |
| `getTreeNode() -> GTreeNode` | 对应树节点 |

RelationType 常量（`GuiObject.XXX`）：

| 常量 | 值 | 常量 | 值 |
|---|---:|---|---:|
| `GuiObject.LEFT_LEFT` | 0 | `GuiObject.TOP_TOP` | 7 |
| `GuiObject.LEFT_CENTER` | 1 | `GuiObject.TOP_MIDDLE` | 8 |
| `GuiObject.LEFT_RIGHT` | 2 | `GuiObject.TOP_BOTTOM` | 9 |
| `GuiObject.CENTER_CENTER` | 3 | `GuiObject.MIDDLE_MIDDLE` | 10 |
| `GuiObject.RIGHT_LEFT` | 4 | `GuiObject.BOTTOM_TOP` | 11 |
| `GuiObject.RIGHT_CENTER` | 5 | `GuiObject.BOTTOM_MIDDLE` | 12 |
| `GuiObject.RIGHT_RIGHT` | 6 | `GuiObject.BOTTOM_BOTTOM` | 13 |
| `GuiObject.WIDTH` | 14 | `GuiObject.HEIGHT` | 15 |

```gdscript
child.addRelation(parent, GuiObject.WIDTH, false)
child.addRelation(parent, GuiObject.HEIGHT, false)
```

### 4.4 事件与拖拽

| 方法 | 说明 |
|---|---|
| `addClickListener(callable)` | 点击 |
| `addEventListener(type, callable)` | 任意事件 |
| `removeEventListener(type)` | 移除该类型监听 |
| `setDraggable(v)` / `isDraggable()` | 拖拽开关 |
| `setDragBounds(rect)` / `getDragBounds()` | 拖拽范围 |
| `startDrag(touch_id = -1)` / `stopDrag()` | 手动拖拽 |
| `tweenScale(end_value: Vector2, duration: float) -> GTweener` | 缩放补间 |

## 5. GComponent（容器）

| 方法/属性 | 说明 |
|---|---|
| `getChild(name) -> Object` | 按名称 |
| `getChildAt(index) -> Object` | 按索引 |
| `getChildByPath(path)` / `getChildById(id)` | 按路径/id |
| `getChildIndex(child)` / `setChildIndex(child, index)` | 索引 |
| `swapChildren(a, b)` / `swapChildrenAt(i, j)` | 交换 |
| `numChildren() -> int` | 子节点数量（方法） |
| `addChild(child)` | 添加 |
| `addChildAt(child, index)` | 插入 |
| `removeChildAt(index)` | 按索引移除 |
| `removeChildren(begin_index = 0, end_index = -1)` | 批量移除 |
| `getController(name) -> GController` | 控制器 |
| `getControllerAt(index)` / `addController(c)` / `removeController(c)` | 控制器管理 |
| `applyController(c)` / `applyAllControllers()` | 应用 |
| `getTransition(name) -> Transition` | 过渡 |
| `getScrollPane() -> ScrollPane` | 滚动面板 |
| `viewWidth` / `viewHeight`（属性）/ `getViewWidth()` / `getViewHeight()` | 视口尺寸 |
| `setOpaque(v)` / `getOpaque()` | 不透明 |
| `ensureBoundsCorrect()` | 修正边界 |
| `isChildInView(child)` / `isAncestorOf(obj)` | 可见/祖先判断 |

示例：

```gdscript
var view = UIPackage.createObject("Basics", "Main")
_groot.addChild(view)

var back_btn = view.getChild("btn_Back")
back_btn.setVisible(false)
back_btn.addClickListener(func(): print("back"))

var container = view.getChild("container") as GComponent
var ctrl = view.getController("c1")
ctrl.setSelectedIndex(1)
```

## 6. GController

| 方法/属性 | 说明 |
|---|---|
| `selectedIndex`（属性）/ `setSelectedIndex(i)` / `getSelectedIndex()` | 当前页索引 |
| `getPageCount() -> int` | 页数 |
| `getPreviousPage() -> String` / `getPreviousPageId() -> String` | 上一页 |
| `setSelectedPage(name)` / `getSelectedPage()` | 当前页名 |
| `setSelectedPageId(id)` / `getSelectedPageId()` | 当前页 id |
| `getParent() -> GComponent` | 所属组件 |

```gdscript
var ctrl = comp.getController("button")
if ctrl != null:
    ctrl.selectedIndex = 1
    print(ctrl.getPageCount(), ctrl.getPreviousPage())
```

## 7. 常用控件

### 7.1 GButton

| 方法/属性 | 说明 |
|---|---|
| `setTitle(t)` / `getTitle()` | 标题 |
| `setSelectedTitle(t)` / `getSelectedTitle()` | 选中标题 |
| `selected`（属性）/ `setSelected(v)` / `isSelected()` | 选中 |
| `setSelectedIcon(icon)` / `getSelectedIcon()` | 选中图标 |
| `titleColor`（属性）/ `getTitleColor()` | 标题颜色 |
| `titleFontSize`（属性）/ `getTitleFontSize()` | 标题字号 |
| `changeStateOnClick`（属性） | 点击切换状态 |
| `getRelatedController()` / `setRelatedController(c)` | 关联控制器 |

### 7.2 GLabel

`setTitle/getTitle`、`setTitleColor/getTitleColor`、`setTitleFontSize/getTitleFontSize`。

### 7.3 GComboBox

`setTitle/getTitle`、`titleColor`、`titleFontSize`、`selectedIndex`、`setValue/getValue`、`getDropdown()`、`refresh()`。

### 7.4 GProgressBar

| 方法/属性 | 说明 |
|---|---|
| `min` / `max` / `value`（属性） | 范围与当前值 |
| `setMin/getMin`、`setMax/getMax`、`setValue/getValue` | 同上 |
| `tweenValue(value, duration)` | 动画到目标值 |

```gdscript
var bar := view.getChild("bar") as GProgressBar
bar.value = 50
bar.tweenValue(100, 2.0)
```

### 7.5 GSlider

`min` / `max` / `value` / `wholeNumbers`（属性）及对应 setter/getter。

### 7.6 GScrollBar

`setDisplayPerc(v)`、`setScrollPerc(v)`、`getMinSize()`。

### 7.7 GTextField / GRichTextField / GTextInput

| 方法 | 说明 |
|---|---|
| `setColor(c)` / `getColor()` | 颜色 |
| `setFontSize(n)` / `getFontSize()` | 字号 |
| `setSingleLine(v)` / `isSingleLine()` | 单行 |
| `setAutoSize(v)` / `getAutoSize()` | 自动尺寸（0 NONE / 1 BOTH / 2 HEIGHT / 3 SHRINK） |
| `setUBBEnabled(v)` / `isUBBEnabled()` | UBB 解析 |
| `setTemplateVars(dict)` | 模板变量 |
| `getTextSize() -> Vector2` | 文本尺寸 |

`GTextInput`：`setPrompt`、`setPassword`、`setKeyboardType`、`setMaxLength`。

富文本示例（`ChatScene.gd`）：

```gdscript
var tf = obj.getChild("msg")
if tf is GRichTextField:
    tf.setUBBEnabled(false)
    tf.setWidth(tf.getInitSize().x)
    tf.setText("<img src='ui://Emoji/r0'/> Hello")
    tf.setWidth(tf.getTextSize().x)
```

### 7.8 GLoader / GLoader3D

- `GLoader`：`setURL(url)` / `getURL()`、`align`、`verticalAlign`、`autoSize`、`fill`、`shrinkOnly`、`playing`、`frame`、`fillMethod`、`fillOrigin`、`fillAmount`、`imageColor`、`getComponent()`。
- `GLoader3D`：`setURL`、`setAnimationName` / `getPlayingAnimationName`、`setSkinName` / `getSkinName`、`isPlaying`、`isLoop` / `setLoop`、`setPlaying`、`setFrame` / `getFrame`、`play(name)`、`stop()`、`getContent` / `setContent`、`color`。

### 7.9 GImage / GGraph / GMovieClip

- `GImage`：`flip`、`imageColor`、`fillMethod`、`fillOrigin`、`fillClockwise`、`fillAmount`。
- `GGraph`：`drawRect(w, h, line_size, line_color, fill_color)`、`drawEllipse(...)`、`isEmpty()`、`color`。
- `GMovieClip`：`setPlaying/isPlaying`、`setFrame/getFrame`、`setTimeScale/getTimeScale`、`advance(time)`、`setFlip/getFlip`、`setColor/getColor`。

FillMethod：`None=0, Horizontal=1, Vertical=2, Radial90=3, Radial180=4, Radial360=5`。
FillOrigin：`Top=0, Bottom=1, Left=2, Right=3`。

### 7.10 GGroup

`layout`（0 NONE / 1 HORIZONTAL / 2 VERTICAL）、`setColumnGap/getColumnGap`、`setLineGap/getLineGap`、`setExcludeInvisibles/isExcludeInvisibles`、`setAutoSizeDisabled/isAutoSizeDisabled`、`setMainGridIndex/getMainGridIndex`、`setMainGridMinSize/getMainGridMinSize`、`ensureBoundsCorrect()`。

读取组名：

```gdscript
var grp = obj.getGroup()
if grp != null and grp.getName() == "btns":
    print("in group btns")
```

## 8. GList（列表）

| 方法/属性 | 说明 |
|---|---|
| `setItemRenderer(callable)` | 渲染回调 `func(index: int, obj: Object)` |
| `setItemProvider(callable)` | 虚拟列表资源回调 `func(index: int) -> String` |
| `setVirtual()` / `setVirtualAndLoop()` | 虚拟/循环虚拟 |
| `setNumItems(n)` / `getNumItems()` | 项目数量 |
| `getFirstChildInView()` | 可视区首个索引 |
| `addSelection(index, scroll_to_view = false)` | 选中 |
| `clearSelection(trigger_event = true)` | 清空选择 |
| `selectedIndex`（属性）/ `setSelectedIndex(i)` / `getSelectedIndex()` | 选中索引 |
| `setSelectionController(c)` / `getSelectionController()` | 关联控制器 |
| `addItemFromPool() -> Object` | 从池取 item（无参） |
| `removeChildrenToPool(begin = 0, end = -1)` | 回收 item |
| `scrollToView(index, ani = false, set_first = false)` | 滚动到项 |
| `layout` / `columnCount` / `lineCount` / `columnGap` / `lineGap` / `autoResizeItem` | 布局配置 |
| `ensureBoundsCorrect()` / `isChildInView(item)` / `getScrollPane()` | 继承自 GComponent |

布局值：`SINGLE_COLUMN=0, SINGLE_ROW=1, FLOW_HORIZONTAL=2, FLOW_VERTICAL=3, PAGINATION=4`。
选择模式：`SINGLE=0, MULTIPLE=1, MULTIPLE_SINGLECLICK=2, NONE=3`。

虚拟列表示例（`VirtualListScene.gd`）：

```gdscript
func continue_init() -> void:
    UIPackage.addPackage("res://Resources/UI/VirtualList")
    _view = UIPackage.createObject("VirtualList", "Main")
    _groot.addChild(_view)

    _list = _view.getChild("mailList")
    if _list != null:
        _list.setItemRenderer(_render_list_item)
        _list.setVirtual()
        _list.setNumItems(1000)

func _render_list_item(index: int, obj: Object) -> void:
    var read_ctrl = obj.getController("IsRead")
    if read_ctrl != null:
        read_ctrl.setSelectedIndex(0 if index % 2 == 0 else 1)
    obj.setText(str(index) + " Mail title here")
```

带 `itemProvider`：

```gdscript
_list.setItemRenderer(_render_list_item)
_list.setItemProvider(_get_list_item_resource)
_list.setVirtual()

func _get_list_item_resource(index: int) -> String:
    return "ui://Emoji/chatRight" if _messages[index].from_me else "ui://Emoji/chatLeft"
```

下拉刷新/上拉加载：监听 `UIEventDispatcher.PULLDOWNRELEASE` / `PULLUPRELEASE`，配合 `ScrollPane.getHeader()/getFooter()`、`lockHeader()/lockFooter()`，见 `PullToRefreshScene.gd`。

## 9. GTree / GTreeNode

### GTree

| 方法/属性 | 说明 |
|---|---|
| `getRootNode() -> GTreeNode` | 根节点 |
| `getSelectedNode() -> GTreeNode` | 当前选中 |
| `selectNode(node, scroll_to_view = false)` / `unselectNode(node)` | 选中/取消 |
| `expandAll(folder_node)` / `collapseAll(folder_node)` | 展开/收起 |
| `indent` / `clickToExpand` | 缩进/点击展开 |
| `setTreeNodeRender(callable)` | 渲染回调 `func(node: GTreeNode, obj: Object)` |
| 其余 | 继承 `GList` |

### GTreeNode

`GTreeNode.new()`；`init(is_folder, res_url = "")`、`setText/getText`、`setIcon/getIcon`、`isFolder()`、`isExpanded()/setExpanded(v)`、`getParent()`、`getTree()`、`getCell()`、`getData()/setData(v)`、`getChildAt(i)`、`numChildren()`、`addChild(node)`。

```gdscript
var root_node = _tree2.getRootNode()
var top_node = GTreeNode.new()
top_node.init(true, "")
top_node.setText("I'm a top node")
root_node.addChild(top_node)

for i in range(5):
    var node = GTreeNode.new()
    node.setText("Hello " + str(i))
    top_node.addChild(node)
```

点击节点：

```gdscript
_tree2.addEventListener(UIEventDispatcher.CLICKITEM, _on_click_node)

func _on_click_node(ctx = null) -> void:
    var item: Object = null
    if ctx != null:
        item = ctx.getData()
    if item == null:
        item = _groot.getTouchTarget()
    if item != null:
        var node = item.getTreeNode()
        if node != null:
            print("click node ", node.getText())
```

## 10. GWindow（窗口）

### 10.1 继承方式（GDScript 推荐）

```gdscript
extends GWindow

func _init() -> void:
    setupDisplay()
    setOnInitCallback(_on_init)
    setOnShownCallback(_on_shown)
    setDoShowAnimationCallback(_do_show_animation)
    setDoHideAnimationCallback(_do_hide_animation)
    setOnHideCallback(_on_hide)

func _on_init() -> void:
    setContentPane(UIPackage.createObject("Basics", "WindowA"))
    center()
    setModal(true)

func _on_shown() -> void:
    var list = getContentPane().getChild("n6")
    if list != null:
        list.removeChildrenToPool()
        for i in range(6):
            var item = list.addItemFromPool()
            item.setTitle(str(i))
            item.setIcon("ui://Basics/r4")

func _do_show_animation() -> void:
    setScale(0.1, 0.1)
    setPivot(0.5, 0.5)
    tweenScale(Vector2(1.0, 1.0), 0.3).onComplete(_on_shown)

func _do_hide_animation() -> void:
    GTweenHelper.getInstance().kill(self, false)
    tweenScale(Vector2(0.1, 0.1), 0.3).onComplete(hideImmediately)

func _on_hide() -> void:
    var trans = getContentPane().getTransition("t1")
    if trans != null:
        trans.stop()
```

### 10.2 直接创建

```gdscript
var win := GWindow.new()
win.setupDisplay()
win.setOnInitCallback(_on_test_win_init)
win.show()

func _on_test_win_init() -> void:
    win.setContentPane(UIPackage.createObject("ModalWaiting", "TestWin"))
```

### 10.3 API

| 方法/属性 | 说明 |
|---|---|
| `show()` / `hide()` / `hideImmediately()` | 显示/隐藏 |
| `toggleStatus()` / `bringToFront()` | 切换/置顶 |
| `isShowing()` / `isTop()` | 状态 |
| `modal`（属性）/ `setModal(v)` / `isModal()` | 模态 |
| `contentPane`（属性）/ `getContentPane()` / `setContentPane(pane)` | 内容面板 |
| `getFrame()` | 框架 |
| `closeButton` / `dragArea` / `contentArea` | 关闭按钮/拖拽区/内容区 |
| `showModalWait(requesting_cmd = 0)` / `closeModalWait(requesting_cmd = 0)` | 窗口等待层 |
| `initWindow()` | 手动初始化 |
| `setOnInitCallback` / `setOnShownCallback` / `setOnHideCallback` | 生命周期 |
| `setDoShowAnimationCallback` / `setDoHideAnimationCallback` | 动画回调 |
| `getModalWaitingPane()` | 等待面板 |

静态 `GWindow.create()` 也可用。

全局模态等待：

```gdscript
UIConfigHelper.getInstance().setGlobalModalWaiting("ui://ModalWaiting/GlobalModalWaiting")
UIConfigHelper.getInstance().setWindowModalWaiting("ui://ModalWaiting/WindowModalWaiting")

_groot.showModalWait()
await get_tree().create_timer(3.0).timeout
_groot.closeModalWait()
```

## 11. GPopupMenu

| 方法 | 说明 |
|---|---|
| `GPopupMenu.create() -> GPopupMenu` | 创建 |
| `addItem(caption) -> GButton` | 添加项 |
| `addSeperator()` | 分隔线 |
| `clearItems()` | 清空 |
| `getItemCount() -> int` | 数量 |
| `getList() -> GList` | 内部列表 |
| `getContentPane() -> GComponent` | 内容面板 |
| `getItemName(index) -> String` | 项名 |
| `setItemText(name, caption)` | 改文本 |
| `setItemVisible(name, visible)` | 显隐 |
| `setItemGrayed(name, grayed)` | 置灰 |
| `setItemCheckable(name, checkable)` / `setItemChecked(name, checked)` / `isItemChecked(name)` | 勾选 |
| `removeItem(name) -> bool` | 移除 |
| `show()` | 显示 |
| `showMenuAt(target, dir)` | 目标旁显示（0 AUTO / 1 UP / 2 DOWN） |

```gdscript
var pm = GPopupMenu.create()
pm.addItem("Item 1")
pm.addItem("Item 2")
pm.showMenuAt(n0, 2)
```

## 12. ScrollPane

| 方法 | 说明 |
|---|---|
| `getPosX()` / `setPosX(value, ani = false)` | 水平位置 |
| `getPosY()` / `setPosY(value, ani = false)` | 垂直位置 |
| `getScrollingPosX()` / `getScrollingPosY()` | 滚动中的位置 |
| `scrollTop(ani = false)` / `scrollBottom(ani = false)` | 到顶/到底 |
| `isBottomMost()` | 是否到底 |
| `getHeader()` / `getFooter()` | 头/尾组件 |
| `lockHeader(size)` / `lockFooter(size)` | 锁定头/尾 |
| `cancelDragging()` | 取消拖动 |
| `refresh()` / `refreshScrollBars()` | 刷新 |

## 13. Transition

| 方法 | 说明 |
|---|---|
| `play(times = 1, delay = 0.0, callback = Callable())` | 播放 |
| `stop()` | 停止 |
| `isPlaying()` | 是否播放中 |
| `getOwner()` | 所属组件 |

```gdscript
comp.getTransition("show").play(1, 0, func(): print("done"))
# 不需要回调时也可：
comp.getTransition("show").play()
```

## 14. Tween（GTweenHelper / GTweener）

### GTweenHelper

| 方法 | 说明 |
|---|---|
| `GTweenHelper.getInstance()` | 单例 |
| `toFloat(start, end, duration)` | 浮点补间 |
| `toVec2(start, end, duration)` | Vector2 补间 |
| `toVec3` / `toVec4` / `toColor` / `toDouble` | 其他类型 |
| `delayedCall(delay)` | 延迟调用 |
| `shake(start, amplitude, duration)` | 抖动 |
| `isTweening(target)` | 是否补间中 |
| `kill(target, complete = false)` | 杀掉目标补间 |
| `clean()` / `killAll(complete = false)` | 清理 |

### GTweener

`setDelay`、`setDuration`、`setEase`、`setRepeat(repeat, yoyo = false)`、`setTimeScale`、`setSnapping`、`setTarget(target, prop_type = 0)`、`setPaused`、`seek(time)`、`kill(complete = false)`、`getNormalizedTime()`、`isCompleted()`、`allCompleted()`、`onStart(callable)`、`onUpdate(callable)`、`onComplete(callable)`。

`GTweener.TweenPropType`：`NONE=0, X=1, Y=2, POSITION=3, WIDTH=4, HEIGHT=5, SIZE=6, SCALE_X=7, SCALE_Y=8, SCALE=9, ROTATION=10, ALPHA=11, PROGRESS=12`。

```gdscript
GTweenHelper.getInstance().toVec2(window.getPosition(), rect.position, 0.5) \
    .setTarget(window, GTweener.TweenPropType.POSITION)
```

## 15. UIConfigHelper（全局配置）

`UIConfigHelper.getInstance()`。

| 属性 / setter | 说明 |
|---|---|
| `defaultFont` / `setDefaultFont` | 默认字体别名 |
| `buttonSound` / `setButtonSound` | 按钮音效 URL |
| `buttonSoundVolumeScale` | 按钮音量 |
| `defaultScrollStep` | 滚动步长 |
| `defaultScrollDecelerationRate` | 减速率 |
| `defaultScrollTouchEffect` | 触摸滚动 |
| `defaultScrollBounceEffect` | 回弹 |
| `defaultScrollBarDisplay` | 滚动条显示策略（0 DEFAULT / 1 VISIBLE / 2 AUTO / 3 HIDDEN） |
| `verticalScrollBar` / `horizontalScrollBar` | 滚动条 URL |
| `touchDragSensitivity` / `clickDragSensitivity` / `touchScrollSensitivity` | 灵敏度 |
| `defaultComboBoxVisibleItemCount` | 下拉可见项数 |
| `globalModalWaiting` / `windowModalWaiting` | 模态等待 URL |
| `modalLayerColor` | 模态层颜色 |
| `tooltipsWin` | 提示组件 URL |
| `bringWindowToFrontOnClick` | 点击置顶 |
| `popupMenu` / `popupMenuSeperator` | 弹出菜单/分隔线 |
| `registerFont(alias, real)` | 注册字体 |
| `getRealFontName(alias)` | 解析字体 |

## 16. 其它 Helper

### DragDropManagerHelper

`getInstance()`、`startDrag(icon, source_data = null, touch_point_id = -1)`、`cancel()`、`isDragging()`。

### UIObjectFactoryHelper

`getInstance().setPackageItemExtension(url, creator)`：为 `ui://` 资源注册自定义扩展，creator 返回 `GComponent`。

### FGUIHelper

`getInstance()`、`getGRoot()`、`createObject(pkg, res)`、`addPackage(path)`、`loadTranslation(xml_path)`、`loadTranslationFromXML(xml)`、`setHtmlButtonResource(url)`、`setHtmlInputResource(url)`、`setHtmlSelectResource(url)`。

## 17. 场景切换与清理

```gdscript
func _prepare_for_scene_change() -> void:
    if DragDropManagerHelper.getInstance().isDragging():
        DragDropManagerHelper.getInstance().cancel()
    _groot.hideTooltips()
    _groot.hidePopup()
    _groot.closeModalWait()
    _groot.closeAllWindows()
    _groot.removeChildren()

    # 切场景
    GRoot.cleanup()
    get_tree().change_scene_to_file("res://gd/Scenes/MainMenu.tscn")
```

推荐分帧执行（`call_deferred`）以避免在事件回调栈内同步清理节点。

## 18. 资源导出

Include Filters 至少包含：

```
*.fui, Resources/UI/*, Resources/fonts/*
```

## 19. 与 TypeScript/C# 的差异速查

| 项目 | GDScript | TS | C# |
|---|---|---|---|
| 方法命名 | camelCase | camelCase | PascalCase |
| 回调 | 方法引用/`func` | `Callable.create(fn)` | `Callable.From(...)` / `new Callable(this, MethodName.X)` |
| 事件常量 | `UIEventDispatcher.CLICKITEM` | 同 GD | `UIEventDispatcher.Clickitem`（或 `FguiEvent.ClickItem`） |
| 关系常量 | `GuiObject.RIGHT_RIGHT` | `FguiRelationType.RightRight` | `GuiObject.RelationType.RightRight` |
| Window | `extends GWindow` | 组合 `new GWindow()` | `partial class X : GWindow` |
| 属性 | 属性或 setter | 用属性 | 属性或 setter |

完整示例见 `modules/fairygui/examples/gd/Scripts/`：`MainMenu.gd`、`BasicsScene.gd`、`VirtualListScene.gd`、`LoopListScene.gd`、`TreeViewScene.gd`、`ScrollPaneScene.gd`、`PullToRefreshScene.gd`、`ModalWaitingScene.gd`、`GuideScene.gd`、`CooldownScene.gd`、`ChatScene.gd`、`BagWindow.gd`、`Window1.gd`、`Window2.gd`、`JoystickModule.gd`。
