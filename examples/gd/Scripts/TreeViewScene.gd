extends DemoSceneBase

var _view: Object
var _tree1: Object
var _tree2: Object

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/TreeView")
	_view = UIPackage.createObject("TreeView", "Main")
	_groot.addChild(_view)

	_tree1 = _view.getChild("tree")
	if _tree1 != null:
		_tree1.addEventListener(UIEventDispatcher.CLICKITEM, _on_click_node)

	_tree2 = _view.getChild("tree2")
	if _tree2 != null:
		_tree2.addEventListener(UIEventDispatcher.CLICKITEM, _on_click_node)
		_tree2.setTreeNodeRender(_render_tree_node)
		_build_tree2()

func _build_tree2() -> void:
	if _tree2 == null:
		return
	var root_node = _tree2.getRootNode()
	if root_node == null:
		return
	var top_node = GTreeNode.new()
	top_node.init(true, "")
	top_node.setText("I'm a top node")
	root_node.addChild(top_node)

	for i in range(5):
		var node = GTreeNode.new()
		node.setText("Hello " + str(i))
		top_node.addChild(node)

	var folder_node = GTreeNode.new()
	folder_node.init(true, "")
	folder_node.setText("A folder node")
	top_node.addChild(folder_node)
	for i in range(5):
		var node = GTreeNode.new()
		node.setText("Good " + str(i))
		folder_node.addChild(node)

	for i in range(3):
		var node = GTreeNode.new()
		node.setText("World " + str(i))
		top_node.addChild(node)

	var another_top = GTreeNode.new()
	another_top.setText("I'm a top node too")
	root_node.addChild(another_top)

func _on_click_node() -> void:
	var obj = _groot.getTouchTarget()
	if obj != null:
		var node = obj.getTreeNode()
		if node != null:
			print("click node ", node.getText())

func _render_tree_node(node: Object, obj: Object) -> void:
	var btn = obj.getChild("btn")
	if btn == null:
		btn = obj
	if node.isFolder():
		btn.setText(node.getText())
	else:
		btn.setIcon("ui://TreeView/file")
		btn.setText(node.getText())
