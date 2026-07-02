extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var groot = GRoot.create(self)
	UIConfigHelper.getInstance().setPopupMenu("ui://Basics/PopupMenu")
	UIPackage.addPackage("res://Resources/UI/Basics")
	var view = UIPackage.createObject("Basics", "Demo_Popup")
	groot.addChild(view)

	var pm = GPopupMenu.create()
	print("GPopupMenu.create=", pm)
	if pm != null:
		pm.addItem("Item 1")
		pm.addItem("Item 2")
		var n0 = view.getChild("n0")
		pm.showMenuAt(n0, 2)
		_print_pane("GPopupMenu", pm.getContentPane())
	else:
		print("Fallback: showPopup via createObjectFromURL")
		var cp = UIPackage.createObjectFromURL("ui://Basics/PopupMenu")
		groot.showPopupSimple(cp)
		_print_pane("manual", cp)
	quit()

func _print_pane(label: String, cp: Object) -> void:
	if cp == null:
		print(label, ": contentPane is null")
		return
	print(label, " parent=", cp.getParent())
	print(label, " size=", cp.getWidth(), "x", cp.getHeight())
	print(label, " pos=", cp.getPosition(), " visible=", cp.isVisible())
	var disp = cp.displayObject()
	if disp == null:
		print(label, " displayObject=null")
		return
	print(label, " display parent=", disp.get_parent())
	print(label, " display in_tree=", disp.is_inside_tree())
	if disp is CanvasItem:
		print(label, " display visible=", disp.visible, " z_index=", disp.z_index)
	if disp is Node2D:
		print(label, " display pos=", disp.position)
