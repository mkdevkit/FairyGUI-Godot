class_name DemoSceneBase
extends Node

var _groot: Object

func _ready() -> void:
	# GRoot 需要先创建才能使用，如果尚未创建则延迟到下一帧
	if GRoot.getInstance() == null:
		call_deferred("_delayed_init")
	else:
		_groot = GRoot.getInstance()
		continue_init()
		_add_close_button()

func _delayed_init() -> void:
	GRoot.create(get_tree())
	_groot = GRoot.getInstance()
	continue_init()
	_add_close_button()

func continue_init() -> void:
	pass

func _add_close_button() -> void:
	var close_btn = UIPackage.createObject("MainMenu", "CloseButton")
	if close_btn == null:
		return
	close_btn.setPosition(_groot.getWidth() - close_btn.getWidth() - 10, _groot.getHeight() - close_btn.getHeight() - 10)
	close_btn.addRelation(_groot, GObject.RIGHT_RIGHT, false)
	close_btn.addRelation(_groot, GObject.BOTTOM_BOTTOM, false)
	close_btn.setSortingOrder(100000)
	close_btn.addClickListener(_on_close)
	_groot.addChild(close_btn)

func _on_close() -> void:
	get_tree().change_scene_to_file("res://gd/Scenes/MainMenu.tscn")
