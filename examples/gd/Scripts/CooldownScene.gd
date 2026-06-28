extends DemoSceneBase

var _view: Object

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/Cooldown")
	_view = UIPackage.createObject("Cooldown", "Main")
	_groot.addChild(_view)

	var b0 = _view.getChild("b0")
	if b0 is GProgressBar:
		var icon0 = b0.getChild("icon")
		if icon0 != null:
			icon0.setIcon("res://Resources/icons/k0.png")
		b0.tweenValue(100, 5)

	var b1 = _view.getChild("b1")
	if b1 is GProgressBar:
		var icon1 = b1.getChild("icon")
		if icon1 != null:
			icon1.setIcon("res://Resources/icons/k1.png")
		b1.tweenValue(0, 10)
