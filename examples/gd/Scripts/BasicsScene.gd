extends DemoSceneBase

var _view: Object
var _back_btn: Object
var _demo_container: Object
var _cc: Object
var _demo_objects: Dictionary = {}

var _win_a: Object = null
var _win_b: Object = null
var _pm: Object = null

var _popup_com: Object = null
var _progress_running: bool = false
var _depth_start_pos: Vector2 = Vector2.ZERO

func continue_init() -> void:
	UIConfigHelper.getInstance().setButtonSound("ui://Basics/click")
	UIConfigHelper.getInstance().setVerticalScrollBar("ui://Basics/ScrollBar_VT")
	UIConfigHelper.getInstance().setHorizontalScrollBar("ui://Basics/ScrollBar_HZ")
	UIConfigHelper.getInstance().setTooltipsWin("ui://Basics/WindowFrame")
	UIConfigHelper.getInstance().setPopupMenu("ui://Basics/PopupMenu")

	UIPackage.addPackage("res://Resources/UI/Basics")
	_view = UIPackage.createObject("Basics", "Main")
	_groot.addChild(_view)

	_back_btn = _view.getChild("btn_Back")
	_back_btn.setVisible(false)
	_back_btn.addClickListener(_on_click_back)

	_demo_container = _view.getChild("container")
	_cc = _view.getController("c1")

	var cnt = _view.numChildren()
	for i in range(cnt):
		var obj = _view.getChildAt(i)
		if obj != null and obj.getGroup() != null and obj.getGroup().getName() == "btns":
			obj.addClickListener(_run_demo)

func _on_click_back() -> void:
	_cleanup_groot_overlays()
	_cc.setSelectedIndex(0)
	_back_btn.setVisible(false)
	_progress_running = false

func _run_demo() -> void:
	_cleanup_groot_overlays()
	var sender = _groot.getTouchTarget()
	if sender == null:
		return
	var type_name = String(sender.getName()).substr(4)

	var obj = _demo_objects.get(type_name)
	if obj == null:
		obj = UIPackage.createObject("Basics", "Demo_" + type_name)
		if obj == null:
			return
		_demo_objects[type_name] = obj

	_demo_container.removeChildren()
	_demo_container.addChild(obj)
	_cc.setSelectedIndex(1)
	_back_btn.setVisible(true)

	match type_name:
		"Text": _play_text(obj)
		"Window": _play_window(obj)
		"Popup": _play_popup(obj)
		"ProgressBar": _play_progress(obj)
		"Drag&Drop": _play_drag_drop(obj)
		"Depth": _play_depth(obj)

func _play_text(obj: Object) -> void:
	var n12 = obj.getChild("n12")
	if n12 != null:
		n12.addEventListener(UIEventDispatcher.CLICKLINK, func():
			n12.setText("[img]ui://Basics/pet[/img][color=#FF0000]You click the link[/color]:clicked")
		)
	var n25 = obj.getChild("n25")
	if n25 != null:
		n25.addClickListener(func():
			var n22 = obj.getChild("n22")
			var n24 = obj.getChild("n24")
			n24.setText(n22.getText())
		)

func _play_popup(obj: Object) -> void:
	if _pm == null:
		_pm = GPopupMenu.create()
		if _pm == null:
			push_error("BasicsScene: GPopupMenu.create failed")
			return
		_pm.addItem("Item 1")
		_pm.addItem("Item 2")
		_pm.addItem("Item 3")
		_pm.addItem("Item 4")

	if _popup_com == null:
		_popup_com = UIPackage.createObject("Basics", "Component12")
		_popup_com.center()

	var n0 = obj.getChild("n0")
	if n0 != null:
		n0.addClickListener(func():
			_pm.showMenuAt(n0, 2)
		)

	var n1 = obj.getChild("n1")
	if n1 != null:
		n1.addClickListener(func():
			_groot.showPopupSimple(_popup_com)
		)

	obj.addEventListener(UIEventDispatcher.RIGHTCLICK, func():
		_pm.show()
	)

func _play_window(obj: Object) -> void:
	if _win_a != null:
		return

	var window1_script = load("res://gd/Scripts/Window1.gd")
	var window2_script = load("res://gd/Scripts/Window2.gd")
	if window1_script == null or window2_script == null:
		return

	_win_a = window1_script.new()
	_win_b = window2_script.new()

	var n0 = obj.getChild("n0")
	if n0 != null:
		n0.addClickListener(func():
			_win_a.show()
		)

	var n1 = obj.getChild("n1")
	if n1 != null:
		n1.addClickListener(func():
			_win_b.show()
		)

func _play_depth(obj: Object) -> void:
	var test_container = obj.getChild("n22")
	if test_container == null:
		return
	var fixed_obj = test_container.getChild("n0")
	fixed_obj.setSortingOrder(100)
	fixed_obj.setDraggable(true)

	var num = test_container.numChildren()
	var i = 0
	while i < num:
		var child = test_container.getChildAt(i)
		if child != fixed_obj:
			test_container.removeChildAt(i)
			num -= 1
		else:
			i += 1
	_depth_start_pos = Vector2(fixed_obj.getX(), fixed_obj.getY())

	var btn0 = obj.getChild("btn0")
	if btn0 != null:
		btn0.addClickListener(func():
			_depth_start_pos.x += 10
			_depth_start_pos.y += 10
			var graph = UIPackage.createObject("Basics", "graph_template")
			if graph != null:
				graph.setPosition(_depth_start_pos.x, _depth_start_pos.y)
				test_container.addChild(graph)
		)

	var btn1 = obj.getChild("btn1")
	if btn1 != null:
		btn1.addClickListener(func():
			_depth_start_pos.x += 10
			_depth_start_pos.y += 10
			var graph = UIPackage.createObject("Basics", "graph_template")
			if graph != null:
				graph.setPosition(_depth_start_pos.x, _depth_start_pos.y)
				graph.setSortingOrder(200)
				test_container.addChild(graph)
		)

func _play_drag_drop(obj: Object) -> void:
	var a = obj.getChild("a")
	if a != null:
		a.setDraggable(true)
	var b = obj.getChild("b")
	if b != null:
		b.setDraggable(true)
		b.addEventListener(UIEventDispatcher.DRAGSTART, func():
			DragDropManagerHelper.getInstance().startDrag(b.getIcon(), b.getIcon(), -1)
		)
	var c = obj.getChild("c")
	if c != null:
		c.addEventListener(UIEventDispatcher.DROP, func():
			var icon = _groot.getTouchTarget()
			if icon != null:
				c.setIcon(icon.getIcon())
		)
	var d = obj.getChild("d")
	if d != null:
		var n7 = obj.getChild("n7")
		if n7 != null:
			var parent = d.getParent()
			var rect = n7.transformRect(Rect2(Vector2.ZERO, Vector2(n7.getWidth(), n7.getHeight())), parent)
			d.setDraggable(true)
			d.setDragBounds(rect)

func _play_progress(obj: Object) -> void:
	_progress_running = true
	obj.addEventListener(UIEventDispatcher.EXIT, func():
		_progress_running = false
	)

func _process(delta: float) -> void:
	if not _progress_running:
		return
	var obj = _demo_objects.get("ProgressBar")
	if obj == null:
		return
	var cnt = obj.numChildren()
	for i in range(cnt):
		var child = obj.getChildAt(i)
		if child is GProgressBar:
			var val = child.getValue() + 1
			if val > child.getMax():
				val = child.getMin()
			child.setValue(val)
