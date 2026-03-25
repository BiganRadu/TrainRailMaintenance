#pragma once

#include <string>
#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"
#include "lab_m1/tema2/component.h"

namespace RailConfig
{
    constexpr float Gauge = 0.75f; // distance between rails (center-to-center)
    constexpr float SegmentLen = 1.3f; // length of a standard rail segment
}

namespace Meshes
{
    // Creates a parallelepiped (box) with configurable size and color.
    // `corner` is the lower-left-back corner.
    Mesh* CreateParallelepiped(const std::string &name,
                               const glm::vec3 &corner,
                               float length,
                               float height,
                               float width,
                               const glm::vec3 &color);

    // Creates a vertical cylinder centered at `baseCenter` on the bottom.
    Mesh* CreateCylinder(const std::string &name,
                         const glm::vec3 &baseCenter,
                         float radiusX,
                         float radiusZ,
                         float height,
                         const glm::vec3 &color,
                         unsigned int segments = 48);

    // Creates a half-cylinder shell open at both ends; outer radius and wall thickness configurable.
    Mesh* CreateHalfCylinderShell(const std::string &name,
                                  const glm::vec3 &center,
                                  float radius,
                                  float length,
                                  float thickness,
                                  const glm::vec3 &color,
                                  unsigned int segments = 48);
}

namespace Components
{
    // Creates a shorter standard rail segment as a component.
    m1::Component CreateStandardRail(const std::string &name);

    // Creates a locomotive component composed of boxes and cylinders.
    m1::Component CreateLocomotive(const std::string &name);

    // Creates a wagon with platform, box load, and two wheels.
    m1::Component CreateWagon(const std::string &name);

    // Creates a draisine handcar component sized to rail gauge.
    m1::Component CreateDraisine(const std::string &name);

    // Curved corner rails (90 degrees).
    m1::Component CreateCornerRailLeft(const std::string &name);
    m1::Component CreateCornerRailRight(const std::string &name);

    // Mountain straight rail variant
    m1::Component CreateMountainRail(const std::string &name);

    // Sea straight rail variant
    m1::Component CreateSeaRail(const std::string &name);

    m1::Component CreateStation(const std::string& name,
        const glm::vec3& color,
        float length,
        float height,
        float width);
}
