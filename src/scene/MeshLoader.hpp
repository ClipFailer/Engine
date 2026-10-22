/**
 * @file MeshLoader.hpp
 * @brief 3D Model file loader supporting Wavefront OBJ files.
 */

#pragma once

#include "Primitives.hpp"
#include <string>

namespace Engine {

    /**
     * @class MeshLoader
     * @brief Parses and imports 3D model formats into Engine MeshGeometry.
     */
    class MeshLoader {
    public:
        /**
         * @brief Loads an OBJ 3D model file from disk.
         * @param filepath Absolute or relative path to the .obj file.
         * @param outGeometry Destination mesh geometry.
         * @return True if model loaded successfully with valid vertices.
         */
        static bool LoadOBJ(const std::string& filepath, MeshGeometry& outGeometry);
    };

} // namespace Engine
