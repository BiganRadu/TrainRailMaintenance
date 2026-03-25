#pragma once

#include <vector>
#include <random>
#include <functional>

#include "components/simple_scene.h"
#include "components/text_renderer.h"
#include "lab_m1/lab5/lab_camera.h"
#include "core/gpu/mesh.h"
#include "lab_m1/tema2/train.h"
#include "lab_m1/tema2/draisine.h"


namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix) override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

     protected:
        implemented::Camera *camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;
		Mesh* paralelepiped;

        // Rail component resources
        Mesh* railMesh = nullptr;
        Mesh* tieMesh = nullptr;
        m1::Component railComponent;
        Mesh* tunnelMesh = nullptr;
        m1::Component cornerRailLeft;
        m1::Component cornerRailRight;
        m1::Component mountainRailComponent;
        m1::Component seaRailComponent;

         // Rails grid
         int railSize = 40;
         std::vector<int> railCells;
         std::vector<float> railRotations; // yaw per cell 

         // Terrain
         int terrainSize = 40;
         float terrainCellSize = RailConfig::SegmentLen;
         std::vector<int> terrainCells; // 0=grass,1=mountain,2=sea
         Mesh* terrainMeshes[3] { nullptr, nullptr, nullptr };

        // Train control
        float originOffset = 0.0f;
        Train* train = nullptr;
        float trainSpeed = 2.0f; // units per second along track

        Draisine* draisine = nullptr;
        float draisineSpeed = 2.0f;

        // Broken rail variants
        m1::Component brokenRailComponent;
        m1::Component brokenMountainRailComponent;
        m1::Component brokenSeaRailComponent;
        m1::Component brokenCornerRailLeft;
        m1::Component brokenCornerRailRight;

        // Breakage system
        std::vector<bool> brokenCells;
        float breakTimer = 0.0f;
        float breakInterval = 30.0f;
        std::mt19937 rng;

        // Third-person camera params
        float cameraDistance = 6.0f;
        float cameraHeight = 3.0f;

        // Game over handling
        gfxc::TextRenderer* textRenderer = nullptr;
        bool gameOver = false;
        float stuckTimer = 0.0f;
        glm::vec3 lastTrainPos = glm::vec3(0.0f);

         struct Station
         {
             m1::Component* comp;
             glm::vec3 pos; // center on rail cell
             float yaw;      // orientation along track
             float halfLen;
             float halfWidth;
         };
         std::vector<Station> stations;
         m1::Component stationCompRed;
         m1::Component stationCompGreen;
         m1::Component stationCompBlue;

         void BreakRandomRail();


        // World vertical lift to place everything above origin
        float worldLift = 0.2f;

        // Rail statistics
        size_t railCountTotal = 0;
        size_t brokenCountCurrent = 0;

        // Timer
        float elapsedTime = 0.0f;
    };
}
