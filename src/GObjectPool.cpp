#include "GObjectPool.h"
#include "GObject.h"
#include "UIPackage.h"

NS_FGUI_BEGIN
GObjectPool::GObjectPool()
{
}

GObjectPool::~GObjectPool()
{
}

Ref<GObject> GObjectPool::getObject(const std::string & url)
{
    std::string url2 = UIPackage::normalizeURL(url);
    if (url2.length() == 0)
        return Ref<GObject>();

    std::vector<Ref<GObject>>& arr = _pool[url2];
    if (!arr.empty())
    {
        Ref<GObject> ref = arr.back();
        arr.pop_back();
        return ref;
    }
    return UIPackage::createObjectFromURL(url2);
}

void GObjectPool::returnObject(GObject* obj)
{
    _pool[obj->getResourceURL()].push_back(obj);
}

NS_FGUI_END