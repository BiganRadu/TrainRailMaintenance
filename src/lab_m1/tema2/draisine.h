#pragma once

#include <vector>
#include <functional>

#include "utils/glm_utils.h"
#include "lab_m1/tema2/component.h"
#include "lab_m1/tema2/meshes.h"

class Draisine
{
public:
 struct Sample
 {
 glm::vec3 position;
 float yaw;
 };

 // pathPoints must describe a closed loop.
 explicit Draisine(const std::vector<glm::vec3>& pathPoints);

 void Forward(float distance);
 void Backward(float distance);

 void Render(Shader* shader,
 const std::function<void(Mesh*, Shader*, const glm::mat4&)>& renderFn) const;

 Sample GetSample() const { return curSample; }
 Sample Preview(float distanceOffset) const { return SampleAtDistance(distanceCur + distanceOffset); }

private:
 Sample SampleAtDistance(float distanceAlongPath) const;

private:
 std::vector<glm::vec3> path;
 std::vector<float> cumulative;
 float loopLength = 0.0f;
 float distanceCur = 0.0f;

 // Geometry/animation data
 m1::Component staticBody; // everything except bar and grips
 Mesh* barMesh = nullptr;
 Mesh* gripMesh = nullptr;
 float barLen = 1.0f;
 float barY = 0.0f;
 float gripLen = 0.45f;
 float gripRadius = 0.05f;

 float swingPhase = 0.0f;
 float swingAmplitude = 0.35f; // radians
 float swingFrequency = 1.5f;   // phase advance per world unit

 Sample curSample;
};
