#include "lab_m1/tema2/train.h"

#include <algorithm>
#include <numeric>

#include "lab_m1/tema2/meshes.h"


Train::Train(const std::vector<glm::vec3>& pathPoints, float wagonSpacing)
    : path(pathPoints), wagonSpacing(wagonSpacing)
{
    // Build rolling stock
    locomotive = Components::CreateLocomotive("train_loc");
    wagons.clear();
    wagons.push_back(Components::CreateWagon("train_wagon"));

    // Build cumulative lengths
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

    distanceHead = 0.0f;
    headSample = SampleAtDistance(distanceHead);
}

void Train::Forward(float distance)
{
    if (loopLength <= 1e-4f) return;
    distanceHead = std::fmod(distanceHead + distance, loopLength);
    if (distanceHead < 0.0f) distanceHead += loopLength;
    headSample = SampleAtDistance(distanceHead);
}

Train::Sample Train::SampleAtDistance(float distanceAlongPath) const
{
    if (path.empty() || loopLength <= 1e-4f)
        return Sample{ glm::vec3(0.0f), 0.0f };

    distanceAlongPath = std::fmod(distanceAlongPath, loopLength);
    if (distanceAlongPath < 0.0f) distanceAlongPath += loopLength;

    // Helper to get Position at strict distance
    auto GetPos = [&](float d) -> glm::vec3 {
        // Wrap internal distance
        d = std::fmod(d, loopLength);
        if (d < 0.0f) d += loopLength;

        auto it = std::upper_bound(cumulative.begin(), cumulative.end(), d);
        size_t idx1 = (it == cumulative.end()) ? cumulative.size() - 2 : std::max<size_t>(1, (size_t)(it - cumulative.begin()));
        size_t idx0 = idx1 - 1;

        float segStart = cumulative[idx0];
        float segEnd = cumulative[idx1];
        float segLen = segEnd - segStart;
        float t = segLen > 1e-6f ? (d - segStart) / segLen : 0.0f;

        const size_t n = path.size();
        return glm::mix(path[idx0 % n], path[idx1 % n], t);
        };

    glm::vec3 pos = GetPos(distanceAlongPath);
    float lookAheadDist = 0.1f;
    glm::vec3 posFront = GetPos(distanceAlongPath + lookAheadDist);

    glm::vec3 dirCur = posFront - pos;
    dirCur.y = 0.0f;

    // Fallback if moving extremely slowly or path is corrupt
    if (glm::length2(dirCur) < 1e-8f) dirCur = glm::vec3(0, 0, 1);
    else dirCur = glm::normalize(dirCur);

    float yaw = std::atan2(dirCur.x, dirCur.z);
    return Sample{ pos, yaw };
}

void Train::Render(Shader* shader,
                   const std::function<void(Mesh*, Shader*, const glm::mat4&)>& renderFn) const
{
    // Locomotive at head
    glm::mat4 headModel = glm::mat4(1.0f);
    headModel = glm::translate(headModel, headSample.position);
    headModel = glm::rotate(headModel, headSample.yaw, glm::vec3(0, 1, 0));
    locomotive.Render(headModel, shader, renderFn);

    // Single wagon (can be extended) placed behind along path
    float wagonDist = distanceHead - wagonSpacing;
    Sample wagonSample = SampleAtDistance(wagonDist);
    glm::mat4 wagonModel = glm::mat4(1.0f);
    wagonModel = glm::translate(wagonModel, wagonSample.position);
    wagonModel = glm::rotate(wagonModel, wagonSample.yaw, glm::vec3(0, 1, 0));
    wagons[0].Render(wagonModel, shader, renderFn);
}
