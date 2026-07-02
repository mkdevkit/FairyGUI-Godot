extends DemoSceneBase

var _bag_window: Object = null
var _bag_window_inited: bool = false

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/Bag")

	_view = UIPackage.createObject("Bag", "Main")
	_groot.addChild(_view)

	var bag_btn = _view.getChild("bagBtn")
	if bag_btn != null:
		bag_btn.addClickListener(_show_bag)

func _show_bag() -> void:
	if not _bag_window_inited:
		_bag_window = UIPackage.createObject("Bag", "BagWin")
		if _bag_window == null:
			return
		_bag_window_inited = true
		_bag_window.onInitCallback = _on_bag_init
	_bag_window.show()
	_bag_window.center()

func _on_bag_init() -> void:
	var list = _bag_window.getChild("list")
	if list != null:
		list.addEventListener(UIEventDispatcher.CLICKITEM, _on_bag_click_item)
		list.setItemRenderer(_render_bag_item)
		list.setNumItems(45)

func _render_bag_item(index: int, obj: Object) -> void:
	var icon_idx = randi() % 10
	obj.setIcon("res://Resources/icons/i" + str(icon_idx) + ".png")
	obj.setText(str(randi() % 100))

func _on_bag_click_item() -> void:
	var item = _groot.getTouchTarget()
	if item == null:
		return
	var n11 = _bag_window.getChild("n11")
	var n13 = _bag_window.getChild("n13")
	if n11 != null and n13 != null:
		n11.setIcon(item.getIcon())
		n13.setText(item.getText())
