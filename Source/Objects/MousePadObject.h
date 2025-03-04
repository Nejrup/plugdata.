/*
 // Copyright (c) 2021-2022 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

// ELSE mousepad
class MousePadObject final : public ObjectBase {
    bool isPressed = false;

    Point<int> lastPosition;

public:
    MousePadObject(void* ptr, Object* object)
        : ObjectBase(ptr, object)
        , mouseListener(this)
    {
        mouseListener.globalMouseDown = [this](MouseEvent const& e) {
            auto relativeEvent = e.getEventRelativeTo(this);

            if (!getLocalBounds().contains(relativeEvent.getPosition()) || !isLocked() || !cnv->isShowing() || isPressed)
                return;

            t_atom at[3];
            SETFLOAT(at, 1.0f);

            if (auto pad = this->ptr.get<t_fake_pad>()) {
                pad->x_x = relativeEvent.getPosition().x;
                pad->x_y = getHeight() - relativeEvent.getPosition().y;

                outlet_anything(pad->x_obj.ob_outlet, pd->generateSymbol("click"), 1, at);
            }

            isPressed = true;
        };
        mouseListener.globalMouseUp = [this](MouseEvent const& e) {
            if (!getScreenBounds().contains(e.getMouseDownScreenPosition()) || !isPressed || !isLocked() || !cnv->isShowing())
                return;

            if (auto pad = this->ptr.get<t_fake_pad>()) {
                t_atom at[1];
                SETFLOAT(at, 0);
                outlet_anything(pad->x_obj.ob_outlet, pd->generateSymbol("click"), 1, at);
            }

            isPressed = false;
        };

        mouseListener.globalMouseMove = [this](MouseEvent const& e) {
            if ((!getScreenBounds().contains(e.getMouseDownScreenPosition()) && !isPressed) || !isLocked() || !cnv->isShowing())
                return;

            auto relativeEvent = e.getEventRelativeTo(this);

            // Don't repeat values
            if (relativeEvent.getPosition() == lastPosition)
                return;

            int xPos = relativeEvent.getPosition().x;
            int yPos = getHeight() - relativeEvent.getPosition().y;

            lastPosition = relativeEvent.getPosition();

            if (auto pad = this->ptr.get<t_fake_pad>()) {
                pad->x_x = xPos;
                pad->x_y = yPos;

                t_atom at[3];
                SETFLOAT(at, xPos);
                SETFLOAT(at + 1, yPos);

                outlet_anything(pad->x_obj.ob_outlet, gensym("list"), 2, at);
            }
        };

        mouseListener.globalMouseDrag = [this](MouseEvent const& e) {
            mouseListener.globalMouseMove(e);
        };

        setInterceptsMouseClicks(false, false);
    }

    ~MousePadObject() = default;

    void paint(Graphics& g) override
    {
        auto* x = ptr.getRaw<t_fake_pad>();
        auto fillColour = Colour(x->x_color[0], x->x_color[1], x->x_color[2]);
        g.setColour(fillColour);
        g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), Corners::objectCornerRadius);

        auto outlineColour = object->findColour(object->isSelected() && !cnv->isGraph ? PlugDataColour::objectSelectedOutlineColourId : PlugDataColour::outlineColourId);

        g.setColour(outlineColour);
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), Corners::objectCornerRadius, 1.0f);
    };

    void setPdBounds(Rectangle<int> b) override
    {
        if (auto pad = ptr.get<t_fake_pad>()) {
            auto* patch = cnv->patch.getPointer().get();
            if (!patch)
                return;

            libpd_moveobj(patch, pad.cast<t_gobj>(), b.getX(), b.getY());
            pad->x_w = b.getWidth() - 1;
            pad->x_h = b.getHeight() - 1;
        }
    }

    Rectangle<int> getPdBounds() override
    {
        if (auto gobj = ptr.get<t_gobj>()) {
            auto* patch = cnv->patch.getPointer().get();
            if (!patch)
                return {};

            int x = 0, y = 0, w = 0, h = 0;
            libpd_get_object_bounds(patch, gobj.get(), &x, &y, &w, &h);

            return { x, y, w + 1, h + 1 };
        }

        return {};
    }

    // Check if top-level canvas is locked to determine if we should respond to mouse events
    bool isLocked()
    {
        // Find top-level canvas
        auto* topLevel = findParentComponentOfClass<Canvas>();
        while (auto* nextCanvas = topLevel->findParentComponentOfClass<Canvas>()) {
            topLevel = nextCanvas;
        }

        return static_cast<bool>(topLevel->locked.getValue() || topLevel->commandLocked.getValue());
    }

    std::vector<hash32> getAllMessages() override
    {
        return {
            hash("color"),
        };
    }

    void receiveObjectMessage(String const& symbol, std::vector<pd::Atom>& atoms) override
    {
        switch (hash(symbol)) {
        case hash("color"): {
            repaint();
            break;
        }
        default:
            break;
        }
    }

    GlobalMouseListener mouseListener;
};
