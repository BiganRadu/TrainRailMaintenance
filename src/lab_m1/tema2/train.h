#pragma once

#include <vector>
#include <functional>

#include "utils/glm_utils.h"
#include "lab_m1/tema2/component.h"
#include "lab_m1/tema2/meshes.h"

class Train
{
public:
    struct Sample
    {
        glm::vec3 position;
        float yaw; // rotation around Y so that +Z is forward
    };

    // pathPoints must describe a closed loop (first/last connected). The class wraps distance automatically.
    Train(const std::vector<glm::vec3>& pathPoints,
          float wagonSpacing = 2.0f);

    // Move forward along the path by a world-space distance.
    void Forward(float distance);

    // Render locomotive and wagons using provided shader and render function.
    void Render(Shader* shader,
                const std::function<void(Mesh*, Shader*, const glm::mat4&)>& renderFn) const;

    // Query current locomotive sample.
    Sample GetHeadSample() const { return headSample; }

    // Preview a sample offset ahead (positive) or behind (negative) without moving.
    Sample Preview(float distanceOffset) const { return SampleAtDistance(distanceHead + distanceOffset); }

private:
    Sample SampleAtDistance(float distanceAlongPath) const;

private:
    std::vector<glm::vec3> path;
    std::vector<float> cumulative; // cumulative lengths
    float loopLength = 0.0f;

    float distanceHead = 0.0f;
    float wagonSpacing;

    m1::Component locomotive;
    std::vector<m1::Component> wagons;

    Sample headSample;
};
