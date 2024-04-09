// #include "Image.h"
#include "mesh.h"
#include "texture.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>
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
#include <random>
#include "camera.h"
#include "managers.cpp"
#include "player.cpp"
#include "fire.h"
#include <stb/stb_image.h>

class Application
{
public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024), OpenGLVersion::GL45), m_camera(&m_window, glm::vec3(-1.0f, 0.2f, -0.5f), glm::vec3(1.0f, 0.0f, 0.4f), 0.03f, 0.0035f) // setup camera with position, forward, move speed, and look speed
    {
        m_window.registerKeyCallback([this](int key, int scancode, int action, int mods)
                                     {
            if (action == GLFW_PRESS)
                onKeyPressed(key, mods);
            else if (action == GLFW_RELEASE)
                onKeyReleased(key, mods); });
        m_window.registerMouseMoveCallback(std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        m_window.registerMouseButtonCallback([this](int button, int action, int mods)
                                             {
            if (action == GLFW_PRESS)
                onMouseClicked(button, mods);
            else if (action == GLFW_RELEASE)
                onMouseReleased(button, mods); });

        setupShaders();
        loadMeshes();
        loadTextures();
        setupSkybox();
    }

    void setupShaders()
    {
        try
        {
            m_shaderManager->loadShader("default", {{GL_VERTEX_SHADER, "shaders/default_vert.glsl"},
                                                    {GL_FRAGMENT_SHADER, "shaders/default_frag.glsl"}});
            m_shaderManager->loadShader("skybox", { {GL_VERTEX_SHADER, "shaders/skybox_vert.glsl"},
                                        {GL_FRAGMENT_SHADER, "shaders/skybox_frag.glsl"} });
            m_shaderManager->loadShader("shadow", {{GL_VERTEX_SHADER, "shaders/shadow_vert.glsl"}});
            m_shaderManager->loadShader("fire", { {GL_VERTEX_SHADER, "shaders/fire_vert.glsl"},
                                                  {GL_FRAGMENT_SHADER, "shaders/fire_frag.glsl"} });
            m_shaderManager->loadShader("point", { {GL_VERTEX_SHADER, "shaders/point_vert.glsl"},
                                      {GL_FRAGMENT_SHADER, "shaders/point_frag.glsl"} });

            // Any new shaders can be added below in similar fashion.
            // ==> Don't forget to reconfigure CMake when you do!
            //     Visual Studio: PROJECT => Generate Cache for ComputerGraphics
            //     VS Code: ctrl + shift + p => CMake: Configure => enter
            // ....
        }
        catch (ShaderLoadingException e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    void loadMeshes()
    {
        m_meshManager->loadMesh("dragon", "resources/dragon.obj");
        m_meshManager->loadMesh("cube", "resources/cube.obj");
        m_meshManager->loadMesh("floor", "resources/floor.obj");
        m_meshManager->loadMesh("sphere", "resources/sphere.obj");
    }

    void loadTextures()
    {
        m_textureManager->loadTexture("checkerboard", "resources/checkerboard.png");
        m_textureManager->loadTexture("cube", "resources/texture.png");

        m_textureManager->loadTexture("floor_albedo", "resources/floor_albedo.png");
        m_textureManager->loadTexture("floor_normal", "resources/floor_normal.png");

        m_textureManager->loadTexture("metal_albedo", "resources/metal/metal_albedo.jpg");
        m_textureManager->loadTexture("metal_normal", "resources/metal/metal_normal.jpg");
        m_textureManager->loadTexture("metal_metallic", "resources/metal/metal_metallic.jpg");
        m_textureManager->loadTexture("metal_roughness", "resources/metal/metal_roughness.jpg");

        m_textureManager->loadTexture("gold_albedo", "resources/gold/gold_albedo.png");
        m_textureManager->loadTexture("gold_normal", "resources/gold/gold_normal.png");
        m_textureManager->loadTexture("gold_metallic", "resources/gold/gold_metallic.png");
        m_textureManager->loadTexture("gold_roughness", "resources/gold/gold_roughness.png");
    }

    // https://github.com/VictorGordan/opengl-tutorials/tree/main/YoutubeOpenGL%2019%20-%20Cubemaps%20%26%20Skyboxes
    // https://www.youtube.com/watch?v=8sVvxeKI9Pk OpenGL Tutorial 19 - Cubemaps & Skyboxes
    void setupSkybox() {
        float skyboxVertices[] =
        {
            //   Coordinates
            -1.0f, -1.0f,  1.0f,//        7--------6
             1.0f, -1.0f,  1.0f,//       /|       /|
             1.0f, -1.0f, -1.0f,//      4--------5 |
            -1.0f, -1.0f, -1.0f,//      | |      | |
            -1.0f,  1.0f,  1.0f,//      | 3------|-2
             1.0f,  1.0f,  1.0f,//      |/       |/
             1.0f,  1.0f, -1.0f,//      0--------1
            -1.0f,  1.0f, -1.0f
        };

        unsigned int skyboxIndices[] =
        {
            // Right
            1, 2, 6,
            6, 5, 1,
            // Left
            0, 4, 7,
            7, 3, 0,
            // Top
            4, 5, 6,
            6, 7, 4,
            // Bottom
            0, 3, 2,
            2, 1, 0,
            // Back
            0, 1, 5,
            5, 4, 0,
            // Front
            3, 7, 6,
            6, 2, 3
        };

        // Create VAO, VBO, and EBO for the skybox
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glGenBuffers(1, &skyboxEBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // All the faces of the cubemap (make sure they are in this exact order)
        std::string facesCubemap[6] =
        {
            "resources/skybox/right.jpg",
            "resources/skybox/left.jpg",
            "resources/skybox/top.jpg",
            "resources/skybox/bottom.jpg",
            "resources/skybox/front.jpg",
            "resources/skybox/back.jpg"
        };

        // Creates the cubemap texture object
        glGenTextures(1, &cubemapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // These are very important to prevent seams
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        // This might help with seams on some systems
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // Cycles through all the textures and attaches them to the cubemap object
        for (unsigned int i = 0; i < 6; i++)
        {
            int width, height, nrChannels;
            unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                stbi_set_flip_vertically_on_load(false);
                glTexImage2D
                (
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    GL_RGB,
                    width,
                    height,
                    0,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    data
                );
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
                stbi_image_free(data);
            }
        }

    }

    void update()
    {
        while (!m_window.shouldClose())
        {
            // This is your game loop
            // Put your real-time logic and rendering in here
            processInput();
            updateScene();
            renderScene();
        }
    }

    void processInput()
    {
        if (m_cameraMode == 0) {
            m_camera.updateInput();
        }
        m_window.updateInput();


        // Use ImGui for easy input/output of ints, floats, strings, etc...
        ImGui::Begin("Window");
        ImGui::InputInt("Camera mode", &m_cameraMode);
        ImGui::Checkbox("Normal Mapping", &m_enableNormalMapping);
        //ImGui::Checkbox("Use material if no texture", &m_useMaterial);

        ImGui::Checkbox("Skybox", &m_renderSkybox);
        ImGui::Checkbox("Night Time", &m_night);
        ImGui::Checkbox("Light a flame", &m_flame);

        ImGui::InputInt("Snake length", &m_numBodySegments);
        ImGui::Checkbox("Animate Snake", &animateSnake);

        if (ImGui::Button("Start Camera Motion") && !isCameraMoving && m_blist.size() > 3)
        {
            // Start camera motion
            isCameraMoving = true;
            cameraMovementTime = 0.0f;
        }

        if (isCameraMoving)
        {
            UpdateCameraPosition(0.5f);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // PBR Shading window
        ImGui::End();
        ImGui::Begin("PBR");
        ImGui::ColorEdit3("Albedo", (float*)&m_albedo);
        ImGui::SliderFloat("Metallic", &m_metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &m_roughness, 0.0f, 1.0f);
        ImGui::End();

        // Point light window
        ImGui::Begin("Light Controls");
        // Light Position Control
        ImGui::Text("Light Position");
        ImGui::DragFloat3("Position", glm::value_ptr(m_lightPos), 0.1f, -20.0f, 20.0f);

        // Light Color Control
        ImGui::Text("Light Color");
        (ImGui::ColorEdit3("Color", glm::value_ptr(m_lightColor)));
        ImGui::End();
    }

    void updateScene() {

        // Move the player
        const float playerSpeed = 0.1f;
        if (glm::length(m_player.getDirection()) > 0.0f) { // if there's no movement, don't move
            m_player.setPosition(m_player.getPosition() + glm::normalize(m_player.getDirection()) * m_player.getSpeed()); // normalize movement vector so diagonal isn't faster
            m_player.setMatrix(glm::translate(glm::mat4(1.0f), m_player.getPosition()));
        }

        if (m_cameraMode == 0 && m_previousCameraMode != 0) { // reset to freecam parameters
            m_player.setDirection(glm::vec3(0.0f, 0.0f, 0.0f)); // reset player direction because player input isnt updated in this camera mode
            m_camera.setPosition(glm::vec3(-1.0f, 0.2f, -0.5f));
            m_camera.setForward(glm::vec3(1.0f, 0.0f, 0.4f));
            m_camera.setUp(glm::vec3(0, 1, 0));
        } else if (m_cameraMode == 1) { // set the camera in top down mode
            glm::vec3 cameraPosition = m_player.getPosition() + glm::vec3(0, 5, 0);
            m_camera.setPosition(cameraPosition);
            m_camera.setForward(glm::normalize(m_player.getPosition() - cameraPosition));
            m_camera.setUp(glm::vec3(0, 0, -1));
        }
        else if (m_cameraMode == 2) { // set the camera in 3rd person mode
            glm::vec3 cameraPosition = m_player.getPosition() + glm::vec3(0, 2, 2);
            m_camera.setPosition(cameraPosition);
            m_camera.setForward(glm::normalize(m_player.getPosition() - cameraPosition));
            m_camera.setUp(glm::vec3(0, 1, 0));
        }

        m_previousCameraMode = m_cameraMode;
    }

    void renderScene()
    {
        // Clear the screen
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ...
        glEnable(GL_DEPTH_TEST);

        // default depth
        glDepthMask(GL_TRUE);
        //glDepthFunc(GL_LEQUAL);
        //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        renderMesh("default", "floor", glm::mat4(1.0f), "floor_albedo", "floor_normal", std::nullopt, std::nullopt, glm::vec3(1.0), 0.0, 1.0);
        // PBR Texture sphere
        renderMesh("default", "sphere", glm::translate(glm::mat4(1.0f), glm::vec3(3.0, 1.0, 0.0)) , "metal_albedo", "metal_normal", "metal_metallic", "metal_roughness");
        renderMesh("default", "sphere", glm::translate(glm::mat4(1.0f), glm::vec3(3.0, 1.0, 3.0)), "gold_albedo", "gold_normal", "gold_metallic", "gold_roughness");
        // PBR sliders control material properties of dragon
        renderMesh("default", "dragon", m_player.getMatrix());

        // Had to comment some code here to see PBR shaders
        //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writes.
        //glEnable(GL_BLEND); // Enable blending.
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending.
        //renderMesh("default", "floor", glm::mat4(1.0f), "floor_diffuse", "floor_normal");
        //renderMesh("default", "dragon", m_player.getMatrix());
        //glDisable(GL_BLEND);

        // Render skybox
        if (m_renderSkybox) {
            renderSkybox();
        }

        // Render a point for the location of the point light
        renderPoint(m_lightPos, m_lightColor);

        // Render point for bezier curve
        if (!m_blist.empty()) {
            for (const auto& point : m_blist) {
                renderPoint(point, glm::vec3(1.0, 0.0, 0.0));
            }
        }

        // Render snake (hierarchical box transform)
        // default length = 6
        if (animateSnake) {
            renderSnake();
        }

        // init fire (only run once)
        if (m_flame && !m_flame_init) {
            initFire(m_fire, MAX_PARTICLES, glm::vec3(-1.0, 0.0, 1.0), glm::vec3(-0.7, 0.4, 1.3)); // TODO: let user set position range
            m_flame_init = true;
        }
        else if (m_flame && m_flame_init) {
            renderFlame();
        }
        else if (!m_flame) {
            m_flame_init = false;
        }

        // Processes input and swaps the window buffer
        m_window.swapBuffers();
    }

    /**
     * Renders a mesh with specified shader, transformation matrices, and optional textures and material properties.
     *
     * @param shaderName The name of the shader to use for rendering.
     * @param meshName The name of the mesh to render.
     * @param modelMatrix The Model matrix for transforming the mesh.
     * @param textureName (Optional) The name of the diffuse texture to apply to the mesh. If not provided, the mesh is rendered without a texture.
     * @param normalMapName (Optional) The name of the normal map to apply to the mesh. If not provided, normal mapping is not used.
     * @param metallicMapName (Optional) The name of the metallic map to apply to the mesh. This enhances the metallic look on the material. If not provided, a default metallic value is used.
     * @param roughnessMapName (Optional) The name of the roughness map to apply to the mesh. This affects the material's roughness. If not provided, a default roughness value is used.
     * @param albedo (Optional) The albedo color of the material. Uses m_albedo if not specified.
     * @param metallic (Optional) The metallic property of the material. Uses m_metallic if not specified. Ignored if metallicMapName is provided.
     * @param roughness (Optional) The roughness property of the material. Uses m_roughness if not specified. Ignored if roughnessMapName is provided.
     */

    void renderMesh(
        const std::string& shaderName,
        const std::string& meshName,
        const glm::mat4& modelMatrix,
        const std::optional<std::string>& textureName = std::nullopt,
        const std::optional<std::string>& normalMapName = std::nullopt,
        const std::optional<std::string>& metallicMapName = std::nullopt, // Optional metallic map
        const std::optional<std::string>& roughnessMapName = std::nullopt, // Optional roughness map
        const std::optional<glm::vec3>& albedo = std::nullopt,
        const std::optional<float>& metallic = std::nullopt,
        const std::optional<float>& roughness = std::nullopt)
    {
        auto& meshes = m_meshManager->getMeshes(meshName);
        const glm::mat4 mvpMatrix = m_projectionMatrix * m_camera.viewMatrix() * modelMatrix;
        const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(modelMatrix));

        for (auto& mesh : meshes)
        {
            Shader& shader = m_shaderManager->getShader(shaderName);
            shader.bind();

            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

            // Albedo Map
            if (mesh.hasTextureCoords() && textureName.has_value())
            {
                Texture* texture = m_textureManager->getTexture(*textureName);
                texture->bind(GL_TEXTURE0);
                glUniform1i(3, 0); // Texture unit 0
                glUniform1i(4, GL_TRUE);
            }
            else
            {
                glUniform1i(4, GL_FALSE);
            }

            // Normal Map
            if (normalMapName.has_value() && m_enableNormalMapping)
            {
                Texture* normalMap = m_textureManager->getTexture(*normalMapName);
                normalMap->bind(GL_TEXTURE1);
                glUniform1i(5, 1); // Texture unit 1
                glUniform1i(6, GL_TRUE);
            }
            else
            {
                glUniform1i(6, GL_FALSE); // Disable normal mapping in the shader
            }

            // Metallic Map
            if (metallicMapName.has_value())
            {
                Texture* metallicMap = m_textureManager->getTexture(*metallicMapName);
                metallicMap->bind(GL_TEXTURE2);
                glUniform1i(7, 2); // Texture unit 2
                glUniform1i(8, GL_TRUE);
            }
            else
            {
                glUniform1i(8, GL_FALSE);
                // Use provided metallic value or fallback to member variable
                float metallicValue = metallic.has_value() ? *metallic : m_metallic;
                glUniform1f(9, metallicValue);
            }

            // Roughness map
            if (roughnessMapName.has_value())
            {
                Texture* roughnessMap = m_textureManager->getTexture(*roughnessMapName);
                roughnessMap->bind(GL_TEXTURE3);
                glUniform1i(10, 3); // Texture unit 3
                glUniform1i(11, GL_TRUE);
            }
            else
            {
                glUniform1i(11, GL_FALSE);
                // Use provided roughness value or fallback to member variable
                float roughnessValue = roughness.has_value() ? *roughness : m_roughness;
                glUniform1f(12, roughnessValue);
            }

            // Albedo
            if (albedo.has_value())
            {
                glUniform3fv(13, 1, glm::value_ptr(*albedo));
            }
            else
            {
                glUniform3fv(13, 1, glm::value_ptr(m_albedo));
            }

            glUniform1i(14, m_useMaterial);
            glUniform3fv(15, 1, glm::value_ptr(m_lightPos));
            glUniform3fv(16, 1, glm::value_ptr(m_lightColor));
            glUniform3fv(17, 1, glm::value_ptr(m_camera.cameraPos()));

            // Use provided albedo, metallic, and roughness values or default to member variables
            glUniform3fv(13, 1, albedo.has_value() ? glm::value_ptr(*albedo) : glm::value_ptr(m_albedo));
            glUniform1f(9, metallic.has_value() ? *metallic : m_metallic);
            glUniform1f(12, roughness.has_value() ? *roughness : m_roughness);
            glUniform1i(20, m_night);

            mesh.draw(shader);
        }
    }

    void renderSkybox() {
        // Since the cubemap will always have a depth of 1.0, we need that equal sign so it doesn't get discarded
        glDepthFunc(GL_LEQUAL);

        Shader& skyboxShader = m_shaderManager->getShader("skybox");
        skyboxShader.bind();
        glm::mat4 viewMatrix = glm::mat4(1.0f);
        // We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
        // The last row and column affect the translation of the skybox (which we don't want to affect)
        viewMatrix = glm::mat4(glm::mat3(m_camera.viewMatrix()));
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        // Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
        // where an object is present (a depth of 1.0f will always fail against any object's depth value)
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Switch back to the normal depth function
        glDepthFunc(GL_LESS);
    }

    void renderPoint(glm::vec3 position, glm::vec3 color) {
        Shader& pointShader = m_shaderManager->getShader("point");
        pointShader.bind();
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix * m_camera.viewMatrix() * modelMatrix));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniform3fv(2, 1, glm::value_ptr(color));

        glPointSize(10.0f);

        // Now draw the point at the light's position
        GLfloat lightPos[] = { m_lightPos.x, m_lightPos.y, m_lightPos.z };
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lightPos), lightPos, GL_STATIC_DRAW);

        // Since you're directly passing array, no need to bind buffer here
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)lightPos);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_POINTS, 0, 1);

        // Clean up
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void renderSnake() {
        if (m_move_right)
            m_snake_pos.x += 0.05f;
        if (m_move_left)
            m_snake_pos.x -= 0.05f;
        if (m_move_down)
            m_snake_pos.z += 0.05f;
        if (m_move_up)
            m_snake_pos.z -= 0.05f;

        glm::vec3 headScale(0.08f, 0.1f, 0.2f);
        glm::vec3 headTranslation = m_snake_pos;
        glm::mat4 trans_axis_to_side_1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0, -1.0));
        glm::mat4 trans_axis_to_side_back_1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0, 1.0));

        glm::vec3 headToBody(0.0f, 0.0f, 2.0f);
        glm::mat4 trans_headToBody = glm::translate(glm::mat4(1.0f), headToBody);

        glm::mat4 headScaleMatrix = glm::scale(glm::mat4(1.0f), headScale);
        glm::mat4 headTranslationMatrix = glm::translate(glm::mat4(1.0f), headTranslation);
        if (m_head_rotationAngle >= 45.0f || m_head_rotationAngle <= -45.0f)
        {
            m_head_rotationDirection *= -1;
        }
        m_head_rotationAngle += float(m_head_rotationDirection);
        // std::cout << m_head_rotationAngle << std::endl;
        glm::mat4 rotz_1 = glm::rotate(glm::mat4(1.0f), glm::radians(m_head_rotationAngle), glm::vec3(0, 1, 0));
        glm::mat4 headModelMatrix = headTranslationMatrix * headScaleMatrix * trans_axis_to_side_1 * rotz_1 * trans_axis_to_side_back_1;

        const glm::mat4 headMvpMatrix = m_projectionMatrix * m_camera.viewMatrix() * headModelMatrix;
        const glm::mat3 headScaledNormalModelMatrix = glm::inverseTranspose(glm::mat3(headModelMatrix));
        renderMesh("default", "cube", headModelMatrix);

        glm::mat4 lastModelMatrix = headTranslationMatrix * headScaleMatrix * trans_axis_to_side_1 * rotz_1 * trans_axis_to_side_back_1;
        int rot_dir = -1;

        for (int i = 0; i < m_numBodySegments; ++i)
        {
            // add random angle to body rotation
            /*std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(0.5f, 2.0f);
            float randomNumber = dist(gen);*/
            glm::mat4 rotz_2 = glm::rotate(glm::mat4(1.0f), glm::radians(rot_dir * m_head_rotationAngle * 1.2f), glm::vec3(0, 1, 0));
            glm::mat4 bodyModelMatrix = lastModelMatrix * trans_headToBody * rotz_2;
            lastModelMatrix = bodyModelMatrix;
            rot_dir *= -1;

            // Render the current body segment
            const glm::mat4 bodyMvpMatrix = m_projectionMatrix * m_camera.viewMatrix() * bodyModelMatrix;
            const glm::mat3 bodyScaledNormalModelMatrix = glm::inverseTranspose(glm::mat3(bodyModelMatrix));
            renderMesh("default", "cube", bodyModelMatrix);
        }
    }

    void renderFlame() {
        // render flame
        for (int i = 0; i < MAX_PARTICLES; i++) {
            glm::vec3 particleScale = glm::vec3(0.004f, 0.005f, 0.004f);
            glm::mat4 particleScaleMatrix = glm::scale(glm::mat4(1.0f), particleScale);
            glm::vec3 particleTranslation = m_fire[i].pos;
            glm::mat4 particleTranslationMatrix = glm::translate(glm::mat4(1.0f), particleTranslation);
            glm::mat4 particleModelMatrix = particleTranslationMatrix * particleScaleMatrix;
            //glEnable(GL_BLEND); // Enable blending.
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending.
            renderParticle("fire", "cube", particleModelMatrix, m_fire[i].color);
            //glDisable(GL_BLEND);
        }
        // update particles
        updateFire(m_fire, MAX_PARTICLES, glm::vec3(-1.0, 0.0, 1.0), glm::vec3(-0.5, 0.6, 1.5));
    }

    void renderParticle(const std::string& shaderName,
        const std::string& meshName,
        const glm::mat4& modelMatrix,
        const std::optional<glm::vec3>& inputColor = std::nullopt)
    {
        auto& meshes = m_meshManager->getMeshes(meshName);
        const glm::mat4 mvpMatrix = m_projectionMatrix * m_camera.viewMatrix() * modelMatrix;
        // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
        const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(modelMatrix));

        for (auto& mesh : meshes)
        {
            Shader& shader = m_shaderManager->getShader(shaderName);
            shader.bind();

            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
            glUniform3fv(3, 1, glm::value_ptr(*inputColor));

            mesh.draw(shader);
        }
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods)
    {
        if (m_cameraMode != 0) {
            glm::vec3 playerDirection = m_player.getDirection();
            switch (key) {
            case GLFW_KEY_W:
                m_player.setDirection(playerDirection + glm::vec3(0.0f, 0.0f, -1.0f)); // Move forward
                break;
            case GLFW_KEY_S:
                m_player.setDirection(playerDirection + glm::vec3(0.0f, 0.0f, 1.0f)); // Move backward
                break;
            case GLFW_KEY_A:
                m_player.setDirection(playerDirection + glm::vec3(-1.0f, 0.0f, 0.0f)); // Move left
                break;
            case GLFW_KEY_D:
                m_player.setDirection(playerDirection + glm::vec3(1.0f, 0.0f, 0.0f)); // Move right
                break;
            }
        }
        switch (key)
        {
            // use UP DOWN LEFT RIGHT to move the snake on y (horizontal) plane
        case GLFW_KEY_RIGHT:
            m_move_right = true;
            break;
        case GLFW_KEY_LEFT:
            m_move_left = true;
            break;
        case GLFW_KEY_DOWN:
            m_move_down = true;
            break;
        case GLFW_KEY_UP:
            m_move_up = true;
            break;
        case GLFW_KEY_ESCAPE:
            m_window.close();
            exit(0);
            break;
        default:
            break;
        }
        std::cout << "Key pressed: " << key << std::endl;
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods)
    {
        if (m_cameraMode != 0) {
            glm::vec3 playerDirection = m_player.getDirection();
            switch (key) {
                case GLFW_KEY_W:
                    m_player.setDirection(playerDirection - glm::vec3(0.0f, 0.0f, -1.0f)); // Stop moving forward
                    break;
                case GLFW_KEY_S:
                    m_player.setDirection(playerDirection - glm::vec3(0.0f, 0.0f, 1.0f)); // Stop moving backward
                    break;
                case GLFW_KEY_A:
                    m_player.setDirection(playerDirection - glm::vec3(-1.0f, 0.0f, 0.0f)); // Stop moving left
                    break;
                case GLFW_KEY_D:
                    m_player.setDirection(playerDirection - glm::vec3(1.0f, 0.0f, 0.0f)); // Stop moving right
                    break;
            }
        }
        std::cout << "Key released: " << key << std::endl;
    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(const glm::dvec2 &cursorPos)
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
        switch (button)
        {
        case 0: // left
            break;
        case 1: // right
            /*glm::ivec2 size = m_window.getWindowSize();
            x = (2.0f * m_mousePos.x) / size.x - 1.0f;
            y = 1.0f - (2.0f * m_mousePos.y) / size.y;*/
            // x = 1.0f - (2.0f * m_mousePos.x) / size.x;
            // y = (2.0f * m_mousePos.y) / size.y - 1.0f;
            // m_blist.push_back(glm::vec3(float(x), float(y), 0.0f)); // TODO: set z

            // predefined bline control points
            m_blist.push_back(glm::vec3(-1.0f, 0.0f, -0.5f));
            m_blist.push_back(glm::vec3(-1.0f, 0.0f, -1.5f));
            m_blist.push_back(glm::vec3(0.0f, 0.0f, -1.5f));
            m_blist.push_back(glm::vec3(0.0f, 0.0f, -0.5f));
            m_blist.push_back(glm::vec3(1.0f, 0.0f, -0.5f));
            m_blist.push_back(glm::vec3(1.0f, 1.0f, -0.5f));
            m_blist.push_back(glm::vec3(0.0f, 1.0f, -0.5f));
            m_blist.push_back(glm::vec3(0.0f, 1.0f, -1.5f));
            m_blist.push_back(glm::vec3(0.0f, 0.0f, -1.5f));
            m_blist.push_back(glm::vec3(0.0f, 0.0f, -0.5f));
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

    glm::vec3 CalculateBezierPoint(float i, int s)
    {
        glm::vec3 posA, posB, posC;
        if (s == 1)
        {
            posA = getPt(m_blist[0], m_blist[1], i);
            posB = getPt(m_blist[1], m_blist[2], i);
            posC = getPt(m_blist[2], m_blist[3], i);
        }
        else if (s == 2)
        {
            posA = getPt(m_blist[3], m_blist[4], i);
            posB = getPt(m_blist[4], m_blist[5], i);
            posC = getPt(m_blist[5], m_blist[6], i);
        }
        else
        {
            posA = getPt(m_blist[6], m_blist[7], i);
            posB = getPt(m_blist[7], m_blist[8], i);
            posC = getPt(m_blist[8], m_blist[9], i);
        }

        glm::vec3 posAB = getPt(posA, posB, i);
        glm::vec3 posBC = getPt(posB, posC, i);

        return getPt(posAB, posBC, i);
    }

    void UpdateCameraPosition(float deltaTime)
    {
        // std::cout << "move" << std::endl;
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
            int s = 0;
            float timePieceDuration = cameraMovementDuration / 3;
            int cameraPosition = int(std::min((cameraMovementTime / timePieceDuration) + 1, 3.0f));
            float p_local = (cameraMovementTime - (float(cameraPosition - 1)) * timePieceDuration) / timePieceDuration;
            m_camera.setPos(CalculateBezierPoint(p_local, cameraPosition));
        }
    }

private:
    Window m_window;
    Camera m_camera;

    // Manager classes
    std::unique_ptr<MeshManager> m_meshManager = std::make_unique<MeshManager>();
    std::unique_ptr<ShaderManager> m_shaderManager = std::make_unique<ShaderManager>();
    std::unique_ptr<TextureManager> m_textureManager = std::make_unique<TextureManager>();

    glm::dvec2 m_mousePos;

    // Player object
    Player m_player;

    // Options
    bool m_enableNormalMapping = false;
    int m_previousCameraMode{ 0 };
    int m_cameraMode{ 0 }; // 0 = freecam, 1 = top down, 2 = 3rd person
    bool m_useMaterial{ true };
    bool m_renderSkybox{ false };
    bool animateSnake{ false };             // only draw snake when true

    // PBR variable
    glm::vec3 m_albedo = glm::vec3(1.0f);
    float m_metallic = 0.0f;
    float m_roughness = 0.0f;

    // Light variables
    glm::vec3 m_lightPos = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_lightColor = glm::vec3(1.0f);

    // Environment map variables
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    unsigned int cubemapTexture;

    // Bezier Variables
    std::vector<glm::vec3> m_blist; // point list for bezier
    // bool m_drawPoint{ false };
    bool isCameraMoving = false;
    float cameraMovementTime = 0.0f;      // Time elapsed during camera movement
    float cameraMovementDuration = 25.0f; // Total duration for camera movement (adjust as needed)
    
    // Snake variables
    float animationTime = 0.0f;           // Animation time
    float animationSpeed = 1.0f;          // Speed of animation
    int m_numBodySegments{5};             // num of body segements
    int m_head_rotationDirection = 1;
    float m_head_rotationAngle = 0.0f;
    glm::vec3 m_snake_pos = glm::vec3(-1.5f, 0.0f, 0.0f);
    bool m_move_right = false;
    bool m_move_left = false;
    bool m_move_down = false;
    bool m_move_up = false;

    bool m_night { false };
    bool m_flame { false };
    bool m_flame_init = false;
    Particle m_fire[MAX_PARTICLES];

    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 0.1f, 30.0f);
    // glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_viewMatrix = glm::lookAt(m_camera.cameraPos(), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix{1.0f};
};

int main()
{
    Application app;
    app.update();

    return 0;
}
