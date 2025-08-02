LoadModel(obj->modelPath, newObj.get());
                    }
                }
                
                g_sceneObjects.push_back(std::move(newObj));
                LogToConsole("Objet duplique: " + g_sceneObjects.back()->name);
            }
            if (ImGui::MenuItem("Delete", "Del")) {
                g_sceneObjects.erase(g_sceneObjects.begin() + i);
                if (g_selectedIndex == i) g_selectedIndex = -1;
                else if (g_selectedIndex > i) g_selectedIndex--;
                LogToConsole("Objet supprime");
            }
            ImGui::EndPopup();
        }
    }
    
    if (g_sceneObjects.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Scene vide");
        ImGui::Text("Utilisez les boutons ci-dessus pour ajouter des objets");
    }
    
    ImGui::End();
}

void RenderTextureManager() {
    if (!g_showTextureManager) return;
    
    ImGui::Begin("Texture Manager", &g_showTextureManager);
    
    if (ImGui::Button("Import Texture...")) {
        ImportTexture();
    }
    
    ImGui::Separator();
    ImGui::Text("Textures disponibles (%zu):", g_textures.size());
    
    const float thumbnail_size = 64.0f;
    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    
    for (auto it = g_textures.begin(); it != g_textures.end(); ) {
        ImGui::PushID(it->first.c_str());
        
        ImGui::BeginGroup();
        
        // Thumbnail
        if (ImGui::ImageButton((void*)(intptr_t)it->second, ImVec2(thumbnail_size, thumbnail_size))) {
            if (g_selectedIndex >= 0 && g_selectedIndex < g_sceneObjects.size()) {
                g_sceneObjects[g_selectedIndex]->textureName = it->first;
                LogToConsole("Texture appliquee: " + it->first);
            }
        }
        
        // Tooltip
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", it->first.c_str());
            ImGui::Image((void*)(intptr_t)it->second, ImVec2(128, 128));
            ImGui::EndTooltip();
        }
        
        // Label
        ImGui::TextWrapped("%s", it->first.c_str());
        
        ImGui::EndGroup();
        
        // Context menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Apply to Selected")) {
                if (g_selectedIndex >= 0 && g_selectedIndex < g_sceneObjects.size()) {
                    g_sceneObjects[g_selectedIndex]->textureName = it->first;
                    LogToConsole("Texture appliquee: " + it->first);
                }
            }
            if (ImGui::MenuItem("Remove Texture")) {
                for (auto& obj : g_sceneObjects) {
                    if (obj->textureName == it->first) {
                        obj->textureName = "";
                    }
                }
                
                glDeleteTextures(1, &it->second);
                it = g_textures.erase(it);
                LogToConsole("Texture supprimee");
                ImGui::PopID();
                ImGui::EndPopup();
                continue;
            }
            ImGui::EndPopup();
        }
        
        // Next item on same line if there's space
        float last_button_x2 = ImGui::GetItemRectMax().x;
        float next_button_x2 = last_button_x2 + ImGui::GetStyle().ItemSpacing.x + thumbnail_size;
        if (next_button_x2 < window_visible_x2) {
            ImGui::SameLine();
        }
        
        ImGui::PopID();
        ++it;
    }
    
    if (g_textures.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Aucune texture chargee");
        ImGui::Text("Cliquez sur 'Import Texture...' pour en ajouter");
    }
    
    ImGui::End();
}

void RenderProperties() {
    if (!g_showProperties) return;
    
    ImGui::Begin("Properties", &g_showProperties);
    
    if (g_selectedIndex >= 0 && g_selectedIndex < g_sceneObjects.size()) {
        auto& obj = g_sceneObjects[g_selectedIndex];
        
        // Name
        char nameBuffer[256];
        strcpy_s(nameBuffer, obj->name.c_str());
        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
            obj->name = nameBuffer;
        }
        
        ImGui::Checkbox("Visible", &obj->visible);
        
        ImGui::Separator();
        
        // Transform
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool transformChanged = false;
            transformChanged |= ImGui::DragFloat3("Position", glm::value_ptr(obj->position), 0.1f);
            transformChanged |= ImGui::DragFloat3("Rotation", glm::value_ptr(obj->rotation), 1.0f);
            transformChanged |= ImGui::DragFloat3("Scale", glm::value_ptr(obj->scale), 0.01f, 0.001f, 10.0f);
            
            if (transformChanged) {
                obj->updateTransform();
            }
            
            if (ImGui::Button("Reset Transform")) {
                obj->position = glm::vec3(0.0f);
                obj->rotation = glm::vec3(0.0f);
                obj->scale = glm::vec3(1.0f);
                obj->updateTransform();
            }
        }
        
        // Material
        if (ImGui::CollapsingHeader("Material")) {
            ImGui::ColorEdit3("Color", glm::value_ptr(obj->color));
            ImGui::SliderFloat("Roughness", &obj->roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &obj->metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Emission", &obj->emission, 0.0f, 5.0f);
            
            ImGui::Separator();
            ImGui::Text("Texture: %s", obj->textureName.empty() ? "None" : obj->textureName.c_str());
            if (ImGui::Button("Remove Texture")) {
                obj->textureName = "";
                LogToConsole("Texture supprimee de l'objet");
            }
            
            if (!g_textures.empty()) {
                ImGui::Text("Available textures:");
                for (const auto& tex : g_textures) {
                    if (ImGui::Selectable(tex.first.c_str(), obj->textureName == tex.first)) {
                        obj->textureName = tex.first;
                        LogToConsole("Texture appliquee: " + tex.first);
                    }
                }
            }
        }
        
        // Object-specific properties
        if (obj->type == ObjectType::LIGHT && ImGui::CollapsingHeader("Light Properties")) {
            ImGui::SliderFloat("Intensity", &obj->lightIntensity, 0.0f, 10.0f);
            ImGui::SliderFloat("Range", &obj->lightRange, 0.1f, 50.0f);
        }
        
        // Type
        ImGui::Separator();
        const char* types[] = { "Cube", "Sphere", "Plane", "Cylinder", "Light", "Model", "Empty" };
        int currentType = (int)obj->type;
        if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types))) {
            if (obj->type != ObjectType::MODEL && currentType != (int)ObjectType::MODEL) {
                obj->type = (ObjectType)currentType;
            }
        }
        
        if (obj->type == ObjectType::MODEL) {
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Model Info")) {
                ImGui::Text("Path: %s", obj->modelPath.c_str());
                ImGui::Text("Meshes: %zu", obj->meshes.size());
                
                if (ImGui::Button("Reload Model") && !obj->modelPath.empty()) {
                    if (LoadModel(obj->modelPath, obj.get())) {
                        LogToConsole("Modele recharge: " + obj->modelPath);
                    } else {
                        LogToConsole("Echec rechargement modele: " + obj->modelPath);
                    }
                }
                
                ImGui::SameLine();
                if (ImGui::Button("Change Model")) {
                    std::string path = OpenFileDialog("3D Models\0*.obj;*.fbx;*.dae;*.3ds;*.blend;*.ply\0All Files\0*.*\0");
                    if (!path.empty()) {
                        obj->modelPath = path;
                        if (LoadModel(path, obj.get())) {
                            std::filesystem::path filePath(path);
                            obj->name = filePath.stem().string();
                            LogToConsole("Nouveau modele charge: " + path);
                        }
                    }
                }
            }
        }
        
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Aucun objet selectionne");
        ImGui::Text("Selectionnez un objet dans la Scene Hierarchy");
        ImGui::Text("pour voir et modifier ses proprietes");
    }
    
    ImGui::End();
}

void RenderAssetBrowser() {
    if (!g_showAssetBrowser) return;
    
    ImGui::Begin("Asset Browser", &g_showAssetBrowser);
    
    if (ImGui::Button("Import Model...")) {
        ImportModel();
    }
    ImGui::SameLine();
    if (ImGui::Button("Import Texture...")) {
        ImportTexture();
    }
    
    ImGui::Separator();
    
    if (ImGui::CollapsingHeader("Models", ImGuiTreeNodeFlags_DefaultOpen)) {
        std::set<std::string> uniqueModels;
        for (const auto& obj : g_sceneObjects) {
            if (obj->type == ObjectType::MODEL && !obj->modelPath.empty()) {
                uniqueModels.insert(obj->modelPath);
            }
        }
        
        if (uniqueModels.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No models loaded");
        } else {
            for (const auto& modelPath : uniqueModels) {
                std::filesystem::path filePath(modelPath);
                if (ImGui::Selectable(filePath.filename().string().c_str())) {
                    CreateObject(ObjectType::MODEL);
                    auto& obj = g_sceneObjects.back();
                    obj->modelPath = modelPath;
                    obj->name = filePath.stem().string() + "_" + std::to_string(g_objectCounter - 1);
                    LoadModel(modelPath, obj.get());
                    LogToConsole("Instance de modele creee: " + obj->name);
                }
                
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Path: %s", modelPath.c_str());
                    ImGui::Text("Click to create new instance");
                    ImGui::EndTooltip();
                }
            }
        }
    }
    
    if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (g_textures.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No textures loaded");
        } else {
            const float thumbnail_size = 32.0f;
            for (const auto& tex : g_textures) {
                ImGui::Image((void*)(intptr_t)tex.second, ImVec2(thumbnail_size, thumbnail_size));
                ImGui::SameLine();
                ImGui::Text("%s", tex.first.c_str());
            }
        }
    }
    
    ImGui::End();
}

void RenderConsole() {
    if (!g_showConsole) return;
    
    ImGui::Begin("Console", &g_showConsole);
    
    if (ImGui::Button("Clear")) {
        g_consoleLog.clear();
    }
    ImGui::SameLine();
    
    static bool autoScroll = true;
    ImGui::Checkbox("Auto Scroll", &autoScroll);
    
    ImGui::Separator();
    
    // Console output
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -35), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& line : g_consoleLog) {
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        if (line.find("Erreur") != std::string::npos) {
            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        } else if (line.find("Lua:") != std::string::npos) {
            color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
        } else if (line.find("charge") != std::string::npos || line.find("cree") != std::string::npos) {
            color = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
        }
        
        ImGui::TextColored(color, "%s", line.c_str());
    }
    
    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    
    // Console input
    ImGui::Separator();
    bool reclaimFocus = false;
    if (ImGui::InputText("##ConsoleInput", &g_consoleInput, ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (!g_consoleInput.empty()) {
            LogToConsole("> " + g_consoleInput);
            
            // Simple console commands
            if (g_consoleInput == "clear") {
                g_consoleLog.clear();
            } else if (g_consoleInput == "help") {
                LogToConsole("Available commands:");
                LogToConsole("  clear - Clear console");
                LogToConsole("  stats - Show statistics");
                LogToConsole("  reset_camera - Reset camera position");
            } else if (g_consoleInput == "stats") {
                LogToConsole("Objects: " + std::to_string(g_sceneObjects.size()));
                LogToConsole("Textures: " + std::to_string(g_textures.size()));
                LogToConsole("FPS: " + std::to_string(g_stats.fps));
            } else if (g_consoleInput == "reset_camera") {
                g_camera.position = glm::vec3(5.0f, 5.0f, 5.0f);
                g_camera.yaw = -90.0f;
                g_camera.pitch = 0.0f;
                g_camera.updateVectors();
                LogToConsole("Camera reset");
            }
#ifndef NO_LUA
            else if (g_luaState) {
                // Execute as Lua code
                int result = luaL_dostring(g_luaState, g_consoleInput.c_str());
                if (result != LUA_OK) {
                    LogToConsole("Lua Error: " + std::string(lua_tostring(g_luaState, -1)));
                    lua_pop(g_luaState, 1);
                }
            }
#endif
            else {
                LogToConsole("Unknown command: " + g_consoleInput);
            }
            
            g_consoleInput.clear();
        }
        reclaimFocus = true;
    }
    
    ImGui::SetItemDefaultFocus();
    if (reclaimFocus) {
        ImGui::SetKeyboardFocusHere(-1);
    }
    
    ImGui::End();
}

void RenderSettings() {
    if (!g_showSettings) return;
    
    ImGui::Begin("Settings", &g_showSettings);
    
    if (ImGui::CollapsingHeader("Project", ImGuiTreeNodeFlags_DefaultOpen)) {
        char projectNameBuffer[256];
        strcpy_s(projectNameBuffer, g_projectSettings.projectName.c_str());
        if (ImGui::InputText("Project Name", projectNameBuffer, sizeof(projectNameBuffer))) {
            g_projectSettings.projectName = projectNameBuffer;
        }
        
        char sceneNameBuffer[256];
        strcpy_s(sceneNameBuffer, g_projectSettings.sceneName.c_str());
        if (ImGui::InputText("Scene Name", sceneNameBuffer, sizeof(sceneNameBuffer))) {
            g_projectSettings.sceneName = sceneNameBuffer;
        }
        
        ImGui::Checkbox("Auto Save", &g_projectSettings.autoSave);
        if (g_projectSettings.autoSave) {
            ImGui::SliderInt("Auto Save Interval (s)", &g_projectSettings.autoSaveInterval, 30, 600);
        }
    }
    
    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::SliderFloat("Speed", &g_camera.speed, 1.0f, 20.0f);
        ImGui::SliderFloat("Sensitivity", &g_camera.sensitivity, 0.01f, 1.0f);
        ImGui::SliderFloat("FOV", &g_camera.fov, 30.0f, 120.0f);
        ImGui::SliderFloat("Near Plane", &g_camera.nearPlane, 0.01f, 1.0f);
        ImGui::SliderFloat("Far Plane", &g_camera.farPlane, 10.0f, 1000.0f);
        
        if (ImGui::Button("Reset Camera")) {
            g_camera.position = glm::vec3(5.0f, 5.0f, 5.0f);
            g_camera.yaw = -90.0f;
            g_camera.pitch = 0.0f;
            g_camera.fov = 45.0f;
            g_camera.updateVectors();
        }
    }
    
    if (ImGui::CollapsingHeader("Skybox")) {
        ImGui::ColorEdit3("Top Color", glm::value_ptr(g_skyboxTopColor));
        ImGui::ColorEdit3("Bottom Color", glm::value_ptr(g_skyboxBottomColor));
        
        if (ImGui::Button("Reset Skybox")) {
            g_skyboxTopColor = glm::vec3(0.5f, 0.7f, 1.0f);
            g_skyboxBottomColor = glm::vec3(0.8f, 0.9f, 1.0f);
        }
    }
    
    if (ImGui::CollapsingHeader("Gizmo")) {
        ImGui::Checkbox("Use Snap", &g_useSnap);
        if (g_useSnap) {
            ImGui::DragFloat("Translate Snap", &g_snapValues[0], 0.1f, 0.1f, 10.0f);
            ImGui::DragFloat("Rotation Snap", &g_snapValues[1], 1.0f, 1.0f, 90.0f);
            ImGui::DragFloat("Scale Snap", &g_snapValues[2], 0.01f,#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <sstream>
#include <SDL.h>
#ifndef APIENTRY
#define APIENTRY
#endif
#include <assimp/version.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#ifndef NO_LUA
#include <lua.hpp>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// File dialog for Windows
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

SDL_Window* g_window = nullptr;
SDL_GLContext g_glContext = nullptr;
lua_State* g_luaState = nullptr;

// OpenGL variables
GLuint g_shaderProgram = 0;
GLuint g_skyboxShader = 0;
GLuint g_skyboxVAO = 0;
GLuint g_skyboxVBO = 0;

// Textures
std::map<std::string, GLuint> g_textures;
GLuint g_skyboxTexture = 0;

// Project settings
struct ProjectSettings {
    std::string projectName = "New Project";
    std::string projectPath = "";
    std::string sceneName = "Main Scene";
    bool autoSave = true;
    int autoSaveInterval = 300; // seconds
} g_projectSettings;

// Mesh data structure
struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
    std::string textureName;
    
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        
        // Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        
        // Texture coordinates
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        
        // Normals
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        
        glBindVertexArray(0);
    }
    
    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    ~Mesh() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

// Camera
struct Camera {
    glm::vec3 position = glm::vec3(5.0f, 5.0f, 5.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float speed = 5.0f;
    float sensitivity = 0.1f;
    
    void updateVectors() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);
        right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        up = glm::normalize(glm::cross(right, front));
        target = position + front;
    }
    
    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }
    
    glm::mat4 getProjectionMatrix(float aspectRatio) {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
};

// Input state
struct InputState {
    bool keys[SDL_NUM_SCANCODES] = {false};
    bool mouseButtons[8] = {false};
    int mouseX = 0, mouseY = 0;
    int mouseDeltaX = 0, mouseDeltaY = 0;
    bool firstMouse = true;
    int lastMouseX = 0, lastMouseY = 0;
    bool rightMousePressed = false;
};

enum class ObjectType {
    CUBE,
    SPHERE,
    PLANE,
    CYLINDER,
    LIGHT,
    MODEL,
    EMPTY
};

struct SceneObject {
    std::string name;
    glm::mat4 transform = glm::mat4(1.0f);
    ObjectType type = ObjectType::CUBE;
    glm::vec3 color = glm::vec3(0.7f, 0.7f, 1.0f);
    bool visible = true;
    bool selected = false;
    std::string textureName = "";
    std::string modelPath = "";
    
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);
    
    // Light properties
    float lightIntensity = 1.0f;
    float lightRange = 10.0f;
    
    // Material properties
    float roughness = 0.5f;
    float metallic = 0.0f;
    float emission = 0.0f;
    
    std::vector<std::unique_ptr<Mesh>> meshes;
    
    void updateTransform() {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0));
        R = glm::rotate(R, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        R = glm::rotate(R, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        transform = T * R * S;
    }
    
    void decomposeTransform() {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat orient;
        glm::decompose(transform, scale, orient, position, skew, perspective);
        rotation = glm::degrees(glm::eulerAngles(orient));
    }
};

std::vector<std::unique_ptr<SceneObject>> g_sceneObjects;
int g_objectCounter = 0;
int g_selectedIndex = -1;
Camera g_camera;
InputState g_input;

// Default meshes
std::unique_ptr<Mesh> g_defaultCubeMesh;
std::unique_ptr<Mesh> g_defaultSphereMesh;
std::unique_ptr<Mesh> g_defaultPlaneMesh;
std::unique_ptr<Mesh> g_defaultCylinderMesh;

// Gizmo settings
ImGuizmo::OPERATION g_gizmoOperation = ImGuizmo::TRANSLATE;
ImGuizmo::MODE g_gizmoMode = ImGuizmo::LOCAL;
bool g_useSnap = false;
float g_snapValues[3] = { 1.0f, 15.0f, 0.1f }; // translate, rotation, scale

// UI State
bool g_showSceneHierarchy = true;
bool g_showProperties = true;
bool g_showViewport = true;
bool g_showAssetBrowser = true;
bool g_showConsole = true;
bool g_showTextureManager = true;
bool g_showSettings = false;
bool g_showStats = true;
bool g_showDemoWindow = false;

// Console
std::vector<std::string> g_consoleLog;
std::string g_consoleInput;

// Skybox colors
glm::vec3 g_skyboxTopColor = glm::vec3(0.5f, 0.7f, 1.0f);
glm::vec3 g_skyboxBottomColor = glm::vec3(0.8f, 0.9f, 1.0f);

// Framebuffer for viewport
GLuint g_framebuffer = 0;
GLuint g_colorTexture = 0;
GLuint g_depthTexture = 0;
int g_viewportWidth = 800;
int g_viewportHeight = 600;

// Performance tracking
struct PerformanceStats {
    float frameTime = 0.0f;
    float fps = 0.0f;
    int drawCalls = 0;
    int vertices = 0;
    int triangles = 0;
} g_stats;

// Undo/Redo system
struct UndoAction {
    enum Type { TRANSFORM, CREATE, DELETE, MODIFY } type;
    int objectIndex;
    std::string data;
};
std::vector<UndoAction> g_undoStack;
std::vector<UndoAction> g_redoStack;
const size_t MAX_UNDO_STACK = 50;

void LogToConsole(const std::string& message) {
    g_consoleLog.push_back(message);
    if (g_consoleLog.size() > 1000) {
        g_consoleLog.erase(g_consoleLog.begin());
    }
}

#ifdef _WIN32
std::string OpenFileDialog(const char* filter) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn)) {
        return std::string(szFile);
    }
    
    return "";
}

std::string SaveFileDialog(const char* filter) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileNameA(&ofn)) {
        return std::string(szFile);
    }
    
    return "";
}
#else
std::string OpenFileDialog(const char* filter) {
    LogToConsole("File dialog not implemented for this platform");
    return "";
}

std::string SaveFileDialog(const char* filter) {
    LogToConsole("Save dialog not implemented for this platform");
    return "";
}
#endif

GLuint LoadTexture(const std::string& path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        LogToConsole("Texture chargee: " + path + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    } else {
        LogToConsole("Erreur chargement texture: " + path);
        glDeleteTextures(1, &textureID);
        return 0;
    }
    
    return textureID;
}

// Shader sources
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool isWireframe;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor;
uniform bool isSelected;
uniform bool isWireframe;
uniform bool hasTexture;
uniform sampler2D texture1;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform float roughness;
uniform float metallic;
uniform float emission;

void main()
{
    if (isWireframe) {
        FragColor = vec4(1.0, 0.5, 0.0, 1.0);
        return;
    }
    
    vec3 color = objectColor;
    
    if (hasTexture) {
        vec3 texColor = texture(texture1, TexCoord).rgb;
        color = color * texColor;
    }
    
    // Simple PBR-like lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * (1.0 - metallic);
    
    // Specular
    float spec = pow(max(dot(norm, halfwayDir), 0.0), (1.0 - roughness) * 256.0);
    vec3 specular = spec * lightColor * mix(vec3(0.04), color, metallic);
    
    vec3 ambient = 0.1 * color;
    vec3 result = ambient + diffuse * color + specular;
    
    // Add emission
    result += color * emission;
    
    if (isSelected) {
        result = mix(result, vec3(1.0, 0.5, 0.0), 0.3);
    }
    
    FragColor = vec4(result, 1.0);
}
)";

const char* skyboxVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 WorldPos;

void main()
{
    WorldPos = aPos;
    mat4 rotView = mat4(mat3(view));
    vec4 clipPos = projection * rotView * vec4(aPos, 1.0);
    gl_Position = clipPos.xyww;
}
)";

const char* skyboxFragmentShader = R"(
#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 topColor;
uniform vec3 bottomColor;

void main()
{
    float t = (normalize(WorldPos).y + 1.0) * 0.5;
    vec3 color = mix(bottomColor, topColor, t);
    FragColor = vec4(color, 1.0);
}
)";

GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        LogToConsole("Erreur compilation shader: " + std::string(infoLog));
    }
    
    return shader;
}

void CreateCylinderMesh() {
    g_defaultCylinderMesh = std::make_unique<Mesh>();
    
    const int segments = 32;
    const float height = 1.0f;
    const float radius = 0.5f;
    
    // Top and bottom centers
    g_defaultCylinderMesh->vertices.push_back({{0.0f, height/2, 0.0f}, {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}});
    g_defaultCylinderMesh->vertices.push_back({{0.0f, -height/2, 0.0f}, {0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}});
    
    // Side vertices
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * i / segments;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        float u = (float)i / segments;
        
        // Top ring
        g_defaultCylinderMesh->vertices.push_back({{x, height/2, z}, {u, 1.0f}, {x, 0.0f, z}});
        // Bottom ring
        g_defaultCylinderMesh->vertices.push_back({{x, -height/2, z}, {u, 0.0f}, {x, 0.0f, z}});
    }
    
    // Top cap indices
    for (int i = 0; i < segments; i++) {
        g_defaultCylinderMesh->indices.push_back(0);
        g_defaultCylinderMesh->indices.push_back(2 + i * 2);
        g_defaultCylinderMesh->indices.push_back(2 + ((i + 1) % segments) * 2);
    }
    
    // Bottom cap indices
    for (int i = 0; i < segments; i++) {
        g_defaultCylinderMesh->indices.push_back(1);
        g_defaultCylinderMesh->indices.push_back(3 + ((i + 1) % segments) * 2);
        g_defaultCylinderMesh->indices.push_back(3 + i * 2);
    }
    
    // Side indices
    for (int i = 0; i < segments; i++) {
        int current = 2 + i * 2;
        int next = 2 + ((i + 1) % segments) * 2;
        
        g_defaultCylinderMesh->indices.push_back(current);
        g_defaultCylinderMesh->indices.push_back(current + 1);
        g_defaultCylinderMesh->indices.push_back(next);
        
        g_defaultCylinderMesh->indices.push_back(next);
        g_defaultCylinderMesh->indices.push_back(current + 1);
        g_defaultCylinderMesh->indices.push_back(next + 1);
    }
    
    g_defaultCylinderMesh->setupMesh();
}

void CreateSphereMesh() {
    g_defaultSphereMesh = std::make_unique<Mesh>();
    
    const int segments = 32;
    const int rings = 16;
    
    // Generate vertices
    for (int ring = 0; ring <= rings; ring++) {
        float phi = M_PI * ring / rings;
        for (int segment = 0; segment <= segments; segment++) {
            float theta = 2.0f * M_PI * segment / segments;
            
            Vertex vertex;
            vertex.position.x = sin(phi) * cos(theta);
            vertex.position.y = cos(phi);
            vertex.position.z = sin(phi) * sin(theta);
            
            vertex.normal = vertex.position;
            vertex.texCoords.x = (float)segment / segments;
            vertex.texCoords.y = (float)ring / rings;
            
            g_defaultSphereMesh->vertices.push_back(vertex);
        }
    }
    
    // Generate indices
    for (int ring = 0; ring < rings; ring++) {
        for (int segment = 0; segment < segments; segment++) {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;
            
            g_defaultSphereMesh->indices.push_back(current);
            g_defaultSphereMesh->indices.push_back(next);
            g_defaultSphereMesh->indices.push_back(current + 1);
            
            g_defaultSphereMesh->indices.push_back(current + 1);
            g_defaultSphereMesh->indices.push_back(next);
            g_defaultSphereMesh->indices.push_back(next + 1);
        }
    }
    
    g_defaultSphereMesh->setupMesh();
}

void CreatePlaneMesh() {
    g_defaultPlaneMesh = std::make_unique<Mesh>();
    
    g_defaultPlaneMesh->vertices = {
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f, 0.0f, -0.5f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f, 0.0f,  0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
    };
    
    g_defaultPlaneMesh->indices = {
        0, 1, 2, 2, 3, 0
    };
    
    g_defaultPlaneMesh->setupMesh();
}

void CreateDefaultCubeMesh() {
    g_defaultCubeMesh = std::make_unique<Mesh>();
    
    g_defaultCubeMesh->vertices = {
        // Front face
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        
        // Back face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        
        // Left face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
        
        // Right face
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        
        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
        
        // Top face
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
    };
    
    g_defaultCubeMesh->indices = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9, 10, 10, 11, 8,    // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Bottom
        20, 21, 22, 22, 23, 20  // Top
    };
    
    g_defaultCubeMesh->setupMesh();
}

void CreateFramebuffer(int width, int height) {
    // Delete existing framebuffer if it exists
    if (g_framebuffer != 0) {
        glDeleteFramebuffers(1, &g_framebuffer);
        glDeleteTextures(1, &g_colorTexture);
        glDeleteTextures(1, &g_depthTexture);
    }
    
    g_viewportWidth = width;
    g_viewportHeight = height;
    
    // Create framebuffer
    glGenFramebuffers(1, &g_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer);
    
    // Create color texture
    glGenTextures(1, &g_colorTexture);
    glBindTexture(GL_TEXTURE_2D, g_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_colorTexture, 0);
    
    // Create depth texture
    glGenTextures(1, &g_depthTexture);
    glBindTexture(GL_TEXTURE_2D, g_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depthTexture, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LogToConsole("Erreur: Framebuffer incomplet!");
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void InitOpenGL() {
    // Compilation des shaders principaux
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    g_shaderProgram = glCreateProgram();
    glAttachShader(g_shaderProgram, vertexShader);
    glAttachShader(g_shaderProgram, fragmentShader);
    glLinkProgram(g_shaderProgram);
    
    GLint success;
    glGetProgramiv(g_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(g_shaderProgram, 512, NULL, infoLog);
        LogToConsole("Erreur link programme: " + std::string(infoLog));
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Compilation des shaders skybox
    GLuint skyVertexShader = CompileShader(GL_VERTEX_SHADER, skyboxVertexShader);
    GLuint skyFragmentShader = CompileShader(GL_FRAGMENT_SHADER, skyboxFragmentShader);
    
    g_skyboxShader = glCreateProgram();
    glAttachShader(g_skyboxShader, skyVertexShader);
    glAttachShader(g_skyboxShader, skyFragmentShader);
    glLinkProgram(g_skyboxShader);
    
    glGetProgramiv(g_skyboxShader, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(g_skyboxShader, 512, NULL, infoLog);
        LogToConsole("Erreur link programme skybox: " + std::string(infoLog));
    }
    
    glDeleteShader(skyVertexShader);
    glDeleteShader(skyFragmentShader);
    
    // Skybox setup
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &g_skyboxVAO);
    glGenBuffers(1, &g_skyboxVBO);
    glBindVertexArray(g_skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    
    // Create default meshes
    CreateDefaultCubeMesh();
    CreateSphereMesh();
    CreatePlaneMesh();
    CreateCylinderMesh();
    
    // Create framebuffer
    CreateFramebuffer(g_viewportWidth, g_viewportHeight);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    LogToConsole("OpenGL initialise avec succes");
}

std::unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    auto newMesh = std::make_unique<Mesh>();
    
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;
        
        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }
        
        newMesh->vertices.push_back(vertex);
    }
    
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            newMesh->indices.push_back(face.mIndices[j]);
        }
    }
    
    newMesh->setupMesh();
    return newMesh;
}

void ProcessNode(aiNode* node, const aiScene* scene, std::vector<std::unique_ptr<Mesh>>& meshes) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, meshes);
    }
}

bool LoadModel(const std::string& path, SceneObject* obj) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs | 
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LogToConsole("Erreur Assimp: " + std::string(importer.GetErrorString()));
        return false;
    }
    
    obj->meshes.clear();
    ProcessNode(scene->mRootNode, scene, obj->meshes);
    
    LogToConsole("Modele charge: " + path + " (" + std::to_string(obj->meshes.size()) + " meshes)");
    return true;
}

// Scene serialization
void SaveScene(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        LogToConsole("Erreur: Impossible d'ouvrir le fichier pour sauvegarde: " + path);
        return;
    }
    
    file << "# SpicyGamesEngine Scene File\n";
    file << "scene_name=" << g_projectSettings.sceneName << "\n";
    file << "skybox_top=" << g_skyboxTopColor.x << "," << g_skyboxTopColor.y << "," << g_skyboxTopColor.z << "\n";
    file << "skybox_bottom=" << g_skyboxBottomColor.x << "," << g_skyboxBottomColor.y << "," << g_skyboxBottomColor.z << "\n";
    file << "camera_pos=" << g_camera.position.x << "," << g_camera.position.y << "," << g_camera.position.z << "\n";
    file << "camera_rot=" << g_camera.yaw << "," << g_camera.pitch << "\n";
    file << "camera_fov=" << g_camera.fov << "\n";
    
    file << "objects=" << g_sceneObjects.size() << "\n";
    
    for (const auto& obj : g_sceneObjects) {
        file << "object_start\n";
        file << "name=" << obj->name << "\n";
        file << "type=" << (int)obj->type << "\n";
        file << "visible=" << (obj->visible ? 1 : 0) << "\n";
        file << "position=" << obj->position.x << "," << obj->position.y << "," << obj->position.z << "\n";
        file << "rotation=" << obj->rotation.x << "," << obj->rotation.y << "," << obj->rotation.z << "\n";
        file << "scale=" << obj->scale.x << "," << obj->scale.y << "," << obj->scale.z << "\n";
        file << "color=" << obj->color.x << "," << obj->color.y << "," << obj->color.z << "\n";
        file << "texture=" << obj->textureName << "\n";
        file << "model_path=" << obj->modelPath << "\n";
        file << "light_intensity=" << obj->lightIntensity << "\n";
        file << "light_range=" << obj->lightRange << "\n";
        file << "roughness=" << obj->roughness << "\n";
        file << "metallic=" << obj->metallic << "\n";
        file << "emission=" << obj->emission << "\n";
        file << "object_end\n";
    }
    
    file.close();
    LogToConsole("Scene sauvegardee: " + path);
}

void LoadScene(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LogToConsole("Erreur: Impossible d'ouvrir le fichier: " + path);
        return;
    }
    
    // Clear current scene
    g_sceneObjects.clear();
    g_selectedIndex = -1;
    g_objectCounter = 0;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        
        if (key == "scene_name") {
            g_projectSettings.sceneName = value;
        } else if (key == "skybox_top") {
            sscanf(value.c_str(), "%f,%f,%f", &g_skyboxTopColor.x, &g_skyboxTopColor.y, &g_skyboxTopColor.z);
        } else if (key == "skybox_bottom") {
            sscanf(value.c_str(), "%f,%f,%f", &g_skyboxBottomColor.x, &g_skyboxBottomColor.y, &g_skyboxBottomColor.z);
        } else if (key == "camera_pos") {
            sscanf(value.c_str(), "%f,%f,%f", &g_camera.position.x, &g_camera.position.y, &g_camera.position.z);
        } else if (key == "camera_rot") {
            sscanf(value.c_str(), "%f,%f", &g_camera.yaw, &g_camera.pitch);
            g_camera.updateVectors();
        } else if (key == "camera_fov") {
            g_camera.fov = std::stof(value);
        } else if (key == "object_start") {
            auto obj = std::make_unique<SceneObject>();
            
            while (std::getline(file, line) && line != "object_end") {
                size_t eq2 = line.find('=');
                if (eq2 == std::string::npos) continue;
                
                std::string objKey = line.substr(0, eq2);
                std::string objValue = line.substr(eq2 + 1);
                
                if (objKey == "name") obj->name = objValue;
                else if (objKey == "type") obj->type = (ObjectType)std::stoi(objValue);
                else if (objKey == "visible") obj->visible = std::stoi(objValue) != 0;
                else if (objKey == "position") {
                    sscanf(objValue.c_str(), "%f,%f,%f", &obj->position.x, &obj->position.y, &obj->position.z);
                } else if (objKey == "rotation") {
                    sscanf(objValue.c_str(), "%f,%f,%f", &obj->rotation.x, &obj->rotation.y, &obj->rotation.z);
                } else if (objKey == "scale") {
                    sscanf(objValue.c_str(), "%f,%f,%f", &obj->scale.x, &obj->scale.y, &obj->scale.z);
                } else if (objKey == "color") {
                    sscanf(objValue.c_str(), "%f,%f,%f", &obj->color.x, &obj->color.y, &obj->color.z);
                } else if (objKey == "texture") obj->textureName = objValue;
                else if (objKey == "model_path") {
                    obj->modelPath = objValue;
                    if (!objValue.empty() && obj->type == ObjectType::MODEL) {
                        LoadModel(objValue, obj.get());
                    }
                } else if (objKey == "light_intensity") obj->lightIntensity = std::stof(objValue);
                else if (objKey == "light_range") obj->lightRange = std::stof(objValue);
                else if (objKey == "roughness") obj->roughness = std::stof(objValue);
                else if (objKey == "metallic") obj->metallic = std::stof(objValue);
                else if (objKey == "emission") obj->emission = std::stof(objValue);
            }
            
            obj->updateTransform();
            g_sceneObjects.push_back(std::move(obj));
            g_objectCounter++;
        }
    }
    
    file.close();
    LogToConsole("Scene chargee: " + path + " (" + std::to_string(g_sceneObjects.size()) + " objets)");
}

void ProcessInput(float deltaTime) {
    float velocity = g_camera.speed * deltaTime;
    
    if (g_input.keys[SDL_SCANCODE_W] || g_input.keys[SDL_SCANCODE_Z]) {
        g_camera.position += g_camera.front * velocity;
    }
    if (g_input.keys[SDL_SCANCODE_S]) {
        g_camera.position -= g_camera.front * velocity;
    }
    if (g_input.keys[SDL_SCANCODE_A] || g_input.keys[SDL_SCANCODE_Q]) {
        g_camera.position -= g_camera.right * velocity;
    }
    if (g_input.keys[SDL_SCANCODE_D]) {
        g_camera.position += g_camera.right * velocity;
    }
    if (g_input.keys[SDL_SCANCODE_SPACE]) {
        g_camera.position += g_camera.up * velocity;
    }
    if (g_input.keys[SDL_SCANCODE_LSHIFT]) {
        g_camera.position -= g_camera.up * velocity;
    }
    
    // Gizmo shortcuts
    if (g_input.keys[SDL_SCANCODE_G]) g_gizmoOperation = ImGuizmo::TRANSLATE;
    if (g_input.keys[SDL_SCANCODE_R]) g_gizmoOperation = ImGuizmo::ROTATE;
    if (g_input.keys[SDL_SCANCODE_T]) g_gizmoOperation = ImGuizmo::SCALE;
    
    // Delete selected object
    if (g_input.keys[SDL_SCANCODE_DELETE] && g_selectedIndex >= 0) {
        g_sceneObjects.erase(g_sceneObjects.begin() + g_selectedIndex);
        g_selectedIndex = -1;
        LogToConsole("Objet supprime");
    }
    
    if (g_input.rightMousePressed) {
        if (g_input.firstMouse) {
            g_input.lastMouseX = g_input.mouseX;
            g_input.lastMouseY = g_input.mouseY;
            g_input.firstMouse = false;
        }
        
        float xoffset = g_input.mouseX - g_input.lastMouseX;
        float yoffset = g_input.lastMouseY - g_input.mouseY;
        
        g_input.lastMouseX = g_input.mouseX;
        g_input.lastMouseY = g_input.mouseY;
        
        xoffset *= g_camera.sensitivity;
        yoffset *= g_camera.sensitivity;
        
        g_camera.yaw += xoffset;
        g_camera.pitch += yoffset;
        
        if (g_camera.pitch > 89.0f) g_camera.pitch = 89.0f;
        if (g_camera.pitch < -89.0f) g_camera.pitch = -89.0f;
        
        g_camera.updateVectors();
    } else {
        g_input.firstMouse = true;
    }
}

void CreateObject(ObjectType type) {
    auto obj = std::make_unique<SceneObject>();
    
    switch (type) {
        case ObjectType::CUBE:
            obj->name = "Cube_" + std::to_string(g_objectCounter++);
            obj->color = glm::vec3(0.7f, 0.7f, 1.0f);
            break;
        case ObjectType::SPHERE:
            obj->name = "Sphere_" + std::to_string(g_objectCounter++);
            obj->color = glm::vec3(1.0f, 0.7f, 0.7f);
            break;
        case ObjectType::PLANE:
            obj->name = "Plane_" + std::to_string(g_objectCounter++);
            obj->color = glm::vec3(0.7f, 1.0f, 0.7f);
            obj->scale = glm::vec3(5.0f, 0.1f, 5.0f);
            break;
        case ObjectType::CYLINDER:
            obj->name = "Cylinder_" + std::to_string(g_objectCounter++);
            obj->color = glm::vec3(0.7f, 1.0f, 1.0f);
            break;
        case ObjectType::LIGHT:
            obj->name = "Light_" + std::to_string(g_objectCounter++);
            obj->color = glm::vec3(1.0f, 1.0f, 0.7f);
            obj->scale = glm::vec3(0.2f, 0.2f, 0.2f);
            obj->lightIntensity = 1.0f;
            obj->lightRange = 10.0f;
            obj->emission = 2.0f;
            break;
        case ObjectType::MODEL:
            obj->name = "Model_" + std::to_string(g_objectCounter++);
            obj->color = glm::vec3(0.8f, 0.8f, 0.8f);
            break;
        case ObjectType::EMPTY:
            obj->name = "Empty_" + std::to_string(g_objectCounter++);
            obj->color = glm::vec3(0.5f, 0.5f, 0.5f);
            obj->scale = glm::vec3(0.1f, 0.1f, 0.1f);
            break;
    }
    
    obj->type = type;
    obj->updateTransform();
    g_sceneObjects.push_back(std::move(obj));
    
    LogToConsole("Objet cree: " + g_sceneObjects.back()->name);
}

void ImportModel() {
    std::string path = OpenFileDialog("3D Models\0*.obj;*.fbx;*.dae;*.3ds;*.blend;*.ply\0All Files\0*.*\0");
    if (!path.empty()) {
        CreateObject(ObjectType::MODEL);
        auto& obj = g_sceneObjects.back();
        obj->modelPath = path;
        
        std::filesystem::path filePath(path);
        obj->name = filePath.stem().string() + "_" + std::to_string(g_objectCounter - 1);
        
        if (LoadModel(path, obj.get())) {
            LogToConsole("Modele importe avec succes: " + path);
        } else {
            LogToConsole("Echec import modele: " + path);
            g_sceneObjects.pop_back();
            g_objectCounter--;
        }
    }
}

void ImportTexture() {
    std::string path = OpenFileDialog("Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0All Files\0*.*\0");
    if (!path.empty()) {
        GLuint textureID = LoadTexture(path);
        if (textureID != 0) {
            std::filesystem::path filePath(path);
            std::string textureName = filePath.stem().string();
            
            int counter = 1;
            std::string originalName = textureName;
            while (g_textures.find(textureName) != g_textures.end()) {
                textureName = originalName + "_" + std::to_string(counter++);
            }
            
            g_textures[textureName] = textureID;
            LogToConsole("Texture importee: " + textureName);
        }
    }
}

void InitLua() {
#ifndef NO_LUA
    g_luaState = luaL_newstate();
    luaL_openlibs(g_luaState);
    
    auto cpp_print_func = [](lua_State* L) -> int {
        const char* msg = luaL_checkstring(L, 1);
        LogToConsole("Lua: " + std::string(msg));
        return 0;
    };
    
    lua_pushcfunction(g_luaState, cpp_print_func);
    lua_setglobal(g_luaState, "cpp_print");
    
    LogToConsole("Lua initialise avec succes");
#else
    LogToConsole("Lua desactive");
    g_luaState = nullptr;
#endif
}

void RenderSkybox(const glm::mat4& view, const glm::mat4& projection) {
    glDepthFunc(GL_LEQUAL);
    glUseProgram(g_skyboxShader);
    
    GLint viewLoc = glGetUniformLocation(g_skyboxShader, "view");
    GLint projectionLoc = glGetUniformLocation(g_skyboxShader, "projection");
    GLint topColorLoc = glGetUniformLocation(g_skyboxShader, "topColor");
    GLint bottomColorLoc = glGetUniformLocation(g_skyboxShader, "bottomColor");
    
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(topColorLoc, 1, glm::value_ptr(g_skyboxTopColor));
    glUniform3fv(bottomColorLoc, 1, glm::value_ptr(g_skyboxBottomColor));
    
    glBindVertexArray(g_skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

Mesh* GetMeshForObjectType(ObjectType type) {
    switch (type) {
        case ObjectType::CUBE:
        default:
            return g_defaultCubeMesh.get();
        case ObjectType::SPHERE:
            return g_defaultSphereMesh.get();
        case ObjectType::PLANE:
            return g_defaultPlaneMesh.get();
        case ObjectType::CYLINDER:
            return g_defaultCylinderMesh.get();
        case ObjectType::LIGHT:
            return g_defaultCubeMesh.get();
        case ObjectType::EMPTY:
            return g_defaultCubeMesh.get();
    }
}

// UI Functions
void RenderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                g_sceneObjects.clear();
                g_selectedIndex = -1;
                g_objectCounter = 0;
                LogToConsole("Nouvelle scene creee");
            }
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                std::string path = OpenFileDialog("Scene Files\0*.scene\0All Files\0*.*\0");
                if (!path.empty()) {
                    LoadScene(path);
                }
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                std::string path = SaveFileDialog("Scene Files\0*.scene\0All Files\0*.*\0");
                if (!path.empty()) {
                    SaveScene(path);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Settings")) {
                g_showSettings = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                SDL_Event quitEvent;
                quitEvent.type = SDL_QUIT;
                SDL_PushEvent(&quitEvent);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !g_undoStack.empty())) {
                // TODO: Implement undo
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, !g_redoStack.empty())) {
                // TODO: Implement redo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del", false, g_selectedIndex >= 0)) {
                if (g_selectedIndex >= 0 && g_selectedIndex < g_sceneObjects.size()) {
                    g_sceneObjects.erase(g_sceneObjects.begin() + g_selectedIndex);
                    g_selectedIndex = -1;
                    LogToConsole("Objet supprime");
                }
            }
            if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, g_selectedIndex >= 0)) {
                if (g_selectedIndex >= 0 && g_selectedIndex < g_sceneObjects.size()) {
                    auto& obj = g_sceneObjects[g_selectedIndex];
                    auto newObj = std::make_unique<SceneObject>(*obj);
                    newObj->name += "_Copy";
                    newObj->position.x += 1.0f;
                    newObj->updateTransform();
                    
                    if (obj->type == ObjectType::MODEL) {
                        newObj->meshes.clear();
                        if (!obj->modelPath.empty()) {
                            LoadModel(obj->modelPath, newObj.get());
                        }
                    }
                    
                    g_sceneObjects.push_back(std::move(newObj));
                    LogToConsole("Objet duplique");
                }
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Cube")) CreateObject(ObjectType::CUBE);
            if (ImGui::MenuItem("Sphere")) CreateObject(ObjectType::SPHERE);
            if (ImGui::MenuItem("Plane")) CreateObject(ObjectType::PLANE);
            if (ImGui::MenuItem("Cylinder")) CreateObject(ObjectType::CYLINDER);
            ImGui::Separator();
            if (ImGui::MenuItem("Light")) CreateObject(ObjectType::LIGHT);
            if (ImGui::MenuItem("Empty")) CreateObject(ObjectType::EMPTY);
            ImGui::Separator();
            if (ImGui::MenuItem("Import Model...")) ImportModel();
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &g_showSceneHierarchy);
            ImGui::MenuItem("Properties", nullptr, &g_showProperties);
            ImGui::MenuItem("Asset Browser", nullptr, &g_showAssetBrowser);
            ImGui::MenuItem("Texture Manager", nullptr, &g_showTextureManager);
            ImGui::MenuItem("Console", nullptr, &g_showConsole);
            ImGui::MenuItem("Statistics", nullptr, &g_showStats);
            ImGui::Separator();
            ImGui::MenuItem("Demo Window", nullptr, &g_showDemoWindow);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                LogToConsole("SpicyGamesEngine v1.0 - Editeur 3D");
                LogToConsole("Controles: WASD/ZQSD pour se deplacer, clic droit pour regarder");
                LogToConsole("G: Translation, R: Rotation, T: Scale");
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
}

void RenderSceneHierarchy() {
    if (!g_showSceneHierarchy) return;
    
    ImGui::Begin("Scene Hierarchy", &g_showSceneHierarchy);
    
    // Quick create buttons
    if (ImGui::Button("Cube")) CreateObject(ObjectType::CUBE);
    ImGui::SameLine();
    if (ImGui::Button("Sphere")) CreateObject(ObjectType::SPHERE);
    ImGui::SameLine();
    if (ImGui::Button("Plane")) CreateObject(ObjectType::PLANE);
    ImGui::SameLine();
    if (ImGui::Button("Cylinder")) CreateObject(ObjectType::CYLINDER);
    
    if (ImGui::Button("Light")) CreateObject(ObjectType::LIGHT);
    ImGui::SameLine();
    if (ImGui::Button("Empty")) CreateObject(ObjectType::EMPTY);
    ImGui::SameLine();
    if (ImGui::Button("Import Model")) ImportModel();
    
    ImGui::Separator();
    
    // Filter
    static char filter[128] = "";
    ImGui::InputText("Filter", filter, sizeof(filter));
    
    ImGui::Separator();
    
    for (size_t i = 0; i < g_sceneObjects.size(); ++i) {
        auto& obj = g_sceneObjects[i];
        
        // Apply filter
        if (strlen(filter) > 0 && obj->name.find(filter) == std::string::npos) {
            continue;
        }
        
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (g_selectedIndex == i) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        
        // Object icon based on type
        const char* icon = "?";
        switch (obj->type) {
            case ObjectType::CUBE: icon = "■"; break;
            case ObjectType::SPHERE: icon = "●"; break;
            case ObjectType::PLANE: icon = "▬"; break;
            case ObjectType::CYLINDER: icon = "◯"; break;
            case ObjectType::LIGHT: icon = "💡"; break;
            case ObjectType::MODEL: icon = "🗿"; break;
            case ObjectType::EMPTY: icon = "◇"; break;
        }
        
        // Visibility toggle
        ImGui::PushID((int)i);
        if (ImGui::Checkbox("##vis", &obj->visible)) {
            LogToConsole(obj->name + (obj->visible ? " rendu visible" : " cache"));
        }
        ImGui::PopID();
        ImGui::SameLine();
        
        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s %s", icon, obj->name.c_str());
        
        if (ImGui::IsItemClicked()) {
            g_selectedIndex = i;
            obj->selected = true;
            for (size_t j = 0; j < g_sceneObjects.size(); ++j) {
                if (j != i) g_sceneObjects[j]->selected = false;
            }
        }
        
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Focus Camera")) {
                g_camera.position = obj->position + glm::vec3(5.0f, 5.0f, 5.0f);
                g_camera.target = obj->position;
                g_camera.updateVectors();
                LogToConsole("Camera focalisee sur " + obj->name);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                auto newObj = std::make_unique<SceneObject>(*obj);
                newObj->name += "_Copy";
                newObj->position.x += 1.0f;
                newObj->updateTransform();
                
                if (obj->type == ObjectType::MODEL) {
                    newObj->meshes.clear();
                    if (!obj->modelPath.empty()) {
						0.01f, 2.0f);
        }
        
        ImGui::Checkbox("Bounds", &g_gizmoBounds);
    }
    
    if (ImGui::CollapsingHeader("Graphics")) {
        ImGui::Checkbox("VSync", &g_vsync);
        ImGui::Checkbox("Wireframe", &g_wireframe);
        ImGui::SliderFloat("Gamma", &g_gamma, 0.1f, 3.0f);
        
        if (ImGui::Button("Reset Graphics")) {
            g_vsync = true;
            g_wireframe = false;
            g_gamma = 2.2f;
        }
    }
    
    if (ImGui::CollapsingHeader("Performance")) {
        ImGui::Text("Frame Time: %.3f ms", g_stats.frameTime * 1000.0f);
        ImGui::Text("FPS: %.1f", g_stats.fps);
        ImGui::Text("Draw Calls: %d", g_stats.drawCalls);
        ImGui::Text("Vertices: %d", g_stats.vertices);
        ImGui::Text("Triangles: %d", g_stats.triangles);
    }
    
    ImGui::End();
}

void RenderStats() {
    if (!g_showStats) return;
    
    ImGui::Begin("Statistics", &g_showStats);
    
    ImGui::Text("Performance");
    ImGui::Separator();
    ImGui::Text("Frame Time: %.3f ms", g_stats.frameTime * 1000.0f);
    ImGui::Text("FPS: %.1f", g_stats.fps);
    ImGui::Text("Draw Calls: %d", g_stats.drawCalls);
    ImGui::Text("Vertices: %d", g_stats.vertices);
    ImGui::Text("Triangles: %d", g_stats.triangles);
    
    ImGui::Separator();
    ImGui::Text("Scene");
    ImGui::Text("Objects: %zu", g_sceneObjects.size());
    ImGui::Text("Selected: %s", (g_selectedIndex >= 0) ? g_sceneObjects[g_selectedIndex]->name.c_str() : "None");
    ImGui::Text("Textures: %zu", g_textures.size());
    
    ImGui::Separator();
    ImGui::Text("Camera");
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", g_camera.position.x, g_camera.position.y, g_camera.position.z);
    ImGui::Text("Rotation: Yaw %.1f°, Pitch %.1f°", g_camera.yaw, g_camera.pitch);
    ImGui::Text("FOV: %.1f°", g_camera.fov);
    
    ImGui::End();
}

void RenderViewport() {
    if (!g_showViewport) return;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport", &g_showViewport);
    
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    // Resize framebuffer if needed
    if (viewportSize.x != g_viewportWidth || viewportSize.y != g_viewportHeight) {
        if (viewportSize.x > 0 && viewportSize.y > 0) {
            CreateFramebuffer((int)viewportSize.x, (int)viewportSize.y);
        }
    }
    
    // Render scene to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer);
    glViewport(0, 0, g_viewportWidth, g_viewportHeight);
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render scene
    RenderScene();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Display framebuffer texture in ImGui
    ImGui::Image((void*)(intptr_t)g_colorTexture, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    
    // Handle viewport interaction
    if (ImGui::IsItemHovered()) {
        ImGuiIO& io = ImGui::GetIO();
        
        // Mouse picking
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 relativePos = ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);
            
            // TODO: Implement ray casting for object selection
        }
        
        // Camera controls
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            g_input.rightMousePressed = true;
            g_input.mouseX = (int)io.MousePos.x;
            g_input.mouseY = (int)io.MousePos.y;
        } else {
            g_input.rightMousePressed = false;
        }
        
        // Mouse wheel for zoom
        if (io.MouseWheel != 0.0f) {
            g_camera.position += g_camera.front * (io.MouseWheel * g_camera.speed * 0.1f);
        }
    }
    
    // Gizmo rendering
    if (g_selectedIndex >= 0 && g_selectedIndex < g_sceneObjects.size()) {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
        
        glm::mat4 view = g_camera.getViewMatrix();
        glm::mat4 projection = g_camera.getProjectionMatrix(viewportSize.x / viewportSize.y);
        glm::mat4 model = g_sceneObjects[g_selectedIndex]->transform;
        
        float* snapValues = g_useSnap ? g_snapValues : nullptr;
        
        if (ImGuizmo::Manipulate(
            glm::value_ptr(view),
            glm::value_ptr(projection),
            g_gizmoOperation,
            g_gizmoMode,
            glm::value_ptr(model),
            nullptr,
            snapValues)) {
            
            g_sceneObjects[g_selectedIndex]->transform = model;
            g_sceneObjects[g_selectedIndex]->decomposeTransform();
        }
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
}

void RenderScene() {
    // Clear stats
    g_stats.drawCalls = 0;
    g_stats.vertices = 0;
    g_stats.triangles = 0;
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    // Calculate matrices
    float aspectRatio = (float)g_viewportWidth / (float)g_viewportHeight;
    glm::mat4 view = g_camera.getViewMatrix();
    glm::mat4 projection = g_camera.getProjectionMatrix(aspectRatio);
    
    // Render skybox first
    RenderSkybox(view, projection);
    
    // Use main shader
    glUseProgram(g_shaderProgram);
    
    // Set common uniforms
    GLint viewLoc = glGetUniformLocation(g_shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(g_shaderProgram, "projection");
    GLint lightPosLoc = glGetUniformLocation(g_shaderProgram, "lightPos");
    GLint lightColorLoc = glGetUniformLocation(g_shaderProgram, "lightColor");
    GLint viewPosLoc = glGetUniformLocation(g_shaderProgram, "viewPos");
    
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(g_camera.position));
    
    // Find first light for basic lighting
    glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    for (const auto& obj : g_sceneObjects) {
        if (obj->type == ObjectType::LIGHT && obj->visible) {
            lightPos = obj->position;
            lightColor = obj->color * obj->lightIntensity;
            break;
        }
    }
    
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    
    // Render all objects
    for (const auto& obj : g_sceneObjects) {
        if (!obj->visible) continue;
        
        // Set object-specific uniforms
        GLint modelLoc = glGetUniformLocation(g_shaderProgram, "model");
        GLint colorLoc = glGetUniformLocation(g_shaderProgram, "objectColor");
        GLint selectedLoc = glGetUniformLocation(g_shaderProgram, "isSelected");
        GLint wireframeLoc = glGetUniformLocation(g_shaderProgram, "isWireframe");
        GLint hasTextureLoc = glGetUniformLocation(g_shaderProgram, "hasTexture");
        GLint roughnessLoc = glGetUniformLocation(g_shaderProgram, "roughness");
        GLint metallicLoc = glGetUniformLocation(g_shaderProgram, "metallic");
        GLint emissionLoc = glGetUniformLocation(g_shaderProgram, "emission");
        
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj->transform));
        glUniform3fv(colorLoc, 1, glm::value_ptr(obj->color));
        glUniform1i(selectedLoc, obj->selected ? 1 : 0);
        glUniform1i(wireframeLoc, g_wireframe ? 1 : 0);
        glUniform1f(roughnessLoc, obj->roughness);
        glUniform1f(metallicLoc, obj->metallic);
        glUniform1f(emissionLoc, obj->emission);
        
        // Bind texture if available
        bool hasTexture = false;
        if (!obj->textureName.empty() && g_textures.find(obj->textureName) != g_textures.end()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g_textures[obj->textureName]);
            glUniform1i(glGetUniformLocation(g_shaderProgram, "texture1"), 0);
            hasTexture = true;
        }
        glUniform1i(hasTextureLoc, hasTexture ? 1 : 0);
        
        // Set wireframe mode
        if (g_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        // Render appropriate mesh
        if (obj->type == ObjectType::MODEL && !obj->meshes.empty()) {
            for (const auto& mesh : obj->meshes) {
                mesh->draw();
                g_stats.drawCalls++;
                g_stats.vertices += mesh->vertices.size();
                g_stats.triangles += mesh->indices.size() / 3;
            }
        } else {
            Mesh* mesh = GetMeshForObjectType(obj->type);
            if (mesh) {
                mesh->draw();
                g_stats.drawCalls++;
                g_stats.vertices += mesh->vertices.size();
                g_stats.triangles += mesh->indices.size() / 3;
            }
        }
    }
    
    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Global variables missing from original code
bool g_wireframe = false;
bool g_vsync = true;
float g_gamma = 2.2f;
bool g_gizmoBounds = false;

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        LogToConsole("Erreur SDL_Init: " + std::string(SDL_GetError()));
        return -1;
    }
    
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    // Create window
    g_window = SDL_CreateWindow(
        "SpicyGamesEngine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    
    if (!g_window) {
        LogToConsole("Erreur creation fenetre: " + std::string(SDL_GetError()));
        SDL_Quit();
        return -1;
    }
    
    // Create OpenGL context
    g_glContext = SDL_GL_CreateContext(g_window);
    if (!g_glContext) {
        LogToConsole("Erreur creation contexte OpenGL: " + std::string(SDL_GetError()));
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return -1;
    }
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LogToConsole("Erreur initialisation GLAD");
        return -1;
    }
    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    ImGui_ImplSDL2_InitForOpenGL(g_window, g_glContext);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Initialize engine components
    InitOpenGL();
    InitLua();
    
    LogToConsole("SpicyGamesEngine initialise avec succes");
    LogToConsole("Version OpenGL: " + std::string((char*)glGetString(GL_VERSION)));
    LogToConsole("GPU: " + std::string((char*)glGetString(GL_RENDERER)));
    
    // Create default scene
    CreateObject(ObjectType::PLANE);
    g_sceneObjects.back()->scale = glm::vec3(10.0f, 1.0f, 10.0f);
    g_sceneObjects.back()->position.y = -1.0f;
    g_sceneObjects.back()->updateTransform();
    g_sceneObjects.back()->name = "Ground";
    
    CreateObject(ObjectType::CUBE);
    g_sceneObjects.back()->position = glm::vec3(2.0f, 0.0f, 0.0f);
    g_sceneObjects.back()->updateTransform();
    
    CreateObject(ObjectType::SPHERE);
    g_sceneObjects.back()->position = glm::vec3(-2.0f, 0.0f, 0.0f);
    g_sceneObjects.back()->updateTransform();
    
    CreateObject(ObjectType::LIGHT);
    g_sceneObjects.back()->position = glm::vec3(0.0f, 3.0f, 0.0f);
    g_sceneObjects.back()->updateTransform();
    
    // Enable VSync
    SDL_GL_SetSwapInterval(1);
    
    // Main loop
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // Update performance stats
        g_stats.frameTime = deltaTime;
        g_stats.fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
        
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                    
                case SDL_KEYDOWN:
                    g_input.keys[event.key.keysym.scancode] = true;
                    break;
                    
                case SDL_KEYUP:
                    g_input.keys[event.key.keysym.scancode] = false;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button < 8) {
                        g_input.mouseButtons[event.button.button] = true;
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button < 8) {
                        g_input.mouseButtons[event.button.button] = false;
                    }
                    break;
                    
                case SDL_MOUSEMOTION:
                    g_input.mouseX = event.motion.x;
                    g_input.mouseY = event.motion.y;
                    break;
                    
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        glViewport(0, 0, event.window.data1, event.window.data2);
                    }
                    break;
            }
        }
        
        // Process input
        ProcessInput(deltaTime);
        
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        
        // Docking
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);
        
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        
        // Render menu bar
        RenderMenuBar();
        
        ImGui::End();
        
        // Render UI windows
        RenderViewport();
        RenderSceneHierarchy();
        RenderProperties();
        RenderAssetBrowser();
        RenderTextureManager();
        RenderConsole();
        RenderSettings();
        RenderStats();
        
        if (g_showDemoWindow) {
            ImGui::ShowDemoWindow(&g_showDemoWindow);
        }
        
        // Render
        int display_w, display_h;
        SDL_GetWindowSize(g_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
        
        SDL_GL_SwapWindow(g_window);
    }
    
    // Cleanup
    if (g_luaState) {
        lua_close(g_luaState);
    }
    
    // Delete OpenGL resources
    for (auto& texture : g_textures) {
        glDeleteTextures(1, &texture.second);
    }
    g_textures.clear();
    
    if (g_framebuffer) {
        glDeleteFramebuffers(1, &g_framebuffer);
        glDeleteTextures(1, &g_colorTexture);
        glDeleteTextures(1, &g_depthTexture);
    }
    
    if (g_shaderProgram) glDeleteProgram(g_shaderProgram);
    if (g_skyboxShader) glDeleteProgram(g_skyboxShader);
    if (g_skyboxVAO) glDeleteVertexArrays(1, &g_skyboxVAO);
    if (g_skyboxVBO) glDeleteBuffers(1, &g_skyboxVBO);
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_GL_DeleteContext(g_glContext);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    
    LogToConsole("SpicyGamesEngine ferme proprement");
    return 0;
}
