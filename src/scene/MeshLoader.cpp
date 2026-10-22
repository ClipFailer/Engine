/**
 * @file MeshLoader.cpp
 * @brief Implementation of Wavefront OBJ 3D model parser.
 */

#include "MeshLoader.hpp"
#include "core/Log.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cmath>

namespace Engine {

    struct OBJIndex {
        int v{0};
        int vt{0};
        int vn{0};

        bool operator==(const OBJIndex& o) const {
            return v == o.v && vt == o.vt && vn == o.vn;
        }
    };

    struct OBJIndexHash {
        std::size_t operator()(const OBJIndex& idx) const noexcept {
            return std::hash<int>()(idx.v) ^ (std::hash<int>()(idx.vt) << 1) ^ (std::hash<int>()(idx.vn) << 2);
        }
    };

    static OBJIndex ParseOBJIndex(const std::string& token) {
        OBJIndex idx;
        std::stringstream ss(token);
        std::string part;

        if (std::getline(ss, part, '/')) {
            if (!part.empty()) idx.v = std::stoi(part);
        }
        if (std::getline(ss, part, '/')) {
            if (!part.empty()) idx.vt = std::stoi(part);
        }
        if (std::getline(ss, part, '/')) {
            if (!part.empty()) idx.vn = std::stoi(part);
        }
        return idx;
    }

    bool MeshLoader::LoadOBJ(const std::string& filepath, MeshGeometry& outGeometry) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("MeshLoader: Failed to open OBJ file '{}'", filepath);
            return false;
        }

        std::vector<glm::vec3> tempPositions;
        std::vector<glm::vec2> tempTexCoords;
        std::vector<glm::vec3> tempNormals;

        std::vector<Vertex> uniqueVertices;
        std::vector<uint32> indices;
        std::unordered_map<OBJIndex, uint32, OBJIndexHash> indexMap;

        std::string line;
        while (std::getline(file, line)) {
            // Trim leading whitespace
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) continue;
            line = line.substr(first);

            if (line.starts_with("#")) continue;

            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                glm::vec3 pos;
                ss >> pos.x >> pos.y >> pos.z;
                tempPositions.push_back(pos);
            } else if (prefix == "vt") {
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                tempTexCoords.push_back(uv);
            } else if (prefix == "vn") {
                glm::vec3 norm;
                ss >> norm.x >> norm.y >> norm.z;
                tempNormals.push_back(glm::normalize(norm));
            } else if (prefix == "f") {
                std::vector<OBJIndex> faceIndices;
                std::string token;
                while (ss >> token) {
                    faceIndices.push_back(ParseOBJIndex(token));
                }

                if (faceIndices.size() < 3) continue;

                // Triangulate face using triangle fan
                for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                    OBJIndex tri[3] = { faceIndices[0], faceIndices[i], faceIndices[i + 1] };

                    for (int k = 0; k < 3; ++k) {
                        OBJIndex idx = tri[k];

                        // Convert 1-based (and negative) OBJ indices
                        if (idx.v < 0) idx.v = static_cast<int>(tempPositions.size()) + idx.v + 1;
                        if (idx.vt < 0) idx.vt = static_cast<int>(tempTexCoords.size()) + idx.vt + 1;
                        if (idx.vn < 0) idx.vn = static_cast<int>(tempNormals.size()) + idx.vn + 1;

                        auto it = indexMap.find(idx);
                        if (it != indexMap.end()) {
                            indices.push_back(it->second);
                        } else {
                            Vertex vert{};
                            if (idx.v > 0 && idx.v <= static_cast<int>(tempPositions.size())) {
                                vert.Position = tempPositions[idx.v - 1];
                            }
                            if (idx.vt > 0 && idx.vt <= static_cast<int>(tempTexCoords.size())) {
                                vert.TexCoord = tempTexCoords[idx.vt - 1];
                            } else {
                                vert.TexCoord = glm::vec2(vert.Position.x, vert.Position.z);
                            }
                            if (idx.vn > 0 && idx.vn <= static_cast<int>(tempNormals.size())) {
                                vert.Normal = tempNormals[idx.vn - 1];
                            } else {
                                vert.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                            }

                            uint32 newIndex = static_cast<uint32>(uniqueVertices.size());
                            uniqueVertices.push_back(vert);
                            indexMap[idx] = newIndex;
                            indices.push_back(newIndex);
                        }
                    }
                }
            }
        }

        if (uniqueVertices.empty()) {
            LOG_WARN("MeshLoader: OBJ file '{}' contained no valid geometry", filepath);
            return false;
        }

        // If normals were not specified in OBJ, calculate flat normals per triangle
        if (tempNormals.empty() && indices.size() >= 3) {
            for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                Vertex& v0 = uniqueVertices[indices[i]];
                Vertex& v1 = uniqueVertices[indices[i + 1]];
                Vertex& v2 = uniqueVertices[indices[i + 2]];

                glm::vec3 normal = glm::normalize(glm::cross(v1.Position - v0.Position, v2.Position - v0.Position));
                v0.Normal = normal;
                v1.Normal = normal;
                v2.Normal = normal;
            }
        }

        outGeometry.Vertices = std::move(uniqueVertices);
        outGeometry.Indices = std::move(indices);

        LOG_INFO("MeshLoader: Successfully loaded OBJ '{}' ({} vertices, {} triangles)",
                 filepath, outGeometry.Vertices.size(), outGeometry.Indices.size() / 3);
        return true;
    }

} // namespace Engine
