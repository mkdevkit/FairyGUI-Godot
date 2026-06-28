#include "GTreeNode.h"
#include "GComponent.h"
#include "GTree.h"

NS_FGUI_BEGIN
GTreeNode* GTreeNode::create(bool isFolder, const std::string& resURL)
{
    Ref<GTreeNode> ref = memnew(GTreeNode);
    GTreeNode* pRet = ref.ptr();
    if (pRet->init(isFolder, resURL))
    {
        pRet->reference(); // keep alive after ref dtor (2→1)
        return pRet;
    }
    // ref dtor cleans up (1→0→freed)
    return nullptr;
}

GTreeNode::GTreeNode()
    : _tree(nullptr),
      _parent(nullptr),
      _cell(nullptr),
      _level(0),
      _expanded(false),
      _isFolder(false)
{
}

GTreeNode::~GTreeNode()
{
    for (auto& it : _children)
        it->_parent = nullptr;

    _children.clear();

    if (_parent)
        _parent->removeChild(this);

    // CC_SAFE_RELEASE(_cell)
}

bool GTreeNode::init(bool isFolder, const std::string& resURL)
{
    _isFolder = isFolder;
    _resURL = resURL;

    return true;
}

void GTreeNode::setExpaned(bool value)
{
    if (!_isFolder)
        return;

    if (_expanded != value)
    {
        _expanded = value;
        if (_tree != nullptr)
        {
            if (_expanded)
                _tree->afterExpanded(this);
            else
                _tree->afterCollapsed(this);
        }
    }
}

const std::string& GTreeNode::getText() const
{
    if (_cell != nullptr)
        return _cell->getText();
    else
        return "";
}

void GTreeNode::setText(const std::string& value)
{
    if (_cell != nullptr)
        return _cell->setText(value);
}

const std::string& GTreeNode::getIcon() const
{
    if (_cell != nullptr)
        return _cell->getIcon();
    else
        return "";
}

void GTreeNode::setIcon(const std::string& value)
{
    if (_cell != nullptr)
        return _cell->setIcon(value);
}

GTreeNode* GTreeNode::addChild(GTreeNode* child)
{
    addChildAt(child, (int)_children.size());
    return child;
}

GTreeNode* GTreeNode::addChildAt(GTreeNode* child, int index)
{
    // CCASSERT(child != nullptr, "Argument must be non-nil")

    if (child->_parent == this)
    {
        setChildIndex(child, index);
    }
    else
    {
        if (child->_parent != nullptr)
            child->_parent->removeChild(child);
        child->_parent = this;

        int cnt = (int)_children.size();
        if (index == cnt)
            _children.push_back(child);
        else
            _children.insert(_children.begin() + index, child);
        child->_level = _level + 1;
        child->setTree(_tree);
        if ((_tree != nullptr && this == _tree->getRootNode()) || (_cell != nullptr && _cell->getParent() != nullptr && _expanded))
            _tree->afterInserted(child);
    }
    return child;
}

void GTreeNode::removeChild(GTreeNode* child)
{
    // CCASSERT(child != nullptr, "Argument must be non-nil")

    auto it = std::find(_children.begin(), _children.end(), child);
    int childIndex = (it != _children.end()) ? (int)std::distance(_children.begin(), it) : -1;
    if (childIndex != -1)
        removeChildAt(childIndex);
}

void GTreeNode::removeChildAt(int index)
{
    // CCASSERT(index >= 0 && index < _children.size(), "Invalid child index");

    GTreeNode* child = _children.at(index);
    child->_parent = nullptr;

    if (_tree != nullptr)
    {
        child->setTree(nullptr);
        _tree->afterRemoved(child);
    }

    _children.erase(_children.begin() + index);
}

void GTreeNode::removeChildren(int beginIndex, int endIndex)
{
    if (endIndex < 0 || endIndex >= _children.size())
        endIndex = (int)_children.size() - 1;

    for (int i = beginIndex; i <= endIndex; ++i)
        removeChildAt(beginIndex);
}

GTreeNode* GTreeNode::getChildAt(int index) const
{
    // CCASSERT(index >= 0 && index < _children.size(), "Invalid child index");

    return _children.at(index);
}

GTreeNode* GTreeNode::getPrevSibling() const
{
    if (_parent == nullptr)
        return nullptr;

    int i = (int)std::distance(_parent->_children.begin(), std::find(_parent->_children.begin(), _parent->_children.end(), (GTreeNode*)this));
    if (i <= 0)
        return nullptr;

    return _parent->_children.at(i - 1);
}

GTreeNode* GTreeNode::getNextSibling() const
{
    if (_parent == nullptr)
        return nullptr;

    int i = (int)std::distance(_parent->_children.begin(), std::find(_parent->_children.begin(), _parent->_children.end(), (GTreeNode*)this));
    if (i < 0 || i >= _parent->_children.size() - 1)
        return nullptr;

    return _parent->_children.at(i + 1);
}

int GTreeNode::getChildIndex(const GTreeNode* child) const
{
    // CCASSERT(child != nullptr, "Argument must be non-nil")

    return (int)(int)std::distance(_children.begin(), std::find(_children.begin(), _children.end(), (GTreeNode*)child));
}

void GTreeNode::setChildIndex(GTreeNode* child, int index)
{
    // CCASSERT(child != nullptr, "Argument must be non-nil")

    int oldIndex = (int)(int)std::distance(_children.begin(), std::find(_children.begin(), _children.end(), child));
    // CCASSERT(oldIndex != -1, "Not a child of this container")

    moveChild(child, oldIndex, index);
}

int GTreeNode::setChildIndexBefore(GTreeNode* child, int index)
{
    // CCASSERT(child != nullptr, "Argument must be non-nil")

    int oldIndex = (int)(int)std::distance(_children.begin(), std::find(_children.begin(), _children.end(), child));
    // CCASSERT(oldIndex != -1, "Not a child of this container")

    if (oldIndex < index)
        return moveChild(child, oldIndex, index - 1);
    else
        return moveChild(child, oldIndex, index);
}

int GTreeNode::moveChild(GTreeNode* child, int oldIndex, int index)
{
    int cnt = (int)_children.size();
    if (index > cnt)
        index = cnt;

    if (oldIndex == index)
        return oldIndex;

    child->_children.erase(child->_children.begin() + oldIndex);
    if (index >= cnt)
        _children.push_back(child);
    else
        _children.insert(_children.begin() + index, child);
    if ((_tree != nullptr && this == _tree->getRootNode()) || (_cell != nullptr && _cell->getParent() != nullptr && _expanded))

    return index;
}

void GTreeNode::swapChildren(GTreeNode* child1, GTreeNode* child2)
{
    // CCASSERT(child1 != nullptr, "Argument1 must be non-nil")
    // CCASSERT(child2 != nullptr, "Argument2 must be non-nil")

    int index1 = (int)std::distance(_children.begin(), std::find(_children.begin(), _children.end(), child1));
    int index2 = (int)std::distance(_children.begin(), std::find(_children.begin(), _children.end(), child2));

    // CCASSERT(index1 != -1, "Not a child of this container")
    // CCASSERT(index2 != -1, "Not a child of this container")

    swapChildrenAt(index1, index2);
}

void GTreeNode::swapChildrenAt(int index1, int index2)
{
    GTreeNode* child1 = _children.at(index1);
    GTreeNode* child2 = _children.at(index2);

    setChildIndex(child1, index2);
    setChildIndex(child2, index1);
}

int GTreeNode::numChildren() const
{
    return (int)_children.size();
}

void GTreeNode::setTree(GTree* value)
{
    _tree = value;
    if (_tree != nullptr && _tree->treeNodeWillExpand != nullptr && _expanded)
        _tree->treeNodeWillExpand(this, true);

    if (_isFolder)
    {
        for (auto& child : _children)
        {
            child->_level = _level + 1;
            child->setTree(value);
        }
    }
}

void GTreeNode::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setText", "text"), &GTreeNode::gd_setText);
    ClassDB::bind_method(D_METHOD("getText"), &GTreeNode::gd_getText);

    ClassDB::bind_method(D_METHOD("setIcon", "icon"), &GTreeNode::gd_setIcon);
    ClassDB::bind_method(D_METHOD("getIcon"), &GTreeNode::gd_getIcon);

    ClassDB::bind_method(D_METHOD("isFolder"), &GTreeNode::isFolder);
    ClassDB::bind_method(D_METHOD("isExpanded"), &GTreeNode::isExpanded);
    ClassDB::bind_method(D_METHOD("setExpanded", "value"), &GTreeNode::setExpaned);

    ClassDB::bind_method(D_METHOD("getParent"), &GTreeNode::getParent);
    ClassDB::bind_method(D_METHOD("getTree"), &GTreeNode::getTree);
    ClassDB::bind_method(D_METHOD("getCell"), &GTreeNode::getCell);
    ClassDB::bind_method(D_METHOD("getData"), &GTreeNode::getData);
    ClassDB::bind_method(D_METHOD("setData", "value"), &GTreeNode::setData);
    ClassDB::bind_method(D_METHOD("getChildAt", "index"), &GTreeNode::getChildAt);
    ClassDB::bind_method(D_METHOD("numChildren"), &GTreeNode::numChildren);
    ClassDB::bind_method(D_METHOD("addChild", "child"), &GTreeNode::addChild);
    ClassDB::bind_method(D_METHOD("init", "is_folder", "res_url"), &GTreeNode::gd_init, DEFVAL(""));
}

void GTreeNode::gd_init(bool is_folder, const String& res_url)
{
    init(is_folder, res_url.utf8().get_data());
}

void GTreeNode::gd_setText(const String& value) { setText(value.utf8().get_data()); }
String GTreeNode::gd_getText() const { return GObject::toGodotStr(getText()); }
void GTreeNode::gd_setIcon(const String& value) { setIcon(value.utf8().get_data()); }
String GTreeNode::gd_getIcon() const { return GObject::toGodotStr(getIcon()); }

NS_FGUI_END
