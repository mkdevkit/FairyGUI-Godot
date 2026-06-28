extends DemoSceneBase

var _view: Object
var _list1: Object
var _list2: Object
var _refreshing1: bool = false
var _refreshing2: bool = false

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/PullToRefresh")
	_view = UIPackage.createObject("PullToRefresh", "Main")
	_groot.addChild(_view)

	_list1 = _view.getChild("list1")
	if _list1 != null:
		_list1.setItemRenderer(_render_list_item1)
		_list1.setVirtual()
		_list1.setNumItems(1)
		_list1.addEventListener(UIEventDispatcher.PULLDOWNRELEASE, _on_pull_down_to_refresh)

	_list2 = _view.getChild("list2")
	if _list2 != null:
		_list2.setItemRenderer(_render_list_item2)
		_list2.setVirtual()
		_list2.setNumItems(1)
		_list2.addEventListener(UIEventDispatcher.PULLUPRELEASE, _on_pull_up_to_refresh)

func _render_list_item1(index: int, obj: Object) -> void:
	var num = _list1.getNumItems()
	obj.setText("Item " + str(num - index - 1))

func _render_list_item2(index: int, obj: Object) -> void:
	obj.setText("Item " + str(index))

func _on_pull_down_to_refresh() -> void:
	if _refreshing1:
		return
	_refreshing1 = true
	var sp = _list1.getScrollPane()
	if sp == null:
		_refreshing1 = false
		return
	var header = sp.getHeader()
	if header != null:
		var c1 = header.getController("c1")
		if c1 != null:
			c1.setSelectedIndex(2)
		sp.lockHeader(header.getHeight())

	await get_tree().create_timer(2.0).timeout

	_list1.setNumItems(_list1.getNumItems() + 5)

	if header != null:
		var c1 = header.getController("c1")
		if c1 != null:
			c1.setSelectedIndex(3)
		sp.lockHeader(35)

	await get_tree().create_timer(2.0).timeout

	if header != null:
		var c1 = header.getController("c1")
		if c1 != null:
			c1.setSelectedIndex(0)
		sp.lockHeader(0)
	_refreshing1 = false

func _on_pull_up_to_refresh() -> void:
	if _refreshing2:
		return
	_refreshing2 = true
	var sp = _list2.getScrollPane()
	if sp == null:
		_refreshing2 = false
		return
	var footer = sp.getFooter()
	if footer != null:
		var c1 = footer.getController("c1")
		if c1 != null:
			c1.setSelectedIndex(1)
		sp.lockFooter(footer.getHeight())

	await get_tree().create_timer(2.0).timeout

	_list2.setNumItems(_list2.getNumItems() + 5)

	if footer != null:
		var c1 = footer.getController("c1")
		if c1 != null:
			c1.setSelectedIndex(0)
		sp.lockFooter(0)
	_refreshing2 = false
