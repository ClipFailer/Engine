/**
 * @file PhysicsWorld.cpp
 * @brief Implementation of physics simulation step, collision resolution, and raycasting.
 */

#include "PhysicsWorld.hpp"
#include "scene/Scene.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "scene/StaticMeshComponent.hpp"
#include <glm/gtx/intersect.hpp>

namespace Engine {

    PhysicsWorld::PhysicsWorld() = default;

    void PhysicsWorld::Step(Scene& scene, float deltaTime) {
        if (deltaTime <= 0.0f) return;

        // Sub-stepping for numerical stability
        const int subSteps = 2;
        const float dt = deltaTime / static_cast<float>(subSteps);

        for (int step = 0; step < subSteps; ++step) {
            const auto& actors = scene.GetActors();

            // 1. Integration (Gravity & Velocity)
            for (const auto& actor : actors) {
                auto rb = actor->GetComponent<RigidBodyComponent>();
                if (!rb || rb->bIsStatic) continue;

                if (rb->bUseGravity) {
                    rb->LinearVelocity += Gravity * dt;
                }

                actor->Transform.Position += rb->LinearVelocity * dt;
                actor->Transform.Rotation += rb->AngularVelocity * dt;
            }

            // 2. Collision detection & resolution
            for (size_t i = 0; i < actors.size(); ++i) {
                auto rbA = actors[i]->GetComponent<RigidBodyComponent>();
                if (!rbA) continue;

                // Collision with floor plane (Y = 0)
                if (!rbA->bIsStatic) {
                    float bottomY = actors[i]->Transform.Position.y;
                    float radiusOrHalfH = (rbA->Shape == ColliderType::Sphere) 
                                          ? rbA->ColliderDimensions.x 
                                          : rbA->ColliderDimensions.y;

                    if (bottomY - radiusOrHalfH < 0.0f) {
                        float penetration = -(bottomY - radiusOrHalfH);
                        actors[i]->Transform.Position.y += penetration;

                        if (rbA->LinearVelocity.y < 0.0f) {
                            rbA->LinearVelocity.y = -rbA->LinearVelocity.y * rbA->Restitution;
                            rbA->LinearVelocity.x *= (1.0f - rbA->Friction);
                            rbA->LinearVelocity.z *= (1.0f - rbA->Friction);

                            // Sleeping threshold
                            if (std::abs(rbA->LinearVelocity.y) < 0.15f) {
                                rbA->LinearVelocity.y = 0.0f;
                            }
                        }
                    }
                }

                // Actor vs Actor collisions
                for (size_t j = i + 1; j < actors.size(); ++j) {
                    auto rbB = actors[j]->GetComponent<RigidBodyComponent>();
                    if (!rbB) continue;
                    if (rbA->bIsStatic && rbB->bIsStatic) continue;

                    // Sphere vs Sphere
                    if (rbA->Shape == ColliderType::Sphere && rbB->Shape == ColliderType::Sphere) {
                        float rA = rbA->ColliderDimensions.x;
                        float rB = rbB->ColliderDimensions.x;
                        glm::vec3 delta = actors[i]->Transform.Position - actors[j]->Transform.Position;
                        float dist = glm::length(delta);
                        float minDist = rA + rB;

                        if (dist < minDist && dist > 0.0001f) {
                            glm::vec3 normal = delta / dist;
                            float penetration = minDist - dist;

                            // Positional correction
                            if (!rbA->bIsStatic && !rbB->bIsStatic) {
                                actors[i]->Transform.Position += normal * (penetration * 0.5f);
                                actors[j]->Transform.Position -= normal * (penetration * 0.5f);
                            } else if (!rbA->bIsStatic) {
                                actors[i]->Transform.Position += normal * penetration;
                            } else if (!rbB->bIsStatic) {
                                actors[j]->Transform.Position -= normal * penetration;
                            }

                            // Impulse resolution
                            glm::vec3 relVel = rbA->LinearVelocity - rbB->LinearVelocity;
                            float velAlongNormal = glm::dot(relVel, normal);

                            if (velAlongNormal < 0.0f) {
                                float e = std::min(rbA->Restitution, rbB->Restitution);
                                float invMassA = rbA->bIsStatic ? 0.0f : (1.0f / rbA->Mass);
                                float invMassB = rbB->bIsStatic ? 0.0f : (1.0f / rbB->Mass);
                                float jImpulse = -(1.0f + e) * velAlongNormal / (invMassA + invMassB);
                                glm::vec3 impulse = normal * jImpulse;

                                if (!rbA->bIsStatic) rbA->LinearVelocity += impulse * invMassA;
                                if (!rbB->bIsStatic) rbB->LinearVelocity -= impulse * invMassB;
                            }
                        }
                    }
                }
            }
        }
    }

    RaycastHit PhysicsWorld::Raycast(const Scene& scene,
                                     const glm::vec3& rayOrigin,
                                     const glm::vec3& rayDir,
                                     float maxDistance) const {
        RaycastHit hit;
        hit.bHit = false;
        hit.Distance = maxDistance;

        for (const auto& actor : scene.GetActors()) {
            if (!actor->bIsVisible) continue;

            glm::vec3 center = actor->Transform.Position;
            float radius = std::max({actor->Transform.Scale.x, actor->Transform.Scale.y, actor->Transform.Scale.z}) * 0.5f;

            // Ray vs Sphere intersection test
            glm::vec3 oc = rayOrigin - center;
            float a = glm::dot(rayDir, rayDir);
            float b = 2.0f * glm::dot(oc, rayDir);
            float c = glm::dot(oc, oc) - radius * radius;
            float discriminant = b * b - 4.0f * a * c;

            if (discriminant >= 0.0f) {
                float dist = (-b - std::sqrt(discriminant)) / (2.0f * a);
                if (dist > 0.0f && dist < hit.Distance) {
                    hit.bHit = true;
                    hit.Distance = dist;
                    hit.HitActor = actor.get();
                    hit.Point = rayOrigin + rayDir * dist;
                    hit.Normal = glm::normalize(hit.Point - center);
                }
            }
        }

        return hit;
    }

} // namespace Engine
