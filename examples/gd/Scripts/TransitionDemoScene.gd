extends DemoSceneBase

var _btn_group: Object

var _g1: Object = null
var _g2: Object = null
var _g3: Object = null
var _g4: Object = null
var _g5: Object = null
var _g6: Object = null

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/Transition")
	_view = UIPackage.createObject("Transition", "Main")
	_groot.addChild(_view)

	_btn_group = _view.getChild("g0")

	_g1 = UIPackage.createObject("Transition", "BOSS")
	_g2 = UIPackage.createObject("Transition", "BOSS_SKILL")
	_g3 = UIPackage.createObject("Transition", "TRAP")
	_g4 = UIPackage.createObject("Transition", "GoodHit")
	_g5 = UIPackage.createObject("Transition", "PowerUp")
	_g6 = UIPackage.createObject("Transition", "PathDemo")

	_setup_transition_btn("btn0", _g1)
	_setup_transition_btn("btn1", _g2)
	_setup_transition_btn("btn2", _g3)
	_setup_transition_btn("btn3", _g4)
	_setup_transition_btn("btn4", _g5)
	_setup_transition_btn("btn5", _g6)

func _setup_transition_btn(btn_name: String, target: Object) -> void:
	var btn = _view.getChild(btn_name)
	if btn == null or target == null:
		return
	btn.addClickListener(func():
		_btn_group.setVisible(false)
		_groot.addChild(target)
		var trans = target.getTransition("t0")
		if trans != null:
			trans.play(1, 0, func():
				_btn_group.setVisible(true)
				target.removeFromParent()
			)
	)
