// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
DISABLE_WARNINGS_POP()

class Player {
public:
    Player()
        : m_position(glm::vec3(0.0f)), m_direction(glm::vec3(0.0f)), m_matrix(glm::mat4(1.0f)), m_speed(0.1f) {}

    const glm::vec3& getPosition() const { return m_position; }
    void setPosition(const glm::vec3& position) { m_position = position; }

    const glm::vec3& getDirection() const { return m_direction; }
    void setDirection(const glm::vec3& direction) { m_direction = direction; }

    const glm::mat4& getMatrix() const { return m_matrix; }
    void setMatrix(const glm::mat4& matrix) { m_matrix = matrix; }

    const float& getSpeed() const { return m_speed; }
    void setSpeed(const float& speed) { m_speed = speed; }

private:
    glm::vec3 m_position; 
    glm::vec3 m_direction;
    glm::mat4 m_matrix;
    float m_speed;
};
