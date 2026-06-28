extends DemoSceneBase

var _view: Object
var _list: Object

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/Extension")
	_view = UIPackage.createObject("Extension", "Main")
	_groot.addChild(_view)

	_list = _view.getChild("mailList")
	if _list == null:
		return

	for i in range(10):
		var item = _list.addItemFromPool()
		if item == null:
			continue
		var time_text = item.getChild("timeText")
		if time_text != null:
			time_text.setText("5 Nov 2015 16:24:33")

		var read_ctrl = item.getController("IsRead")
		if read_ctrl != null:
			read_ctrl.setSelectedIndex(0 if i % 2 == 0 else 1)

		var fetch_ctrl = item.getController("c1")
		if fetch_ctrl != null:
			fetch_ctrl.setSelectedIndex(0 if i % 3 != 0 else 1)

		item.setTitle("Mail title here")

	_list.ensureBoundsCorrect()
	var delay := 1.0
	for i in range(10):
		var item = _list.getChildAt(i)
		if item == null:
			break
		if _list.isChildInView(item):
			var trans = item.getTransition("t0")
			if trans != null:
				trans.play(1, delay, Callable())
			delay += 0.2
		else:
			break
