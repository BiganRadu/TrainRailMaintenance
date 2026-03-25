#include "lab_m1/tema2/tema2.h"
#include "lab_m1/tema2/meshes.h"
#include "lab_m1/tema2/component.h"
#include "lab_m1/tema2/train.h"
#include "lab_m1/tema2/draisine.h"
#include "core/engine.h"
#include "components/text_renderer.h"

#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <cmath>
#include <limits>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace m1;


Tema2::Tema2()
{
    breakInterval = 5.0f;
}


Tema2::~Tema2()
{
    delete train;
    delete draisine;
    delete textRenderer;
}


void Tema2::Init()
{
    renderCameraTarget = false;

    camera = new implemented::Camera();
    camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

    paralelepiped = Meshes::CreateParallelepiped("paralelepiped", glm::vec3(0, 0, 0), 1, 0.1f, 0.1f, glm::vec3(1, 0, 0));

    // Rail deformation shader
    {
        Shader* shader = new Shader("RailDeform");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Station meshes (different colors)
    float stationLen = RailConfig::SegmentLen * 3.5f;
    float stationWidth = 1.0f;
    float stationHeight = 0.4f;
    stationCompRed = Components::CreateStation("station_red", glm::vec3(0.85f, 0.1f, 0.1f), stationLen, stationHeight, stationWidth);
    stationCompGreen = Components::CreateStation("station_green", glm::vec3(0.1f, 0.8f, 0.2f), stationLen, stationHeight, stationWidth);
    stationCompBlue = Components::CreateStation("station_blue", glm::vec3(0.1f, 0.4f, 0.9f), stationLen, stationHeight, stationWidth);


	// Rail setup   
    railComponent = Components::CreateStandardRail("standard_rail");
    mountainRailComponent = Components::CreateMountainRail("mountain_rail");
    seaRailComponent = Components::CreateSeaRail("sea_rail");
    cornerRailLeft = Components::CreateCornerRailLeft("corner_left");
    cornerRailRight = Components::CreateCornerRailRight("corner_right");
    tunnelMesh = Meshes::CreateHalfCylinderShell("tunnel", glm::vec3(0.0f, 0.0f, 0.0f), 2.0f, 2.5f, 0.15f, glm::vec3(0.6f, 0.6f, 0.6f));

    // Terrain setup
    terrainCellSize = RailConfig::SegmentLen;
    railSize = terrainSize;
    terrainCells.resize(terrainSize * terrainSize);
    originOffset = -(terrainSize * terrainCellSize) * 0.5f;

    // Colors for terrain types
    glm::vec3 grassColor(0.1f, 0.6f, 0.1f);
    glm::vec3 mountainColor(0.45f, 0.3f, 0.15f);
    glm::vec3 seaColor(0.0f, 0.35f, 0.7f);

    // One mesh per terrain type
    float tileHeight = 0.02f;
    terrainMeshes[0] = Meshes::CreateParallelepiped("tile_grass", glm::vec3(0), terrainCellSize, tileHeight, terrainCellSize, grassColor);
    terrainMeshes[1] = Meshes::CreateParallelepiped("tile_mountain", glm::vec3(0), terrainCellSize, tileHeight, terrainCellSize, mountainColor);
    terrainMeshes[2] = Meshes::CreateParallelepiped("tile_sea", glm::vec3(0), terrainCellSize, tileHeight, terrainCellSize, seaColor);

    // Predefined terrain: start with grass everywhere
    terrainCells.assign(terrainSize * terrainSize, 0);
    railCells.assign(railSize * railSize, 0);
    railRotations.assign(railSize * railSize, 0.0f);
    brokenCells.assign(railSize * railSize, false);
    railCountTotal = 0;
    brokenCountCurrent = 0;

    auto fillRect = [&](int type, int x0, int x1, int z0, int z1)
        {
            x0 = std::max(0, x0); x1 = std::min(terrainSize - 1, x1);
            z0 = std::max(0, z0); z1 = std::min(terrainSize - 1, z1);
            for (int z = z0; z <= z1; ++z)
                for (int x = x0; x <= x1; ++x)
                    terrainCells[z * terrainSize + x] = type;
        };

    // Keep edges grass; place clustered mountain and sea blocks away from borders
    int margin = 3;
    int blockSize = 12;
    // Mountain cluster
    fillRect(1, margin + 6, margin + 6 + blockSize, margin + 22, margin + 22 + blockSize);
    // Sea cluster
    fillRect(2, margin + 25, margin + 25 + blockSize, margin + 5, margin + 5 + blockSize);

    // Rail route
    auto setRailLine = [&](int type, int x0, int z0, int x1, int z1)
        {
            if (type == 1)
            {
                int xa = std::min(x0, x1), xb = std::max(x0, x1);
                for (int x = xa; x <= xb; ++x) railCells[z0 * railSize + x] = 1;
            }
            else if (type == 2)
            {
                int za = std::min(z0, z1), zb = std::max(z0, z1);
                for (int z = za; z <= zb; ++z) railCells[z * railSize + x0] = 2;
            }
        };

    auto markCorner = [&](int x, int z, const glm::ivec2& prevDir, const glm::ivec2& nextDir)
        {
            int idx = z * railSize + x;
            float cross = prevDir.x * nextDir.y - prevDir.y * nextDir.x;
            bool isLeft = cross > 0;
            railCells[idx] = isLeft ? 4 : 3;
            railRotations[idx] = std::atan2((float)prevDir.x, (float)prevDir.y);
        };

    // Directions between waypoints on 40x40 grid
    glm::ivec2 d1(1, 0);
    glm::ivec2 d2(0, 1);
    glm::ivec2 d3(-1, 0);
    glm::ivec2 d4(0, 1);
    glm::ivec2 d5(-1, 0);
    glm::ivec2 d6(0, -1);

    // Waypoints (x,z)
    setRailLine(1, 3, 3, 32, 3);
    setRailLine(2, 32, 3, 32, 22);
    setRailLine(1, 32, 22, 15, 22);
    setRailLine(2, 15, 22, 15, 38);
    setRailLine(1, 15, 38, 3, 38);
    setRailLine(2, 3, 38, 3, 3);

    // Mark corners after lines so they aren't overwritten by subsequent segments
    markCorner(32, 3, d1, d2);
    markCorner(32, 22, d2, d3);
    markCorner(15, 22, d3, d4);
    markCorner(15, 38, d4, d5);
    markCorner(3, 38, d5, d6);
    markCorner(3, 3, d6, d1);

    // Count total rails for HUD and lose threshold
    railCountTotal = 0;
    for (int z = 0; z < railSize; ++z)
    {
        for (int x = 0; x < railSize; ++x)
        {
            if (railCells[z * railSize + x] != 0) ++railCountTotal;
        }
    }

    // Build train path (world positions centered on rail cells)
    auto cellToWorld = [&](int x, int z)
        {
            return glm::vec3(originOffset + (x + 0.5f) * terrainCellSize, worldLift, originOffset + (z + 0.5f) * terrainCellSize);
        };

    // Stations on grass horizontal segments (z=3 and z=38) and vertical (x=3)
    stations.clear();
    auto addStation = [&](int cx, int cz, m1::Component* comp, float yaw)
        {
            glm::vec3 p = cellToWorld(cx, cz);
            stations.push_back({ comp, p, yaw, stationLen * 0.5f, stationWidth * 0.5f });
        };
    // Place one station on each grass side
    float stationYawHorizontal = glm::half_pi<float>();
    float stationYawVertical = 0.0f;
    addStation(10, 3, &stationCompRed, stationYawHorizontal);
    addStation(24, 22, &stationCompGreen, stationYawHorizontal);
    addStation(3, 20, &stationCompBlue, stationYawVertical);

    // Build a path that follows track centers with quarter-circle arcs inside corner cells
    std::vector<glm::ivec2> waypointCells = {
        {3, 3}, {32, 3}, {32, 22}, {15, 22}, {15, 38}, {3, 38}
    };
    std::vector<glm::ivec2> dirs = { d1, d2, d3, d4, d5, d6 };
    const float cornerRadius = terrainCellSize * 0.5f; // rails corner radius (half cell)
    const float straightStep = terrainCellSize * 0.08f; // denser sampling
    auto addPoint = [](std::vector<glm::vec3>& pts, const glm::vec3& p)
        {
            if (!pts.empty() && glm::length2(pts.back() - p) < 1e-6f) return;
            pts.push_back(p);
        };

	// Generate path for train and draisine
    std::vector<glm::vec3> trainPath;
    for (size_t i = 0; i < waypointCells.size(); ++i)
    {
        size_t nextIdx = (i + 1) % waypointCells.size();
        const glm::ivec2& cellStart = waypointCells[i];
        const glm::ivec2& cellCorner = waypointCells[nextIdx];
        glm::ivec2 dirPrev = dirs[i];
        glm::ivec2 dirNext = dirs[nextIdx];

        // Helper to convert ivec2 direction to vec3
        glm::vec3 vDirPrev((float)dirPrev.x, 0.0f, (float)dirPrev.y);
        glm::vec3 vDirNext((float)dirNext.x, 0.0f, (float)dirNext.y);

        glm::vec3 startPos = cellToWorld(cellStart.x, cellStart.y);
        if (trainPath.empty()) addPoint(trainPath, startPos);

        glm::vec3 cornerCenter = cellToWorld(cellCorner.x, cellCorner.y);

        // Calculate Arc Start and End points
        glm::vec3 arcStart = cornerCenter - vDirPrev * cornerRadius;
        glm::vec3 arcEnd = cornerCenter + vDirNext * cornerRadius;

        // Generate Straight Segment up to Arc Start
        glm::vec3 from = trainPath.back();
        glm::vec3 straightVec = arcStart - from;
        float straightLen = glm::length(straightVec);
        if (straightLen > 1e-4f) {
            int straightSteps = std::max(1, (int)std::ceil(straightLen / straightStep));
            for (int s = 1; s <= straightSteps; ++s)
            {
                float t = (float)s / (float)straightSteps;
                addPoint(trainPath, glm::mix(from, arcStart, t));
            }
        }

        // Ensure exact start point
        addPoint(trainPath, arcStart);

        // Calculate the Pivot Point (Center of Curvature)
        glm::vec3 pivot = cornerCenter - vDirPrev * cornerRadius + vDirNext * cornerRadius;

        // Calculate Angles relative to the Pivot
        glm::vec3 radiusVecStart = arcStart - pivot;
        glm::vec3 radiusVecEnd = arcEnd - pivot;
        float angStart = std::atan2(radiusVecStart.x, radiusVecStart.z);
        float angEnd = std::atan2(radiusVecEnd.x, radiusVecEnd.z);

        // Handle Angle Wrapping to ensure we take the shortest path
        float angleDiff = angEnd - angStart;
        while (angleDiff < -glm::pi<float>()) angleDiff += glm::two_pi<float>();
        while (angleDiff > glm::pi<float>()) angleDiff -= glm::two_pi<float>();

        // Generate Arc Points
        int arcSteps = 300;
        for (int a = 1; a <= arcSteps; ++a)
        {
            float t = (float)a / (float)arcSteps;
            float ang = angStart + angleDiff * t;

            // Reconstruct position from pivot
            glm::vec3 offset(std::sin(ang), 0.0f, std::cos(ang));
            addPoint(trainPath, pivot + offset * cornerRadius);
        }
    }

    // Close the loop with a straight from last point to the first point
    if (!trainPath.empty())
    {
        glm::vec3 startLoop = trainPath.front();
        glm::vec3 endLoop = trainPath.back();
        glm::vec3 straightVec = startLoop - endLoop;
        float straightLen = glm::length(straightVec);
        int straightSteps = std::max(1, (int)std::ceil(straightLen / straightStep));
        for (int s = 1; s <= straightSteps; ++s)
        {
            float t = (float)s / (float)straightSteps;
            addPoint(trainPath, glm::mix(endLoop, startLoop, t));
        }
    }

    delete train;
    trainSpeed = 2.6f;
    train = new Train(trainPath, trainSpeed);
    delete draisine;
    draisine = new Draisine(trainPath);

    draisineSpeed = trainSpeed * 1.5f;

    if (train)
    {
        lastTrainPos = train->GetHeadSample().position;
        stuckTimer = 0.0f;
    }

    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

    // Text renderer for HUD
    {
        glm::ivec2 resolution = window->GetResolution();
        textRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
        textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 50);
    }
}


void Tema2::FrameStart()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema2::Update(float deltaTimeSeconds)
{
    auto renderGameOver = [&]()
    {
        glDisable(GL_DEPTH_TEST);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (textRenderer)
        {
            glm::ivec2 res = window->GetResolution();
            float scaleTitle = 1.2f;
            textRenderer->RenderText("Game over", res.x * 0.35f, res.y * 0.55f, scaleTitle, glm::vec3(1.0f));

            std::ostringstream oss;
            oss << "You survived for " << std::fixed << std::setprecision(1) << elapsedTime << " seconds";
            float scaleInfo = 0.7f;
            textRenderer->RenderText(oss.str(), res.x * 0.30f, res.y * 0.42f, scaleInfo, glm::vec3(1.0f));
        }
        glEnable(GL_DEPTH_TEST);
    };

    if (gameOver)
    {
        renderGameOver();
        return;
    }

    // Update elapsed time
    elapsedTime += deltaTimeSeconds;


    // Check if more than half of rails are broken
    if (railCountTotal > 0 && brokenCountCurrent > railCountTotal / 2)
    {
        gameOver = true;
    }
    if (gameOver)
    {
        renderGameOver();
        return;
    }

    // Terrain rendering
    float totalSize = terrainSize * terrainCellSize;
    float originOffset = -totalSize * 0.5f;
    glm::vec3 camPos = camera->position;

    int xMin = 0;
    int xMax = terrainSize - 1;
    int zMin = 0;
    int zMax = terrainSize - 1;

    for (int z = zMin; z <= zMax; ++z)
    {
        for (int x = xMin; x <= xMax; ++x)
        {
            int type = terrainCells[z * terrainSize + x];
            Mesh* tileMesh = terrainMeshes[type];
            if (!tileMesh) continue;
            glm::vec3 pos(originOffset + x * terrainCellSize, worldLift - 0.02f, originOffset + z * terrainCellSize);
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), pos);
            RenderMesh(tileMesh, shaders["VertexColor"], modelMatrix);
        }
    }

    // Rail grid rendering
    float railOriginOffset = originOffset;
    int rxMin = 0;
    int rxMax = railSize - 1;
    int rzMin = 0;
    int rzMax = railSize - 1;

    Shader* railShader = shaders["RailDeform"];
    auto setRailUniforms = [&](Shader* sh, float amp, float seed, int railType)
        {
            sh->Use();
            glUniform1f(glGetUniformLocation(sh->program, "u_amp"), amp);
            glUniform1f(glGetUniformLocation(sh->program, "u_seed"), seed);
            glUniform1i(glGetUniformLocation(sh->program, "u_type"), railType);
        };

    for (int z = rzMin; z <= rzMax; ++z)
    {
        for (int x = rxMin; x <= rxMax; ++x)
        {
            int type = railCells[z * railSize + x];
            if (type == 0) continue;
            int terrainType = terrainCells[z * terrainSize + x];

            glm::vec3 pos(railOriginOffset + x * terrainCellSize, worldLift + 0.02f, railOriginOffset + z * terrainCellSize);
            // Rails are modeled around their local origin; place them centered in the terrain cell
            pos.x += terrainCellSize * 0.5f;
            pos.z += terrainCellSize * 0.5f;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            bool isBroken = brokenCells[z * railSize + x];
            float deformAmp = isBroken ? 0.12f : 0.0f;
            float seed = (float)(x * 73 + z * 151);
            int railType = (terrainType == 1) ? 1 : (terrainType == 2 ? 2 : 0);
            auto& straightComp = (terrainType == 1)
                ? mountainRailComponent
                : (terrainType == 2 ? seaRailComponent : railComponent);

            auto renderStraight = [&](bool horizontal)
                {
                    if (horizontal) model = glm::rotate(model, RADIANS(-90.0f), glm::vec3(0, 1, 0));
                    if (isBroken)
                    {
                        setRailUniforms(railShader, deformAmp, seed, railType);
                        straightComp.Render(model, railShader,
                            [this](Mesh* m, Shader* s, const glm::mat4& mm)
                            {
                                RenderMesh(m, s, mm);
                            });
                    }
                    else
                    {
                        straightComp.Render(model, shaders["VertexColor"],
                            [this](Mesh* m, Shader* s, const glm::mat4& mm)
                            {
                                RenderMesh(m, s, mm);
                            });
                    }
                };

            if (type == 1)
            {
                renderStraight(true);
            }
            else if (type == 2)
            {
                renderStraight(false);
            }
            else if (type == 3 || type == 4)
            {
                if (terrainType != 0) continue; // no corners on mountain or sea
                float yaw = railRotations[z * railSize + x];
                model = glm::rotate(model, yaw, glm::vec3(0, 1, 0));
                bool isBrokenCorner = brokenCells[z * railSize + x];
                auto& cornerComp = (type == 3) ? cornerRailRight : cornerRailLeft;
                if (isBrokenCorner)
                {
                    setRailUniforms(railShader, deformAmp, seed, railType);
                    cornerComp.Render(model, railShader,
                        [this](Mesh* m, Shader* s, const glm::mat4& mm)
                        {
                            RenderMesh(m, s, mm);
                        });
                }
                else
                {
                    cornerComp.Render(model, shaders["VertexColor"],
                        [this](Mesh* m, Shader* s, const glm::mat4& mm)
                        {
                            RenderMesh(m, s, mm);
                        });
                }
            }
        }
    }

    // Tunnel arches over mountain rail tiles
    // Place the half-cylinder shell centered on the rail cell and oriented along the rail direction.
    for (int z = 0; z < railSize; ++z)
    {
        for (int x = 0; x < railSize; ++x)
        {
            int type = railCells[z * railSize + x];
            if (type == 0) continue;
            int terrainType = terrainCells[z * terrainSize + x];
            if (terrainType != 1) continue;


            glm::vec3 pos(railOriginOffset + x * terrainCellSize, worldLift + 0.02f, railOriginOffset + z * terrainCellSize);
            pos.x += terrainCellSize * 0.5f;
            pos.z += terrainCellSize * 0.5f;

            glm::mat4 tunnelModel = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.0f, -0.5f));
            tunnelModel = glm::scale(tunnelModel, glm::vec3(1.0f, 1.0f, RailConfig::SegmentLen / 2.5f));
            RenderMesh(tunnelMesh, shaders["VertexColor"], tunnelModel);
        }
    }

    // Advance and render train along track (stop if next rail is broken)
    if (train)
    {
        float step = trainSpeed * deltaTimeSeconds;
        auto toIndex = [&](const glm::vec3& p) -> int
            {
                int cx = (int)std::floor((p.x - originOffset) / terrainCellSize);
                int cz = (int)std::floor((p.z - originOffset) / terrainCellSize);
                if (cx < 0 || cx >= railSize || cz < 0 || cz >= railSize) return -1;
                return cz * railSize + cx;
            };

        bool canMove = true;
        auto curSample = train->GetHeadSample();
        int curIdx = toIndex(curSample.position);
        auto nextSample = train->Preview(step);
        int nextIdx = toIndex(nextSample.position);

        // Predict locomotive front overlap from BOTH current and next pose
        float frontOffset = 1.2f;
        glm::vec3 fwdCur(std::sin(curSample.yaw), 0.0f, std::cos(curSample.yaw));
        glm::vec3 frontPosCur = curSample.position + fwdCur * frontOffset;
        int frontIdxCur = toIndex(frontPosCur);

        glm::vec3 fwdNext(std::sin(nextSample.yaw), 0.0f, std::cos(nextSample.yaw));
        glm::vec3 frontPosNext = nextSample.position + fwdNext * frontOffset;
        int frontIdxNext = toIndex(frontPosNext);

        // If next cell differs and is broken, stop before entering
        if ((nextIdx >= 0 && nextIdx != curIdx && brokenCells[nextIdx]) ||
            (frontIdxCur >= 0 && frontIdxCur != curIdx && brokenCells[frontIdxCur]) ||
            (frontIdxNext >= 0 && frontIdxNext != curIdx && brokenCells[frontIdxNext]))
        {
            canMove = false;
        }

        if (canMove)
        {
            train->Forward(step);
        }

        // Stuck detection: if train barely moves for >30s
        glm::vec3 curPos = train->GetHeadSample().position;
        float distMoved = glm::length(curPos - lastTrainPos);
        if (distMoved < 1e-4f)
        {
            stuckTimer += deltaTimeSeconds;
            if (stuckTimer >= 30.0f) gameOver = true;
        }
        else
        {
            stuckTimer = 0.0f;
            lastTrainPos = curPos;
        }

        train->Render(shaders["VertexColor"],
            [this](Mesh* m, Shader* s, const glm::mat4& mm)
            {
                RenderMesh(m, s, mm);
            });
    }

    if (draisine)
    {
        draisine->Render(shaders["VertexColor"],
            [this](Mesh* m, Shader* s, const glm::mat4& mm)
            {
                RenderMesh(m, s, mm);
            });
    }

    // Render stations
    for (const auto& st : stations)
    {
        float baseY = worldLift + 0.02f; // slightly above ground
        glm::mat4 model = glm::translate(glm::mat4(1.0f), st.pos + glm::vec3(0.0f, baseY, 0.0f));
        model = glm::rotate(model, st.yaw, glm::vec3(0, 1, 0));
        if (st.comp)
        {
            st.comp->Render(model, shaders["VertexColor"],
                [this](Mesh* m, Shader* s, const glm::mat4& mm)
                {
                    RenderMesh(m, s, mm);
                });
        }
    }

    // Periodically break a random intact rail
    breakTimer += deltaTimeSeconds;
    if (breakTimer >= breakInterval)
    {
        breakTimer = 0.0f;
        BreakRandomRail();
    }

    // HUD timer in the left corner
    if (textRenderer)
    {
        glm::ivec2 res = window->GetResolution();
        glDisable(GL_DEPTH_TEST);

        std::ostringstream ossTime;
        ossTime << "Time elapsed: " << std::fixed << std::setprecision(1) << elapsedTime;
        textRenderer->RenderText(ossTime.str(), 10.0f, res.y - 40.0f, 0.6f, glm::vec3(1.0f));

        size_t loseThreshold = railCountTotal / 2;
        std::ostringstream ossRails;
        ossRails << "Rails broken: " << brokenCountCurrent << " / " << loseThreshold;
        textRenderer->RenderText(ossRails.str(), 10.0f, res.y - 80.0f, 0.6f, glm::vec3(1.0f));

        glEnable(GL_DEPTH_TEST);
    }
}


void Tema2::FrameEnd()
{
    if (gameOver) return;
    DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    if (gameOver) return;

    // Draisine controls along rails
    if (draisine)
    {
        auto toIndex = [&](const glm::vec3& p) -> int
            {
                int cx = (int)std::floor((p.x - originOffset) / terrainCellSize);
                int cz = (int)std::floor((p.z - originOffset) / terrainCellSize);
                if (cx < 0 || cx >= railSize || cz < 0 || cz >= railSize) return -1;
                return cz * railSize + cx;
            };

        auto canMoveStep = [&](float step)
            {
                auto cur = draisine->GetSample();
                auto next = draisine->Preview(step);

                int curIdx = toIndex(cur.position);
                int nextIdx = toIndex(next.position);

                // Direction aligned with movement 
                float sign = (step >= 0.0f) ? 1.0f : -1.0f;
                float frontOffset = 0.5f;
                glm::vec3 dirCur = glm::vec3(std::sin(cur.yaw), 0.0f, std::cos(cur.yaw)) * sign;
                glm::vec3 dirNext = glm::vec3(std::sin(next.yaw), 0.0f, std::cos(next.yaw)) * sign;
                int frontIdxCur = toIndex(cur.position + dirCur * frontOffset);
                int frontIdxNext = toIndex(next.position + dirNext * frontOffset);

                if ((nextIdx >= 0 && nextIdx != curIdx && brokenCells[nextIdx]) ||
                    (frontIdxCur >= 0 && frontIdxCur != curIdx && brokenCells[frontIdxCur]) ||
                    (frontIdxNext >= 0 && frontIdxNext != curIdx && brokenCells[frontIdxNext]))
                {
                    return false;
                }
                return true;
            };

        if (window->KeyHold(GLFW_KEY_W))
        {
            float step = draisineSpeed * deltaTime;
            if (canMoveStep(step)) draisine->Forward(step);
        }
        if (window->KeyHold(GLFW_KEY_S))
        {
            float step = draisineSpeed * deltaTime;
            if (canMoveStep(-step)) draisine->Backward(step);
        }

        // Update third-person camera to follow draisine
        auto sample = draisine->GetSample();
        glm::vec3 forward(std::sin(sample.yaw), 0.0f, std::cos(sample.yaw));
        glm::vec3 targetPos = sample.position + glm::vec3(0.0f, 0.5f, 0.0f);
        float heightOffset = cameraHeight + cameraDistance * 0.35f;
        glm::vec3 camPos = targetPos - forward * cameraDistance + glm::vec3(0.0f, heightOffset, 0.0f);
        camera->Set(camPos, targetPos, glm::vec3(0, 1, 0));
    }
}


void Tema2::OnKeyPress(int key, int mods)
{
    if (gameOver) return;

    if (key == GLFW_KEY_F && draisine)
    {
        // Repair nearest broken rail if very close to the draisine
        auto toIndex = [&](const glm::vec3& p) -> int
            {
                int cx = (int)std::floor((p.x - originOffset) / terrainCellSize);
                int cz = (int)std::floor((p.z - originOffset) / terrainCellSize);
                if (cx < 0 || cx >= railSize || cz < 0 || cz >= railSize) return -1;
                return cz * railSize + cx;
            };

        glm::vec3 dPos = draisine->GetSample().position;
        float repairRadius = terrainCellSize * 1.1f;
        float repairRadius2 = repairRadius * repairRadius;

        int bestIdx = -1;
        float bestDist2 = std::numeric_limits<float>::max();

        for (int i = 0; i < (int)brokenCells.size(); ++i)
        {
            if (!brokenCells[i])
                continue;

            int cx = i % railSize;
            int cz = i / railSize;
            glm::vec3 center(
                originOffset + (cx + 0.5f) * terrainCellSize,
                worldLift,
                originOffset + (cz + 0.5f) * terrainCellSize);

            float dist2 = glm::length2(center - dPos);
            if (dist2 < bestDist2)
            {
                bestDist2 = dist2;
                bestIdx = i;
            }
        }

        if (bestIdx != -1 && bestDist2 <= repairRadius2 && brokenCells[bestIdx])
        {
            brokenCells[bestIdx] = false;
            if (brokenCountCurrent > 0) --brokenCountCurrent;
        }
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Third-person camera follows draisine; no manual rotation needed
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    if (gameOver) return;

    // Zoom in/out by adjusting camera distance
    cameraDistance -= offsetY * 0.5f;
    if (cameraDistance < 2.0f) cameraDistance = 2.0f;
    if (cameraDistance > 20.0f) cameraDistance = 20.0f;
}


void Tema2::OnWindowResize(int width, int height)
{
}

void Tema2::BreakRandomRail()
{
    std::vector<int> candidates;
    candidates.reserve(railCells.size());
    for (size_t i = 0; i < railCells.size(); ++i)
    {
        if (railCells[i] == 0) continue;
        if (brokenCells[i]) continue;
        candidates.push_back((int)i);
    }
    if (candidates.empty()) return; // nothing left to break

    std::uniform_int_distribution<int> dist(0, (int)candidates.size() - 1);
    int pick = candidates[dist(rng)];
    if (!brokenCells[pick])
    {
        brokenCells[pick] = true;
        ++brokenCountCurrent;
    }
}