# FairyGUI for Godot — TypeScript / GodotJS 接口使用文档

> 面向后续 AI 与开发者。内容以 `modules/fairygui/src` 的绑定实现和 `modules/fairygui/examples/ts` 为准。
> 适用：Godot 4.5+ + GodotJS 模块 + FairyGUI 内置模块。

## 0. 最重要的事实（先读）

1. **FairyGUI 类型不是 `"godot"` 模块的具名导出**。运行时需要 `import "./fgui-globals";`，它会把 ClassDB 类型注册到 `globalThis`。
2. **TypeScript 只能用 `Callable.create(fn)` 传回调**。`addClickListener`、`setItemRenderer`、`setItemProvider`、`setTreeNodeRender`、`Transition.play` 的 callback、`GTweener.onComplete` 等参数都是 `Callable`，不要直接传裸函数。
3. **带 `ADD_PROPERTY` 的字段用属性读写，不要调用 setter 方法**（GodotJS 会隐藏 setter）。例如：
   - `ui.defaultFont`（不是 `setDefaultFont()`）
   - `obj.icon`（不是 `setIcon()`）
   - `ctrl.selectedIndex`（不是 `setSelectedIndex()`）
   - `bar.value` / `bar.min` / `bar.max`
   - `win.contentPane` / `win.modal` / `win.onInitCallback`
   - `list.selectedIndex`
4. **`GComponent.viewWidth` / `viewHeight` 是属性**；GodotJS 下 `getViewWidth()` 可能不存在。
5. **不能 `class X extends GWindow`**（GodotJS 对原生 ClassDB 类继承会报 `proxy: defineProperty exception`）。用组合：`new GWindow()` + `setupDisplay()` + 回调属性。
6. **场景根节点必须是 `Node`（或非 `Control` 的 CanvasItem）**。`Control` 默认 `mouse_filter = STOP`，会截断输入，导致 FairyGUI 收不到点击。
7. **点击回调里不要同步 `removeChildren()` / 切场景**。用 `callDeferred` 分帧，并在切场景前 `GRoot.cleanup()`。
8. **GDScript / TS 的 FairyGUI 方法名是 camelCase**（如 `addPackage`、`getChild`），不是 Godot 常见的 snake_case。

## 1. 目录与类型声明

- `examples/ts/fairygui.d.ts`：手写的 FairyGUI 类型声明（仅覆盖示例用到的 API，**不是完整 API**）。
- `examples/ts/Scripts/fgui-globals.ts`：运行时注册全局类型。
- `examples/ts/Scripts/*.ts`：场景与辅助类示例。
- Godot 内置类型（`Node`、`Callable`、`Vector2`、`Rect2`、`SceneTree`…）从 `"godot"` 导入。

`fgui-globals` 注册的名称：

```
DragDropManagerHelper, FguiEventContext, GButton, GComponent, GController,
GLabel, GList, GLoader, GPopupMenu, GProgressBar, GRichTextField, GRoot,
GTextField, GTree, GTreeNode, GTweenHelper, GTweener, GWindow, ScrollPane,
Transition, UIConfigHelper, UIEventDispatcher, UIPackage, GuiObject,
FGUIEventContext, randf, randi
```

未注册但运行时存在的类：`GImage`、`GGraph`、`GGroup`、`GComboBox`、`GSlider`、`GScrollBar`、`GMovieClip`、`GTextInput`、`GBasicTextField`、`GLoader3D`、`UIObjectFactoryHelper`、`FGUIHelper`。需要时在 `fgui-globals.ts` 的 `names` 数组中追加，并在 `fairygui.d.ts` 补声明；或直接从 GodotJS API 取：

```ts
// 仅示意：不要在模块顶层拷贝，应在使用时读取
const api = require("godot.lib.api") as Record<string, any>;
const GImageClass = api.GImage;
```

## 2. 初始化

### 2.1 创建 GRoot

`GRoot` 是单例，整个进程只创建一次。

```ts
import "./fgui-globals";
import { Engine, SceneTree } from "godot";

const tree = Engine.getMainLoop() as SceneTree;
GRoot.create(tree);           // 在节点已就绪时
// 在 _ready() 内，推荐：
// GRoot.createDeferred(tree); // 延迟一帧挂载，避免 "Parent node is busy setting up children"
```

> `createDeferred`、`getDisplayObject` 等在 `fairygui.d.ts` 中可能没有声明；运行时存在，必要时补充声明。

- `GRoot.create(tree, zOrder = 1000): GRoot`
- `GRoot.createDeferred(tree, zOrder = 1000): GRoot`
- `GRoot.getInstance(): GRoot | null`
- `GRoot.cleanup(): void`：切场景前卸载显示层。

### 2.2 加载 UI 包

`.fui` 文件放在 `res://` 下，`addPackage` 传**不带 `.fui` 的路径**。

```ts
UIPackage.addPackage("res://Resources/UI/MainMenu");

const view = UIPackage.createObject("MainMenu", "Main");
GRoot.getInstance()!.addChild(view);

// 或按 URL
const btn = UIPackage.createObjectFromURL("ui://MainMenu/MyButton");
```

若编辑器开启了 branch 导出，组件名会带分支前缀（如 `7iys1/Menu`），此时 `createObject` 要用完整名称。

### 2.3 字体

```ts
const ui = UIConfigHelper.getInstance()!;
ui.registerFont("default", "res://Resources/fonts/NotoSansSC-Regular.ttf");
ui.registerFont("msyh", "res://fonts/msyh.ttf"); // .ttf/.otf -> 文件字体
ui.registerFont("SimHei", "SimHei");             // 无扩展名 -> 系统字体
ui.defaultFont = "default";
```

BMFont（编辑器导出的 `.fnt`）随包自动加载，无需注册。

### 2.4 滚动条（必须显式注册）

`ScrollPane` 不会自动读取编辑器 `Common.json` 的滚动条配置。使用 `overflow="scroll"` 的组件之前：

```ts
UIPackage.addPackage("res://Resources/UI/Basics");
const ui = UIConfigHelper.getInstance()!;
ui.verticalScrollBar = "ui://Basics/ScrollBar_VT";
ui.horizontalScrollBar = "ui://Basics/ScrollBar_HZ";
```

### 2.5 屏幕适配

```ts
const root = GRoot.create(tree);
root.setContentScaleFactor(1136, 640, GRoot.ScreenMatchMode.MatchWidthOrHeight);
root.onWindowSizeChanged();
```

| 模式 | 说明 |
|---|---|
| `MatchWidthOrHeight` (0) | 等比缩放，留黑边，内容完整可见 |
| `MatchWidth` (1) | 固定宽度、动态高度 |
| `MatchHeight` (2) | 固定高度、动态宽度 |
| `MatchFill` (3) | 非等比拉伸铺满 |

> GodotJS 的嵌套枚举成员通常为 camelCase；若当前 typings 未导出 `ScreenMatchMode`，直接传数值 `0/1/2/3`。

窗口尺寸变化时重新布局：

```ts
const display = root.getDisplayObject(); // d.ts 未声明时补声明，或改用 getDisplayObject 的等价 API
if (display && display.is_inside_tree()) {
    const viewport = display.get_viewport();
    if (viewport instanceof Window) {
        viewport.size_changed.connect(() => root.onWindowSizeChanged());
    }
}
```

## 3. 事件

### 3.1 事件常量（`UIEventDispatcher`）

| 含义 | 值 | TS 常量 |
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

### 3.2 添加/移除监听

```ts
// 点击
btn.addClickListener(Callable.create(() => console.log("click")));

// 带事件上下文
list.addEventListener(
    UIEventDispatcher.CLICKITEM,
    Callable.create((ctx: FGUIEventContext) => {
        const item = ctx.getData() as GuiObject | null; // 也可用 ctx.getItemText()
    }),
);

// 移除（按事件类型移除该类型全部监听）
list.removeEventListener(UIEventDispatcher.CLICKITEM);
```

`FGUIEventContext`（运行时类名 `FguiEventContext`）方法：

| 方法 | 说明 |
|---|---|
| `preventDefault()` | 阻止默认行为 |
| `getData()` | 事件数据，常见为 `GuiObject` |
| `getItemText()` | 列表项文本（仅需文本时更稳） |
| `getTouchId()` | 触点 id |

> `getData()` 返回 `unknown`，需显式 `as` 转换。GodotJS 需应用 FairyGUI 文档中的 GodotJS 补丁，`getData()` 才能稳定返回 `GuiObject`。

### 3.3 手势/拖拽

```ts
obj.setDraggable(true);
obj.setDragBounds(new Rect2(0, 0, 500, 400));

obj.addEventListener(
    UIEventDispatcher.DRAGSTART,
    Callable.create((ctx: FGUIEventContext) => {
        ctx.preventDefault();
        DragDropManagerHelper.getInstance().startDrag(
            (obj.icon as string), // 或 obj.getIcon()
            obj.getIcon(),
            ctx.getTouchId(),
        );
    }),
);

target.addEventListener(
    UIEventDispatcher.DROP,
    Callable.create((ctx: FGUIEventContext) => {
        target.icon = String(ctx.getData());
    }),
);
```

## 4. 类层次

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

## 5. GuiObject（所有 UI 对象基类）

### 5.1 变换与外观

| 方法/属性 | 说明 |
|---|---|
| `getX()` / `setX(v)` | X 坐标 |
| `getY()` / `setY(v)` | Y 坐标 |
| `getPosition(): Vector2` / `setPosition(x, y)` | 位置 |
| `getWidth()` / `setWidth(v)` | 宽 |
| `getHeight()` / `setHeight(v)` | 高 |
| `getSize(): Vector2` / `setSize(w, h, ignorePivot=false)` | 尺寸 |
| `getInitSize(): Vector2` | 初始尺寸 |
| `setPivot(x, y, asAnchor=false)` / `getPivot()` | 轴心 |
| `setScale(x, y)` / `getScale()` | 缩放 |
| `setScaleX(v)` / `getScaleX()` | X 缩放 |
| `setScaleY(v)` / `getScaleY()` | Y 缩放 |
| `setRotation(v)` / `getRotation()` | 旋转 |
| `setAlpha(v)` / `getAlpha()` | 透明度 |
| `setGrayed(v)` / `isGrayed()` | 灰度 |
| `setVisible(v)` / `isVisible()` | 可见 |
| `setTouchable(v)` / `isTouchable()` | 可触摸 |
| `setSortingOrder(v)` / `getSortingOrder()` | 显示层级 |
| `center(restraint=false)` | 在父容器居中 |
| `makeFullScreen()` | 填满 GRoot |
| `setPixelSnapping(v)` / `isPixelSnapping()` | 像素对齐 |
| `setSkewX(v)` / `getSkewX()` | X 斜切 |
| `setSkewY(v)` / `getSkewY()` | Y 斜切 |

### 5.2 文本/图标/名称

| 方法/属性 | 说明 |
|---|---|
| `setText(text)` / `getText()` | 文本 |
| `setName(name)` / `getName()` | 名称 |
| `icon` (属性) / `getIcon()` | 图标 URL |
| `setTooltips(text)` / `getTooltips()` | 悬浮提示 |
| `getResourceURL()` | 资源 URL |

> `icon` 是 `ADD_PROPERTY`，在 TS 中用属性；`setIcon()` 可能不可用。`name` 用 `getName()`，避免与 Godot `Node.name` 混淆。

### 5.3 层级与关系

| 方法/属性 | 说明 |
|---|---|
| `getParent(): GComponent \| null` | 父组件 |
| `removeFromParent()` | 从父移除 |
| `onStage(): boolean` | 是否在舞台 |
| `group` (属性) / `getGroup()` | 所属 GGroup |
| `addRelation(target, relationType, usePercent=false)` | 添加布局关系 |
| `removeRelation(target, relationType)` | 移除关系（d.ts 未声明） |
| `localToGlobal(pt): Vector2` | 本地转全局 |
| `globalToLocal(pt): Vector2` | 全局转本地 |
| `transformRect(rect, targetSpace): Rect2` | 矩形坐标变换 |
| `getTreeNode(): GTreeNode \| null` | 对应树节点 |
| `getInitSize` | 同 `getSize()` |

RelationType 数值（`FguiRelationType.*` 或 `GuiObject.RelationType.*`）：

| 名称 | 值 | 名称 | 值 |
|---|---:|---|---:|
| `LeftLeft` | 0 | `TopTop` | 7 |
| `LeftCenter` | 1 | `TopMiddle` | 8 |
| `LeftRight` | 2 | `TopBottom` | 9 |
| `CenterCenter` | 3 | `MiddleMiddle` | 10 |
| `RightLeft` | 4 | `BottomTop` | 11 |
| `RightCenter` | 5 | `BottomMiddle` | 12 |
| `RightRight` | 6 | `BottomBottom` | 13 |
| `Width` | 14 | `Height` | 15 |

```ts
child.addRelation(parent, FguiRelationType.Width, false);
child.addRelation(parent, FguiRelationType.Height, false);
// 或 GuiObject.RelationType.RightRight
```

### 5.4 事件与拖拽

| 方法 | 说明 |
|---|---|
| `addClickListener(callable)` | 点击（无参数回调） |
| `addEventListener(type, callable)` | 任意事件 |
| `removeEventListener(type)` | 移除该类型监听 |
| `setDraggable(v)` / `isDraggable()` | 拖拽开关 |
| `setDragBounds(bounds: Rect2)` / `getDragBounds()` | 拖拽范围 |
| `startDrag(touchId=-1)` / `stopDrag()` | 手动拖拽 |
| `tweenScale(endValue, duration): GTweener` | 缩放补间 |

### 5.5 子节点

`GuiObject` 也有 `addChild(node)` / `removeChild(node)`，但通常用 `GComponent` 的接口。

## 6. GComponent（容器）

| 方法/属性 | 说明 |
|---|---|
| `getChild(name): GuiObject \| null` | 按名称 |
| `getChildAt(index): GuiObject \| null` | 按索引 |
| `getChildByPath(path)` / `getChildById(id)` | 按路径/id（d.ts 未声明） |
| `getChildIndex(child)` / `setChildIndex(child, index)` | 索引 |
| `swapChildren(a, b)` / `swapChildrenAt(i, j)` | 交换 |
| `numChildren(): number` | 子节点数量（**方法**） |
| `addChild(child)` | 添加（忽略返回值；d.ts 标为 `GuiObject`） |
| `addChildAt(child, index)` | 插入（同上） |
| `removeChildAt(index)` | 按索引移除（d.ts 未声明） |
| `removeChildren(begin=0, end=-1)` | 批量移除 |
| `getController(name): GController \| null` | 获取控制器 |
| `getControllerAt(index)` / `addController(c)` / `removeController(c)` | 控制器管理 |
| `applyController(c)` / `applyAllControllers()` | 立即应用 |
| `getTransition(name): Transition \| null` | 获取过渡 |
| `getScrollPane(): ScrollPane \| null` | 获取滚动面板 |
| `viewWidth` / `viewHeight` (属性) | 视口尺寸 |
| `setOpaque(v)` / `getOpaque()` | 不透明 |
| `ensureBoundsCorrect()` | 修正边界 |
| `isChildInView(child)` | 子项是否可见 |
| `isAncestorOf(obj)` | 是否为祖先 |

> `numChildren`、`viewWidth` 在 README 中曾写成属性，但实际绑定是 `numChildren()` 方法、`viewWidth` 属性。以本文为准。

## 7. GController（控制器）

| 方法/属性 | 说明 |
|---|---|
| `selectedIndex` (属性) | 当前页索引 |
| `getSelectedIndex()` | 当前页索引 |
| `getPageCount(): number` | 页数 |
| `getPreviousPage(): string` / `getPreviousPageId(): string` | 上一页 |
| `selectedPage` (setter 方法 `setSelectedPage`) | 当前页名（d.ts 未声明） |
| `selectedPageId` | 当前页 id |
| `getParent(): GComponent` | 所属组件 |

```ts
const ctrl = comp.getController("c1");
if (ctrl) {
    ctrl.selectedIndex = 1;
    console.log(ctrl.getPageCount(), ctrl.getPreviousPage());
}
```

## 8. 常用控件

### 8.1 GButton

| 方法/属性 | 说明 |
|---|---|
| `setTitle(t)` / `getTitle()` | 标题（**方法**，不是 `title` 属性） |
| `setSelectedTitle(t)` / `getSelectedTitle()` | 选中标题 |
| `selected` (属性) | 选中状态 |
| `selectedIcon`（setter 方法） | 选中图标 |
| `titleColor` (属性) | 标题颜色 |
| `titleFontSize` (属性) | 标题字号 |
| `changeStateOnClick` (属性) | 点击是否切换状态 |
| `getRelatedController()` / `setRelatedController(c)` | 关联控制器 |

### 8.2 GLabel

`setTitle/getTitle`、`setTitleColor/getTitleColor`、`setTitleFontSize/getTitleFontSize`。

### 8.3 GComboBox

`setTitle/getTitle`、`titleColor`、`titleFontSize`、`selectedIndex`、`setValue/getValue`、`getDropdown()`、`refresh()`。

### 8.4 GProgressBar

| 方法/属性 | 说明 |
|---|---|
| `min` / `max` / `value` (属性) | 进度范围与当前值 |
| `setMin/getMin`、`setMax/getMax`、`setValue/getValue` | 同上 |
| `tweenValue(value, duration)` | 动画过渡到目标值 |

```ts
const bar = view.getChild("bar") as GProgressBar;
bar.value = 50;
bar.tweenValue(100, 2.0);
```

### 8.5 GSlider

`min` / `max` / `value` / `wholeNumbers`（属性），以及对应 setter/getter。

### 8.6 GScrollBar

`setDisplayPerc(v)`、`setScrollPerc(v)`、`getMinSize()`。

### 8.7 GTextField / GRichTextField / GTextInput

`GTextField` 为抽象基类，常用：

| 方法 | 说明 |
|---|---|
| `setColor(c)` / `getColor()` | 颜色 |
| `setFontSize(n)` / `getFontSize()` | 字号 |
| `setSingleLine(v)` / `isSingleLine()` | 单行 |
| `setAutoSize(v)` / `getAutoSize()` | 自动尺寸（0 NONE / 1 BOTH / 2 HEIGHT / 3 SHRINK） |
| `setUBBEnabled(v)` / `isUBBEnabled()` | UBB 解析 |
| `setTemplateVars(dict)` | 模板变量 |
| `getTextSize(): Vector2` | 文本尺寸 |

`GTextInput`：`setPrompt`、`setPassword`、`setKeyboardType`、`setMaxLength`。

`GRichTextField` 支持 HTML（如 `<img src='ui://Pkg/Icon'/>`），示例见 `ChatScene.ts`。

### 8.8 GLoader / GLoader3D

- `GLoader`：`setURL(url)` / `getURL()`、`align`、`verticalAlign`、`autoSize`、`fill`、`shrinkOnly`、`playing`、`frame`、`fillMethod`、`fillOrigin`、`fillAmount`、`imageColor`、`getComponent()`。
- `GLoader3D`：`setURL`、`setAnimationName` / `getPlayingAnimationName`、`setSkinName` / `getSkinName`、`isPlaying`、`isLoop` / `setLoop`、`setPlaying`、`setFrame` / `getFrame`、`play(name)`、`stop()`、`getContent` / `setContent`、`color`。

> 这些类型默认未在 `fgui-globals` 注册，需要时按第 1 节扩展。

### 8.9 GImage / GGraph / GMovieClip

- `GImage`：`flip`、`imageColor`、`fillMethod`、`fillOrigin`、`fillClockwise`、`fillAmount`。
- `GGraph`：`drawRect(w, h, lineSize, lineColor, fillColor)`、`drawEllipse(...)`、`isEmpty()`、`color`。
- `GMovieClip`：`setPlaying` / `isPlaying`、`setFrame` / `getFrame`、`setTimeScale` / `getTimeScale`、`advance(time)`、`setFlip` / `getFlip`、`setColor` / `getColor`。

FillMethod：`None=0, Horizontal=1, Vertical=2, Radial90=3, Radial180=4, Radial360=5`。
FillOrigin：`Top=0, Bottom=1, Left=2, Right=3`。

### 8.10 GGroup

`layout`（0 NONE / 1 HORIZONTAL / 2 VERTICAL）、`setColumnGap` / `getColumnGap`、`setLineGap` / `getLineGap`、`setExcludeInvisibles` / `isExcludeInvisibles`、`setAutoSizeDisabled` / `isAutoSizeDisabled`、`setMainGridIndex` / `getMainGridIndex`、`setMainGridMinSize` / `getMainGridMinSize`、`ensureBoundsCorrect()`。

> 读取组名：`fguiGroupName(obj)`（示例提供的兼容函数），不要直接依赖 `obj.group.getName()`。

## 9. GList（列表）

| 方法/属性 | 说明 |
|---|---|
| `setItemRenderer(callable)` | 渲染回调 `(index: number, obj: GComponent) => void` |
| `setItemProvider(callable)` | 虚拟列表资源回调 `(index: number) => string` |
| `setVirtual()` | 开启虚拟列表 |
| `setVirtualAndLoop()` | 开启循环虚拟列表 |
| `setNumItems(n)` / `getNumItems()` | 虚拟列表项目数量 |
| `getFirstChildInView()` | 可视区首个索引 |
| `addSelection(index, scrollToView=false)` | 选中 |
| `clearSelection()` | 清空选择 |
| `setSelectedIndex(i)` / `selectedIndex` | 选中索引 |
| `setSelectionController(c)` / `getSelectionController()` | 关联控制器 |
| `addItemFromPool(): GuiObject \| null` | 从池取一个 item |
| `removeChildrenToPool(begin=0, end=-1)` | 回收 item |
| `scrollToView(index, ani=false, setFirst=false)` | 滚动到项 |
| `layout` / `columnCount` / `lineCount` / `columnGap` / `lineGap` / `autoResizeItem` | 布局配置 |
| `ensureBoundsCorrect()`、`isChildInView(item)`、`getScrollPane()` | 继承自 GComponent |

布局数值：`SINGLE_COLUMN=0, SINGLE_ROW=1, FLOW_HORIZONTAL=2, FLOW_VERTICAL=3, PAGINATION=4`。
选择模式：`SINGLE=0, MULTIPLE=1, MULTIPLE_SINGLECLICK=2, NONE=3`。

虚拟列表写法（同 `VirtualListScene.ts`）：

```ts
const list = view.getChild("mailList") as GList;
list.setItemRenderer(Callable.create((index: number, obj: GComponent) => {
    obj.setText(`${index} Mail title here`);
    const ctrl = obj.getController("IsRead");
    if (ctrl) ctrl.selectedIndex = index % 2 === 0 ? 0 : 1;
}));
list.setVirtual();
list.setNumItems(1000);
```

带 `itemProvider`（不同行用不同资源，见 `ChatScene.ts`）：

```ts
list.setItemRenderer(Callable.create(this._renderItem.bind(this)));
list.setItemProvider(Callable.create((index: number) =>
    this._messages[index].from_me ? "ui://Emoji/chatRight" : "ui://Emoji/chatLeft"));
list.setVirtual();
```

下拉刷新/上拉加载：监听 `UIEventDispatcher.PULLDOWNRELEASE` / `PULLUPRELEASE`，配合 `ScrollPane.getHeader()/getFooter()`、`lockHeader()/lockFooter()`，见 `PullToRefreshScene.ts`。

## 10. GTree / GTreeNode（树）

### GTree

| 方法/属性 | 说明 |
|---|---|
| `getRootNode(): GTreeNode \| null` | 根节点 |
| `getSelectedNode(): GTreeNode` | 当前选中节点 |
| `selectNode(node, scrollToView=false)` / `unselectNode(node)` | 选中/取消 |
| `expandAll(folderNode)` / `collapseAll(folderNode)` | 展开/收起 |
| `indent` | 缩进 |
| `clickToExpand` | 点击展开 |
| `setTreeNodeRender(callable)` | 渲染回调 `(node: GTreeNode, obj: GComponent) => void` |
| 其余 | 继承 `GList` |

### GTreeNode

`new GTreeNode()`；`init(isFolder, resUrl="")`、`setText/getText`、`setIcon/getIcon`、`isFolder()`、`isExpanded()/setExpanded(v)`、`getParent()`、`getTree()`、`getCell()`、`getData()/setData(v)`、`getChildAt(i)`、`numChildren()`、`addChild(node)`。

```ts
const rootNode = tree.getRootNode()!;
const top = new GTreeNode();
top.init(true, "");
top.setText("Top");
rootNode.addChild(top);

const leaf = new GTreeNode();
leaf.setText("Hello");
top.addChild(leaf);
```

点击节点：监听 `UIEventDispatcher.CLICKITEM`，用 `ctx.getData()` 取 `GuiObject`，再 `getTreeNode()`。

## 11. GWindow（窗口）

### 11.1 组合方式（TS 必须这样写）

```ts
const win = new GWindow();
win.setupDisplay();

win.onInitCallback = Callable.create(() => {
    win.contentPane = UIPackage.createObject("Pkg", "Win") as GComponent;
    win.center();
    win.modal = true;
    // 或者：win.setContentPane(pane); win.setModal(true);
});
win.onShownCallback = Callable.create(() => {});
win.onHideCallback = Callable.create(() => {});
win.doShowAnimationCallback = Callable.create(() => {
    win.setScale(0.1, 0.1);
    win.setPivot(0.5, 0.5);
    win.tweenScale(new Vector2(1, 1), 0.3).onComplete(Callable.create(() => {}));
});
win.doHideAnimationCallback = Callable.create(() => {
    GTweenHelper.getInstance().kill(win, false);
    win.tweenScale(new Vector2(0.1, 0.1), 0.3)
        .onComplete(Callable.create(() => win.hideImmediately()));
});
win.show();
```

### 11.2 API

| 方法/属性 | 说明 |
|---|---|
| `show()` / `hide()` / `hideImmediately()` | 显示/隐藏 |
| `toggleStatus()` / `bringToFront()` | 切换/置顶 |
| `isShowing()` / `isTop()` | 状态 |
| `modal` (属性) / `setModal(v)` | 模态 |
| `contentPane` (属性) / `getContentPane()` / `setContentPane(pane)` | 内容面板 |
| `getFrame()` | 框架 |
| `closeButton` / `dragArea` / `contentArea` | 关闭按钮/拖拽区/内容区 |
| `showModalWait(requestingCmd=0)` / `closeModalWait(requestingCmd=0)` | 窗口等待层 |
| `initWindow()` | 手动初始化 |
| `onInitCallback` / `onShownCallback` / `onHideCallback` | 生命周期回调 |
| `doShowAnimationCallback` / `doHideAnimationCallback` | 显示/隐藏动画回调 |
| `getModalWaitingPane()` | 等待面板 |

也可以 `GWindow.create()` 静态创建。

全局模态等待由 `GRoot` 提供：`root.showModalWait()` / `root.closeModalWait()`，需先设置 `UIConfigHelper.getInstance().globalModalWaiting` / `.windowModalWaiting`，见 `ModalWaitingScene.ts`。

## 12. GPopupMenu（弹出菜单）

| 方法 | 说明 |
|---|---|
| `GPopupMenu.create(): GPopupMenu \| null` | 创建 |
| `addItem(caption): GButton` | 添加项 |
| `addSeperator()` | 分隔线 |
| `clearItems()` | 清空 |
| `getItemCount(): number` | 数量 |
| `getList(): GList` | 内部列表 |
| `getContentPane(): GComponent` | 内容面板 |
| `getItemName(index): string` | 项名 |
| `setItemText(name, caption)` | 改文本 |
| `setItemVisible(name, visible)` | 显隐 |
| `setItemGrayed(name, grayed)` | 置灰 |
| `setItemCheckable(name, checkable)` / `setItemChecked(name, checked)` / `isItemChecked(name)` | 勾选 |
| `removeItem(name): boolean` | 移除 |
| `show()` | 显示 |
| `showMenuAt(target, dir)` | 在目标旁显示（dir：0 AUTO / 1 UP / 2 DOWN） |

```ts
const pm = GPopupMenu.create();
pm?.addItem("Item 1");
pm?.addItem("Item 2");
pm?.showMenuAt(btn, 2);
```

## 13. ScrollPane

| 方法 | 说明 |
|---|---|
| `getPosX()` / `setPosX(value, ani=false)` | 水平位置 |
| `getPosY()` / `setPosY(value, ani=false)` | 垂直位置 |
| `getScrollingPosX()` / `getScrollingPosY()` | 滚动中的位置 |
| `scrollTop(ani=false)` / `scrollBottom(ani=false)` | 到顶/到底 |
| `isBottomMost()` | 是否到底 |
| `getHeader()` / `getFooter()` | 头/尾组件 |
| `lockHeader(size)` / `lockFooter(size)` | 锁定头/尾 |
| `cancelDragging()` | 取消拖动 |
| `refresh()` / `refreshScrollBars()` | 刷新（d.ts 未声明） |

> `LoopListScene.ts` 对 `getScrollingPosX()` 做了兼容回退；若旧 typings 缺该 API，可改用 `getPosX()`。

## 14. Transition

| 方法 | 说明 |
|---|---|
| `play(times=1, delay=0, callback?: Callable)` | 播放 |
| `stop()` | 停止 |
| `isPlaying()` | 是否播放中 |
| `getOwner()` | 所属组件 |

```ts
comp.getTransition("show")?.play(1, 0, Callable.create(() => console.log("done")));
```

## 15. Tween（GTweenHelper / GTweener）

### GTweenHelper

| 方法 | 说明 |
|---|---|
| `GTweenHelper.getInstance()` | 单例 |
| `toFloat(start, end, duration)` | 浮点补间 |
| `toVec2(start, end, duration)` | Vector2 补间 |
| `toVec3` / `toVec4` / `toColor` / `toDouble` | 其他类型补间 |
| `delayedCall(delay)` | 延迟调用 |
| `shake(start, amplitude, duration)` | 抖动 |
| `isTweening(target)` | 是否补间中 |
| `kill(target, complete=false)` | 杀掉目标补间 |
| `clean()` / `killAll(complete=false)` | 清理 |

### GTweener

`setDelay`、`setDuration`、`setEase`、`setRepeat(repeat, yoyo=false)`、`setTimeScale`、`setSnapping`、`setTarget(target, propType=0)`、`setPaused`、`seek(time)`、`kill(complete=false)`、`getNormalizedTime()`、`isCompleted()`、`allCompleted()`、`onStart(callable)`、`onUpdate(callable)`、`onComplete(callable)`。

`TweenPropType`（`GTweener.TweenPropType`）：`NONE=0, X=1, Y=2, POSITION=3, WIDTH=4, HEIGHT=5, SIZE=6, SCALE_X=7, SCALE_Y=8, SCALE=9, ROTATION=10, ALPHA=11, PROGRESS=12`。

```ts
GTweenHelper.getInstance()
    .toVec2(window.getPosition(), targetPos, 0.5)
    .setTarget(window, GTweener.TweenPropType.POSITION);
```

## 16. UIConfigHelper（全局配置）

`UIConfigHelper.getInstance()`。

| 属性 / 方法（TS 用属性） | 说明 |
|---|---|
| `defaultFont` | 默认字体别名 |
| `buttonSound` | 按钮音效 URL |
| `buttonSoundVolumeScale` | 按钮音量 |
| `defaultScrollStep` | 滚动步长 |
| `defaultScrollDecelerationRate` | 减速率 |
| `defaultScrollTouchEffect` | 触摸滚动 |
| `defaultScrollBounceEffect` | 回弹 |
| `defaultScrollBarDisplay` | 滚动条显示策略（0 DEFAULT / 1 VISIBLE / 2 AUTO / 3 HIDDEN） |
| `verticalScrollBar` / `horizontalScrollBar` | 滚动条组件 URL |
| `touchDragSensitivity` / `clickDragSensitivity` / `touchScrollSensitivity` | 灵敏度 |
| `defaultComboBoxVisibleItemCount` | 下拉可见项数 |
| `globalModalWaiting` / `windowModalWaiting` | 模态等待组件 URL |
| `modalLayerColor` | 模态层颜色 |
| `tooltipsWin` | 提示组件 URL |
| `bringWindowToFrontOnClick` | 点击置顶窗口 |
| `popupMenu` / `popupMenuSeperator` | 弹出菜单组件/分隔线 |
| `registerFont(alias, real)` | 注册字体 |
| `getRealFontName(alias)` | 解析字体名 |

> GodotJS 下对这些 `ADD_PROPERTY` 使用属性，不要调用 `setDefaultFont()` 等 setter。

## 17. UIPackage

| 静态方法 | 说明 |
|---|---|
| `addPackage(assetPath): UIPackage` | 加载 `.fui`（路径不带扩展名） |
| `createObject(pkgName, resName): GComponent \| null` | 创建组件 |
| `createObjectFromURL(url): GComponent \| null` | 按 `ui://` 创建 |
| `getById(id)` / `getByName(name)` | 查找包 |
| `removePackage(idOrName)` / `removeAllPackages()` | 卸载 |
| `getItemURL(pkgName, resName)` | 获取 URL |
| `registerFont(alias, real)` / `setDefaultFont(name)` | 字体（运行时存在，d.ts 未声明） |

## 18. 其它 Helper

### DragDropManagerHelper

`getInstance()`、`startDrag(icon: string, sourceData?: unknown, touchPointId?: number)`、`cancel()`、`isDragging()`。

### UIObjectFactoryHelper

`getInstance().setPackageItemExtension(url: string, creator: Callable)`：为指定 `ui://` 资源注册自定义组件扩展（creator 无参返回 `GComponent`）。

### FGUIHelper

`getInstance()`、`getGRoot()`、`createObject(pkg, res)`、`addPackage(path)`、`loadTranslation(xmlPath)`、`loadTranslationFromXML(xml)`、`setHtmlButtonResource(url)`、`setHtmlInputResource(url)`、`setHtmlSelectResource(url)`。
（未在 `fgui-globals` 注册，需要时自行扩展。）

## 19. 场景切换与清理（TS 特有）

```ts
// 清理 GRoot 上的窗口/弹窗/模态/提示
if (DragDropManagerHelper.getInstance().isDragging()) {
    DragDropManagerHelper.getInstance().cancel();
}
const root = GRoot.getInstance();
root?.hideTooltips();
root?.hidePopup();
root?.closeModalWait();
root?.closeAllWindows();

// 切场景前卸载 GRoot（建议分帧）
GRoot.cleanup();
```

注意：

- 切场景不要用节点已离树后的 `getTree()`；用 `Engine.getMainLoop() as SceneTree`。
- 点击回调内需 `callDeferred` 延迟清理与切场景，避免 JSCallable 在 V8 栈内析构崩溃。
- `GRoot.cleanup()` 后再 `GRoot.create()` 重建。

## 20. 资源导出

Web/导出时 Include Filters 至少包含：

```
*.fui, Resources/UI/*, Resources/fonts/*
```

排除：`gen/godot/*`、`*.cs`。

## 21. 与 GDScript/C# 的差异速查

| 项目 | TS | GDScript | C# |
|---|---|---|---|
| 方法命名 | camelCase | camelCase | PascalCase |
| 回调 | `Callable.create(fn)` | 方法引用/`func` | `Callable.From(...)` / `new Callable(this, MethodName.X)` |
| 事件常量 | `UIEventDispatcher.CLICKITEM` | 同 TS | `UIEventDispatcher.Clickitem`（或用 `FguiEvent.ClickItem`） |
| 关系枚举 | `FguiRelationType.RightRight` | `GuiObject.RIGHT_RIGHT` | `GuiObject.RelationType.RightRight`（传 int 需转换） |
| Window | 组合 `new GWindow()` | `extends GWindow` | `partial class X : GWindow` |
| ADD_PROPERTY | 用属性 | 属性或 setter | 属性或 setter |
| 事件上下文类名 | `FGUIEventContext` | `FGUIEventContext` | `FguiEventContext` |

完整示例见 `modules/fairygui/examples/ts/Scripts/`：`MainMenu.ts`、`BasicsScene.ts`、`VirtualListScene.ts`、`LoopListScene.ts`、`TreeViewScene.ts`、`ScrollPaneScene.ts`、`PullToRefreshScene.ts`、`ModalWaitingScene.ts`、`GuideScene.ts`、`CooldownScene.ts`、`ChatScene.ts`、`BagWindow.ts`、`Window1.ts`、`Window2.ts`、`JoystickModule.ts`。
