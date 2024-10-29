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
#include <stb/stb_image.h>
#include <map>


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
        loadHDR();
        setupCubemap();
        setupIrradiance();
		initializeProbeGrid(2, 2, 2, 2.0f);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    }

    void setupShaders()
    {
        try
        {
            m_shaderManager->loadShader("default", {{GL_VERTEX_SHADER, "shaders/default_vert.glsl"},
                                                    {GL_FRAGMENT_SHADER, "shaders/default_frag.glsl"}});
            m_shaderManager->loadShader("skybox", { {GL_VERTEX_SHADER, "shaders/skybox_vert.glsl"},
                                        {GL_FRAGMENT_SHADER, "shaders/skybox_frag.glsl"} });
            m_shaderManager->loadShader("cubemap", { {GL_VERTEX_SHADER, "shaders/cubemap_vert.glsl"},
                            {GL_FRAGMENT_SHADER, "shaders/cubemap_frag.glsl"} });
            m_shaderManager->loadShader("irradiance", { {GL_VERTEX_SHADER, "shaders/skybox_vert.glsl"},
                            {GL_FRAGMENT_SHADER, "shaders/irradiance_frag.glsl"} });
            m_shaderManager->loadShader("shadow", {{GL_VERTEX_SHADER, "shaders/shadow_vert.glsl"}});
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
        m_meshManager->loadMesh("sphere", "resources/sphere_highpoly.obj");
		m_meshManager->loadMesh("scene", "resources/scene.obj");
    }

    void loadTextures()
    {
        m_textureManager->loadTexture("checkerboard", "resources/checkerboard.png");
        m_textureManager->loadTexture("grass", "resources/grass.jpg");
        m_textureManager->loadTexture("archi", "resources/archi.jpg");
        m_textureManager->loadTexture("cube", "resources/texture.png");

        // https://www.poliigon.com/texture/large-concrete-panels-texture/7856
        m_textureManager->loadTexture("floor_albedo", "resources/floor_albedo.png");
        m_textureManager->loadTexture("floor_normal", "resources/floor_normal.png");


        m_textureManager->loadTexture("metal_albedo", "resources/metal/metal_albedo.jpg");
        m_textureManager->loadTexture("metal_normal", "resources/metal/metal_normal.jpg");
        m_textureManager->loadTexture("metal_metallic", "resources/metal/metal_metallic.jpg");
        m_textureManager->loadTexture("metal_roughness", "resources/metal/metal_roughness.jpg");

        // https://www.poliigon.com/texture/gold-paint-metal-texture/7253
        m_textureManager->loadTexture("gold_albedo", "resources/gold/gold_albedo.png");
        m_textureManager->loadTexture("gold_normal", "resources/gold/gold_normal.png");
        m_textureManager->loadTexture("gold_metallic", "resources/gold/gold_metallic.png");
        m_textureManager->loadTexture("gold_roughness", "resources/gold/gold_roughness.png");

        // https://www.poliigon.com/texture/exterior-painted-stucco-texture-red/7805
        m_textureManager->loadTexture("stucco_albedo", "resources/stucco/stucco_albedo.png");
        m_textureManager->loadTexture("stucco_normal", "resources/stucco/stucco_normal.png");
        m_textureManager->loadTexture("stucco_metallic", "resources/stucco/stucco_metallic.png");
        m_textureManager->loadTexture("stucco_roughness", "resources/stucco/stucco_roughness.png");
    }

    // https://learnopengl.com/PBR/IBL/Diffuse-irradiance
    // pbr: load the HDR environment map
    // ---------------------------------
    void loadHDR() {
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrComponents;
        // https://hdri-haven.com/hdri/dark-room
        float* data = stbi_loadf("resources/room.hdr", &width, &height, &nrComponents, 0);
        if (data)
        {
            glGenTextures(1, &hdrTexture);
            glBindTexture(GL_TEXTURE_2D, hdrTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load HDR image." << std::endl;
        }
    }

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    void setupCubemap() {
        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

        glGenTextures(1, &envCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
        // ----------------------------------------------------------------------------------------------
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        // pbr: convert HDR equirectangular environment map to cubemap equivalent
        // ----------------------------------------------------------------------
        Shader& cubemapShader = m_shaderManager->getShader("cubemap");
        cubemapShader.bind();

        glUniform1i(2, 0);
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(captureProjection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(captureViews[i]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // pbr: setup irradiance
    void setupIrradiance() {
        glGenTextures(1, &irradianceMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
                GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

        // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
        // ----------------------------------------------------------------------------------------------
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        Shader& irradianceShader = m_shaderManager->getShader("irradiance");
        irradianceShader.bind();
        glUniform1i(2, 0);
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(captureProjection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(captureViews[i]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport size
        glViewport(0, 0, 1024, 1024);
    }

    void initializeProbeGrid(int gridSizeX = 3, int gridSizeY = 3, int gridSizeZ = 3, float spacing = 1.0f) {
        // Calculate the offsets to center the grid around the origin
        float offsetX = (gridSizeX - 1) * spacing / 2.0f;
        float offsetZ = (gridSizeZ - 1) * spacing / 2.0f;

        for (int x = 0; x < gridSizeX; ++x) {
            for (int y = 0; y < gridSizeY; ++y) {
                for (int z = 0; z < gridSizeZ; ++z) {
                    glm::vec3 probePosition = glm::vec3(x * spacing - offsetX, y * spacing, z * spacing - offsetZ);
                    m_probePositions.push_back(probePosition);
                }
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
            render();
            // Processes input and swaps the window buffer
            m_window.swapBuffers();
        }
    }

    void processInput()
    {
        m_camera.updateInput();
        m_window.updateInput();


        //// Use ImGui for easy input/output of ints, floats, strings, etc...
        //ImGui::Begin("Window");
        //ImGui::Checkbox("Normal Mapping", &m_enableNormalMapping);
        ////ImGui::Checkbox("Use material if no texture", &m_useMaterial);
        //ImGui::Checkbox("Skybox", &m_renderSkybox);
        //ImGui::End();

        //// PBR Shading window;
        //ImGui::Begin("PBR");
        //ImGui::ColorEdit3("Albedo", (float*)&m_albedo);
        //ImGui::SliderFloat("Metallic", &m_metallic, 0.0f, 1.0f);
        //ImGui::SliderFloat("Roughness", &m_roughness, 0.0f, 1.0f);
        //ImGui::End();

        // Interpolation point window
        ImGui::Begin("Interpolation Point");
        ImGui::SliderFloat("X", &m_pointX, 0.0f, 1.0f);
        ImGui::SliderFloat("Y", &m_pointY, 0.0f, 1.0f);
        ImGui::SliderFloat("Z", &m_pointZ, 0.0f, 1.0f);
        ImGui::End();

        //// Point light window
        //ImGui::Begin("Light Controls");
        //// Light Position Control
        //ImGui::Text("Light Position");
        //ImGui::DragFloat3("Position", glm::value_ptr(m_lightPos), 0.1f, -20.0f, 20.0f);

        //// Light Color Control
        //ImGui::Text("Light Color");
        //(ImGui::ColorEdit3("Color", glm::value_ptr(m_lightColor)));
        //ImGui::End();

        // Interpolation weights view
        std::vector<float> weights = getTrilinearWeights(m_pointX, m_pointY, m_pointZ);
        ImGui::Begin("Probe Interpolation Weights");
        ImVec4 color;
        for (size_t i = 0; i < weights.size(); ++i) {
            // Map the weight to a color gradient (e.g., from blue to red)
            color = ImVec4(1.0f - weights[i], weights[i], 0.0f, 1.0f); // Red to green gradient
            ImGui::TextColored(color, "w%03d: %.2f", i, weights[i]);
        }
        ImGui::End();
    }

    void render() {
        // Render to Default Framebuffer (Main scene)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_window.getWindowSize().x, m_window.getWindowSize().y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear main framebuffer
        renderScene(); // Render the main scene
    }

    void renderScene()
    {
        // Clear the screen

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ...
        glEnable(GL_DEPTH_TEST);

        // default depth
        glDepthMask(GL_TRUE);
        //glDepthFunc(GL_LEQUAL);
        //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // Had to comment some code here to see PBR shaders
        //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writes.
        //glEnable(GL_BLEND); // Enable blending.
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending.
        //renderMesh("default", "floor", glm::mat4(1.0f), "floor_diffuse", "floor_normal");
        //renderMesh("default", "dragon", m_player.getMatrix());
        //glDisable(GL_BLEND);

        // Render a point for the location of the point light
        //renderPoint(m_lightPos, m_lightColor);

        // Render skybox
        if (m_renderSkybox) {
            renderSkybox();
        }

        // Render the interpolation point and visualize weights
        glm::vec3 interpolatedPoint = getInterpolatedPoint(m_pointX, m_pointY, m_pointZ);
        std::vector<float> weights = getTrilinearWeights(m_pointX, m_pointY, m_pointZ);

        renderProbes(weights);
        // Render the interpolation point
        renderPoint(interpolatedPoint, glm::vec3(0.0f, 1.0f, 0.0f)); // Green color
    }

    /**
     * Renders a mesh with specified shader, transformation matrices, and optional textures and material properties.
     *
     * @param shaderName The name of the shader to use for rendering.
     * @param meshName The name of the mesh to render.
     * @param modelMatrix The Model matrix for transforming the mesh.
     * @param camera The camera to render the scene with.
     * @param textureName (Optional) The name of the diffuse texture to apply to the mesh. If not provided, the mesh is rendered without a texture.
     * @param normalMapName (Optional) The name of the normal map to apply to the mesh. If not provided, normal mapping is not used.
     * @param metallicMapName (Optional) The name of the metallic map to apply to the mesh. This enhances the metallic look on the material. If not provided, a default metallic value is used.
     * @param roughnessMapName (Optional) The name of the roughness map to apply to the mesh. This affects the material's roughness. If not provided, a default roughness value is used.
     * @param albedo (Optional) The albedo color of the material. Uses m_albedo if not specified.
     * @param metallic (Optional) The metallic property of the material. Uses m_metallic if not specified. Ignored if metallicMapName is provided.
     * @param roughness (Optional) The roughness property of the material. Uses m_roughness if not specified. Ignored if roughnessMapName is provided.
     */
    struct RenderMeshOptions {
        std::string shaderName;
        std::string meshName;
        glm::mat4 modelMatrix;
        Camera camera;
        std::optional<std::string> textureName = std::nullopt;
        std::optional<std::string> normalMapName = std::nullopt;
        std::optional<std::string> metallicMapName = std::nullopt;
        std::optional<std::string> roughnessMapName = std::nullopt;
        std::optional<glm::vec3> albedo = std::nullopt;
        std::optional<float> metallic = std::nullopt;
        std::optional<float> roughness = std::nullopt;
    };
    void renderMesh(const RenderMeshOptions& options)
    {
        auto& meshes = m_meshManager->getMeshes(options.meshName);
        const glm::mat4 mvpMatrix = m_projectionMatrix * options.camera.viewMatrix() * options.modelMatrix;
        const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(options.modelMatrix));

        for (auto& mesh : meshes)
        {
            Shader& shader = m_shaderManager->getShader(options.shaderName);
            shader.bind();

            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(options.modelMatrix));
            glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

            // Albedo Map
            if (mesh.hasTextureCoords() && options.textureName.has_value())
            {
                Texture* texture = m_textureManager->getTexture(*options.textureName);
                texture->bind(GL_TEXTURE0);
                glUniform1i(3, 0); // Texture unit 0
                glUniform1i(4, GL_TRUE);
            }
            else
            {
                glUniform1i(4, GL_FALSE);
            }

            // Normal Map
            if (options.normalMapName.has_value() && m_enableNormalMapping)
            {
                Texture* normalMap = m_textureManager->getTexture(*options.normalMapName);
                normalMap->bind(GL_TEXTURE1);
                glUniform1i(5, 1); // Texture unit 1
                glUniform1i(6, GL_TRUE);
            }
            else
            {
                glUniform1i(6, GL_FALSE); // Disable normal mapping in the shader
            }

            // Metallic Map
            if (options.metallicMapName.has_value())
            {
                Texture* metallicMap = m_textureManager->getTexture(*options.metallicMapName);
                metallicMap->bind(GL_TEXTURE2);
                glUniform1i(7, 2); // Texture unit 2
                glUniform1i(8, GL_TRUE);
            }
            else
            {
                glUniform1i(8, GL_FALSE);
                // Use provided metallic value or fallback to member variable
                float metallicValue = options.metallic.has_value() ? *options.metallic : m_metallic;
                glUniform1f(9, metallicValue);
            }

            // Roughness map
            if (options.roughnessMapName.has_value())
            {
                Texture* roughnessMap = m_textureManager->getTexture(*options.roughnessMapName);
                roughnessMap->bind(GL_TEXTURE3);
                glUniform1i(10, 3); // Texture unit 3
                glUniform1i(11, GL_TRUE);
            }
            else
            {
                glUniform1i(11, GL_FALSE);
                // Use provided roughness value or fallback to member variable
                float roughnessValue = options.roughness.has_value() ? *options.roughness : m_roughness;
                glUniform1f(12, roughnessValue);
            }

            // Albedo
            if (options.albedo.has_value())
            {
                glUniform3fv(13, 1, glm::value_ptr(*options.albedo));
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
            glUniform3fv(13, 1, options.albedo.has_value() ? glm::value_ptr(*options.albedo) : glm::value_ptr(m_albedo));
            glUniform1f(9, options.metallic.has_value() ? *options.metallic : m_metallic);
            glUniform1f(12, options.roughness.has_value() ? *options.roughness : m_roughness);

            // Environment mapping irradiance
            if (m_renderSkybox) {
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
                glUniform1i(19, 4); // Texture unit 4
                glUniform1i(20, GL_TRUE);
            }
            else {
                glUniform1i(20, GL_FALSE);
            }


            mesh.draw(shader);
        }
    }

    void renderProbes(std::vector<float> weights) {
        for (size_t i = 0; i < m_probePositions.size(); ++i) {
            // Scale sphere size based on weight
            glm::vec3 color = glm::mix(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), weights[i]); // Color gradient from red to green

            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), m_probePositions[i]);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f)); // Scale down to 10%

            RenderMeshOptions probeSphereOptions{
                .shaderName = "default",
                .meshName = "sphere",
                .modelMatrix = modelMatrix,
                .camera = m_camera,
                .albedo = color,
                .metallic = 0.0f,
                .roughness = 0.5f
            };
            renderMesh(probeSphereOptions);
        }
    }

    // renderCube() renders a 1x1 3D cube in NDC.
    // -------------------------------------------------
    void renderCube()
    {
        // initialize (if necessary)
        if (cubeVAO == 0)
        {
            float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                    1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
                    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                    1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                    1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
                    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                    1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
                    // bottom face
                    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                    1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                    // top face
                    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                    1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                    1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
                    1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
            };
            glGenVertexArrays(1, &cubeVAO);
            glGenBuffers(1, &cubeVBO);
            // fill buffer
            glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            // link vertex attributes
            glBindVertexArray(cubeVAO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        // render Cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);;
        //glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
        
        renderCube();

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
    void onMouseMove(const glm::dvec2 &cursorPos)
    {
        m_mousePos = cursorPos;
        //std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
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
            break;
        case 2: // middle
            break;
        default:
            break;
        }
        //std::cout << "Pressed mouse button: " << button << std::endl;
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseReleased(int button, int mods)
    {
        //std::cout << "Released mouse button: " << button << std::endl;
    }


    glm::vec3 getInterpolatedPoint(float tX, float tY, float tZ) {
        // Clamp tX, tY, tZ to [0, 1]
        tX = glm::clamp(tX, 0.0f, 1.0f);
        tY = glm::clamp(tY, 0.0f, 1.0f);
        tZ = glm::clamp(tZ, 0.0f, 1.0f);

        glm::vec3 p000 = m_probePositions[0];
        glm::vec3 p100 = m_probePositions[1];
        glm::vec3 p010 = m_probePositions[2];
        glm::vec3 p110 = m_probePositions[3];
        glm::vec3 p001 = m_probePositions[4];
        glm::vec3 p101 = m_probePositions[5];
        glm::vec3 p011 = m_probePositions[6];
        glm::vec3 p111 = m_probePositions[7];

        // Trilinear interpolation
        glm::vec3 p00 = glm::mix(p000, p100, tX);
        glm::vec3 p01 = glm::mix(p001, p101, tX);
        glm::vec3 p10 = glm::mix(p010, p110, tX);
        glm::vec3 p11 = glm::mix(p011, p111, tX);

        glm::vec3 p0 = glm::mix(p00, p10, tY);
        glm::vec3 p1 = glm::mix(p01, p11, tY);

        return glm::mix(p0, p1, tZ);
    }

    std::vector<float> getTrilinearWeights(float tX, float tY, float tZ) {
        // Clamp tX, tY, tZ to [0, 1]
        tX = glm::clamp(tX, 0.0f, 1.0f);
        tY = glm::clamp(tY, 0.0f, 1.0f);
        tZ = glm::clamp(tZ, 0.0f, 1.0f);

        // Compute weights
        float w000 = (1 - tX) * (1 - tY) * (1 - tZ);
        float w100 = tX * (1 - tY) * (1 - tZ);
        float w010 = (1 - tX) * tY * (1 - tZ);
        float w110 = tX * tY * (1 - tZ);
        float w001 = (1 - tX) * (1 - tY) * tZ;
        float w101 = tX * (1 - tY) * tZ;
        float w011 = (1 - tX) * tY * tZ;
        float w111 = tX * tY * tZ;

        return { w000, w100, w010, w110, w001, w101, w011, w111 };
    }



    
private:
    Window m_window;
    Camera m_camera;
    std::map<std::string, std::vector<GLuint>> animations;

    // Manager classes
    std::unique_ptr<MeshManager> m_meshManager = std::make_unique<MeshManager>();
    std::unique_ptr<ShaderManager> m_shaderManager = std::make_unique<ShaderManager>();
    std::unique_ptr<TextureManager> m_textureManager = std::make_unique<TextureManager>();

    glm::dvec2 m_mousePos;

    // Options
    bool m_enableNormalMapping = false;
    bool m_useMaterial{ true };
    bool m_renderSkybox{ true };

	// Probe variables
    std::vector<glm::vec3> m_probePositions;  // Stores probe positions
	float m_pointX = 0.5f;
	float m_pointY = 0.5f;
	float m_pointZ = 0.5f;

    // PBR variable
    glm::vec3 m_albedo = glm::vec3(1.0f);
    float m_metallic = 0.0f;
    float m_roughness = 1.0f;

    // Light variables
    glm::vec3 m_lightPos = glm::vec3(0.0f, 3.0f, 0.0f);
    glm::vec3 m_lightColor = glm::vec3(0.0f);

    // Environment map variables
    unsigned int captureFBO;
    unsigned int captureRBO;
    unsigned int hdrTexture;
    unsigned int envCubemap;
    unsigned int irradianceMap;
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;

    float timer = 0.0f;
    std::list<std::string> textures = std::list<std::string>{ "checkerboard", "grass", "archi" };
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
