extends DemoSceneBase

var _view: Object
var _list: Object

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/ScrollPane")
	_view = UIPackage.createObject("ScrollPane", "Main")
	_groot.addChild(_view)

	_list = _view.getChild("list")
	if _list != null:
		_list.setItemRenderer(_render_list_item)
		_list.setVirtual()
		_list.setNumItems(1000)
		_list.addEventListener(UIEventDispatcher.TOUCHBEGIN, _on_click_list)

func _render_list_item(index: int, obj: Object) -> void:
	obj.setTitle("Item " + str(index))
	var sp = obj.getScrollPane()
	if sp != null:
		sp.setPosX(0, false)
	var b0 = obj.getChild("b0")
	if b0 != null:
		b0.addEventListener(UIEventDispatcher.CLICK, func():
			var txt = _view.getChild("txt")
			if txt != null:
				txt.setText("Stick " + obj.getText())
		)
	var b1 = obj.getChild("b1")
	if b1 != null:
		b1.addEventListener(UIEventDispatcher.CLICK, func():
			var txt = _view.getChild("txt")
			if txt != null:
				txt.setText("Delete " + obj.getText())
		)

func _on_click_list() -> void:
	if _list == null:
		return
	var cnt = _list.numChildren()
	for i in range(cnt):
		var item = _list.getChildAt(i)
		if item == null:
			continue
		var sp = item.getScrollPane()
		if sp != null and sp.getPosX() != 0:
			var touch_target = _groot.getTouchTarget()
			var b0 = item.getChild("b0")
			var b1 = item.getChild("b1")
			if (b0 != null and b0.isAncestorOf(touch_target)) or (b1 != null and b1.isAncestorOf(touch_target)):
				return
			sp.setPosX(0, true)
			sp.cancelDragging()
			_list.getScrollPane().cancelDragging()
			break
