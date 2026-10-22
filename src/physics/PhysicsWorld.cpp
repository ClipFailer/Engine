/**
 * @file PhysicsWorld.cpp
 * @brief Implementation of physics simulation step, collision resolution, and raycasting.
 */

#include "PhysicsWorld.hpp"
#include "scene/Scene.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "scene/StaticMeshComponent.hpp"
#include <glm/gtx/intersect.hpp>

namespace Engine {

    PhysicsWorld::PhysicsWorld() = default;

    struct PhysicsBodyAdapter {
        bool bValid{false};
        bool bIsStatic{false};
        bool bUseGravity{true};
        float Mass{1.0f};
        glm::vec3& LinearVelocity;
        glm::vec3& AngularVelocity;
        float Restitution{0.6f};
        float Friction{0.2f};
        bool bIsSphere{false};
        float RadiusOrHalfH{0.5f};
    };

    static std::optional<PhysicsBodyAdapter> GetBodyAdapter(Actor* actor) {
        if (!actor) return std::nullopt;

        if (auto phys = actor->GetComponent<PhysicsComponent>()) {
            auto coll = actor->GetComponent<CollisionComponent>();
            bool isSphere = coll ? (coll->Shape == CollisionShape::Sphere) : false;
            float dim = coll ? (isSphere ? coll->SphereRadius : coll->BoxHalfExtents.y) : 0.5f;
            float rest = coll ? coll->Restitution : 0.6f;
            float fric = coll ? coll->Friction : 0.2f;

            return PhysicsBodyAdapter{
                true,
                phys->bIsStatic,
                phys->bUseGravity,
                phys->Mass,
                phys->LinearVelocity,
                phys->AngularVelocity,
                rest,
                fric,
                isSphere,
                dim
            };
        } else if (auto rb = actor->GetComponent<RigidBodyComponent>()) {
            bool isSphere = (rb->Shape == ColliderType::Sphere);
            float dim = isSphere ? rb->ColliderDimensions.x : rb->ColliderDimensions.y;
            return PhysicsBodyAdapter{
                true,
                rb->bIsStatic,
                rb->bUseGravity,
                rb->Mass,
                rb->LinearVelocity,
                rb->AngularVelocity,
                rb->Restitution,
                rb->Friction,
                isSphere,
                dim
            };
        }
        return std::nullopt;
    }

    void PhysicsWorld::Step(Scene& scene, float deltaTime) {
        if (deltaTime <= 0.0f) return;

        const int subSteps = 2;
        const float dt = deltaTime / static_cast<float>(subSteps);

        for (int step = 0; step < subSteps; ++step) {
            const auto& actors = scene.GetActors();

            // 1. Integration (Gravity & Velocity)
            for (const auto& actor : actors) {
                auto body = GetBodyAdapter(actor.get());
                if (!body || body->bIsStatic) continue;

                if (body->bUseGravity) {
                    body->LinearVelocity += Gravity * dt;
                }

                actor->Transform.Position += body->LinearVelocity * dt;
                actor->Transform.Rotation += body->AngularVelocity * dt;
            }

            // 2. Collision detection & resolution
            for (size_t i = 0; i < actors.size(); ++i) {
                auto bodyA = GetBodyAdapter(actors[i].get());
                if (!bodyA) continue;

                // Collision with floor plane (Y = 0)
                if (!bodyA->bIsStatic) {
                    float bottomY = actors[i]->Transform.Position.y;
                    if (bottomY - bodyA->RadiusOrHalfH < 0.0f) {
                        float penetration = -(bottomY - bodyA->RadiusOrHalfH);
                        actors[i]->Transform.Position.y += penetration;

                        if (bodyA->LinearVelocity.y < 0.0f) {
                            bodyA->LinearVelocity.y = -bodyA->LinearVelocity.y * bodyA->Restitution;
                            bodyA->LinearVelocity.x *= (1.0f - bodyA->Friction);
                            bodyA->LinearVelocity.z *= (1.0f - bodyA->Friction);

                            if (std::abs(bodyA->LinearVelocity.y) < 0.15f) {
                                bodyA->LinearVelocity.y = 0.0f;
                            }
                        }
                    }
                }

                // Actor vs Actor collisions
                for (size_t j = i + 1; j < actors.size(); ++j) {
                    auto bodyB = GetBodyAdapter(actors[j].get());
                    if (!bodyB) continue;
                    if (bodyA->bIsStatic && bodyB->bIsStatic) continue;

                    // Sphere vs Sphere
                    if (bodyA->bIsSphere && bodyB->bIsSphere) {
                        float rA = bodyA->RadiusOrHalfH;
                        float rB = bodyB->RadiusOrHalfH;
                        glm::vec3 delta = actors[i]->Transform.Position - actors[j]->Transform.Position;
                        float dist = glm::length(delta);
                        float minDist = rA + rB;

                        if (dist < minDist && dist > 0.0001f) {
                            glm::vec3 normal = delta / dist;
                            float penetration = minDist - dist;

                            if (!bodyA->bIsStatic && !bodyB->bIsStatic) {
                                actors[i]->Transform.Position += normal * (penetration * 0.5f);
                                actors[j]->Transform.Position -= normal * (penetration * 0.5f);
                            } else if (!bodyA->bIsStatic) {
                                actors[i]->Transform.Position += normal * penetration;
                            } else if (!bodyB->bIsStatic) {
                                actors[j]->Transform.Position -= normal * penetration;
                            }

                            glm::vec3 relVel = bodyA->LinearVelocity - bodyB->LinearVelocity;
                            float velAlongNormal = glm::dot(relVel, normal);

                            if (velAlongNormal < 0.0f) {
                                float e = std::min(bodyA->Restitution, bodyB->Restitution);
                                float invMassA = bodyA->bIsStatic ? 0.0f : (1.0f / bodyA->Mass);
                                float invMassB = bodyB->bIsStatic ? 0.0f : (1.0f / bodyB->Mass);
                                float jImpulse = -(1.0f + e) * velAlongNormal / (invMassA + invMassB);
                                glm::vec3 impulse = normal * jImpulse;

                                if (!bodyA->bIsStatic) bodyA->LinearVelocity += impulse * invMassA;
                                if (!bodyB->bIsStatic) bodyB->LinearVelocity -= impulse * invMassB;
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
