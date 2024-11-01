#include "camera.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()

Camera::Camera(Window* pWindow)
    : Camera(pWindow, glm::vec3(0), glm::vec3(0, 0, -1), 0.0f, 0.0f)
{
}

Camera::Camera(Window* pWindow, const glm::vec3& pos, const glm::vec3& forward, float moveSpeed, float lookSpeed)
    : m_position(pos)
    , m_forward(glm::normalize(forward))
    , m_pWindow(pWindow)
    , m_moveSpeed(moveSpeed)
    , m_lookSpeed(lookSpeed)
    , m_cameraMode(0)
    , m_previousCameraMode(0)
{
}


void Camera::setUserInteraction(bool enabled)
{
    m_userInteraction = enabled;
}

glm::vec3 Camera::cameraPos() const
{
    return m_position;
}

glm::vec3 Camera::forward() const {
    return m_forward;
}

glm::vec3 Camera::up() const {
    return m_up;
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_forward, m_up);
}

void Camera::rotateX(float angle)
{
    const glm::vec3 horAxis = glm::cross(s_yAxis, m_forward);

    m_forward = glm::normalize(glm::angleAxis(angle, horAxis) * m_forward);
    m_up = glm::normalize(glm::cross(m_forward, horAxis));
}

void Camera::rotateY(float angle)
{
    const glm::vec3 horAxis = glm::cross(s_yAxis, m_forward);

    m_forward = glm::normalize(glm::angleAxis(angle, s_yAxis) * m_forward);
    m_up = glm::normalize(glm::cross(m_forward, horAxis));
}

void Camera::updateInput()
{
    if (m_userInteraction) {
        glm::vec3 localMoveDelta{ 0 };
        const glm::vec3 right = glm::normalize(glm::cross(m_forward, m_up));
        if (m_pWindow->isKeyPressed(GLFW_KEY_A))
            m_position -= m_moveSpeed * right;
        if (m_pWindow->isKeyPressed(GLFW_KEY_D))
            m_position += m_moveSpeed * right;
        if (m_pWindow->isKeyPressed(GLFW_KEY_W))
            m_position += m_moveSpeed * m_forward;
        if (m_pWindow->isKeyPressed(GLFW_KEY_S))
            m_position -= m_moveSpeed * m_forward;
        if (m_pWindow->isKeyPressed(GLFW_KEY_R))
            m_position += m_moveSpeed * m_up;
        if (m_pWindow->isKeyPressed(GLFW_KEY_F))
            m_position -= m_moveSpeed * m_up;

        const glm::dvec2 cursorPos = m_pWindow->getCursorPos();
        const glm::vec2 delta = m_lookSpeed * glm::vec2(cursorPos - m_prevCursorPos);
        m_prevCursorPos = cursorPos;

        if (m_pWindow->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && !ImGui::GetIO().WantCaptureMouse) {
            if (delta.x != 0.0f)
                rotateY(delta.x);
            if (delta.y != 0.0f)
                rotateX(delta.y);
        }
    }
    else {
        m_prevCursorPos = m_pWindow->getCursorPos();
    }
}

void Camera::setPos(glm::vec3 newPos)
{
    if (m_userInteraction) {
        m_position = newPos;
        const glm::vec3 horAxis = glm::cross(s_yAxis, m_forward);

        m_forward = glm::normalize(-newPos);
        m_up = glm::normalize(glm::cross(m_forward, horAxis));
    }
    else {
        m_prevCursorPos = m_pWindow->getCursorPos();
    }
}

void Camera::setPosition(glm::vec3 newPos) {
    m_position = newPos;
}

void Camera::setForward(glm::vec3 newForward) {
    m_forward = newForward;
}

void Camera::setUp(glm::vec3 newUp) {
    m_up = newUp;
}