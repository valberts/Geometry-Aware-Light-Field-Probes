//#include "Image.h"
#include "mesh.h"
#include "texture.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <imgui/imgui.h>
#include <thread>
#include <chrono>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/window.h>
#include <functional>
#include <iostream>
#include <vector>
#include "camera.h"

class Application {
public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024), OpenGLVersion::GL45)
        , m_camera(&m_window, glm::vec3(-1.0f, 0.2f, -0.5f), glm::vec3(1.0f, 0.0f, 0.4f), 0.03f, 0.0035f) // setup camera with position, forward, move speed, and look speed
        , m_texture("resources/checkerboard.png")
    {
        m_window.registerKeyCallback([this](int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS)
                onKeyPressed(key, mods);
            else if (action == GLFW_RELEASE)
                onKeyReleased(key, mods);
        });
        m_window.registerMouseMoveCallback(std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        m_window.registerMouseButtonCallback([this](int button, int action, int mods) {
            if (action == GLFW_PRESS)
                onMouseClicked(button, mods);
            else if (action == GLFW_RELEASE)
                onMouseReleased(button, mods);
        });

        m_meshes = GPUMesh::loadMeshGPU("resources/dragon.obj");

        try {
            ShaderBuilder defaultBuilder;
            defaultBuilder.addStage(GL_VERTEX_SHADER, "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();

            ShaderBuilder shadowBuilder;
            shadowBuilder.addStage(GL_VERTEX_SHADER, "shaders/shadow_vert.glsl");
            m_shadowShader = shadowBuilder.build();

            ShaderBuilder lightBuilder;
            lightBuilder.addStage(GL_VERTEX_SHADER, "shaders/light_vertex.glsl");
            lightBuilder.addStage(GL_FRAGMENT_SHADER, "shaders/light_frag.glsl");
            m_lightShader = lightBuilder.build();

            // Any new shaders can be added below in similar fashion.
            // ==> Don't forget to reconfigure CMake when you do!
            //     Visual Studio: PROJECT => Generate Cache for ComputerGraphics
            //     VS Code: ctrl + shift + p => CMake: Configure => enter
            // ....
        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void update()
    {
        int dummyInteger = 0; // Initialized to 0
        while (!m_window.shouldClose()) {
            // This is your game loop
            // Put your real-time logic and rendering in here
            m_window.updateInput();
            m_camera.updateInput();

            // Use ImGui for easy input/output of ints, floats, strings, etc...
            ImGui::Begin("Window");
            ImGui::InputInt("This is an integer input", &dummyInteger); // Use ImGui::DragInt or ImGui::DragFloat for larger range of numbers.
            ImGui::Text("Value is: %i", dummyInteger); // Use C printf formatting rules (%i is a signed integer)
            ImGui::Checkbox("Use material if no texture", &m_useMaterial);
            

            if (ImGui::Button("Start Camera Motion") && !isCameraMoving && m_blist.size() > 3)
            {
                // Start camera motion
                isCameraMoving = true;
                cameraMovementTime = 0.0f;
            }

            if (isCameraMoving) {
                UpdateCameraPosition(0.5f);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }


            ImGui::End();



            // Clear the screen
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ...
            glEnable(GL_DEPTH_TEST);

            const glm::mat4 mvpMatrix = m_projectionMatrix * m_camera.viewMatrix() * m_modelMatrix;
            // Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
            // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            for (GPUMesh& mesh : m_meshes) {
                m_defaultShader.bind();
                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                if (mesh.hasTextureCoords()) {
                    m_texture.bind(GL_TEXTURE0);
                    glUniform1i(3, 0);
                    glUniform1i(4, GL_TRUE);
                    glUniform1i(5, GL_FALSE);
                } else {
                    glUniform1i(4, GL_FALSE);
                    glUniform1i(5, m_useMaterial);
                }
                mesh.draw(m_defaultShader);
            }

            // Render points ----
            m_lightShader.bind();
            for (int i = 0; i < m_blist.size(); i++) {
                const glm::vec4 screenPos = mvpMatrix * glm::vec4(m_blist[i], 1.0f);
                const glm::vec3 color{ 1, 0, 0 };

                glPointSize(15.0f);
                glUniform4fv(0, 1, glm::value_ptr(screenPos));
                glUniform3fv(1, 1, glm::value_ptr(color));
                glDrawArrays(GL_POINTS, 0, 1);
            }
            // render points -----

            // Processes input and swaps the window buffer
            m_window.swapBuffers();
        }
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods)
    {
        std::cout << "Key pressed: " << key << std::endl;
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods)
    {
        std::cout << "Key released: " << key << std::endl;
    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(const glm::dvec2& cursorPos)
    {
        m_mousePos = cursorPos;
        std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
    }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseClicked(int button, int mods)
    {
        float x, y;
        switch (button) {
        case 0: // left
            break;
        case 1: // right
            /*glm::ivec2 size = m_window.getWindowSize();
            x = (2.0f * m_mousePos.x) / size.x - 1.0f;
            y = 1.0f - (2.0f * m_mousePos.y) / size.y;*/
            //x = 1.0f - (2.0f * m_mousePos.x) / size.x;
            //y = (2.0f * m_mousePos.y) / size.y - 1.0f;
            //m_blist.push_back(glm::vec3(float(x), float(y), 0.0f)); // TODO: set z
            m_blist.push_back(glm::vec3(1.0f, 0.0f, 0.5f));
            m_blist.push_back(glm::vec3(1.0f, 0.0f, 1.5f));
            m_blist.push_back(glm::vec3(0.0f, 0.0f, 1.5f));
            m_blist.push_back(glm::vec3(0.0f, 0.0f, 0.5f));
            
            break;
        case 2: // middle
            break;
        default:
            break;
        }
        std::cout << "Pressed mouse button: " << button << std::endl;
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseReleased(int button, int mods)
    {
        std::cout << "Released mouse button: " << button << std::endl;
    }

    // calculate a point on the Bezier curve at time t
    glm::vec3 getPt(glm::vec3 start, glm::vec3 end, float t)
    {
        return start + ((end - start) * t);
    }

    glm::vec3 CalculateBezierPoint(float i) {
        glm::vec3 posA = getPt(m_blist[0], m_blist[1], i);
        glm::vec3 posB = getPt(m_blist[1], m_blist[2], i);
        glm::vec3 posC = getPt(m_blist[2], m_blist[3], i);

        glm::vec3 posAB = getPt(posA, posB, i);
        glm::vec3 posBC = getPt(posB, posC, i);

        return getPt(posAB, posBC, i);
    }

    void UpdateCameraPosition(float deltaTime)
    {
        //std::cout << "move" << std::endl;
        if (isCameraMoving)
        {
            // Update camera position along Bezier path
            cameraMovementTime += deltaTime;
            if (cameraMovementTime >= cameraMovementDuration)
            {
                // Camera reached the end of the path
                isCameraMoving = false;
                cameraMovementTime = 0.0f;
            }

            // Calculate new camera position based on Bezier path and cameraMovementTime
            m_camera.setPos(CalculateBezierPoint(cameraMovementTime / cameraMovementDuration));
        }
    }

private:
    Window m_window;
    Camera m_camera;

    // Shader for default rendering and for depth rendering
    Shader m_defaultShader;
    Shader m_shadowShader;
    Shader m_lightShader;

    std::vector<GPUMesh> m_meshes;
    Texture m_texture;
    bool m_useMaterial { true };
    //bool m_drawPoint{ false };
    glm::dvec2 m_mousePos;
    std::vector<glm::vec3> m_blist; // point list for bezier
    bool isCameraMoving = false;
    float cameraMovementTime = 0.0f; // Time elapsed during camera movement
    float cameraMovementDuration = 25.0f; // Total duration for camera movement (adjust as needed)

    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 0.1f, 30.0f);
    //glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_viewMatrix = glm::lookAt(m_camera.cameraPos(), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix { 1.0f };
};

int main()
{
    Application app;
    app.update();

    return 0;
}
