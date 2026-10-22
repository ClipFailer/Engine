/**
 * @file EngineTests.cpp
 * @brief Automated unit and integration test suite verifying engine core subsystems.
 */

#include "core/Types.hpp"
#include "core/UUID.hpp"
#include "core/Math.hpp"
#include "core/Log.hpp"
#include "reflection/Reflection.hpp"
#include "scene/Object.hpp"
#include "scene/Component.hpp"
#include "scene/Actor.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "scene/Scene.hpp"
#include "scene/Primitives.hpp"
#include "physics/PhysicsWorld.hpp"
#include "project/Project.hpp"

#include <iostream>
#include <cassert>
#include <cmath>

#define TEST_ASSERT(cond, msg) \
    if (!(cond)) { \
        std::cerr << "[TEST FAILED] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        return false; \
    }

namespace EngineTests {

    bool TestUUID() {
        Engine::UUID id1;
        Engine::UUID id2;
        TEST_ASSERT(id1 != id2, "Generated UUIDs must be unique");

        Engine::UUID idCopy = id1;
        TEST_ASSERT(idCopy == id1, "Copied UUID must match original");

        std::unordered_map<Engine::UUID, int> map;
        map[id1] = 42;
        map[id2] = 100;
        TEST_ASSERT(map[id1] == 42 && map[id2] == 100, "UUID hash map lookup failed");

        return true;
    }

    bool TestMathAndTransform() {
        Engine::Transform t;
        t.Position = glm::vec3(10.0f, 5.0f, -2.0f);
        t.Rotation = glm::vec3(0.0f, 90.0f, 0.0f);
        t.Scale = glm::vec3(2.0f, 2.0f, 2.0f);

        glm::mat4 m = t.GetMatrix();
        TEST_ASSERT(m[3][0] == 10.0f && m[3][1] == 5.0f && m[3][2] == -2.0f, "Translation in matrix is incorrect");

        glm::vec3 right = t.GetRight();
        TEST_ASSERT(std::abs(right.z - 1.0f) < 0.01f || std::abs(right.x) < 0.01f, "Right vector after yaw rotation is incorrect");

        return true;
    }

    bool TestReflection() {
        Engine::Component::RegisterReflection();
        Engine::Actor::RegisterReflection();
        Engine::StaticMeshComponent::RegisterReflection();
        Engine::LightComponent::RegisterReflection();
        Engine::RigidBodyComponent::RegisterReflection();

        const auto* cls = Engine::TypeRegistry::Get().FindClass("StaticMeshComponent");
        TEST_ASSERT(cls != nullptr, "Class StaticMeshComponent not found in TypeRegistry");
        TEST_ASSERT(cls->GetName() == "StaticMeshComponent", "Class name mismatch");

        // Verify property inspection and modification
        Engine::StaticMeshComponent meshComp;
        meshComp.Metallic = 0.25f;
        meshComp.Roughness = 0.75f;

        bool foundMetallic = false;
        for (const auto& prop : cls->GetProperties()) {
            if (prop.GetName() == "Metallic") {
                foundMetallic = true;
                float val = prop.GetValue<float>(&meshComp);
                TEST_ASSERT(std::abs(val - 0.25f) < 0.001f, "Reflected float value read mismatch");

                // Write new value via reflection
                prop.GetValue<float>(&meshComp) = 0.99f;
                TEST_ASSERT(std::abs(meshComp.Metallic - 0.99f) < 0.001f, "Reflected float value write failed");
            }
        }
        TEST_ASSERT(foundMetallic, "Property 'Metallic' not found in StaticMeshComponent");

        return true;
    }

    bool TestSceneAndActorHierarchy() {
        Engine::Scene scene;
        auto actor = scene.SpawnActor("PlayerShip");
        TEST_ASSERT(actor != nullptr, "SpawnActor returned nullptr");
        TEST_ASSERT(actor->GetName() == "PlayerShip", "Actor name mismatch");

        auto mesh = actor->AddComponent<Engine::StaticMeshComponent>(Engine::PrimitiveType::Cube);
        TEST_ASSERT(mesh != nullptr, "AddComponent StaticMeshComponent failed");
        TEST_ASSERT(actor->GetComponent<Engine::StaticMeshComponent>() == mesh, "GetComponent returned wrong pointer");

        auto light = actor->AddComponent<Engine::LightComponent>(Engine::LightType::Point);
        TEST_ASSERT(light != nullptr, "AddComponent LightComponent failed");
        TEST_ASSERT(actor->GetComponents().size() == 2, "Component count mismatch");

        // Test snapshot & simulation restore
        actor->Transform.Position = glm::vec3(0.0f, 10.0f, 0.0f);
        scene.StartSimulation();
        TEST_ASSERT(scene.GetState() == Engine::SimulationState::Playing, "Scene state should be Playing");

        actor->Transform.Position = glm::vec3(50.0f, 50.0f, 50.0f);
        scene.StopSimulation();
        TEST_ASSERT(scene.GetState() == Engine::SimulationState::Stopped, "Scene state should be Stopped");
        TEST_ASSERT(actor->Transform.Position.y == 10.0f, "Scene stop did not restore actor transform");

        return true;
    }

    bool TestPhysics() {
        Engine::Scene scene;
        auto fallingSphere = scene.SpawnActor("FallingSphere");
        fallingSphere->Transform.Position = glm::vec3(0.0f, 10.0f, 0.0f);

        auto rb = fallingSphere->AddComponent<Engine::RigidBodyComponent>();
        rb->bUseGravity = true;
        rb->bIsStatic = false;
        rb->Mass = 1.0f;
        rb->Shape = Engine::ColliderType::Sphere;
        rb->ColliderDimensions = glm::vec3(0.5f);

        Engine::PhysicsWorld physics;
        float initialY = fallingSphere->Transform.Position.y;

        // Step 1: Check gravity causes downward acceleration
        physics.Step(scene, 0.1f);
        TEST_ASSERT(fallingSphere->Transform.Position.y < initialY, "Gravity failed to move actor downwards");
        TEST_ASSERT(rb->LinearVelocity.y < 0.0f, "Velocity should be negative due to gravity");

        // Step 2: Raycast test
        auto hit = physics.Raycast(scene, glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 50.0f);
        TEST_ASSERT(hit.bHit == true, "Raycast should hit the sphere actor");
        TEST_ASSERT(hit.HitActor == fallingSphere, "Raycast hit incorrect actor");

        return true;
    }

    bool TestPrimitives() {
        auto cube = Engine::Primitives::CreateCube(1.0f);
        TEST_ASSERT(cube.Vertices.size() == 24, "Cube must have 24 vertices");
        TEST_ASSERT(cube.Indices.size() == 36, "Cube must have 36 indices");

        auto sphere = Engine::Primitives::CreateSphere(0.5f, 16, 16);
        TEST_ASSERT(!sphere.Vertices.empty(), "Sphere vertices should not be empty");
        TEST_ASSERT(!sphere.Indices.empty(), "Sphere indices should not be empty");

        auto plane = Engine::Primitives::CreatePlane(10.0f);
        TEST_ASSERT(plane.Vertices.size() == 4, "Plane must have 4 vertices");
        TEST_ASSERT(plane.Indices.size() == 6, "Plane must have 6 indices");

        return true;
    }

    bool TestProjectScaffolding() {
        std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "EngineTestProject";
        std::filesystem::remove_all(tempDir);

        auto project = Engine::Project::CreateNew(tempDir, "TestGame");
        TEST_ASSERT(project != nullptr, "CreateNew project failed");
        TEST_ASSERT(std::filesystem::exists(tempDir / "TestGame.uproj"), "Project file .uproj does not exist");
        TEST_ASSERT(std::filesystem::exists(tempDir / "Source" / "SampleGameplayActor.hpp"), "Scaffolded C++ header missing");
        TEST_ASSERT(std::filesystem::exists(tempDir / "Source" / "SampleGameplayActor.cpp"), "Scaffolded C++ source missing");

        std::filesystem::remove_all(tempDir);
        return true;
    }

} // namespace EngineTests

int main() {
    std::cout << "========================================\n";
    std::cout << "  RUNNING ENGINE AUTOMATED TEST SUITE   \n";
    std::cout << "========================================\n";

    int passed = 0;
    int total = 7;

    auto runTest = [&](const char* name, bool(*func)()) {
        std::cout << "[RUNNING] " << name << "... ";
        if (func()) {
            std::cout << "PASSED\n";
            passed++;
        } else {
            std::cout << "FAILED!\n";
        }
    };

    runTest("UUID Uniqueness & Hashing", EngineTests::TestUUID);
    runTest("Math, Vectors & Transform Matrix", EngineTests::TestMathAndTransform);
    runTest("Runtime Reflection & Property Inspection", EngineTests::TestReflection);
    runTest("Scene, Actor & Component Hierarchy", EngineTests::TestSceneAndActorHierarchy);
    runTest("Physics Gravity, Collisions & Raycast", EngineTests::TestPhysics);
    runTest("Procedural Geometry Primitives", EngineTests::TestPrimitives);
    runTest("Project Scaffolding & C++ Code Generator", EngineTests::TestProjectScaffolding);

    std::cout << "========================================\n";
    std::cout << "Tests Passed: " << passed << " / " << total << "\n";
    std::cout << "========================================\n";

    return (passed == total) ? 0 : 1;
}
