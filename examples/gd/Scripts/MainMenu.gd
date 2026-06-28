extends DemoSceneBase

var _view: Object

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/MainMenu")
	_view = UIPackage.createObject("MainMenu", "Main")
	_groot.addChild(_view)

	_nav_btn("n1", "res://gd/Scenes/BasicsScene.tscn")
	_nav_btn("n2", "res://gd/Scenes/TransitionDemoScene.tscn")
	_nav_btn("n4", "res://gd/Scenes/VirtualListScene.tscn")
	_nav_btn("n5", "res://gd/Scenes/LoopListScene.tscn")
	_nav_btn("n6", "res://gd/Scenes/HitTestScene.tscn")
	_nav_btn("n7", "res://gd/Scenes/PullToRefreshScene.tscn")
	_nav_btn("n8", "res://gd/Scenes/ModalWaitingScene.tscn")
	_nav_btn("n9", "res://gd/Scenes/JoystickScene.tscn")
	_nav_btn("n10", "res://gd/Scenes/BagScene.tscn")
	_nav_btn("n11", "res://gd/Scenes/ChatScene.tscn")
	_nav_btn("n12", "res://gd/Scenes/ListEffectScene.tscn")
	_nav_btn("n13", "res://gd/Scenes/ScrollPaneScene.tscn")
	_nav_btn("n14", "res://gd/Scenes/TreeViewScene.tscn")
	_nav_btn("n15", "res://gd/Scenes/GuideScene.tscn")
	_nav_btn("n16", "res://gd/Scenes/CooldownScene.tscn")

func _nav_btn(child_name: String, scene_path: String) -> void:
	var btn = _view.getChild(child_name)
	if btn != null:
		btn.addClickListener(func():
			_groot.removeChildren()
			get_tree().change_scene_to_file(scene_path)
		)

func _on_close() -> void:
	get_tree().quit()
