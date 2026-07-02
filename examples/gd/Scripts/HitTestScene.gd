extends DemoSceneBase


func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/HitTest")
	_view = UIPackage.createObject("HitTest", "Main")
	_groot.addChild(_view)
