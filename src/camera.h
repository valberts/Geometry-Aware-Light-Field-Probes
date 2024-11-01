// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

class Camera {
public:
    Camera(Window* pWindow);
    Camera(Window* pWindow, const glm::vec3& position, const glm::vec3& forward, float moveSpeed, float lookSpeed);

    void updateInput();
    void setUserInteraction(bool enabled);
    void setPos(glm::vec3 newPos);
    void setForward(glm::vec3 newForward);
    void setUp(glm::vec3 newUp);
    void setPosition(glm::vec3 newPos);

    glm::vec3 cameraPos() const;
    glm::vec3 forward() const;
    glm::vec3 up() const;
    glm::mat4 viewMatrix() const;

    std::string toString() const {
        std::ostringstream ss;
        ss << "Camera {" << "\n"
            << "  Position: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")\n"
            << "  Forward: (" << m_forward.x << ", " << m_forward.y << ", " << m_forward.z << ")\n"
            << "  Up: (" << m_up.x << ", " << m_up.y << ", " << m_up.z << ")\n"
            << "}";
        return ss.str();
    }

private:
    void rotateX(float angle);
    void rotateY(float angle);

private:
    static constexpr glm::vec3 s_yAxis{ 0, 1, 0 };
    glm::vec3 m_position{ 0 };
    glm::vec3 m_forward{ 0, 0, -1 };
    glm::vec3 m_up{ 0, 1, 0 };
    float m_moveSpeed{ 0.0f };
    float m_lookSpeed{ 0.0f };
    int m_previousCameraMode{ 0 };
    int m_cameraMode{ 0 }; // 0 = freecam, 1 = top down, 2 = 3rd person

    const Window* m_pWindow;
    bool m_userInteraction{ true };
    glm::dvec2 m_prevCursorPos{ 0 };
};
