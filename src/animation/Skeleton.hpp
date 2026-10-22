/**
 * @file Skeleton.hpp
 * @brief Bone hierarchy, joints, and bind pose matrix definitions for skeletal animation.
 */

#pragma once

#include "core/Math.hpp"
#include "core/Types.hpp"
#include <string>
#include <vector>

namespace Engine {

    /**
     * @struct Bone
     * @brief A single joint in a skeletal hierarchy.
     */
    struct Bone {
        std::string Name;
        int32 Index{0};
        int32 ParentIndex{-1};
        glm::mat4 InverseBindPose{1.0f};
        Transform LocalTransform;
    };

    /**
     * @class Skeleton
     * @brief Collection of hierarchical bones forming an articulable skeletal structure.
     */
    class Skeleton {
    public:
        Skeleton() = default;

        /**
         * @brief Adds a bone to the skeleton.
         * @param name Unique bone name.
         * @param parentIndex Parent bone index or -1 for root.
         * @param localTransform Initial rest pose transform.
         * @return Index of added bone.
         */
        int32 AddBone(std::string name, int32 parentIndex, const Transform& localTransform) {
            Bone bone;
            bone.Name = std::move(name);
            bone.Index = static_cast<int32>(m_Bones.size());
            bone.ParentIndex = parentIndex;
            bone.LocalTransform = localTransform;
            bone.InverseBindPose = glm::inverse(localTransform.GetMatrix());

            m_Bones.push_back(bone);
            m_GlobalMatrices.push_back(glm::mat4(1.0f));
            return bone.Index;
        }

        [[nodiscard]] const std::vector<Bone>& GetBones() const noexcept { return m_Bones; }
        [[nodiscard]] std::vector<Bone>& GetBones() noexcept { return m_Bones; }
        [[nodiscard]] const std::vector<glm::mat4>& GetGlobalMatrices() const noexcept { return m_GlobalMatrices; }

        /**
         * @brief Recalculates hierarchical global matrices from bone local transforms.
         */
        void UpdateHierarchy() {
            for (size_t i = 0; i < m_Bones.size(); ++i) {
                glm::mat4 localMat = m_Bones[i].LocalTransform.GetMatrix();
                if (m_Bones[i].ParentIndex >= 0 && m_Bones[i].ParentIndex < static_cast<int32>(i)) {
                    m_GlobalMatrices[i] = m_GlobalMatrices[m_Bones[i].ParentIndex] * localMat;
                } else {
                    m_GlobalMatrices[i] = localMat;
                }
            }
        }

    private:
        std::vector<Bone> m_Bones;
        std::vector<glm::mat4> m_GlobalMatrices;
    };

} // namespace Engine
