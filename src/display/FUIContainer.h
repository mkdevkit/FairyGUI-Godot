#ifndef __FUICONTANER_H__
#define __FUICONTANER_H__

#include "FairyGUIMacros.h"
#include <functional>

NS_FGUI_BEGIN

class GObject;

class FUIContainer : public Node2D
{
    GDCLASS(FUIContainer, Node2D)

public:
    FUIContainer();
    virtual ~FUIContainer();

    static FUIContainer* create();

    bool isClippingEnabled() const;
    void setClippingEnabled(bool value);
    const Rect2& getClippingRegion() const;
    void setClippingRegion(const Rect2& clippingRegion);

    Node* getStencil() const;
    void setStencil(Node* stencil);
    float getAlphaThreshold() const;
    void setAlphaThreshold(float alphaThreshold);
    bool isInverted() const;
    void setInverted(bool inverted);

    GObject* gOwner;

    std::function<void(float)> _processCallback;

protected:
    static void _bind_methods();

    void _notification(int p_what);
    void _deferred_redraw_all();
    void _draw();
    virtual void unhandled_input(const Ref<::InputEvent>& event) override;

private:
    void applyClipping();

    // Rect clipping support
    bool _clippingEnabled;
    Rect2 _clippingRegion;

    // Stencil clipping support
    Node* _stencil;
    float _alphaThreshold;
    bool _inverted;
};

// Internal use - inner container
class FUIInnerContainer : public FUIContainer
{
    GDCLASS(FUIInnerContainer, FUIContainer)
public:
    FUIInnerContainer() = default;
    static void _bind_methods() {}
};

NS_FGUI_END

#endif
