#pragma once

#include <unordered_set>
#include <glm/glm.hpp>
#include <ECS/Event.hpp>

enum class ButtonState
{
    None,
    Pressed,
    Held,
    Released
};

enum class Key : uint32_t
{
    Num_0, Num_1, Num_2, Num_3, Num_4, Num_5, Num_6, Num_7, Num_8, Num_9,

    Comma, Dot, ForwardSlash, SemiColon, Apostrophe, LeftBracket, RightBracket, BackwardSlash, Dash, Equals, Accent,

    Escape, LeftControl, RightControl, LeftShift, RightShift, LeftAlt, RightAlt, Tab, LeftSuper, RightSuper, Menu, Space, Enter, Insert, Backspace, Delete, Home, End, PageUp, PageDown,

    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    NumPad_0, NumPad_1, NumPad_2, NumPad_3, NumPad_4, NumPad_5, NumPad_6, NumPad_7, NumPad_8, NumPad_9,

    NumPad_Dot, NumPad_Enter, NumPad_ForwardSlash, NumPad_Asterisk, NumPad_Dash,

    Up, Down, Left, Right
};

template<>
struct std::hash<Key>
{
    size_t operator()(const Key& key) const { return std::hash<uint32_t>()(uint32_t(key)); }
};

class KeyboardDevice
{
private:
    std::unordered_set<Key> m_pressedKeys;
    std::unordered_set<Key> m_heldKeys;
    std::unordered_set<Key> m_releasedKeys;
protected:
    void TriggerKeyDown(Key key)
    {
        OnKeyDown(key);
        m_pressedKeys.insert(key);
    }

    void TriggerKeyUp(Key key)
    {
        OnKeyUp(key);

        m_pressedKeys.erase(key);
        m_heldKeys.erase(key);

        m_releasedKeys.insert(key);
    }

    void Update()
    {
        for(Key key : m_pressedKeys)
            m_heldKeys.insert(key);

        m_pressedKeys.clear();
        m_releasedKeys.clear();
    }
public:
    KeyboardDevice()
    {

    }

    KeyboardDevice(const KeyboardDevice&) = delete;

    ECS::Event<Key> OnKeyDown;
    ECS::Event<Key> OnKeyUp;

    KeyboardDevice& operator=(const KeyboardDevice&) = delete;

    ButtonState GetKeyState(Key key) const
    {
        if(m_pressedKeys.find(key) != m_pressedKeys.end())
            return ButtonState::Pressed;
        else if(m_heldKeys.find(key) != m_heldKeys.end())
            return ButtonState::Held;
        else if(m_releasedKeys.find(key) != m_releasedKeys.end())
            return ButtonState::Released;

        return ButtonState::None;
    }

    bool IsKeyDown(Key key) const
    { 
        ButtonState keyState = GetKeyState(key);
        return keyState == ButtonState::Pressed || keyState == ButtonState::Held;
    }

    friend class Game;
};

enum class MouseButton : uint32_t
{
    Left, Middle, Right
};

template<>
struct std::hash<MouseButton>
{
    size_t operator()(const MouseButton& button) const { return std::hash<uint32_t>()(uint32_t(button)); }
};

class MouseDevice
{
private:
    std::unordered_set<MouseButton> m_pressedButtons;
    std::unordered_set<MouseButton> m_heldButtons;
    std::unordered_set<MouseButton> m_releasedButtons;

    glm::vec2 m_prevPosition;
    glm::vec2 m_currPosition;
    glm::vec2 m_downPosition;

    glm::ivec2 m_prevPixelPosition;
    glm::ivec2 m_currPixelPosition;
    glm::ivec2 m_downPixelPosition;
protected:
    void TriggerButtonDown(MouseButton button)
    {
        OnButtonDown(button);
        m_pressedButtons.insert(button);
        m_downPosition = m_currPosition;
        m_downPixelPosition = GetPixelPosition();
    }

    void TriggerButtonUp(MouseButton button)
    {
        OnButtonUp(button);
        m_pressedButtons.erase(button);
        m_heldButtons.erase(button);

        m_releasedButtons.insert(button);
    }

    void TriggerMove(const glm::vec2& position, const glm::ivec2& pixelPosition)
    {
        m_prevPosition = m_currPosition;
        m_currPosition = position;

        m_prevPixelPosition = m_currPixelPosition;
        m_currPixelPosition = pixelPosition;

        OnMove(m_prevPosition, m_currPosition);
    }

    void Update()
    {
        m_prevPosition      = m_currPosition;
        m_prevPixelPosition = m_currPixelPosition;

        for(MouseButton button : m_pressedButtons)
        {
            m_heldButtons.insert(button);
        }

        m_pressedButtons.clear();
        m_releasedButtons.clear();
    }
public:
    MouseDevice() : 
        m_prevPosition(0), 
        m_currPosition(0),
        m_downPosition(0), 
        m_prevPixelPosition(0), 
        m_currPixelPosition(0),
        m_downPixelPosition(0) {}

    MouseDevice(const MouseDevice&) = delete;

    ECS::Event<MouseButton> OnButtonDown;
    ECS::Event<MouseButton> OnButtonUp;

    ECS::Event<glm::vec2, glm::vec2> OnMove;

    MouseDevice& operator=(const MouseDevice&) = delete;

    ButtonState GetButtonState(MouseButton button) const
    {
        if(m_pressedButtons.contains(button))
        {
            return ButtonState::Pressed;
        }
        else if(m_heldButtons.contains(button))
        {
            return ButtonState::Held;
        }
        else if(m_releasedButtons.contains(button))
        {
            return ButtonState::Released;
        }
        return ButtonState::None;
    }

    bool IsButtonDown(MouseButton button) const
    {
        ButtonState buttonState = GetButtonState(button);
        return buttonState == ButtonState::Pressed || buttonState == ButtonState::Held;
    }

    glm::vec2  GetPosition()          const { return m_currPosition; }
    glm::vec2  GetDownPosition()      const { return m_downPosition; }
    glm::ivec2 GetPixelPosition()     const { return m_currPixelPosition; }
    glm::ivec2 GetDownPixelPosition() const { return m_downPixelPosition; }

    glm::vec2 GetDelta()      const { return m_currPosition      - m_prevPosition;      }
    glm::vec2 GetPixelDelta() const { return m_currPixelPosition - m_prevPixelPosition; }

    void ResetDownPosition()
    {
        m_downPosition      = m_currPosition;
        m_downPixelPosition = GetPixelPosition();
    }

    virtual void SetPixelPosition(const glm::ivec2& position) = 0;

    virtual void SetPosition(const glm::vec2& position) = 0;

    friend class Game;
};