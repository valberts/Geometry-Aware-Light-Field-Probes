#include <unordered_map>
#include <memory>
#include "mesh.h"
#include "texture.h"
#include <framework/shader.h>

class MeshManager {
public:
    MeshManager() = default;

    void loadMesh(const std::string& name, const std::string& filepath) {
        auto meshes = GPUMesh::loadMeshGPU(filepath);
        meshesMap[name] = std::move(meshes);
    }

    std::vector<GPUMesh>& getMeshes(const std::string& name) {
        return meshesMap.at(name);
    }

private:
    std::unordered_map<std::string, std::vector<GPUMesh>> meshesMap;
};

class ShaderManager {
public:
    ShaderManager() = default;

    void loadShader(const std::string& name, const std::vector<std::pair<GLenum, std::string>>& stages) {
        ShaderBuilder builder;
        for (const auto& [type, path] : stages) {
            builder.addStage(type, path);
        }
        shaders[name] = builder.build();
    }

    Shader& getShader(const std::string& name) {
        return shaders.at(name);
    }

private:
    std::unordered_map<std::string, Shader> shaders;
};

class TextureManager {
public:
    TextureManager() = default; 

    void loadTexture(const std::string& name, const std::filesystem::path& filepath) {
        textures[name] = std::make_unique<Texture>(filepath);
    }

    Texture* getTexture(const std::string& name) {
        auto it = textures.find(name);
        if (it != textures.end()) {
            return it->second.get();
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
};