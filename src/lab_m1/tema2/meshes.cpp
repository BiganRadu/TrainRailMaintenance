#include "lab_m1/tema2/meshes.h"

#include <vector>

#include "core/engine.h"
#include "utils/gl_utils.h"
#include "lab_m1/tema2/component.h"

namespace Meshes
{
    Mesh* CreateParallelepiped(const std::string &name,
                               const glm::vec3 &corner,
                               float length,
                               float height,
                               float width,
                               const glm::vec3 &color)
    {
        std::vector<VertexFormat> vertices = {
            // Front
            VertexFormat(corner + glm::vec3(0,       0,      width), color, glm::vec3(0, 0, 1)),
            VertexFormat(corner + glm::vec3(length,  0,      width), color, glm::vec3(0, 0, 1)),
            VertexFormat(corner + glm::vec3(length,  height, width), color, glm::vec3(0, 0, 1)),
            VertexFormat(corner + glm::vec3(0,       height, width), color, glm::vec3(0, 0, 1)),

            // Back
            VertexFormat(corner + glm::vec3(length,  0,      0), color, glm::vec3(0, 0, -1)),
            VertexFormat(corner + glm::vec3(0,       0,      0), color, glm::vec3(0, 0, -1)),
            VertexFormat(corner + glm::vec3(0,       height, 0), color, glm::vec3(0, 0, -1)),
            VertexFormat(corner + glm::vec3(length,  height, 0), color, glm::vec3(0, 0, -1)),

            // Left
            VertexFormat(corner + glm::vec3(0,       0,      0), color, glm::vec3(-1, 0, 0)),
            VertexFormat(corner + glm::vec3(0,       0,      width), color, glm::vec3(-1, 0, 0)),
            VertexFormat(corner + glm::vec3(0,       height, width), color, glm::vec3(-1, 0, 0)),
            VertexFormat(corner + glm::vec3(0,       height, 0), color, glm::vec3(-1, 0, 0)),

            // Right
            VertexFormat(corner + glm::vec3(length,  0,      width), color, glm::vec3(1, 0, 0)),
            VertexFormat(corner + glm::vec3(length,  0,      0), color, glm::vec3(1, 0, 0)),
            VertexFormat(corner + glm::vec3(length,  height, 0), color, glm::vec3(1, 0, 0)),
            VertexFormat(corner + glm::vec3(length,  height, width), color, glm::vec3(1, 0, 0)),

            // Top
            VertexFormat(corner + glm::vec3(0,       height, width), color, glm::vec3(0, 1, 0)),
            VertexFormat(corner + glm::vec3(length,  height, width), color, glm::vec3(0, 1, 0)),
            VertexFormat(corner + glm::vec3(length,  height, 0), color, glm::vec3(0, 1, 0)),
            VertexFormat(corner + glm::vec3(0,       height, 0), color, glm::vec3(0, 1, 0)),

            // Bottom
            VertexFormat(corner + glm::vec3(0,       0,      0), color, glm::vec3(0, -1, 0)),
            VertexFormat(corner + glm::vec3(length,  0,      0), color, glm::vec3(0, -1, 0)),
            VertexFormat(corner + glm::vec3(length,  0,      width), color, glm::vec3(0, -1, 0)),
            VertexFormat(corner + glm::vec3(0,       0,      width), color, glm::vec3(0, -1, 0)),
        };

        std::vector<unsigned int> indices = {
            // Front
            0, 1, 2, 0, 2, 3,
            // Back
            5, 4, 7, 5, 7, 6,
            // Left
            9, 8, 11, 9, 11, 10,
            // Right
            12, 13, 14, 12, 14, 15,
            // Top
            16, 17, 18, 16, 18, 19,
            // Bottom
            21, 20, 23, 21, 23, 22
        };

        Mesh* box = new Mesh(name);
        box->InitFromData(vertices, indices);
        return box;
    }

    Mesh* CreateCylinder(const std::string &name,
                         const glm::vec3 &baseCenter,
                         float radiusX,
                         float radiusZ,
                         float height,
                         const glm::vec3 &color,
                         unsigned int segments)
    {
        if (segments < 48) segments = 48; // ensure well-rounded appearance

        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;

        // Side vertices
        for (unsigned int i = 0; i < segments; ++i)
        {
            float angle = 2.0f * (float)M_PI * i / segments;
            float cx = std::cos(angle);
            float cz = std::sin(angle);
            glm::vec3 normal = glm::normalize(glm::vec3(cx * radiusZ, 0.0f, cz * radiusX));

            glm::vec3 posBottom = baseCenter + glm::vec3(radiusX * cx, 0.0f, radiusZ * cz);
            glm::vec3 posTop    = baseCenter + glm::vec3(radiusX * cx, height, radiusZ * cz);

            vertices.emplace_back(posBottom, color, normal);
            vertices.emplace_back(posTop,    color, normal);
        }

        // Side indices (two triangles per segment)
        for (unsigned int i = 0; i < segments; ++i)
        {
            unsigned int i0 = 2 * i;
            unsigned int i1 = 2 * ((i + 1) % segments);
            unsigned int i0Top = i0 + 1;
            unsigned int i1Top = i1 + 1;

            // First triangle
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i1Top);

            // Second triangle
            indices.push_back(i0);
            indices.push_back(i1Top);
            indices.push_back(i0Top);
        }

        // Centers for caps
        unsigned int bottomCenterIndex = (unsigned int)vertices.size();
        vertices.emplace_back(baseCenter, color, glm::vec3(0, -1, 0));
        unsigned int topCenterIndex = (unsigned int)vertices.size();
        vertices.emplace_back(baseCenter + glm::vec3(0, height, 0), color, glm::vec3(0, 1, 0));

        // Bottom cap
        for (unsigned int i = 0; i < segments; ++i)
        {
            unsigned int next = (i + 1) % segments;
            indices.push_back(bottomCenterIndex);
            indices.push_back(2 * next);
            indices.push_back(2 * i);
        }

        // Top cap
        for (unsigned int i = 0; i < segments; ++i)
        {
            unsigned int next = (i + 1) % segments;
            indices.push_back(topCenterIndex);
            indices.push_back(2 * i + 1);
            indices.push_back(2 * next + 1);
        }

        Mesh* cyl = new Mesh(name);
        cyl->InitFromData(vertices, indices);
        return cyl;
    }

    Mesh* CreateHalfCylinderShell(const std::string &name,
                                  const glm::vec3 &center,
                                  float radius,
                                  float length,
                                  float thickness,
                                  const glm::vec3 &color,
                                  unsigned int segments)
    {
        if (segments < 12) segments = 12;
        if (thickness <= 0.0f) thickness = radius * 0.05f;
        float innerRadius = std::max(0.001f, radius - thickness);

        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;

        // Angles from 0..pi
        for (unsigned int i = 0; i <= segments; ++i)
        {
            float t = (float)i / segments;
            float angle = (float)M_PI * t;
            float ca = std::cos(angle);
            float sa = std::sin(angle);

            glm::vec3 outerNormal = glm::normalize(glm::vec3(ca, sa, 0.0f));
            glm::vec3 innerNormal = -outerNormal;

            glm::vec3 outer0 = center + glm::vec3(radius * ca, radius * sa, 0.0f);
            glm::vec3 outer1 = center + glm::vec3(radius * ca, radius * sa, length);
            glm::vec3 inner0 = center + glm::vec3(innerRadius * ca, innerRadius * sa, 0.0f);
            glm::vec3 inner1 = center + glm::vec3(innerRadius * ca, innerRadius * sa, length);

            // outer side
            vertices.emplace_back(outer0, color, outerNormal);
            vertices.emplace_back(outer1, color, outerNormal);
            // inner side
            vertices.emplace_back(inner0, color, innerNormal);
            vertices.emplace_back(inner1, color, innerNormal);
        }

        // Build quads along angle for outer and inner surfaces
        for (unsigned int i = 0; i < segments; ++i)
        {
            unsigned int o0 = 4 * i;
            unsigned int o1 = 4 * (i + 1);
            unsigned int o0b = o0 + 1;
            unsigned int o1b = o1 + 1;

            unsigned int in0 = o0 + 2;
            unsigned int in1 = o1 + 2;
            unsigned int in0b = in0 + 1;
            unsigned int in1b = in1 + 1;

            // outer
            indices.push_back(o0);
            indices.push_back(o1);
            indices.push_back(o1b);
            indices.push_back(o0);
            indices.push_back(o1b);
            indices.push_back(o0b);

            // inner
            indices.push_back(in0);
            indices.push_back(in0b);
            indices.push_back(in1b);
            indices.push_back(in0);
            indices.push_back(in1b);
            indices.push_back(in1);
        }

        // Rims at z=0 and z=length connecting outer/inner edges
        auto addRim = [&](bool atBack)
        {
            unsigned int base = (unsigned int)vertices.size();
            float z = atBack ? 0.0f : length;
            glm::vec3 normal = atBack ? glm::vec3(0, 0, -1) : glm::vec3(0, 0, 1);

            for (unsigned int i = 0; i <= segments; ++i)
            {
                float t = (float)i / segments;
                float angle = (float)M_PI * t;
                float ca = std::cos(angle);
                float sa = std::sin(angle);
                glm::vec3 outerP = center + glm::vec3(radius * ca, radius * sa, z);
                glm::vec3 innerP = center + glm::vec3(innerRadius * ca, innerRadius * sa, z);
                vertices.emplace_back(outerP, color, normal);
                vertices.emplace_back(innerP, color, normal);
            }

            for (unsigned int i = 0; i < segments; ++i)
            {
                unsigned int a0 = base + 2 * i;
                unsigned int a1 = base + 2 * (i + 1);
                unsigned int b0 = a0 + 1;
                unsigned int b1 = a1 + 1;
                indices.push_back(a0);
                indices.push_back(a1);
                indices.push_back(b1);
                indices.push_back(a0);
                indices.push_back(b1);
                indices.push_back(b0);
            }
        };

        addRim(true);
        addRim(false);

        Mesh* arch = new Mesh(name);
        arch->InitFromData(vertices, indices);
        return arch;
    }
}

namespace Components
{

    m1::Component CreateLocomotive(const std::string &name)
    {
        m1::Component comp;

        // Overall dimensions
        float baseLen   = 2.4f;
        float baseHeight= 0.12f;
        float baseWidth  = RailConfig::Gauge + 0.2f;

        // Cab
        float cabLen    = 0.9f;
        float cabHeight = 0.8f;
        float cabWidth  = baseWidth;

        // Boiler
        float boilerLen = 1.2f;
        float boilerRadiusX = 0.25f;
        float boilerRadiusZ = 0.25f;

        // Nose plug
        float plugRadius = 0.06f;
        float plugDepth  = 0.08f;

        // Wheels
        float wheelRadius = 0.12f;
        float wheelThickness = 0.08f;
        int wheelCount = 7;

        // Colors
        glm::vec3 baseColor   (1.0f, 0.9f, 0.1f); // yellowish
        glm::vec3 cabColor    (0.0f, 0.9f, 0.0f); // green
        glm::vec3 boilerColor (0.1f, 0.1f, 0.9f); // blue
        glm::vec3 plugColor   (0.9f, 0.1f, 0.8f); // pink
        glm::vec3 wheelColor  (0.9f, 0.0f, 0.0f); // red

        // Shared meshes
        Mesh* baseMesh   = Meshes::CreateParallelepiped(name + "_base", glm::vec3(0), baseWidth, baseHeight, baseLen, baseColor);
        Mesh* cabMesh    = Meshes::CreateParallelepiped(name + "_cab",  glm::vec3(0), cabWidth,  cabHeight, cabLen, cabColor);
        Mesh* boilerMesh = Meshes::CreateCylinder(name + "_boiler", glm::vec3(0), boilerRadiusX, boilerRadiusZ, boilerLen, boilerColor);
        Mesh* plugMesh   = Meshes::CreateCylinder(name + "_plug",   glm::vec3(0), plugRadius, plugRadius, plugDepth, plugColor);
        Mesh* wheelMesh  = Meshes::CreateCylinder(name + "_wheel",  glm::vec3(0), wheelRadius, wheelRadius, wheelThickness, wheelColor);

        // Base placement
        float baseOffsetY = wheelRadius * 2.0f;
        comp.AddPart(baseMesh, glm::translate(glm::mat4(1.0f), glm::vec3(-baseWidth * 0.5f, baseOffsetY, -baseLen * 0.5f)));

        // Cab positioned at back
        float cabZ = -baseLen * 0.5f + cabLen * 0.5f;
        comp.AddPart(cabMesh,
            glm::translate(glm::mat4(1.0f), glm::vec3(-cabWidth * 0.5f, baseOffsetY + baseHeight, cabZ - cabLen * 0.5f)));

        // Boiler sitting on base, aligned along Z
        float boilerY = baseOffsetY + baseHeight + 0.9f * boilerRadiusZ;
        float boilerZ = cabZ + cabLen * 0.5f + boilerLen * 0.5f;
        glm::mat4 boilerTransform = glm::mat4(1.0f);
        boilerTransform = glm::translate(boilerTransform, glm::vec3(0.0f, boilerY, boilerZ - boilerLen * 0.5f));
        boilerTransform = glm::rotate(boilerTransform, RADIANS(90.0f), glm::vec3(1, 0, 0));
        comp.AddPart(boilerMesh, boilerTransform);

        // Nose plug at front of boiler
        glm::mat4 plugTransform = glm::mat4(1.0f);
        float plugZ = boilerZ + boilerLen * 0.5f;
        plugTransform = glm::translate(plugTransform, glm::vec3(0.0f, boilerY, plugZ - plugDepth * 0.5f));
        plugTransform = glm::rotate(plugTransform, RADIANS(90.0f), glm::vec3(1, 0, 0));
        comp.AddPart(plugMesh, plugTransform);

        // Wheels: 7 per side along Z
        float wheelStartZ = -baseLen * 0.5f + 0.2f;
        float wheelSpacing = (baseLen - 0.4f) / (wheelCount - 1);
        float wheelY = wheelRadius;
        float wheelMargin = 0.02f;
        float wheelXLeft = -RailConfig::Gauge * 0.5f;
        float wheelXRight = RailConfig::Gauge * 0.5f;
        for (int i = 0; i < wheelCount; ++i)
        {
            float z = wheelStartZ + i * wheelSpacing;
            glm::mat4 wL = glm::translate(glm::mat4(1.0f), glm::vec3(wheelXLeft, wheelY, z)) *
                           glm::rotate(glm::mat4(1.0f), RADIANS(90.0f), glm::vec3(0, 0, 1));
            glm::mat4 wR = glm::translate(glm::mat4(1.0f), glm::vec3(wheelXRight, wheelY, z)) *
                           glm::rotate(glm::mat4(1.0f), RADIANS(-90.0f), glm::vec3(0, 0, 1));
            comp.AddPart(wheelMesh, wL);
            comp.AddPart(wheelMesh, wR);
        }

        return comp;
    }

    m1::Component CreateWagon(const std::string &name)
    {
        m1::Component comp;

        // Dimensions proportional to locomotive
        float baseLen    = 2.0f;
        float baseHeight = 0.12f;
        float baseWidth  = RailConfig::Gauge + 0.2f;

        float boxHeight  = 0.9f;
        float boxWidth   = baseWidth;
        float boxLen     = baseLen;

        float wheelRadius    = 0.12f;
        float wheelThickness = 0.08f;

        // Colors
        glm::vec3 baseColor (1.0f, 0.9f, 0.1f); // yellow
        glm::vec3 boxColor  (0.0f, 0.9f, 0.0f); // green
        glm::vec3 wheelColor(0.9f, 0.0f, 0.0f); // red

        // Meshes
        Mesh* baseMesh  = Meshes::CreateParallelepiped(name + "_base", glm::vec3(0), baseWidth, baseHeight, baseLen, baseColor);
        Mesh* boxMesh   = Meshes::CreateParallelepiped(name + "_box",  glm::vec3(0), boxWidth, boxHeight, boxLen, boxColor);
        Mesh* wheelMesh = Meshes::CreateCylinder(name + "_wheel", glm::vec3(0), wheelRadius, wheelRadius, wheelThickness, wheelColor);

        float baseOffsetY = wheelRadius * 2.0f;
        comp.AddPart(baseMesh, glm::translate(glm::mat4(1.0f), glm::vec3(-baseWidth * 0.5f, baseOffsetY, -baseLen * 0.5f)));

        // Box sitting on platform
        comp.AddPart(boxMesh, glm::translate(glm::mat4(1.0f), glm::vec3(-boxWidth * 0.5f, baseOffsetY + baseHeight, -boxLen * 0.5f)));

        // Wheels: 2 per side (front/back) aligned to rail gauge
        float wheelY = wheelRadius;
        float wheelXLeft  = -RailConfig::Gauge * 0.5f;
        float wheelXRight =  RailConfig::Gauge * 0.5f;
        float wheelZFront = -baseLen * 0.5f + 0.25f;
        float wheelZBack  =  baseLen * 0.5f - 0.25f;
        glm::mat4 wLF = glm::translate(glm::mat4(1.0f), glm::vec3(wheelXLeft, wheelY, wheelZFront)) *
                        glm::rotate(glm::mat4(1.0f), RADIANS(90.0f), glm::vec3(0, 0, 1));
        glm::mat4 wRF = glm::translate(glm::mat4(1.0f), glm::vec3(wheelXRight, wheelY, wheelZFront)) *
                        glm::rotate(glm::mat4(1.0f), RADIANS(-90.0f), glm::vec3(0, 0, 1));
        glm::mat4 wLB = glm::translate(glm::mat4(1.0f), glm::vec3(wheelXLeft, wheelY, wheelZBack)) *
                        glm::rotate(glm::mat4(1.0f), RADIANS(90.0f), glm::vec3(0, 0, 1));
        glm::mat4 wRB = glm::translate(glm::mat4(1.0f), glm::vec3(wheelXRight, wheelY, wheelZBack)) *
                        glm::rotate(glm::mat4(1.0f), RADIANS(-90.0f), glm::vec3(0, 0, 1));
        comp.AddPart(wheelMesh, wLF);
        comp.AddPart(wheelMesh, wRF);
        comp.AddPart(wheelMesh, wLB);
        comp.AddPart(wheelMesh, wRB);

        return comp;
    }

    m1::Component CreateDraisine(const std::string &name)
    {
        m1::Component comp;

        // Dimensions
        float baseLen    = 1.2f;
        float baseWidth  = RailConfig::Gauge + 0.15f;
        float baseHeight = 0.12f;

        float postWidth  = 0.18f;
        float postDepth  = 0.18f;
        float postHeight = 0.45f;

        float pivotRadius = 0.07f;
        float pivotHeight = 0.4f;

        float barRadius  = 0.06f;
        float barLen     = 1.0f;
        float gripRadius = 0.05f;
        float gripLen    = 0.45f;

        float wheelRadius    = 0.14f;
        float wheelThickness = 0.10f;

        // Colors
        glm::vec3 baseColor (0.9f, 0.5f, 0.1f); // orange
		glm::vec3 postColor(0.9f, 0.5f, 0.1f); // orange
        glm::vec3 barColor  (0.2f, 0.2f, 0.2f); // dark gray
        glm::vec3 gripColor (0.1f, 0.9f, 0.1f); // green
		glm::vec3 wheelColor(0.2f, 0.2f, 0.2f); // dark gray

        // Meshes
        Mesh* baseMesh  = Meshes::CreateParallelepiped(name + "_base", glm::vec3(0), baseWidth, baseHeight, baseLen, baseColor);
        Mesh* postMesh  = Meshes::CreateParallelepiped(name + "_post", glm::vec3(0), postWidth, postHeight, postDepth, postColor);
        Mesh* pivotMesh = Meshes::CreateCylinder(name + "_pivot", glm::vec3(0), pivotRadius, pivotRadius, pivotHeight, barColor);
        Mesh* barMesh   = Meshes::CreateCylinder(name + "_bar",  glm::vec3(0), barRadius, barRadius, barLen, barColor);
        Mesh* gripMesh  = Meshes::CreateCylinder(name + "_grip", glm::vec3(0), gripRadius, gripRadius, gripLen, gripColor);
        Mesh* wheelMesh = Meshes::CreateCylinder(name + "_wheel", glm::vec3(0), wheelRadius, wheelRadius, wheelThickness, wheelColor);

        float baseOffsetY = wheelRadius * 2.0f;
        comp.AddPart(baseMesh, glm::translate(glm::mat4(1.0f), glm::vec3(-baseWidth * 0.5f, baseOffsetY, -baseLen * 0.5f)));

        // Post centered on base
        comp.AddPart(postMesh, glm::translate(glm::mat4(1.0f), glm::vec3(-postWidth * 0.5f, baseOffsetY + baseHeight, -postDepth * 0.5f)));

        // Pivot cylinder on top of post
        float pivotY = baseOffsetY + baseHeight + postHeight;
        comp.AddPart(pivotMesh, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, pivotY, 0.0f)));

        // Bar: horizontal along X, centered above post
        float barY = pivotY + pivotHeight;
        glm::mat4 barTransform = glm::mat4(1.0f);
        barTransform = glm::translate(barTransform, glm::vec3(0.0f, barY, 0.0f));
        barTransform = glm::rotate(barTransform, RADIANS(90.0f), glm::vec3(0, 0, 1));
        barTransform = glm::rotate(barTransform, RADIANS(90.0f), glm::vec3(1, 0, 0));
        barTransform = glm::translate(barTransform, glm::vec3(0.0f, -barLen * 0.5f, 0.0f));
        comp.AddPart(barMesh, barTransform);

        // Grips at both ends of bar
        float gripCenterOffset = barLen * 0.55f;
        glm::mat4 gripL = glm::mat4(1.0f);
        gripL = glm::translate(gripL, glm::vec3(gripLen / 2, barY, -gripRadius - barLen / 2 ));
        gripL = glm::rotate(gripL, RADIANS(90.0f), glm::vec3(0, 0, 1));
        comp.AddPart(gripMesh, gripL);

        glm::mat4 gripR = glm::mat4(1.0f);
        gripR = glm::translate(gripR, glm::vec3(gripLen / 2, barY, -gripRadius + barLen / 2 + 0.15));
        gripR = glm::rotate(gripR, RADIANS(90.0f), glm::vec3(0, 0, 1));
        comp.AddPart(gripMesh, gripR);

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
        comp.AddPart(wheelMesh, wheelTransform(wheelXLeft, wheelZFront, 1.0f));
        comp.AddPart(wheelMesh, wheelTransform(wheelXRight, wheelZFront, -1.0f));
        comp.AddPart(wheelMesh, wheelTransform(wheelXLeft, wheelZBack, 1.0f));
        comp.AddPart(wheelMesh, wheelTransform(wheelXRight, wheelZBack, -1.0f));

        return comp;
    }

    m1::Component CreateStandardRail(const std::string& name)
    {
        m1::Component comp;

        // Rail segment parameters
        float railLen = RailConfig::SegmentLen;
        float gauge = RailConfig::Gauge;
        float railThickX = 0.05f;
        float railHeight = 0.05f;
        float tieLen = gauge + 0.25f;
        float tieHeight = 0.03f;
        float tieWidth = 0.10f;
        float tieSpacing = 0.22f;

        glm::vec3 railColor(0.6f, 0.6f, 0.6f);
        glm::vec3 tieColor(0.35f, 0.2f, 0.1f);

        // Shared meshes for parts
        Mesh* railMesh = Meshes::CreateParallelepiped(name + "_rail", glm::vec3(0), railThickX, railHeight, railLen, railColor);
        Mesh* tieMesh = Meshes::CreateParallelepiped(name + "_tie", glm::vec3(0), tieLen, tieHeight, tieWidth, tieColor);

        // Rails offset on X, centered on Z
        comp.AddPart(
            railMesh,
            glm::translate(glm::mat4(1.0f), glm::vec3(-gauge * 0.5f - railThickX * 0.5f, 0.0f, -railLen * 0.5f)));
        comp.AddPart(
            railMesh,
            glm::translate(glm::mat4(1.0f), glm::vec3(gauge * 0.5f - railThickX * 0.5f, 0.0f, -railLen * 0.5f)));

        // Sleepers only in the interior, leave margins at both ends
        float endMargin = 0.12f;
        float startCenterZ = -railLen * 0.5f + endMargin + tieWidth * 0.5f;
        float endCenterZ = railLen * 0.5f - endMargin - tieWidth * 0.5f;
        for (float centerZ = startCenterZ; centerZ <= endCenterZ + 1e-4f; centerZ += tieSpacing)
        {
            comp.AddPart(
                tieMesh,
                glm::translate(glm::mat4(1.0f), glm::vec3(-tieLen * 0.5f, 0.0f, centerZ - tieWidth * 0.5f)));
        }

        return comp;
    }

    m1::Component CreateCornerRail(bool rightTurn, const std::string &baseName)
    {
        m1::Component comp;

        // Geometry parameters sized to fit within a single terrain cell
        float gauge       = RailConfig::Gauge;
        float railThickX  = 0.04f;
        float railHeight  = 0.05f;
        float tieLen      = gauge + 0.05f;
        float tieHeight   = 0.03f;
        float tileAngle   = glm::half_pi<float>();
        int segments      = 10;
        float radius      = RailConfig::SegmentLen * 0.5f;
        float angleStep   = tileAngle / segments;

        float railLenSeg  = radius * std::abs(angleStep);
        float tieWidth    = railLenSeg * 1.15f;

        glm::vec3 railColor(0.6f, 0.6f, 0.6f);
        glm::vec3 tieColor(0.35f, 0.2f, 0.1f);

        Mesh* railMesh = Meshes::CreateParallelepiped(baseName + "_rail", glm::vec3(0), railThickX, railHeight, railLenSeg, railColor);
        Mesh* tieMesh  = Meshes::CreateParallelepiped(baseName + "_tie",  glm::vec3(0), tieLen,    tieHeight, tieWidth,  tieColor);

        auto addSegment = [&](float angle)
        {
            float sign = angleStep > 0 ? 1.0f : -1.0f;
            float ct = std::cos(angle);
            float st = std::sin(angle);

            glm::vec3 center = rightTurn ? glm::vec3( radius, 0.0f, -radius)
                                          : glm::vec3(-radius, 0.0f, -radius);
            glm::vec3 posCenter = center + glm::vec3(radius * ct, 0.0f, radius * st);

            glm::vec3 dir = glm::normalize(glm::vec3(-st * sign, 0.0f, ct * sign));
            glm::vec3 normal = glm::vec3(-dir.z, 0.0f, dir.x);

            // Two rails offset by gauge/2 along normal
            float offset = gauge * 0.5f;
            glm::vec3 leftPos  = posCenter - normal * offset;
            glm::vec3 rightPos = posCenter + normal * offset;

            float yaw = std::atan2(dir.x, dir.z);

            auto addRail = [&](const glm::vec3& p)
            {
                // Position rail centered on segment along tangent direction
                glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
                m = glm::rotate(m, yaw, glm::vec3(0, 1, 0));
                m = glm::translate(m, glm::vec3(-railThickX * 0.5f, 0.0f, -railLenSeg * 0.5f));
                comp.AddPart(railMesh, m);
            };

            // Sleepers centered under segment
            glm::mat4 tieM = glm::translate(glm::mat4(1.0f), posCenter);
            tieM = glm::rotate(tieM, yaw, glm::vec3(0,1,0));
            tieM = glm::translate(tieM, glm::vec3(-tieLen * 0.5f, -tieHeight, -tieWidth * 0.5f));
            comp.AddPart(tieMesh, tieM);

            addRail(leftPos);
            addRail(rightPos);
        };

        for (int i = 0; i < segments; ++i)
        {
            float angleStart = rightTurn ? glm::pi<float>() : 0.0f;
            float aMid = angleStart + (i + 0.5f) * (rightTurn ? -angleStep : angleStep);
            addSegment(aMid);
        }

        return comp;
    }

    m1::Component CreateCornerRailLeft(const std::string &name)
    {
        return CreateCornerRail(false, name);
    }

    m1::Component CreateCornerRailRight(const std::string &name)
    {
        return CreateCornerRail(true, name);
    }

    m1::Component CreateMountainRail(const std::string &name)
    {
        m1::Component comp;

        float railLen     = RailConfig::SegmentLen + 0.1f;
        float tieLen      = RailConfig::Gauge;
        float tieHeight   = 0.06f;
        float tieWidth = 0.25f;
        float tieSpacing  = tieWidth + 0.05f;
        float endMargin   = 0.05f;

        glm::vec3 tieColorBright(0.65f, 0.65f, 0.7f);
        glm::vec3 tieColorDark  (0.40f, 0.40f, 0.45f);

        Mesh* tieBright = Meshes::CreateParallelepiped(name + "_bright", glm::vec3(0), tieLen, tieHeight, tieWidth, tieColorBright);
        Mesh* tieDark   = Meshes::CreateParallelepiped(name + "_dark",   glm::vec3(0), tieLen, tieHeight, tieWidth, tieColorDark);

        float startCenterZ = -railLen * 0.5f + endMargin + tieWidth * 0.5f;
        float endCenterZ   =  railLen * 0.5f - endMargin - tieWidth * 0.5f;

        int idx = 0;
        for (float centerZ = startCenterZ; centerZ <= endCenterZ + 1e-4f; centerZ += tieSpacing, ++idx)
        {
            Mesh* m = (idx % 2 == 0) ? tieBright : tieDark;
            glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(-tieLen * 0.5f, 0.0f, centerZ - tieWidth * 0.5f));
            comp.AddPart(m, t);
        }

        return comp;
    }

    m1::Component CreateSeaRail(const std::string &name)
    {
        m1::Component comp;

        float railLen    = RailConfig::SegmentLen - 0.1;
        float boardHeight= 0.05f;
        float boardLenZ  = 0.70f;
        float gapZ       = 0.05f;
        float endMargin  = 0.05f;

        float totalWidthX = RailConfig::Gauge + 0.10f;
        float xGap        = 0.02f;  // small gap between planks in the same rail
        float boardWidthX = (totalWidthX - 3.0f * xGap) / 4.0f;

        glm::vec3 c0(0.36f, 0.20f, 0.08f);
        glm::vec3 c1(0.48f, 0.26f, 0.12f);
        glm::vec3 palette[2] = { c0, c1 };

        Mesh* boardMeshes[2];
        for (int i = 0; i < 2; ++i)
        {
            boardMeshes[i] = Meshes::CreateParallelepiped(name + "_board" + std::to_string(i), glm::vec3(0), boardWidthX, boardHeight, railLen, palette[i]);
        }

        float startCenterZ = -railLen * 0.5f + endMargin + boardLenZ * 0.5f;
        float endCenterZ   =  railLen * 0.5f - endMargin - boardLenZ * 0.5f;

        for (float centerZ = startCenterZ; centerZ <= endCenterZ + 1e-4f; centerZ += boardLenZ + gapZ)
        {
            float xStart = -totalWidthX * 0.5f;
            for (int side = 0; side < 4; ++side)
            {
                float x = xStart + side * (boardWidthX + xGap);
                Mesh* m = boardMeshes[side % 2];
                glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, centerZ - boardLenZ * 0.5f));
                comp.AddPart(m, t);
            }
        }

        return comp;
    }

    m1::Component CreateStation(const std::string& name,
        const glm::vec3& color,
        float length,
        float height,
        float width)
    {
        m1::Component comp;

        glm::vec3 baseColor = color;
        glm::vec3 capColor = glm::clamp(color * glm::vec3(0.85f) + glm::vec3(0.08f), 0.0f, 1.0f);
        glm::vec3 roofColor = glm::clamp(color * glm::vec3(0.65f) + glm::vec3(0.15f), 0.0f, 1.0f);

        Mesh* platformBase = Meshes::CreateParallelepiped(name + "_base", glm::vec3(0), width, height, length, baseColor);
        Mesh* platformCap = Meshes::CreateParallelepiped(name + "_cap", glm::vec3(0), width, height * 0.35f, length, capColor);

        // Taller pillars
        float pillarSizeX = width * 0.18f;
        float pillarSizeZ = width * 0.18f;
        float pillarHeight = height * 2.8f;
        Mesh* pillarMesh = Meshes::CreateParallelepiped(name + "_pillar", glm::vec3(0), pillarSizeX, pillarHeight, pillarSizeZ, capColor);

        // Roof
        float roofThickness = height * 0.30f;
        float roofOverX = width * 0.18f;
        float roofOverZ = length * 0.06f;
        float roofWidth = width + roofOverX * 2.0f;
        float roofLen = length + roofOverZ * 2.0f;
        Mesh* roofMesh = Meshes::CreateParallelepiped(name + "_roof", glm::vec3(0), roofWidth, roofThickness, roofLen, roofColor);

        float sideOffset = RailConfig::Gauge * 0.5f + width * 0.5f + 0.5f;

        auto addPlatform = [&](float sign)
            {
                glm::mat4 t = glm::translate(glm::mat4(1.0f),
                    glm::vec3(sideOffset * sign - width * 0.5f, 0.0f, -length * 0.5f));

                // Deck + cap
                comp.AddPart(platformBase, t);
                comp.AddPart(platformCap, glm::translate(t, glm::vec3(0.0f, height, 0.0f)));

                // Pillars
                float pillarX = width * 0.5f - pillarSizeX * 0.5f;
                float pillarZ1 = length * 0.25f - pillarSizeZ * 0.5f;
                float pillarZ2 = length * 0.75f - pillarSizeZ * 0.5f;
                comp.AddPart(pillarMesh, glm::translate(t, glm::vec3(pillarX, height, pillarZ1)));
                comp.AddPart(pillarMesh, glm::translate(t, glm::vec3(pillarX, height, pillarZ2)));

                // Roof above taller pillars
                glm::mat4 roofT = glm::translate(t, glm::vec3(-roofOverX, height + pillarHeight, -roofOverZ));
                comp.AddPart(roofMesh, roofT);
            };

        addPlatform(-1.0f);
        addPlatform(1.0f);

        return comp;
    }
}
