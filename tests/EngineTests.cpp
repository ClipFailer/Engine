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
#include "scene/PositionComponent.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/MovementComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/Scene.hpp"
#include "scene/Primitives.hpp"
#include "physics/PhysicsWorld.hpp"
#include "project/Project.hpp"
#include "module/ModuleManager.hpp"
#include "ui/UIModule.hpp"
#include "animation/AnimationModule.hpp"
#include "animation/SkeletalMeshComponent.hpp"

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
        Engine::PositionComponent::RegisterReflection();
        Engine::StaticMeshComponent::RegisterReflection();
        Engine::PhysicsComponent::RegisterReflection();
        Engine::CollisionComponent::RegisterReflection();
        Engine::MovementComponent::RegisterReflection();
        Engine::LightComponent::RegisterReflection();
        Engine::SkeletalMeshComponent::RegisterReflection();

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

        // Check default components on Actor (Position, StaticMesh, Physics, Collision)
        TEST_ASSERT(actor->GetComponent<Engine::PositionComponent>() != nullptr, "Actor must have default PositionComponent");
        TEST_ASSERT(actor->GetComponent<Engine::StaticMeshComponent>() != nullptr, "Actor must have default StaticMeshComponent");
        TEST_ASSERT(actor->GetComponent<Engine::PhysicsComponent>() != nullptr, "Actor must have default PhysicsComponent");
        TEST_ASSERT(actor->GetComponent<Engine::CollisionComponent>() != nullptr, "Actor must have default CollisionComponent");

        auto light = actor->AddComponent<Engine::LightComponent>(Engine::LightType::Point);
        TEST_ASSERT(light != nullptr, "AddComponent LightComponent failed");
        TEST_ASSERT(actor->GetComponents().size() == 5, "Component count mismatch");

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

    bool TestPhysicsAndCollisionFitting() {
        Engine::Scene scene;
        auto actor = scene.SpawnActor("TestObject");
        actor->Transform.Position = glm::vec3(0.0f, 8.0f, 0.0f);

        auto mesh = actor->GetComponent<Engine::StaticMeshComponent>();
        auto coll = actor->GetComponent<Engine::CollisionComponent>();
        auto phys = actor->GetComponent<Engine::PhysicsComponent>();

        TEST_ASSERT(mesh && coll && phys, "Missing core physics/collision components");

        // Test FitToMesh
        mesh->MeshType = Engine::PrimitiveType::Cube;
        coll->FitToMesh(*mesh);
        TEST_ASSERT(coll->BoxHalfExtents.x == 0.5f && coll->BoxHalfExtents.y == 0.5f, "FitToMesh incorrect for Cube");

        Engine::PhysicsWorld physics;
        float initialY = actor->Transform.Position.y;

        // Step 1: Check gravity causes downward acceleration
        physics.Step(scene, 0.1f);
        TEST_ASSERT(actor->Transform.Position.y < initialY, "Gravity failed to move actor downwards");
        TEST_ASSERT(phys->LinearVelocity.y < 0.0f, "Velocity should be negative due to gravity");

        // Step 2: Raycast test
        auto hit = physics.Raycast(scene, glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 50.0f);
        TEST_ASSERT(hit.bHit == true, "Raycast should hit the actor");
        TEST_ASSERT(hit.HitActor == actor, "Raycast hit incorrect actor");

        return true;
    }

    bool TestMovementComponent() {
        Engine::Scene scene;
        auto pawn = scene.SpawnActor("Pawn");
        auto moveComp = pawn->AddComponent<Engine::MovementComponent>();
        moveComp->MaxSpeed = 10.0f;
        moveComp->Acceleration = 100.0f;

        glm::vec3 startPos = pawn->Transform.Position;
        moveComp->AddInputVector(glm::vec3(1.0f, 0.0f, 0.0f));
        moveComp->Tick(0.1f);

        TEST_ASSERT(pawn->Transform.Position.x > startPos.x, "MovementComponent failed to advance actor along input vector");
        return true;
    }

    bool TestModuleSystemAndHTMLUI() {
        auto& mm = Engine::ModuleManager::Get();
        auto* uiMod = mm.RegisterModule<Engine::UIModule>();
        TEST_ASSERT(uiMod != nullptr, "Failed to register UIModule");
        TEST_ASSERT(mm.GetModule<Engine::UIModule>() == uiMod, "GetModule returned wrong instance");

        // Test HTML + CSS parser
        std::string testHTML =
            "<div style=\"background-color: #ff0000; padding: 10px;\">"
            "  <button id=\"myBtn\" style=\"color: #00ff00;\">Start Game</button>"
            "</div>";

        Engine::UIDocument doc;
        doc.LoadFromString(testHTML);

        auto btn = doc.GetRoot()->FindById("myBtn");
        TEST_ASSERT(btn != nullptr, "Failed to find element by ID 'myBtn'");
        TEST_ASSERT(btn->Text == "Start Game", "Button text mismatch");
        TEST_ASSERT(btn->Style.TextColor.g > 0.9f, "CSS color parsing failed");

        bool bClicked = false;
        btn->OnClick = [&bClicked]() { bClicked = true; };
        btn->OnClick();
        TEST_ASSERT(bClicked == true, "Button click callback failed");

        return true;
    }

    bool TestSkeletalAnimation() {
        Engine::Scene scene;
        auto character = scene.SpawnActor("Character");
        auto skelComp = character->AddComponent<Engine::SkeletalMeshComponent>();
        TEST_ASSERT(!skelComp->MeshSkeleton.GetBones().empty(), "Skeleton bones must not be empty");

        auto anim = std::make_shared<Engine::AnimationSequence>("Walk", 1.0f, 30.0f);
        Engine::BoneTrack track;
        track.BoneName = "Arm";
        track.Positions.push_back({0.0f, glm::vec3(0.0f, 1.0f, 0.0f)});
        track.Positions.push_back({1.0f, glm::vec3(0.0f, 2.0f, 0.0f)});
        anim->AddTrack(track);

        skelComp->PlayAnimation(anim, true);
        skelComp->Tick(0.5f);

        TEST_ASSERT(std::abs(skelComp->CurrentTime - 0.5f) < 0.001f, "Animation time didn't step correctly");
        const auto& bones = skelComp->MeshSkeleton.GetBones();
        TEST_ASSERT(bones.size() >= 2, "Bone count mismatch");
        TEST_ASSERT(bones[1].LocalTransform.Position.y > 1.2f, "Skeletal animation failed to evaluate bone track LERP");

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
        std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "EngineTestProject2";
        std::filesystem::remove_all(tempDir);

        auto project = Engine::Project::CreateNew(tempDir, "CustomGame");
        TEST_ASSERT(project != nullptr, "CreateNew project failed");
        TEST_ASSERT(std::filesystem::exists(tempDir / "CustomGame.uproj"), "Project file .uproj does not exist");

        // Test custom class generation
        bool gen = project->GenerateActorClass("MonsterPawn");
        TEST_ASSERT(gen == true, "GenerateActorClass failed");
        TEST_ASSERT(std::filesystem::exists(tempDir / "Source" / "MonsterPawn.hpp"), "Scaffolded MonsterPawn.hpp missing");
        TEST_ASSERT(std::filesystem::exists(tempDir / "Source" / "MonsterPawn.cpp"), "Scaffolded MonsterPawn.cpp missing");

        std::filesystem::remove_all(tempDir);
        return true;
    }

} // namespace EngineTests

int main() {
    std::cout << "========================================\n";
    std::cout << "  RUNNING ENGINE AUTOMATED TEST SUITE   \n";
    std::cout << "========================================\n";

    int passed = 0;
    int total = 10;

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
    runTest("Physics, Collisions & Bounds Fitting", EngineTests::TestPhysicsAndCollisionFitting);
    runTest("Movement Locomotion Component", EngineTests::TestMovementComponent);
    runTest("Modular Engine System & HTML+CSS UI", EngineTests::TestModuleSystemAndHTMLUI);
    runTest("Skeletal Mesh Animation System", EngineTests::TestSkeletalAnimation);
    runTest("Procedural Geometry Primitives", EngineTests::TestPrimitives);
    runTest("Project Scaffolding & Dynamic Class Generator", EngineTests::TestProjectScaffolding);

    std::cout << "========================================\n";
    std::cout << "Tests Passed: " << passed << " / " << total << "\n";
    std::cout << "========================================\n";

    return (passed == total) ? 0 : 1;
}
