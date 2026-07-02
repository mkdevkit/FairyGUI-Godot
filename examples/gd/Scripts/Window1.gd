extends GWindow
class_name Window1

func _init() -> void:
	init()
	setOnInitCallback(_on_init)
	setOnShownCallback(_on_shown)

func _on_init() -> void:
	setContentPane(UIPackage.createObject("Basics", "WindowA"))
	center()

func _on_shown() -> void:
	var pane = getContentPane()
	if pane == null:
		return
	var list = pane.getChild("n6")
	if list == null:
		return
	list.removeChildrenToPool()
	for i in range(6):
		var item = list.addItemFromPool()
		item.setTitle(str(i))
		item.setIcon("ui://Basics/r4")
