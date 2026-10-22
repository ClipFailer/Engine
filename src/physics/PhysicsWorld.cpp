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
        glm::vec3 HalfExtents{0.5f};
        float SphereRadius{0.5f};
        glm::vec3 CenterOffset{0.0f};
    };

    static glm::vec3 s_StaticZeroVelocity{0.0f};

    static std::optional<PhysicsBodyAdapter> GetBodyAdapter(Actor* actor) {
        if (!actor) return std::nullopt;

        auto rb = actor->GetComponent<RigidBodyComponent>();
        auto phys = actor->GetComponent<PhysicsComponent>();
        auto coll = actor->GetComponent<CollisionComponent>();

        if (!rb && !phys && !coll) {
            return std::nullopt;
        }

        bool isStatic = (rb && rb->bIsStatic) || (phys && phys->bIsStatic) || (!rb && !phys && coll);
        bool useGravity = !isStatic && ((rb && rb->bUseGravity) || (phys && phys->bUseGravity));
        float mass = rb ? rb->Mass : (phys ? phys->Mass : 1.0f);

        glm::vec3& linVel = rb ? rb->LinearVelocity : (phys ? phys->LinearVelocity : s_StaticZeroVelocity);
        glm::vec3& angVel = rb ? rb->AngularVelocity : (phys ? phys->AngularVelocity : s_StaticZeroVelocity);

        float rest = coll ? coll->Restitution : (rb ? rb->Restitution : 0.6f);
        float fric = coll ? coll->Friction : (rb ? rb->Friction : 0.2f);

        bool isSphere = (coll && coll->Shape == CollisionShape::Sphere) || (rb && rb->Shape == ColliderType::Sphere);

        glm::vec3 halfExt{0.5f};
        if (rb && (rb->ColliderDimensions != glm::vec3(0.5f) || !coll)) {
            halfExt = rb->ColliderDimensions;
        } else if (coll) {
            halfExt = coll->BoxHalfExtents;
        }
        halfExt = glm::abs(halfExt * actor->Transform.Scale);

        float maxScale = std::max({actor->Transform.Scale.x, actor->Transform.Scale.y, actor->Transform.Scale.z});
        float radius = (coll ? coll->SphereRadius : (rb ? rb->ColliderDimensions.x : 0.5f)) * maxScale;
        glm::vec3 offset = coll ? coll->Offset : glm::vec3(0.0f);

        return PhysicsBodyAdapter{
            true,
            isStatic,
            useGravity,
            mass,
            linVel,
            angVel,
            rest,
            fric,
            isSphere,
            halfExt,
            radius,
            offset
        };
    }

    struct CollisionResult {
        bool bColliding{false};
        glm::vec3 Normal{0.0f, 1.0f, 0.0f}; // Points from B to A
        float Penetration{0.0f};
    };

    static CollisionResult TestCollision(const glm::vec3& posA, const PhysicsBodyAdapter& a,
                                         const glm::vec3& posB, const PhysicsBodyAdapter& b) {
        CollisionResult result;
        glm::vec3 centerA = posA + a.CenterOffset;
        glm::vec3 centerB = posB + b.CenterOffset;

        // 1. Sphere vs Sphere
        if (a.bIsSphere && b.bIsSphere) {
            glm::vec3 delta = centerA - centerB;
            float dist = glm::length(delta);
            float minDist = a.SphereRadius + b.SphereRadius;
            if (dist < minDist) {
                result.bColliding = true;
                result.Penetration = minDist - dist;
                result.Normal = (dist > 0.0001f) ? (delta / dist) : glm::vec3(0.0f, 1.0f, 0.0f);
            }
            return result;
        }

        // 2. Box vs Box (AABB minimum penetration axis)
        if (!a.bIsSphere && !b.bIsSphere) {
            glm::vec3 delta = centerA - centerB;
            float ox = (a.HalfExtents.x + b.HalfExtents.x) - std::abs(delta.x);
            float oy = (a.HalfExtents.y + b.HalfExtents.y) - std::abs(delta.y);
            float oz = (a.HalfExtents.z + b.HalfExtents.z) - std::abs(delta.z);

            if (ox > 0.0f && oy > 0.0f && oz > 0.0f) {
                result.bColliding = true;
                if (oy <= ox && oy <= oz) {
                    result.Penetration = oy;
                    result.Normal = glm::vec3(0.0f, delta.y >= 0.0f ? 1.0f : -1.0f, 0.0f);
                } else if (ox <= oz) {
                    result.Penetration = ox;
                    result.Normal = glm::vec3(delta.x >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
                } else {
                    result.Penetration = oz;
                    result.Normal = glm::vec3(0.0f, 0.0f, delta.z >= 0.0f ? 1.0f : -1.0f);
                }
            }
            return result;
        }

        // 3. Box vs Sphere
        bool aIsSphere = a.bIsSphere;
        const glm::vec3& sphereCenter = aIsSphere ? centerA : centerB;
        float sphereRadius = aIsSphere ? a.SphereRadius : b.SphereRadius;

        const glm::vec3& boxCenter = aIsSphere ? centerB : centerA;
        const glm::vec3& boxHalf = aIsSphere ? b.HalfExtents : a.HalfExtents;

        glm::vec3 closest = glm::clamp(sphereCenter, boxCenter - boxHalf, boxCenter + boxHalf);
        glm::vec3 delta = sphereCenter - closest;
        float dist = glm::length(delta);

        if (dist < sphereRadius) {
            result.bColliding = true;
            if (dist > 0.0001f) {
                result.Penetration = sphereRadius - dist;
                glm::vec3 n = delta / dist;
                result.Normal = aIsSphere ? n : -n;
            } else {
                glm::vec3 localDelta = sphereCenter - boxCenter;
                float ox = boxHalf.x - std::abs(localDelta.x);
                float oy = boxHalf.y - std::abs(localDelta.y);
                float oz = boxHalf.z - std::abs(localDelta.z);

                glm::vec3 n{0.0f, 1.0f, 0.0f};
                float pen = sphereRadius;
                if (oy <= ox && oy <= oz) {
                    n = glm::vec3(0.0f, localDelta.y >= 0.0f ? 1.0f : -1.0f, 0.0f);
                    pen += oy;
                } else if (ox <= oz) {
                    n = glm::vec3(localDelta.x >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
                    pen += ox;
                } else {
                    n = glm::vec3(0.0f, 0.0f, localDelta.z >= 0.0f ? 1.0f : -1.0f);
                    pen += oz;
                }
                result.Penetration = pen;
                result.Normal = aIsSphere ? n : -n;
            }
        }

        return result;
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

                // Air resistance / linear damping
                body->LinearVelocity *= std::clamp(1.0f - 0.05f * dt, 0.0f, 1.0f);
                body->AngularVelocity *= std::clamp(1.0f - 0.1f * dt, 0.0f, 1.0f);

                // Resting body threshold (sleep stabilization to eliminate micro-jitter)
                if (glm::length(body->LinearVelocity) < 0.01f && std::abs(body->LinearVelocity.y) < 0.02f) {
                    body->LinearVelocity.x = 0.0f;
                    body->LinearVelocity.z = 0.0f;
                }

                actor->Transform.Position += body->LinearVelocity * dt;
                actor->Transform.Rotation += body->AngularVelocity * dt;
            }

            // 2. Collision detection & resolution (Box-Box, Box-Sphere, Sphere-Sphere)
            for (size_t i = 0; i < actors.size(); ++i) {
                auto bodyA = GetBodyAdapter(actors[i].get());
                if (!bodyA) continue;

                for (size_t j = i + 1; j < actors.size(); ++j) {
                    auto bodyB = GetBodyAdapter(actors[j].get());
                    if (!bodyB) continue;
                    if (bodyA->bIsStatic && bodyB->bIsStatic) continue;

                    auto hit = TestCollision(actors[i]->Transform.Position, *bodyA,
                                             actors[j]->Transform.Position, *bodyB);

                    if (hit.bColliding && hit.Penetration > 0.0001f) {
                        glm::vec3 normal = hit.Normal;
                        float penetration = hit.Penetration;

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

                            // Tangential friction damping
                            glm::vec3 tangent = relVel - normal * velAlongNormal;
                            float tLen = glm::length(tangent);
                            if (tLen > 0.001f) {
                                tangent /= tLen;
                                float friction = std::sqrt(bodyA->Friction * bodyB->Friction);
                                float jt = -glm::dot(relVel, tangent) / (invMassA + invMassB);
                                jt = glm::clamp(jt, -jImpulse * friction, jImpulse * friction);
                                glm::vec3 frictionImpulse = tangent * jt;
                                if (!bodyA->bIsStatic) bodyA->LinearVelocity += frictionImpulse * invMassA;
                                if (!bodyB->bIsStatic) bodyB->LinearVelocity -= frictionImpulse * invMassB;
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
            float radius = 0.6f;

            if (auto coll = actor->GetComponent<CollisionComponent>()) {
                if (coll->Shape == CollisionShape::Sphere) {
                    radius = coll->SphereRadius * std::max({actor->Transform.Scale.x, actor->Transform.Scale.y, actor->Transform.Scale.z});
                } else {
                    radius = glm::length(coll->BoxHalfExtents * actor->Transform.Scale);
                }
                center += coll->Offset;
            } else {
                float maxScale = std::max({actor->Transform.Scale.x, actor->Transform.Scale.y, actor->Transform.Scale.z});
                radius = std::max(0.6f, maxScale * 0.866f);
            }

            // Ray vs Sphere intersection test
            glm::vec3 oc = rayOrigin - center;
            float a = glm::dot(rayDir, rayDir);
            float b = 2.0f * glm::dot(oc, rayDir);
            float c = glm::dot(oc, oc) - radius * radius;
            float discriminant = b * b - 4.0f * a * c;

            if (discriminant >= 0.0f) {
                float sqrtDisc = std::sqrt(discriminant);
                float t0 = (-b - sqrtDisc) / (2.0f * a);
                float t1 = (-b + sqrtDisc) / (2.0f * a);
                float dist = t0 > 0.0f ? t0 : t1;

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
