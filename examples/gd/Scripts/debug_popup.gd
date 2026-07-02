extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var groot = GRoot.create(self)
	UIConfigHelper.getInstance().setPopupMenu("ui://Basics/PopupMenu")
	print("UIConfig popupMenu=", UIConfigHelper.getInstance().getPopupMenu())
	var pkg = UIPackage.addPackage("res://Resources/UI/Basics")
	print("addPackage Basics=", pkg)
	var obj = UIPackage.createObjectFromURL("ui://Basics/PopupMenu")
	print("createObjectFromURL PopupMenu=", obj)
	if obj:
		print("  size=", obj.getWidth(), "x", obj.getHeight())
		var list_obj = obj.getChild("list")
		print("  list child=", list_obj)
		if list_obj:
			print("  list class=", list_obj.get_class())
	var view = UIPackage.createObject("Basics", "Demo_Popup")
	print("Demo_Popup=", view)
	var pm = GPopupMenu.create()
	print("GPopupMenu.create=", pm)
	if pm == null:
		quit()
		return
	pm.addItem("Item 1")
	pm.addItem("Item 2")
	var n0 = view.getChild("n0")
	pm.showMenuAt(n0, 2)
	var cp = pm.getContentPane()
	print("contentPane parent=", cp.getParent())
	print("contentPane size=", cp.getWidth(), "x", cp.getHeight())
	print("contentPane pos=", cp.getPosition())
	print("contentPane visible=", cp.isVisible())
	var disp = cp.displayObject()
	print("displayObject=", disp)
	if disp:
		print("display parent=", disp.get_parent())
		print("display visible=", disp.visible if disp is CanvasItem else "n/a")
		print("display pos=", disp.position if disp is Node2D else "n/a")
		print("display in tree=", disp.is_inside_tree())
		print("display z_index=", disp.z_index if disp is CanvasItem else "n/a")
	quit()
