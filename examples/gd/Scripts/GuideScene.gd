extends DemoSceneBase

var _guide_layer: Object
var _bag_btn: Object

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/Guide")

	_view = UIPackage.createObject("Guide", "Main")
	_groot.addChild(_view)

	_guide_layer = UIPackage.createObject("Guide", "GuideLayer")
	if _guide_layer != null:
		_guide_layer.makeFullScreen()
		_guide_layer.addRelation(_groot, GObject.RIGHT_RIGHT, false)

	_bag_btn = _view.getChild("bagBtn")
	if _bag_btn != null:
		_bag_btn.addClickListener(func():
			if _guide_layer != null and _guide_layer.getParent() != null:
				_guide_layer.removeFromParent()
		)

	var n2 = _view.getChild("n2")
	if n2 != null:
		n2.addClickListener(_show_guide)

func _show_guide() -> void:
	if _guide_layer == null or _bag_btn == null:
		return
	_groot.addChild(_guide_layer)

	var rect = _bag_btn.transformRect(Rect2(Vector2.ZERO, Vector2(_bag_btn.getWidth(), _bag_btn.getHeight())), _guide_layer)
	var window = _guide_layer.getChild("window")
	if window != null:
		window.setSize(rect.size.x, rect.size.y)
		GTweenHelper.getInstance().toVec2(window.getPosition(), rect.position, 0.5).setTarget(window, GTweener.TweenPropType.POSITION)
