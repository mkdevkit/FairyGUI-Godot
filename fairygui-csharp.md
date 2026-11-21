# FairyGUI for Godot — C# 接口使用文档

> 面向后续 AI 与开发者。内容以 `modules/mono/glue/GodotSharp` 中 FairyGUI 生成的 C# 绑定和 `modules/fairygui/examples/csharp` 为准。
> 适用：Godot 4.5+（Mono/C#）+ 自编译 GodotSharp（必须包含 FairyGUI 绑定，不能用官方 NuGet GodotSharp）。

## 0. 最重要的事实（先读）

1. **C# 方法名是 PascalCase**：`AddPackage`、`GetChild`、`NumChildren`、`SetPosition`。
2. **FairyGUI 类型都在 `Godot` 命名空间**（示例通过 `global using Godot;`）。类名：`GRoot`、`GuiObject`、`GComponent`、`GList`、`GTree`、`GTreeNode`、`GWindow`、`GButton`、`GTextField`、`FguiEventContext`、`UIPackage` 等。
3. **`GuiObject` 没有 `GetChild/GetChildAt/NumChildren/GetController/GetTransition/GetScrollPane`**。这些在 `GComponent`（`GList`/`GTree` 继承）上。若变量是 `GuiObject`，必须：
   - 保持变量为 `GComponent`/`GList`/`GTree`；或
   - `((GComponent)obj).GetChild(...)`；或
   - 使用示例里的 `FguiExtensions` 扩展方法。
4. **`AddRelation` 第二参数是 `int`**：需 `(int)GuiObject.RelationType.RightRight`，或使用扩展方法自动转换。
5. **事件常量命名不是标准 PascalCase**：`UIEventDispatcher.Clickitem`、`Touchbegin`、`Pulldownrelease`。推荐在项目里定义示例的 `FguiEvent` 别名（见第 3 节）。
6. **事件回调要带 `FguiEventContext`**：`Callable.From<FguiEventContext>(ctx => ...)` 或方法签名 `void Handler(FguiEventContext ctx)`。0 参 lambda 用于 `AddEventListener` 会运行时报 `Invalid argument count`。`AddClickListener` 无参，可用 `Callable.From(() => ...)`。
7. **`GWindow` 继承类必须 `public partial class X : GWindow`**，构造函数中先 `SetupDisplay()`，再设置回调。
8. **场景根节点必须是 `Node`，不能是 `Control`**，否则输入被 `mouse_filter` 拦截。
9. **切场景前 `GRoot.Cleanup()`**，下一场景重新 `GRoot.Create()`。
10. **UI 包路径不带 `.fui` 扩展名**。

## 1. 初始化

### 1.1 创建 GRoot

```csharp
using Godot;

public partial class MyScene : Node
{
    public override void _Ready()
    {
        GRoot.CreateDeferred(GetTree()); // _ready 推荐
        // 或 GRoot.Create(GetTree());
    }
}
```

| 静态方法 | 说明 |
|---|---|
| `GRoot.Create(SceneTree tree, int zOrder = 1000)` | 创建并挂载 |
| `GRoot.CreateDeferred(SceneTree tree, int zOrder = 1000)` | 延迟一帧挂载 |
| `GRoot.GetInstance()` | 单例 |
| `GRoot.Cleanup()` | 卸载显示层 |

### 1.2 加载 UI 包并创建对象

```csharp
UIPackage.AddPackage("res://Resources/UI/MainMenu");

var view = UIPackage.CreateObject("MainMenu", "Main"); // 返回 GuiObject
GRoot.GetInstance().AddChild(view);

// 按 URL
var btn = UIPackage.CreateObjectFromURL("ui://MainMenu/MyButton");
```

| 静态方法 | 说明 |
|---|---|
| `UIPackage.AddPackage(string assetPath)` | 加载 `.fui`（路径不带扩展名） |
| `UIPackage.CreateObject(string pkgName, string resName)` | 创建组件（`GuiObject`） |
| `UIPackage.CreateObjectFromURL(string url)` | 按 `ui://` 创建 |
| `UIPackage.GetById/GetByName` | 查找包 |
| `UIPackage.RemovePackage/RemoveAllPackages` | 卸载 |
| `UIPackage.GetItemURL(pkgName, resName)` | 生成 URL |
| `UIPackage.RegisterFont(aliasName, realName)` | 注册字体 |
| `UIPackage.SetDefaultFont(fontName)` | 默认字体 |

若编辑器开启 branch 导出，组件名会带分支前缀（如 `7iys1/Menu`），`CreateObject` 需使用完整名称。

### 1.3 字体

```csharp
var ui = UIConfigHelper.GetInstance();
ui.RegisterFont("default", "res://Resources/fonts/NotoSansSC-Regular.ttf");
ui.RegisterFont("msyh", "res://fonts/msyh.ttf"); // 文件字体
ui.RegisterFont("SimHei", "SimHei");             // 系统字体
ui.SetDefaultFont("default");
// 或 ui.DefaultFont = "default";
```

BMFont（编辑器导出的 `.fnt`）随包自动加载。

### 1.4 滚动条（必须显式注册）

```csharp
UIPackage.AddPackage("res://Resources/UI/Basics");
var ui = UIConfigHelper.GetInstance();
ui.SetVerticalScrollBar("ui://Basics/ScrollBar_VT");
ui.SetHorizontalScrollBar("ui://Basics/ScrollBar_HZ");
// 或 ui.VerticalScrollBar = "..."; ui.HorizontalScrollBar = "...";
```

必须在创建任何 `overflow="scroll"` 组件之前完成。

### 1.5 屏幕适配

```csharp
var root = GRoot.Create(GetTree());
root.SetContentScaleFactor(1136, 640, (int)GRoot.ScreenMatchMode.WidthOrHeight);
root.OnWindowSizeChanged();

var display = root.GetDisplayObject();
if (display != null && display.IsInsideTree())
{
    var viewport = display.GetViewport();
    if (viewport is Window window)
        window.SizeChanged += root.OnWindowSizeChanged;
}
```

| 模式 | 值 | 说明 |
|---|---:|---|
| `GRoot.ScreenMatchMode.WidthOrHeight` | 0 | 等比缩放，留黑边 |
| `GRoot.ScreenMatchMode.Width` | 1 | 固定宽度、动态高度 |
| `GRoot.ScreenMatchMode.Height` | 2 | 固定高度、动态宽度 |
| `GRoot.ScreenMatchMode.Fill` | 3 | 非等比拉伸铺满 |

> C# glue 会去掉枚举成员共同前缀，`ScreenMatchMode` 实际成员是 `WidthOrHeight/Width/Height/Fill`，不是 `MatchWidthOrHeight`。

> `SetContentScaleFactor` 第三参数类型是 `int`，C# 需要把枚举强转为 `int`（或直接传 0/1/2/3）。

## 2. 事件

### 2.1 事件常量

`UIEventDispatcher` 的常量在 C# 中由 Godot glue 转换，命名并非标准 PascalCase：

| 含义 | 值 | C# 直接常量 | 推荐别名 |
|---|---:|---|---|
| Enter | 0 | `UIEventDispatcher.Enter` | `FguiEvent.Enter` |
| Exit | 1 | `UIEventDispatcher.Exit` | `FguiEvent.Exit` |
| Changed | 2 | `UIEventDispatcher.Changed` | `FguiEvent.Changed` |
| Submit | 3 | `UIEventDispatcher.Submit` | `FguiEvent.Submit` |
| TouchBegin | 10 | `UIEventDispatcher.Touchbegin` | `FguiEvent.TouchBegin` |
| TouchMove | 11 | `UIEventDispatcher.Touchmove` | `FguiEvent.TouchMove` |
| TouchEnd | 12 | `UIEventDispatcher.Touchend` | `FguiEvent.TouchEnd` |
| Click | 13 | `UIEventDispatcher.Click` | `FguiEvent.Click` |
| RollOver | 14 | `UIEventDispatcher.Rollover` | `FguiEvent.RollOver` |
| RollOut | 15 | `UIEventDispatcher.Rollout` | `FguiEvent.RollOut` |
| MouseWheel | 16 | `UIEventDispatcher.Mousewheel` | `FguiEvent.MouseWheel` |
| RightClick | 17 | `UIEventDispatcher.Rightclick` | `FguiEvent.RightClick` |
| MiddleClick | 18 | `UIEventDispatcher.Middleclick` | `FguiEvent.MiddleClick` |
| KeyDown | 30 | `UIEventDispatcher.Keydown` | `FguiEvent.KeyDown` |
| KeyUp | 31 | `UIEventDispatcher.Keyup` | `FguiEvent.KeyUp` |
| Scroll | 40 | `UIEventDispatcher.Scroll` | `FguiEvent.Scroll` |
| ScrollEnd | 41 | `UIEventDispatcher.Scrollend` | `FguiEvent.ScrollEnd` |
| PullDownRelease | 42 | `UIEventDispatcher.Pulldownrelease` | `FguiEvent.PullDownRelease` |
| PullUpRelease | 43 | `UIEventDispatcher.Pulluprelease` | `FguiEvent.PullUpRelease` |
| PositionChange | 20 | `UIEventDispatcher.Positionchange` | `FguiEvent.PositionChange` |
| SizeChange | 21 | `UIEventDispatcher.Sizechange` | `FguiEvent.SizeChange` |
| ClickItem | 50 | `UIEventDispatcher.Clickitem` | `FguiEvent.ClickItem` |
| ClickLink | 51 | `UIEventDispatcher.Clicklink` | `FguiEvent.ClickLink` |
| ClickMenu | 52 | `UIEventDispatcher.Clickmenu` | `FguiEvent.ClickMenu` |
| RightClickItem | 53 | `UIEventDispatcher.Rightclickitem` | `FguiEvent.RightClickItem` |
| DragStart | 60 | `UIEventDispatcher.Dragstart` | `FguiEvent.DragStart` |
| DragMove | 61 | `UIEventDispatcher.Dragmove` | `FguiEvent.DragMove` |
| DragEnd | 62 | `UIEventDispatcher.Dragend` | `FguiEvent.DragEnd` |
| Drop | 63 | `UIEventDispatcher.Drop` | `FguiEvent.Drop` |
| GearStop | 70 | `UIEventDispatcher.Gearstop` | `FguiEvent.GearStop` |

推荐在项目中定义（与示例 `Scripts/FguiExtensions.cs` 相同）：

```csharp
public static class FguiEvent
{
    public const int Enter = (int)UIEventDispatcher.Enter;
    public const int Exit = (int)UIEventDispatcher.Exit;
    public const int Changed = (int)UIEventDispatcher.Changed;
    public const int Submit = (int)UIEventDispatcher.Submit;
    public const int Click = (int)UIEventDispatcher.Click;
    public const int TouchBegin = (int)UIEventDispatcher.Touchbegin;
    public const int TouchMove = (int)UIEventDispatcher.Touchmove;
    public const int TouchEnd = (int)UIEventDispatcher.Touchend;
    public const int RightClick = (int)UIEventDispatcher.Rightclick;
    public const int ClickItem = (int)UIEventDispatcher.Clickitem;
    public const int ClickLink = (int)UIEventDispatcher.Clicklink;
    public const int DragStart = (int)UIEventDispatcher.Dragstart;
    public const int Drop = (int)UIEventDispatcher.Drop;
    public const int PullDownRelease = (int)UIEventDispatcher.Pulldownrelease;
    public const int PullUpRelease = (int)UIEventDispatcher.Pulluprelease;
    // 其余按需补充
}
```

### 2.2 添加/移除监听

```csharp
// 点击：无参
btn.AddClickListener(Callable.From(() => GD.Print("click")));
btn.AddClickListener(new Callable(this, MethodName.OnClick));

private void OnClick() { GD.Print("click"); }

// 任意事件：必须接收 FguiEventContext
list.AddEventListener(FguiEvent.ClickItem, Callable.From<FguiEventContext>(ctx =>
{
    var item = ctx.GetData().As<GuiObject>();
    var text = ctx.GetItemText();
}));

// 或用方法引用
list.AddEventListener(FguiEvent.ClickItem, new Callable(this, MethodName.OnClickItem));

private void OnClickItem(FguiEventContext ctx)
{
    var item = ctx.GetData().As<GuiObject>();
    if (item == null)
        item = GRoot.GetInstance().GetTouchTarget();
}

// 移除该事件类型全部监听
list.RemoveEventListener(FguiEvent.ClickItem);
```

`FguiEventContext` 方法：

| 方法 | 说明 |
|---|---|
| `PreventDefault()` | 阻止默认行为 |
| `GetData(): Variant` | 事件数据，用 `.As<GuiObject>()` / `.AsString()` |
| `GetItemText(): string` | 列表项文本 |
| `GetTouchId(): int` | 触点 id |

### 2.3 拖拽

```csharp
obj.SetDraggable(true);
obj.SetDragBounds(new Rect2(0, 0, 500, 400));

obj.AddEventListener(FguiEvent.DragStart, Callable.From<FguiEventContext>(ctx =>
{
    ctx.PreventDefault();
    DragDropManagerHelper.GetInstance().StartDrag(
        obj.GetIcon(),
        obj.GetIcon(),
        ctx.GetTouchId());
}));

target.AddEventListener(FguiEvent.Drop, Callable.From<FguiEventContext>(ctx =>
{
    target.SetIcon(ctx.GetData().AsString());
}));
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

`GTreeNode`、`ScrollPane`、`Transition`、`GTweener`、`GTweenHelper`、`UIPackage`、`GController`、`UIEventDispatcher`、`FguiEventContext` 均继承 `RefCounted`。

## 4. C# 类型转换要点

`UIPackage.CreateObject`、`GComponent.GetChild`、`GList.AddItemFromPool` 返回 `GuiObject`。访问组件专有 API 时：

```csharp
var view = UIPackage.CreateObject("Basics", "Main") as GComponent;
var btn = view.GetChild("btn_Back") as GButton;
btn.SetTitle("Back");

var list = view.GetChild("list") as GList;
var item = list.AddItemFromPool() as GComponent;
var title = item.GetChild("title");

// 或直接强转
var container = (GComponent)view.GetChild("container");
```

`GComponent`/`GList`/`GTree` 才有 `GetChild`、`GetChildAt`、`NumChildren`、`GetController`、`GetTransition`、`GetScrollPane`。`GuiObject` 上只有基础 API。

> 建议场景脚本把根视图声明为 `GComponent`（而不是示例里的 `GuiObject`），这样无需扩展方法：
>
> ```csharp
> protected GComponent _view;
> _view = UIPackage.CreateObject("Pkg", "Main") as GComponent;
> var child = _view.GetChild("btn");
> ```
>
> 若必须使用 `GuiObject`，请参考 `examples/csharp/Scripts/FguiExtensions.cs` 增加扩展方法，或显式强转 `((GComponent)obj)`。

## 5. GuiObject（所有 UI 对象基类）

### 5.1 变换与外观

| 方法/属性 | 说明 |
|---|---|
| `SetX/GetX`、`SetY/GetY` | 坐标 |
| `SetPosition(float x, float y)` / `GetPosition(): Vector2` | 位置 |
| `SetWidth/GetWidth`、`SetHeight/GetHeight` | 宽高 |
| `SetSize(float w, float h, bool ignorePivot=false)` / `GetSize(): Vector2` | 尺寸 |
| `GetInitSize(): Vector2` | 初始尺寸 |
| `SetPivot(x, y, asAnchor=false)` / `GetPivot(): Vector2` | 轴心 |
| `SetScale(x, y)` / `GetScale()` / `SetScaleX` / `GetScaleY` | 缩放 |
| `SetRotation/GetRotation` | 旋转 |
| `SetAlpha/GetAlpha` | 透明度 |
| `SetGrayed/IsGrayed` | 灰度 |
| `SetVisible/IsVisible` | 可见 |
| `SetTouchable/IsTouchable` | 可触摸 |
| `SetSortingOrder/GetSortingOrder` | 显示层级 |
| `Center(bool restraint=false)` | 父容器居中 |
| `MakeFullScreen()` | 铺满 GRoot |
| `SetPixelSnapping/IsPixelSnapping` | 像素对齐 |
| `SetSkewX/GetSkewX`、`SetSkewY/GetSkewY` | 斜切 |

### 5.2 文本/图标/名称

| 方法/属性 | 说明 |
|---|---|
| `SetText(string)` / `GetText(): string` | 文本 |
| `SetName(string)` / `GetName(): string` | 名称 |
| `Icon`（属性）/ `SetIcon/GetIcon` | 图标 URL |
| `SetTooltips/GetTooltips` | 悬浮提示 |
| `GetResourceURL()` | 资源 URL |

> `GButton` 没有 `Title` 属性，用 `SetTitle()`；`GTextField` 没有 `Text` 属性，用 `SetText()`（继承自 `GuiObject`）。

### 5.3 层级与关系

| 方法/属性 | 说明 |
|---|---|
| `GetParent(): GuiObject` | 父组件 |
| `RemoveFromParent()` | 从父移除 |
| `OnStage(): bool` | 是否在舞台 |
| `Group`（属性）/ `GetGroup()/SetGroup()` | 所属 GGroup |
| `AddRelation(GodotObject target, int relationType, bool usePercent=false)` | 添加关系 |
| `LocalToGlobal/GlobalToLocal` | 坐标转换 |
| `TransformRect(Rect2, GuiObject targetSpace): Rect2` | 矩形变换 |
| `GetTreeNode(): GTreeNode` | 对应树节点 |

RelationType（`GuiObject.RelationType`）：

| 成员 | 值 | 成员 | 值 |
|---|---:|---|---:|
| `LeftLeft` | 0 | `TopTop` | 7 |
| `LeftCenter` | 1 | `TopMiddle` | 8 |
| `LeftRight` | 2 | `TopBottom` | 9 |
| `CenterCenter` | 3 | `MiddleMiddle` | 10 |
| `RightLeft` | 4 | `BottomTop` | 11 |
| `RightCenter` | 5 | `BottomMiddle` | 12 |
| `RightRight` | 6 | `BottomBottom` | 13 |
| `Width` | 14 | `Height` | 15 |

```csharp
child.AddRelation(parent, (int)GuiObject.RelationType.Width, false);
child.AddRelation(parent, (int)GuiObject.RelationType.Height, false);
```

### 5.4 事件与拖拽

| 方法 | 说明 |
|---|---|
| `AddClickListener(Callable)` | 点击（无参回调） |
| `AddEventListener(int type, Callable)` | 任意事件 |
| `RemoveEventListener(int type)` | 移除该类型监听 |
| `SetDraggable/IsDraggable` | 拖拽开关 |
| `SetDragBounds(Rect2)` | 拖拽范围 |
| `StartDrag(int touchId=-1)` / `StopDrag()` | 手动拖拽 |
| `TweenScale(Vector2 endValue, float duration): GTweener` | 缩放补间 |

## 6. GComponent（容器）

| 方法/属性 | 说明 |
|---|---|
| `GetChild(string name): GuiObject` | 按名称 |
| `GetChildAt(int index): GuiObject` | 按索引 |
| `GetChildByPath/GetChildById` | 按路径/id |
| `GetChildIndex/SetChildIndex` | 索引 |
| `SwapChildren/SwapChildrenAt` | 交换 |
| `NumChildren(): int` | 子节点数量（方法） |
| `AddChild(GodotObject child)` | 添加（继承自 GuiObject，返回 void） |
| `AddChildAt(GodotObject child, int index)` | 插入 |
| `RemoveChildAt(int index)` | 按索引移除 |
| `RemoveChildren(int beginIndex=0, int endIndex=-1)` | 批量移除 |
| `GetController(string name): GController` | 控制器 |
| `GetControllerAt(int)` / `AddController` / `RemoveController` | 控制器管理 |
| `ApplyController/ApplyAllControllers` | 应用 |
| `GetTransition(string name): Transition` | 过渡 |
| `GetScrollPane(): ScrollPane` | 滚动面板 |
| `Opaque`（属性）/ `SetOpaque/GetOpaque` | 不透明 |
| `ViewWidth` / `ViewHeight`（属性）/ `GetViewWidth()` / `GetViewHeight()` | 视口尺寸 |
| `EnsureBoundsCorrect()` | 修正边界 |
| `IsChildInView(GuiObject)` / `IsAncestorOf(GuiObject)` | 可见/祖先判断 |

示例：

```csharp
var view = UIPackage.CreateObject("Basics", "Main") as GComponent;
_groot.AddChild(view);

var backBtn = view.GetChild("btn_Back") as GButton;
backBtn.SetVisible(false);
backBtn.AddClickListener(new Callable(this, MethodName.OnBack));

var container = view.GetChild("container") as GComponent;
var ctrl = view.GetController("c1");
ctrl.SetSelectedIndex(1);
```

## 7. GController

| 方法/属性 | 说明 |
|---|---|
| `SelectedIndex`（属性）/ `SetSelectedIndex/GetSelectedIndex` | 当前页索引 |
| `GetPageCount(): int` | 页数 |
| `GetPreviousPage(): string` / `GetPreviousPageId(): string` | 上一页 |
| `SetSelectedPage/GetSelectedPage` | 当前页名 |
| `SetSelectedPageId/GetSelectedPageId` | 当前页 id |
| `GetParent(): GComponent` | 所属组件 |

```csharp
var ctrl = comp.GetController("button");
if (ctrl != null)
{
    ctrl.SelectedIndex = 1;
    GD.Print(ctrl.GetPageCount(), ctrl.GetPreviousPage());
}
```

## 8. 常用控件

### 8.1 GButton

| 方法/属性 | 说明 |
|---|---|
| `SetTitle/GetTitle` | 标题 |
| `SetSelectedTitle/GetSelectedTitle` | 选中标题 |
| `Selected`（属性）/ `SetSelected/IsSelected` | 选中 |
| `SetSelectedIcon/GetSelectedIcon` | 选中图标 |
| `TitleColor`（属性）/ `GetTitleColor` | 标题颜色 |
| `TitleFontSize`（属性）/ `GetTitleFontSize` | 标题字号 |
| `ChangeStateOnClick`（属性） | 点击切换状态 |
| `GetRelatedController/SetRelatedController` | 关联控制器 |

### 8.2 GLabel

`SetTitle/GetTitle`、`SetTitleColor/GetTitleColor`、`SetTitleFontSize/GetTitleFontSize`。

### 8.3 GComboBox

`SetTitle/GetTitle`、`TitleColor`、`TitleFontSize`、`SelectedIndex`、`SetValue/GetValue`、`GetDropdown()`、`Refresh()`。

### 8.4 GProgressBar

| 方法/属性 | 说明 |
|---|---|
| `Min` / `Max` / `Value`（属性，double） | 范围与当前值 |
| `SetMin/GetMin`、`SetMax/GetMax`、`SetValue/GetValue` | 同上 |
| `TweenValue(double value, float duration)` | 动画到目标值 |

```csharp
var bar = view.GetChild("bar") as GProgressBar;
bar.Value = 50;
bar.TweenValue(100, 2.0f);
```

### 8.5 GSlider

`Min` / `Max` / `Value` / `WholeNumbers`（属性）及对应 setter/getter。

### 8.6 GScrollBar

`SetDisplayPerc(float)`、`SetScrollPerc(float)`、`GetMinSize()`。

### 8.7 GTextField / GRichTextField / GTextInput

| 方法 | 说明 |
|---|---|
| `SetColor/GetColor` | 颜色 |
| `SetFontSize/GetFontSize` | 字号 |
| `SetSingleLine/IsSingleLine` | 单行 |
| `SetAutoSize/GetAutoSize` | 自动尺寸（0 NONE / 1 BOTH / 2 HEIGHT / 3 SHRINK） |
| `SetUBBEnabled/IsUBBEnabled` | UBB 解析 |
| `SetTemplateVars(Godot.Collections.Dictionary)` | 模板变量 |
| `GetTextSize(): Vector2` | 文本尺寸 |

`GTextInput`：`SetPrompt`、`SetPassword`、`SetKeyboardType`、`SetMaxLength`。

`GBasicTextField` 还提供 `Color`、`FontSize`、`SingleLine`、`AutoSize`、`UbbEnabled` 属性。

富文本示例：

```csharp
var tf = obj.GetChild("msg");
if (tf is GRichTextField rtf)
{
    rtf.SetUBBEnabled(false);
    rtf.SetWidth(rtf.GetInitSize().X);
    rtf.SetText("<img src='ui://Emoji/r0'/> Hello");
    rtf.SetWidth(rtf.GetTextSize().X);
}
```

### 8.8 GLoader / GLoader3D

- `GLoader`：`SetURL/GetURL`、`Align`、`VerticalAlign`、`AutoSize`、`Fill`、`ShrinkOnly`、`Playing`、`Frame`、`FillMethod`、`FillOrigin`、`FillAmount`、`ImageColor`、`GetComponent()`。
- `GLoader3D`：`SetURL`、`SetAnimationName/GetPlayingAnimationName`、`SetSkinName/GetSkinName`、`IsPlaying`、`IsLoop/SetLoop`、`SetPlaying`、`SetFrame/GetFrame`、`Play(name)`、`Stop()`、`GetContent/SetContent`、`Color`。

### 8.9 GImage / GGraph / GMovieClip

- `GImage`：`Flip`、`ImageColor`、`FillMethod`、`FillOrigin`、`FillClockwise`、`FillAmount`。
- `GGraph`：`DrawRect(w, h, lineSize, lineColor, fillColor)`、`DrawEllipse(...)`、`IsEmpty()`、`Color`。
- `GMovieClip`：`SetPlaying/IsPlaying`、`SetFrame/GetFrame`、`SetTimeScale/GetTimeScale`、`Advance(time)`、`SetFlip/GetFlip`、`SetColor/GetColor`。

FillMethod：`None=0, Horizontal=1, Vertical=2, Radial90=3, Radial180=4, Radial360=5`。
FillOrigin：`Top=0, Bottom=1, Left=2, Right=3`。

### 8.10 GGroup

`Layout`（0 NONE / 1 HORIZONTAL / 2 VERTICAL）、`SetColumnGap/GetColumnGap`、`SetLineGap/GetLineGap`、`SetExcludeInvisibles/IsExcludeInvisibles`、`SetAutoSizeDisabled/IsAutoSizeDisabled`、`SetMainGridIndex/GetMainGridIndex`、`SetMainGridMinSize/GetMainGridMinSize`、`EnsureBoundsCorrect()`。

读取组名：

```csharp
var grp = obj.GetGroup();
if (grp != null && grp.GetName() == "btns")
    GD.Print("in group btns");
```

## 9. GList（列表）

| 方法/属性 | 说明 |
|---|---|
| `SetItemRenderer(Callable)` | 渲染回调 `(int index, GodotObject obj)` |
| `SetItemProvider(Callable)` | 虚拟列表资源回调 `(int index) -> string` |
| `SetVirtual()` / `SetVirtualAndLoop()` | 虚拟/循环虚拟 |
| `SetNumItems(int)` / `GetNumItems()` | 项目数量 |
| `GetFirstChildInView()` | 可视区首个索引 |
| `AddSelection(int index, bool scrollToView=false)` | 选中 |
| `ClearSelection(bool triggerEvent=true)` | 清空选择 |
| `SelectedIndex` / `SetSelectedIndex/GetSelectedIndex` | 选中索引 |
| `SetSelectionController/GetSelectionController` | 关联控制器 |
| `AddItemFromPool(): GuiObject` | 从池取 item（无参） |
| `RemoveChildrenToPool(int beginIndex=0, int endIndex=-1)` | 回收 item |
| `ScrollToView(int index, bool ani=false, bool setFirst=false)` | 滚动到项 |
| `Layout` / `ColumnCount` / `LineCount` / `ColumnGap` / `LineGap` / `AutoResizeItem` | 布局配置 |
| `EnsureBoundsCorrect()` / `IsChildInView(item)` / `GetScrollPane()` | 继承自 GComponent |

布局值：`SINGLE_COLUMN=0, SINGLE_ROW=1, FLOW_HORIZONTAL=2, FLOW_VERTICAL=3, PAGINATION=4`。
选择模式：`SINGLE=0, MULTIPLE=1, MULTIPLE_SINGLECLICK=2, NONE=3`。

虚拟列表示例（C# 回调签名为 `(int, GodotObject)`）：

```csharp
var list = ((GComponent)_view).GetChild("mailList") as GList;
list.SetItemRenderer(Callable.From(new Action<int, GodotObject>((index, obj) =>
{
    var item = (GComponent)obj;
    item.SetText(index + " Mail title here");
    var ctrl = item.GetController("IsRead");
    if (ctrl != null)
        ctrl.SetSelectedIndex(index % 2 == 0 ? 0 : 1);
})));
list.SetVirtual();
list.SetNumItems(1000);
```

使用成员方法（推荐，避免 lambda 每次分配）：

```csharp
list.SetItemRenderer(new Callable(this, MethodName.RenderListItem));

private void RenderListItem(int index, GodotObject obj)
{
    var item = (GComponent)obj;
    item.SetText(index + " Mail title here");
}
```

带 `ItemProvider`：

```csharp
list.SetItemProvider(new Callable(this, MethodName.GetListItemResource));

private string GetListItemResource(int index)
{
    return _messages[index].FromMe ? "ui://Emoji/chatRight" : "ui://Emoji/chatLeft";
}
```

下拉刷新/上拉加载：监听 `FguiEvent.PullDownRelease` / `PullUpRelease`，配合 `ScrollPane.GetHeader()/GetFooter()`、`LockHeader(int)/LockFooter(int)`，见 `PullToRefreshScene.cs`。

## 10. GTree / GTreeNode

### GTree

| 方法/属性 | 说明 |
|---|---|
| `GetRootNode(): GTreeNode` | 根节点 |
| `GetSelectedNode(): GTreeNode` | 当前选中 |
| `SelectNode(node, bool scrollToView=false)` / `UnselectNode(node)` | 选中/取消 |
| `ExpandAll(folderNode)` / `CollapseAll(folderNode)` | 展开/收起 |
| `Indent` / `ClickToExpand` | 缩进/点击展开 |
| `SetTreeNodeRender(Callable)` | 渲染回调 `(GTreeNode node, GodotObject obj)` |
| 其余 | 继承 `GList` |

### GTreeNode

`new GTreeNode()`；`Init(bool isFolder, string resUrl="")`、`SetText/GetText`、`SetIcon/GetIcon`、`IsFolder()`、`IsExpanded()/SetExpanded(bool)`、`GetParent()`、`GetTree()`、`GetCell()`、`GetData()/SetData(Variant)`、`GetChildAt(i)`、`NumChildren()`、`AddChild(node)`。

```csharp
var rootNode = tree.GetRootNode();
var top = new GTreeNode();
top.Init(true, "");
top.SetText("Top");
rootNode.AddChild(top);

for (var i = 0; i < 5; i++)
{
    var node = new GTreeNode();
    node.SetText("Hello " + i);
    top.AddChild(node);
}
```

点击节点：

```csharp
tree.AddEventListener(FguiEvent.ClickItem, new Callable(this, MethodName.OnClickNode));

private void OnClickNode(FguiEventContext ctx)
{
    var item = ctx?.GetData().As<GuiObject>() ?? _groot.GetTouchTarget();
    if (item == null)
        return;
    var node = item.GetTreeNode();
    if (node != null)
        GD.Print("click node ", node.GetText());
}
```

## 11. GWindow（窗口）

### 11.1 继承方式（C# 推荐）

```csharp
public partial class Window1 : GWindow
{
    public Window1()
    {
        SetupDisplay();
        SetOnInitCallback(Callable.From(OnInit));
        SetOnShownCallback(Callable.From(OnShown));
        SetDoShowAnimationCallback(Callable.From(DoShowAnimation));
        SetDoHideAnimationCallback(Callable.From(DoHideAnimation));
        SetOnHideCallback(Callable.From(OnHide));
    }

    private void OnInit()
    {
        SetContentPane(UIPackage.CreateObject("Basics", "WindowA") as GComponent);
        Center();
        SetModal(true);
    }

    private void OnShown()
    {
        var list = GetContentPane()?.GetChild("n6") as GList;
        if (list == null)
            return;
        list.RemoveChildrenToPool();
        for (var i = 0; i < 6; i++)
        {
            var item = list.AddItemFromPool();
            (item as GButton)?.SetTitle(i.ToString());
            item.SetIcon("ui://Basics/r4");
        }
    }

    private void DoShowAnimation()
    {
        SetScale(0.1f, 0.1f);
        SetPivot(0.5f, 0.5f);
        TweenScale(new Vector2(1, 1), 0.3f).OnComplete(Callable.From(OnShown));
    }

    private void DoHideAnimation()
    {
        GTweenHelper.GetInstance().Kill(this, false);
        TweenScale(new Vector2(0.1f, 0.1f), 0.3f).OnComplete(Callable.From(HideImmediately));
    }

    private void OnHide()
    {
        GetContentPane()?.GetTransition("t1")?.Stop();
    }
}
```

### 11.2 直接创建

```csharp
var win = new GWindow();
win.SetupDisplay();
win.SetOnInitCallback(new Callable(this, MethodName.OnTestWinInit));
win.Show();

private void OnTestWinInit()
{
    win.SetContentPane(UIPackage.CreateObject("ModalWaiting", "TestWin") as GComponent);
}
```

### 11.3 API

| 方法/属性 | 说明 |
|---|---|
| `Show/Hide/HideImmediately` | 显示/隐藏 |
| `ToggleStatus/BringToFront` | 切换/置顶 |
| `IsShowing/IsTop` | 状态 |
| `Modal`（属性）/ `SetModal/IsModal` | 模态 |
| `ContentPane`（属性）/ `GetContentPane/SetContentPane` | 内容面板 |
| `GetFrame()` | 框架 |
| `CloseButton/DragArea/ContentArea` | 关闭按钮/拖拽区/内容区 |
| `ShowModalWait(int requestingCmd=0)` / `CloseModalWait(int requestingCmd=0)` | 窗口等待层 |
| `InitWindow()` | 手动初始化 |
| `SetOnInitCallback/SetOnShownCallback/SetOnHideCallback` | 生命周期 |
| `SetDoShowAnimationCallback/SetDoHideAnimationCallback` | 动画回调 |
| `GetModalWaitingPane()` | 等待面板 |

静态 `GWindow.Create()` 也可用。

全局模态等待：

```csharp
UIConfigHelper.GetInstance().SetGlobalModalWaiting("ui://ModalWaiting/GlobalModalWaiting");
UIConfigHelper.GetInstance().SetWindowModalWaiting("ui://ModalWaiting/WindowModalWaiting");

_groot.ShowModalWait();
await ToSignal(GetTree().CreateTimer(3.0), SceneTreeTimer.SignalName.Timeout);
_groot.CloseModalWait();
```

## 12. GPopupMenu

| 方法 | 说明 |
|---|---|
| `GPopupMenu.Create(): GPopupMenu` | 创建 |
| `AddItem(string caption): GButton` | 添加项 |
| `AddSeperator()` | 分隔线 |
| `ClearItems()` | 清空 |
| `GetItemCount(): int` | 数量 |
| `GetList(): GList` | 内部列表 |
| `GetContentPane(): GComponent` | 内容面板 |
| `GetItemName(index)` / `SetItemText(name, caption)` | 项名/文本 |
| `SetItemVisible(name, visible)` / `SetItemGrayed(name, grayed)` | 显隐/置灰 |
| `SetItemCheckable/SetItemChecked/IsItemChecked` | 勾选 |
| `RemoveItem(name): bool` | 移除 |
| `Show()` | 显示 |
| `ShowMenuAt(GuiObject target, int dir)` | 目标旁显示（0 AUTO / 1 UP / 2 DOWN） |

```csharp
var pm = GPopupMenu.Create();
pm.AddItem("Item 1");
pm.AddItem("Item 2");
pm.ShowMenuAt(btn, 2);
```

## 13. ScrollPane

| 方法 | 说明 |
|---|---|
| `GetPosX()` / `SetPosX(float value, bool ani=false)` | 水平位置 |
| `GetPosY()` / `SetPosY(float value, bool ani=false)` | 垂直位置 |
| `GetScrollingPosX()` / `GetScrollingPosY()` | 滚动中的位置 |
| `ScrollTop(bool ani=false)` / `ScrollBottom(bool ani=false)` | 到顶/到底 |
| `IsBottomMost()` | 是否到底 |
| `GetHeader()` / `GetFooter()` | 头/尾组件 |
| `LockHeader(int size)` / `LockFooter(int size)` | 锁定头/尾 |
| `CancelDragging()` | 取消拖动 |
| `Refresh()` / `RefreshScrollBars()` | 刷新 |

## 14. Transition

| 方法 | 说明 |
|---|---|
| `Play(int times=1, float delay=0, Callable callback=default)` | 播放 |
| `Stop()` | 停止 |
| `IsPlaying()` | 是否播放中 |
| `GetOwner()` | 所属组件 |

```csharp
comp.GetTransition("show")?.Play(1, 0, Callable.From(() => GD.Print("done")));
```

## 15. Tween（GTweenHelper / GTweener）

### GTweenHelper

| 方法 | 说明 |
|---|---|
| `GTweenHelper.GetInstance()` | 单例 |
| `ToFloat/ToDouble` | 数值补间 |
| `ToVec2/ToVec3/ToVec4/ToColor` | 向量/颜色补间 |
| `DelayedCall(float delay)` | 延迟调用 |
| `Shake(Vector2 start, float amplitude, float duration)` | 抖动 |
| `IsTweening(RefCounted target)` | 是否补间中 |
| `Kill(RefCounted target, bool complete=false)` | 杀掉目标补间 |
| `Clean()` / `KillAll(bool complete=false)` | 清理 |

### GTweener

`SetDelay`、`SetDuration`、`SetEase`、`SetRepeat(int repeat, bool yoyo=false)`、`SetTimeScale`、`SetSnapping`、`SetTarget(GodotObject target, int propType=0)`、`SetPaused`、`Seek(float)`、`Kill(bool complete=false)`、`GetNormalizedTime()`、`IsCompleted()`、`AllCompleted()`、`OnStart(Callable)`、`OnUpdate(Callable)`、`OnComplete(Callable)`。

`GTweener.TweenPropType`：`None=0, X=1, Y=2, Position=3, Width=4, Height=5, Size=6, ScaleX=7, ScaleY=8, Scale=9, Rotation=10, Alpha=11, Progress=12`。

```csharp
GTweenHelper.GetInstance()
    .ToVec2(window.GetPosition(), targetPos, 0.5f)
    .SetTarget(window, (int)GTweener.TweenPropType.Position);
```

## 16. UIConfigHelper（全局配置）

`UIConfigHelper.GetInstance()`。

| 属性 / setter | 说明 |
|---|---|
| `DefaultFont` / `SetDefaultFont` | 默认字体别名 |
| `ButtonSound` / `SetButtonSound` | 按钮音效 URL |
| `ButtonSoundVolumeScale` | 按钮音量 |
| `DefaultScrollStep` | 滚动步长 |
| `DefaultScrollDecelerationRate` | 减速率 |
| `DefaultScrollTouchEffect` | 触摸滚动 |
| `DefaultScrollBounceEffect` | 回弹 |
| `DefaultScrollBarDisplay` | 滚动条显示策略（0 DEFAULT / 1 VISIBLE / 2 AUTO / 3 HIDDEN） |
| `VerticalScrollBar` / `HorizontalScrollBar` | 滚动条 URL |
| `TouchDragSensitivity` / `ClickDragSensitivity` / `TouchScrollSensitivity` | 灵敏度 |
| `DefaultComboBoxVisibleItemCount` | 下拉可见项数 |
| `GlobalModalWaiting` / `WindowModalWaiting` | 模态等待 URL |
| `ModalLayerColor` | 模态层颜色 |
| `TooltipsWin` | 提示组件 URL |
| `BringWindowToFrontOnClick` | 点击置顶 |
| `PopupMenu` / `PopupMenuSeperator` | 弹出菜单/分隔线 |
| `RegisterFont(alias, real)` | 注册字体 |
| `GetRealFontName(alias)` | 解析字体 |

## 17. 其它 Helper

### DragDropManagerHelper

`GetInstance()`、`StartDrag(string icon, Variant sourceData = default, int touchPointId = -1)`、`Cancel()`、`IsDragging()`。

### UIObjectFactoryHelper

`GetInstance().SetPackageItemExtension(string url, Callable creator)`：为 `ui://` 资源注册自定义扩展，creator 返回 `GComponent`。

### FguiHelper

`GetInstance()`、`GetGRoot()`、`CreateObject(pkg, res)`、`AddPackage(path)`、`LoadTranslation(xmlPath)`、`LoadTranslationFromXML(xml)`、`SetHtmlButtonResource(url)`、`SetHtmlInputResource(url)`、`SetHtmlSelectResource(url)`。

## 18. 场景切换与清理

```csharp
if (DragDropManagerHelper.GetInstance().IsDragging())
    DragDropManagerHelper.GetInstance().Cancel();

_groot.HideTooltips();
_groot.HidePopup();
_groot.CloseModalWait();
_groot.CloseAllWindows();
_groot.RemoveChildren();

GRoot.Cleanup();
GetTree().ChangeSceneToFile("res://csharp/Scenes/MainMenu.tscn");
```

## 19. 资源导出

Include Filters 至少包含：

```
*.fui, Resources/UI/*, Resources/fonts/*
```

C# / Mono 不支持 Web 导出；Web 请用 GDScript 或 TypeScript 版本。

## 20. 与 GDScript/TypeScript 的差异速查

| 项目 | C# | GDScript | TS |
|---|---|---|---|
| 方法命名 | PascalCase | camelCase | camelCase |
| 回调 | `Callable.From(...)` / `new Callable(this, MethodName.X)` | 方法引用/`func` | `Callable.create(fn)` |
| 事件常量 | `UIEventDispatcher.Clickitem` 等 | `UIEventDispatcher.CLICKITEM` | `UIEventDispatcher.CLICKITEM` |
| 关系枚举 | `GuiObject.RelationType.RightRight`（需 `(int)`） | `GuiObject.RIGHT_RIGHT` | `FguiRelationType.RightRight` |
| Window | `partial class X : GWindow` | `extends GWindow` | 组合 `new GWindow()` |
| 事件上下文 | `FguiEventContext` | `FGUIEventContext` | `FGUIEventContext` |
| GetChild | 仅 `GComponent`/`GList`/`GTree` | 动态 `Object` 可直接调 | 声明在 `GComponent`，按 `GComponent` 使用 |

完整示例见 `modules/fairygui/examples/csharp/Scripts/`：`MainMenu.cs`、`BasicsScene.cs`、`VirtualListScene.cs`、`LoopListScene.cs`、`TreeViewScene.cs`、`ScrollPaneScene.cs`、`PullToRefreshScene.cs`、`ModalWaitingScene.cs`、`GuideScene.cs`、`CooldownScene.cs`、`ChatScene.cs`、`BagWindow.cs`、`Window1.cs`、`Window2.cs`、`JoystickModule.cs`。
