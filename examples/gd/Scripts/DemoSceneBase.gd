class_name DemoSceneBase
extends Node

var _groot: Object
var _view: Object = null

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

func _cleanup_groot_overlays() -> void:
	if _groot == null:
		return
	if DragDropManagerHelper.getInstance().isDragging():
		DragDropManagerHelper.getInstance().cancel()
	GTweenHelper.getInstance().killAll(true)
	_groot.hideTooltips()
	_groot.hidePopup()
	_groot.closeAllWindows()
	# Remove any overlay nodes still parented to GRoot (popups, drag agent, etc.).
	var i = _groot.numChildren() - 1
	while i >= 0:
		var child = _groot.getChildAt(i)
		if child == _view:
			i -= 1
			continue
		if child.getSortingOrder() >= 100000:
			i -= 1
			continue
		if child.getSortingOrder() >= 50000:
			_groot.removeChildAt(i)
		else:
			i -= 1

func _on_close() -> void:
	_cleanup_groot_overlays()
	get_tree().change_scene_to_file("res://gd/Scenes/MainMenu.tscn")
