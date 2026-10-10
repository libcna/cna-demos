// SPDX-License-Identifier: MS-PL
#pragma once

// Simple meshes for 3D demos, like the sphere and cube "primitives" XNA's own samples carried.
// Demos about effects, lighting or cameras need geometry with normals and texture coordinates
// but are not about building it. Triangles are clockwise seen from outside -- XNA's front faces
// under the default RasterizerState::CullCounterClockwise.

#include <cmath>
#include <cstdint>
#include <vector>

#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CnaDemos
{
    /** Vertices plus 16-bit triangle-list indices, ready for DrawUserIndexedPrimitives. */
    struct Mesh
    {
        std::vector<Microsoft::Xna::Framework::Graphics::VertexPositionNormalTexture> vertices;
        std::vector<std::uint16_t> indices;

        [[nodiscard]] int VertexCount() const { return static_cast<int>(vertices.size()); }
        [[nodiscard]] int PrimitiveCount() const { return static_cast<int>(indices.size() / 3); }
    };

    /** A UV sphere of the given radius: rings from pole to pole, segments around. */
    [[nodiscard]] inline Mesh MakeSphere(float radius, int rings = 24, int segments = 48)
    {
        using Microsoft::Xna::Framework::MathHelper;
        using Microsoft::Xna::Framework::Vector2;
        using Microsoft::Xna::Framework::Vector3;
        Mesh mesh;
        for (int i = 0; i <= rings; ++i)
        {
            const float theta = MathHelper::Pi * static_cast<float>(i) / static_cast<float>(rings);
            for (int j = 0; j <= segments; ++j)
            {
                const float phi = MathHelper::TwoPi * static_cast<float>(j) / static_cast<float>(segments);
                const Vector3 normal(std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi));
                mesh.vertices.emplace_back(normal * radius, normal,
                                           Vector2(static_cast<float>(j) / static_cast<float>(segments),
                                                   static_cast<float>(i) / static_cast<float>(rings)));
            }
        }
        const auto at = [segments](int ring, int segment) {
            return static_cast<std::uint16_t>(ring * (segments + 1) + segment);
        };
        for (int i = 0; i < rings; ++i)
        {
            for (int j = 0; j < segments; ++j)
            {
                const std::uint16_t a = at(i, j), b = at(i, j + 1), c = at(i + 1, j), d = at(i + 1, j + 1);
                mesh.indices.insert(mesh.indices.end(), {a, c, d, a, d, b});
            }
        }
        return mesh;
    }

    /** An axis-aligned cube with the given half size; each face has its own normals and 0..1 UVs. */
    [[nodiscard]] inline Mesh MakeCube(float half)
    {
        using Microsoft::Xna::Framework::Vector2;
        using Microsoft::Xna::Framework::Vector3;
        struct Face
        {
            Vector3 normal;
            Vector3 up;
        };
        const Face faces[6] = {{Vector3::UnitZ, Vector3::UnitY},  {-Vector3::UnitZ, Vector3::UnitY},
                               {Vector3::UnitX, Vector3::UnitY},  {-Vector3::UnitX, Vector3::UnitY},
                               {Vector3::UnitY, -Vector3::UnitZ}, {-Vector3::UnitY, Vector3::UnitZ}};
        Mesh mesh;
        for (const Face& face : faces)
        {
            // Seen from outside (looking along -normal), screen right is (-normal) x up.
            const Vector3 right = Vector3::Cross(-face.normal, face.up);
            const auto base = static_cast<std::uint16_t>(mesh.vertices.size());
            const Vector3 n = face.normal * half, r = right * half, u = face.up * half;
            mesh.vertices.emplace_back(n - r + u, face.normal, Vector2(0.0f, 0.0f));  // top left
            mesh.vertices.emplace_back(n + r + u, face.normal, Vector2(1.0f, 0.0f));  // top right
            mesh.vertices.emplace_back(n - r - u, face.normal, Vector2(0.0f, 1.0f));  // bottom left
            mesh.vertices.emplace_back(n + r - u, face.normal, Vector2(1.0f, 1.0f));  // bottom right
            mesh.indices.insert(mesh.indices.end(),
                                {base, static_cast<std::uint16_t>(base + 1), static_cast<std::uint16_t>(base + 3), base,
                                 static_cast<std::uint16_t>(base + 3), static_cast<std::uint16_t>(base + 2)});
        }
        return mesh;
    }
}
