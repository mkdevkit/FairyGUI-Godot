extends DemoSceneBase

var _view: Object
var _test_win: Object = null

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/ModalWaiting")
	UIConfigHelper.getInstance().setGlobalModalWaiting("ui://ModalWaiting/GlobalModalWaiting")
	UIConfigHelper.getInstance().setWindowModalWaiting("ui://ModalWaiting/WindowModalWaiting")

	_view = UIPackage.createObject("ModalWaiting", "Main")
	_groot.addChild(_view)

	_test_win = UIPackage.createObject("ModalWaiting", "TestWin")
	if _test_win != null:
		var n1 = _test_win.getChild("n1")
		if n1 != null:
			n1.addClickListener(func():
				_test_win.showModalWait()
				await get_tree().create_timer(3.0).timeout
				_test_win.closeModalWait()
			)
		_test_win.show()

	_groot.showModalWait()
	await get_tree().create_timer(3.0).timeout
	_groot.closeModalWait()
