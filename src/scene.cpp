#include <vector>
#include <string>
#include <memory>
#include <framework/shader.h>
#include "mesh.h"
#include "texture.h"

class SceneObject {
public:
    SceneObject(std::string name) : name(std::move(name)) {}

    void addChild(std::shared_ptr<SceneObject> child) {
        children.push_back(child);
    }

    virtual void render(const Shader& shader) {
        for (auto& child : children) {
            child->render(shader);
        }
    }

protected:
    std::string name;
    std::vector<std::shared_ptr<SceneObject>> children;
};

class SceneMeshObject : public SceneObject {
public:
    // Modified constructor to accept a Texture pointer
    SceneMeshObject(const std::string& name, GPUMesh* mesh, Texture* texture = nullptr)
        : SceneObject(name), m_mesh(mesh), m_texture(texture) {}

    void render(const Shader& shader) override {
        if (m_texture) {
            m_texture->bind(GL_TEXTURE0); // Bind the texture if it exists
            shader.bind();
            glUniform1i(3, 0);
        }
        if (m_mesh) {
            m_mesh->draw(shader);
        }
        SceneObject::render(shader); // Render children
    }

private:
    GPUMesh* m_mesh; // Does not own the mesh, just references it
    Texture* m_texture; // Optional texture for the mesh
};
