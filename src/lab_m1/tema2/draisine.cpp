#include "lab_m1/tema2/draisine.h"

#include <algorithm>

Draisine::Draisine(const std::vector<glm::vec3>& pathPoints)
    : path(pathPoints)
{
    // Dimensions
    float baseLen    = 1.2f;
    float baseWidth  = RailConfig::Gauge + 0.15f;
    float baseHeight = 0.12f;

    float postWidth  = 0.18f;
    float postDepth  = 0.18f;
    float postHeight = 0.45f;

    float pivotRadius = 0.07f;
    float pivotHeight = 0.4f;

    barLen     = 1.0f;
    float barRadius  = 0.06f;
    gripRadius = 0.05f;
    gripLen    = 0.45f;

    float wheelRadius    = 0.14f;
    float wheelThickness = 0.10f;

    // Colors
    glm::vec3 baseColor (0.9f, 0.5f, 0.1f); // orange
	glm::vec3 postColor(0.9f, 0.5f, 0.1f); // orange
    glm::vec3 barColor  (0.2f, 0.2f, 0.2f); // dark gray
    glm::vec3 gripColor (0.1f, 0.9f, 0.1f); // green
	glm::vec3 wheelColor(0.2f, 0.2f, 0.2f); // dark gray

    // Meshes
    Mesh* baseMesh  = Meshes::CreateParallelepiped("draisine_base", glm::vec3(0), baseWidth, baseHeight, baseLen, baseColor);
    Mesh* postMesh  = Meshes::CreateParallelepiped("draisine_post", glm::vec3(0), postWidth, postHeight, postDepth, postColor);
    Mesh* pivotMesh = Meshes::CreateCylinder("draisine_pivot", glm::vec3(0), pivotRadius, pivotRadius, pivotHeight, barColor);
    barMesh         = Meshes::CreateCylinder("draisine_bar", glm::vec3(0), barRadius, barRadius, barLen, barColor);
    gripMesh        = Meshes::CreateCylinder("draisine_grip", glm::vec3(0), gripRadius, gripRadius, gripLen, gripColor);
    Mesh* wheelMesh = Meshes::CreateCylinder("draisine_wheel", glm::vec3(0), wheelRadius, wheelRadius, wheelThickness, wheelColor);

    // Build static body (base, post, pivot, wheels)
    float baseOffsetY = wheelRadius * 2.0f;
    staticBody.AddPart(baseMesh, glm::translate(glm::mat4(1.0f), glm::vec3(-baseWidth * 0.5f, baseOffsetY, -baseLen * 0.5f)));

    staticBody.AddPart(postMesh, glm::translate(glm::mat4(1.0f), glm::vec3(-postWidth * 0.5f, baseOffsetY + baseHeight, -postDepth * 0.5f)));

    float pivotY = baseOffsetY + baseHeight + postHeight;
    staticBody.AddPart(pivotMesh, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, pivotY, 0.0f)));

    // Wheels: 2 per side (front/back) aligned to gauge
    float wheelY = wheelRadius;
    float wheelXLeft  = -RailConfig::Gauge * 0.5f;
    float wheelXRight =  RailConfig::Gauge * 0.5f;
    float wheelZFront = -baseLen * 0.5f + 0.2f;
    float wheelZBack  =  baseLen * 0.5f - 0.2f;
    auto wheelTransform = [&](float x, float z, float rotSign)
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(x, wheelY, z));
        t = glm::rotate(t, RADIANS(rotSign * 90.0f), glm::vec3(0, 0, 1));
        return t;
    };
    staticBody.AddPart(wheelMesh, wheelTransform(wheelXLeft, wheelZFront, 1.0f));
    staticBody.AddPart(wheelMesh, wheelTransform(wheelXRight, wheelZFront, -1.0f));
    staticBody.AddPart(wheelMesh, wheelTransform(wheelXLeft, wheelZBack, 1.0f));
    staticBody.AddPart(wheelMesh, wheelTransform(wheelXRight, wheelZBack, -1.0f));

    // Store bar height above ground for animation
    barY = pivotY + pivotHeight;

    // Path setup
    loopLength = 0.0f;
    cumulative.clear();
    cumulative.push_back(0.0f);
    for (size_t i = 0; i < path.size(); ++i)
    {
        const glm::vec3& a = path[i];
        const glm::vec3& b = path[(i + 1) % path.size()];
        float segLen = glm::length(b - a);
        loopLength += segLen;
        cumulative.push_back(loopLength);
    }

    // start somewhere else on the loop so it doesn't overlap the train
    distanceCur = loopLength * 0.25f;
    curSample = SampleAtDistance(distanceCur);
}

void Draisine::Forward(float distance)
{
    if (loopLength <= 1e-4f) return;
    distanceCur = std::fmod(distanceCur + distance, loopLength);
    if (distanceCur < 0.0f) distanceCur += loopLength;
    curSample = SampleAtDistance(distanceCur);

    // Advance swing phase based on travel distance
    swingPhase += distance * swingFrequency;
}

void Draisine::Backward(float distance)
{
    Forward(-distance);
}

Draisine::Sample Draisine::SampleAtDistance(float distanceAlongPath) const
{
    if (path.empty() || loopLength <= 1e-4f)
    {
        return Sample{ glm::vec3(0.0f), 0.0f };
    }

    distanceAlongPath = std::fmod(distanceAlongPath, loopLength);
    if (distanceAlongPath < 0.0f) distanceAlongPath += loopLength;

    auto it = std::upper_bound(cumulative.begin(), cumulative.end(), distanceAlongPath);
    size_t idx1 = (it == cumulative.end()) ? cumulative.size() - 2 : std::max<size_t>(1, (size_t)(it - cumulative.begin()));
    size_t idx0 = idx1 - 1;

    float segStart = cumulative[idx0];
    float segEnd = cumulative[idx1];
    float segLen = segEnd - segStart;
    float t = segLen > 1e-6f ? (distanceAlongPath - segStart) / segLen : 0.0f;

    const size_t n = path.size();
    const glm::vec3& p0 = path[idx0 % n];
    const glm::vec3& p1 = path[idx1 % n];
    glm::vec3 pos = glm::mix(p0, p1, t);

    glm::vec3 tangent = p1 - p0;
    tangent.y = 0.0f;
    if (glm::length2(tangent) < 1e-8f)
    {
        tangent = glm::vec3(0, 0, 1);
    }
    tangent = glm::normalize(tangent);

    float yaw = std::atan2(tangent.x, tangent.z);
    return Sample{ pos, yaw };
}

void Draisine::Render(Shader* shader,
    const std::function<void(Mesh*, Shader*, const glm::mat4&)>& renderFn) const
{
    glm::mat4 model = glm::mat4(1.0f);
    // lift slightly above rails
    model = glm::translate(model, curSample.position + glm::vec3(0.0f, 0.03f, 0.0f));
    model = glm::rotate(model, curSample.yaw, glm::vec3(0, 1, 0));

    // Render static body
    staticBody.Render(model, shader, renderFn);

    // Animated swing for bar and grips
    float angle = std::sin(swingPhase) * swingAmplitude;

    auto renderBar = [&](Mesh* mesh, const glm::mat4& local)
    {
        renderFn(mesh, shader, model * local);
    };

    // Base orientation for bar 
    glm::mat4 barBase = glm::mat4(1.0f);
    barBase = glm::translate(barBase, glm::vec3(0.0f, barY, 0.0f));
    barBase = glm::rotate(barBase, RADIANS(90.0f), glm::vec3(0, 0, 1));
    barBase = glm::rotate(barBase, RADIANS(90.0f), glm::vec3(1, 0, 0));
    barBase = glm::rotate(barBase, angle, glm::vec3(0, 0, 1));
    barBase = glm::translate(barBase, glm::vec3(0.0f, -barLen * 0.5f, 0.0f));

    renderBar(barMesh, barBase);

    // Grips inherit bar swing; positions match original offsets
    glm::mat4 gripL = glm::mat4(1.0f);
    gripL = glm::translate(gripL, glm::vec3(gripLen / 2, barY, -gripRadius - barLen / 2));
    gripL = glm::rotate(gripL, RADIANS(90.0f), glm::vec3(0, 0, 1));
    gripL = glm::translate(gripL, glm::vec3(gripLen * std::sin(angle), 0.0f, gripLen / 2 - gripLen / 2 * std::cos(angle)));
    renderBar(gripMesh, gripL);

    glm::mat4 gripR = glm::mat4(1.0f);
    gripR = glm::translate(gripR, glm::vec3(gripLen / 2, barY, -gripRadius + barLen / 2 + 0.15f));
    gripR = glm::rotate(gripR, RADIANS(90.0f), glm::vec3(0, 0, 1));
	gripR = glm::translate(gripR, glm::vec3(-gripLen * std::sin(angle), 0.0f, gripLen / 2 * std::cos(angle) - gripLen / 2));
    renderBar(gripMesh, gripR);
}
