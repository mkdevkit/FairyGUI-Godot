extends DemoSceneBase

var _list: Object

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/VirtualList")
	_view = UIPackage.createObject("VirtualList", "Main")
	_groot.addChild(_view)

	var n6 = _view.getChild("n6")
	if n6 != null:
		n6.addClickListener(func():
			if _list != null:
				_list.addSelection(500, true)
		)
	var n7 = _view.getChild("n7")
	if n7 != null:
		n7.addClickListener(func():
			if _list != null:
				var sp = _list.getScrollPane()
				if sp != null:
					sp.scrollTop(true)
		)
	var n8 = _view.getChild("n8")
	if n8 != null:
		n8.addClickListener(func():
			if _list != null:
				var sp = _list.getScrollPane()
				if sp != null:
					sp.scrollBottom(true)
		)

	_list = _view.getChild("mailList")
	if _list != null:
		_list.setItemRenderer(_render_list_item)
		_list.setVirtual()
		_list.setNumItems(1000)

func _render_list_item(index: int, obj: Object) -> void:
	var read_ctrl = obj.getController("IsRead")
	if read_ctrl != null:
		read_ctrl.setSelectedIndex(0 if index % 2 == 0 else 1)
	var fetch_ctrl = obj.getController("c1")
	if fetch_ctrl != null:
		fetch_ctrl.setSelectedIndex(0 if index % 3 != 0 else 1)
	var time_text = obj.getChild("timeText")
	if time_text != null:
		time_text.setText("5 Nov 2015 16:24:33")
	obj.setText(str(index) + " Mail title here")
