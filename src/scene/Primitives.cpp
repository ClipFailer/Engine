/**
 * @file Primitives.cpp
 * @brief Implementation of procedural primitive generators.
 */

#include "Primitives.hpp"
#include <numbers>

namespace Engine {

    MeshGeometry Primitives::CreateCube(float size) {
        float h = size * 0.5f;
        MeshGeometry mesh;

        mesh.Vertices = {
            // Front face (+Z)
            {{-h, -h,  h}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ h, -h,  h}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ h,  h,  h}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-h,  h,  h}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

            // Back face (-Z)
            {{ h, -h, -h}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{-h, -h, -h}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{-h,  h, -h}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{ h,  h, -h}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

            // Top face (+Y)
            {{-h,  h,  h}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ h,  h,  h}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ h,  h, -h}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-h,  h, -h}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

            // Bottom face (-Y)
            {{-h, -h, -h}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ h, -h, -h}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ h, -h,  h}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-h, -h,  h}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

            // Right face (+X)
            {{ h, -h,  h}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ h, -h, -h}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ h,  h, -h}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{ h,  h,  h}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            // Left face (-X)
            {{-h, -h, -h}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-h, -h,  h}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-h,  h,  h}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-h,  h, -h}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}
        };

        mesh.Indices = {
             0,  1,  2,  2,  3,  0, // Front
             4,  5,  6,  6,  7,  4, // Back
             8,  9, 10, 10, 11,  8, // Top
            12, 13, 14, 14, 15, 12, // Bottom
            16, 17, 18, 18, 19, 16, // Right
            20, 21, 22, 22, 23, 20  // Left
        };

        return mesh;
    }

    MeshGeometry Primitives::CreateSphere(float radius, uint32 rings, uint32 sectors) {
        MeshGeometry mesh;
        const float R = 1.0f / static_cast<float>(rings - 1);
        const float S = 1.0f / static_cast<float>(sectors - 1);
        constexpr float pi = std::numbers::pi_v<float>;

        mesh.Vertices.reserve(rings * sectors);
        mesh.Indices.reserve(rings * sectors * 6);

        for (uint32 r = 0; r < rings; ++r) {
            for (uint32 s = 0; s < sectors; ++s) {
                float y = std::sin(-pi * 0.5f + pi * r * R);
                float x = std::cos(2.0f * pi * s * S) * std::sin(pi * r * R);
                float z = std::sin(2.0f * pi * s * S) * std::sin(pi * r * R);

                Vertex v;
                v.Position = glm::vec3(x, y, z) * radius;
                v.Normal = glm::normalize(glm::vec3(x, y, z));
                v.TexCoord = glm::vec2(s * S, r * R);
                mesh.Vertices.push_back(v);
            }
        }

        for (uint32 r = 0; r < rings - 1; ++r) {
            for (uint32 s = 0; s < sectors - 1; ++s) {
                uint32 cur = r * sectors + s;
                uint32 next = (r + 1) * sectors + s;

                mesh.Indices.push_back(cur);
                mesh.Indices.push_back(next);
                mesh.Indices.push_back(next + 1);

                mesh.Indices.push_back(cur);
                mesh.Indices.push_back(next + 1);
                mesh.Indices.push_back(cur + 1);
            }
        }

        return mesh;
    }

    MeshGeometry Primitives::CreatePlane(float size) {
        float h = size * 0.5f;
        MeshGeometry mesh;

        mesh.Vertices = {
            {{-h, 0.0f,  h}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ h, 0.0f,  h}, {0.0f, 1.0f, 0.0f}, {size, 0.0f}},
            {{ h, 0.0f, -h}, {0.0f, 1.0f, 0.0f}, {size, size}},
            {{-h, 0.0f, -h}, {0.0f, 1.0f, 0.0f}, {0.0f, size}}
        };

        mesh.Indices = {
            0, 1, 2, 2, 3, 0
        };

        return mesh;
    }

} // namespace Engine
