extends GWindow

func _init() -> void:
	setOnInitCallback(_on_init)
	setDoShowAnimationCallback(_do_show_animation)
	setDoHideAnimationCallback(_do_hide_animation)
	setOnHideCallback(_on_hide)

func _on_init() -> void:
	setContentPane(UIPackage.createObject("Basics", "WindowB"))
	center()

func _do_show_animation() -> void:
	setScale(0.1, 0.1)
	setPivot(0.5, 0.5)
	GTweenHelper.getInstance().toVec2(Vector2(0.1, 0.1), Vector2(1.0, 1.0), 0.3) \
		.setTarget(self, GTweener.TweenPropType.SCALE) \
		.onComplete(_on_shown)

func _on_shown() -> void:
	var pane = getContentPane()
	if pane == null:
		return
	var trans = pane.getTransition("t1")
	if trans != null:
		trans.play(1, 0, Callable())

func _do_hide_animation() -> void:
	GTweenHelper.getInstance().toVec2(getScale(), Vector2(0.1, 0.1), 0.3) \
		.setTarget(self, GTweener.TweenPropType.SCALE) \
		.onComplete(hideImmediately)

func _on_hide() -> void:
	var pane = getContentPane()
	if pane == null:
		return
	var trans = pane.getTransition("t1")
	if trans != null:
		trans.stop()
