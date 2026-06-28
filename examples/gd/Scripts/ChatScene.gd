extends DemoSceneBase

var _view: Object
var _list: Object
var _input: Object
var _messages: Array = []
var _emoji_select_ui: Object = null

func continue_init() -> void:
	UIPackage.addPackage("res://Resources/UI/Emoji")

	_view = UIPackage.createObject("Emoji", "Main")
	_groot.addChild(_view)

	_list = _view.getChild("list")
	if _list != null:
		_list.setVirtual()
		_list.setItemRenderer(_render_list_item)
		_list.setItemProvider(_get_list_item_resource)

	_input = _view.getChild("input")
	if _input != null:
		_input.addEventListener(UIEventDispatcher.SUBMIT, _on_submit)

	var send_btn = _view.getChild("btnSend")
	if send_btn != null:
		send_btn.addClickListener(_on_click_send_btn)

	var emoji_btn = _view.getChild("btnEmoji")
	if emoji_btn != null:
		emoji_btn.addClickListener(_on_click_emoji_btn)

	_emoji_select_ui = UIPackage.createObject("Emoji", "EmojiSelectUI")
	if _emoji_select_ui != null:
		var emoji_list = _emoji_select_ui.getChild("list")
		if emoji_list != null:
			emoji_list.addEventListener(UIEventDispatcher.CLICKITEM, _on_click_emoji)

	_add_msg("Unity", "r0", "Hello!", true)

func _on_click_send_btn() -> void:
	if _input == null:
		return
	var msg = _input.getText()
	if msg.is_empty():
		return
	_add_msg("Unity", "r0", msg, true)
	_input.setText("")

func _on_click_emoji_btn() -> void:
	if _emoji_select_ui == null:
		return
	_groot.showPopupSimple(_emoji_select_ui)

func _on_click_emoji() -> void:
	var item = _groot.getTouchTarget()
	if item == null or _input == null:
		return
	_input.setText(_input.getText() + "[: " + item.getText() + "]")

func _on_submit() -> void:
	_on_click_send_btn()

func _parse_emoji(text: String) -> String:
	# Simple emoji tag parser: [:xxx] -> <img src='ui://Emoji/xxx'/>
	var regex = RegEx.new()
	regex.compile("\\[:\\s*(\\w+)\\]")
	var result = regex.search(text)
	if result == null:
		return text
	var tag = result.get_string(1)
	return text.replace(result.get_string(0), "<img src='ui://Emoji/" + tag + "'/>")

func _render_list_item(index: int, obj: Object) -> void:
	if index >= _messages.size():
		return
	var msg = _messages[index]
	obj.setIcon("ui://Emoji/" + msg.sender_icon)

	var tf = obj.getChild("msg")
	if tf is GRichTextField:
		tf.setUBBEnabled(false)
		tf.setText(_parse_emoji(msg.text))
		tf.setWidth(tf.getInitSize().x)
		tf.setWidth(tf.getTextSize().x)

	var name_obj = obj.getChild("name")
	if name_obj != null and not msg.from_me:
		name_obj.setText(msg.sender)

func _get_list_item_resource(index: int) -> String:
	if index >= _messages.size():
		return ""
	var msg = _messages[index]
	return "ui://Emoji/chatRight" if msg.from_me else "ui://Emoji/chatLeft"

func _add_msg(sender: String, sender_icon: String, text: String, from_me: bool) -> void:
	var sp = _list.getScrollPane()
	var is_scroll_bottom = sp != null and sp.isBottomMost()

	var msg = {
		"sender": sender,
		"sender_icon": sender_icon,
		"text": text,
		"from_me": from_me
	}
	_messages.append(msg)

	if from_me and (_messages.size() == 1 or randf() < 0.5):
		_messages.append({
			"sender": "FairyGUI",
			"sender_icon": "r1",
			"text": "Today is a good day. [:cool]",
			"from_me": false
		})

	if _messages.size() > 100:
		_messages = _messages.slice(-100)

	if _list != null:
		_list.setNumItems(_messages.size())
		if is_scroll_bottom and sp != null:
			sp.scrollBottom(true)
