#pragma once

#include <vector>
#include <functional>

#include "core/gpu/mesh.h"
#include "core/gpu/shader.h"
#include "utils/glm_utils.h"

namespace m1
{
    class Component
    {
     public:
        struct Part
        {
            Mesh* mesh = nullptr;
            Shader* shader = nullptr;        // Optional override
            glm::mat4 localModel = glm::mat4(1.0f); // Relative to component origin
        };

        void AddPart(Mesh* mesh, const glm::mat4& localModel, Shader* shader = nullptr)
        {
            parts.push_back({ mesh, shader, localModel });
        }

        void Clear()
        {
            parts.clear();
        }

        // renderFn must call RenderMesh
        void Render(const glm::mat4& worldModel,
                    Shader* defaultShader,
                    const std::function<void(Mesh*, Shader*, const glm::mat4&)>& renderFn) const
        {
            for (const auto& part : parts)
            {
                if (!part.mesh) continue;
                Shader* useShader = part.shader ? part.shader : defaultShader;
                renderFn(part.mesh, useShader, worldModel * part.localModel);
            }
        }

     private:
        std::vector<Part> parts;
    };
}
