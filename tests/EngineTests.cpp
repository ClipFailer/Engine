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
#include "scene/RigidBodyComponent.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/MovementComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/Character.hpp"
#include "scene/WidgetComponent.hpp"
#include "scene/CustomComponent.hpp"
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
        Engine::Character::RegisterReflection();
        Engine::PositionComponent::RegisterReflection();
        Engine::StaticMeshComponent::RegisterReflection();
        Engine::PhysicsComponent::RegisterReflection();
        Engine::CollisionComponent::RegisterReflection();
        Engine::MovementComponent::RegisterReflection();
        Engine::LightComponent::RegisterReflection();
        Engine::WidgetComponent::RegisterReflection();
        Engine::CustomComponent::RegisterReflection();
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

        // Verify SpawnLightActor creates lightweight actor without mesh, physics, or collision
        auto sunLight = scene.SpawnLightActor("SunLight", Engine::LightType::Directional);
        TEST_ASSERT(sunLight != nullptr, "SpawnLightActor failed");
        TEST_ASSERT(sunLight->GetComponent<Engine::PositionComponent>() != nullptr, "Light must have PositionComponent");
        TEST_ASSERT(sunLight->GetComponent<Engine::LightComponent>() != nullptr, "Light must have LightComponent");
        TEST_ASSERT(sunLight->GetComponent<Engine::StaticMeshComponent>() == nullptr, "Directional Light must NOT have StaticMeshComponent");
        TEST_ASSERT(sunLight->GetComponent<Engine::PhysicsComponent>() == nullptr, "Directional Light must NOT have PhysicsComponent");
        TEST_ASSERT(sunLight->GetComponent<Engine::CollisionComponent>() == nullptr, "Directional Light must NOT have CollisionComponent");

        // Verify RemoveComponent<T>()
        actor->RemoveComponent<Engine::StaticMeshComponent>();
        TEST_ASSERT(actor->GetComponent<Engine::StaticMeshComponent>() == nullptr, "RemoveComponent<StaticMeshComponent> failed");

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

        bool genComp = project->GenerateComponentClass("StaminaComponent");
        TEST_ASSERT(genComp == true, "GenerateComponentClass failed");
        TEST_ASSERT(std::filesystem::exists(tempDir / "Source" / "StaminaComponent.hpp"), "Scaffolded StaminaComponent.hpp missing");
        TEST_ASSERT(std::filesystem::exists(tempDir / "Source" / "StaminaComponent.cpp"), "Scaffolded StaminaComponent.cpp missing");

        std::filesystem::remove_all(tempDir);
        return true;
    }

    bool TestCharacterAndUIComponents() {
        Engine::Scene scene;

        // 1. Test Character class
        auto chr = scene.SpawnActor<Engine::Character>("TestHero");
        TEST_ASSERT(chr != nullptr, "SpawnActor<Character> failed");
        TEST_ASSERT(chr->WalkSpeed == 6.0f, "Default Character WalkSpeed mismatch");
        TEST_ASSERT(chr->GetComponent<Engine::MovementComponent>() != nullptr, "Character must have MovementComponent");

        chr->AddMovementInput(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f);
        chr->Jump();
        TEST_ASSERT(chr->bIsGrounded == false, "Character should not be grounded immediately after jump");

        // 2. Test WidgetComponent attachment
        auto widget = chr->AddComponent<Engine::WidgetComponent>("<div class='hp'><h1>Hero HP</h1></div>", ".hp { color: #fff; }");
        TEST_ASSERT(widget != nullptr, "AddComponent WidgetComponent failed");
        TEST_ASSERT(widget->bIsWorldSpace == true, "WidgetComponent default should be world space");

        // 3. Test dynamic CustomComponent attachment
        auto custom = chr->AddComponent<Engine::CustomComponent>("StaminaSubsystem");
        TEST_ASSERT(custom != nullptr, "AddComponent CustomComponent failed");
        TEST_ASSERT(custom->ComponentType == "StaminaSubsystem", "CustomComponent type name mismatch");

        return true;
    }

    bool TestSceneSerializationAndCoordinates() {
        std::filesystem::path testScenePath = std::filesystem::temp_directory_path() / "engine_scene_coords_test.json";

        // 1. Setup Scene with Actors at specific coordinates
        {
            Engine::Scene scene;
            scene.ClearAllActors();

            auto chr = scene.SpawnActor<Engine::Character>("PlayerHero");
            chr->Transform.Position = glm::vec3(12.5f, 3.2f, -7.8f);
            chr->Transform.Rotation = glm::vec3(0.0f, 90.0f, 0.0f);
            chr->Transform.Scale = glm::vec3(1.2f);
            chr->WalkSpeed = 8.5f;

            auto cube = scene.SpawnActor("PhysicsCube");
            cube->Transform.Position = glm::vec3(-4.0f, 6.0f, 2.0f);
            auto mesh = cube->GetComponent<Engine::StaticMeshComponent>();
            if (mesh) {
                mesh->MeshType = Engine::PrimitiveType::Cube;
                mesh->BaseColor = glm::vec4(1.0f, 0.5f, 0.2f, 1.0f);
            }
            auto rb = cube->AddComponent<Engine::RigidBodyComponent>();
            rb->Mass = 5.0f;

            auto sun = scene.SpawnLightActor("SunLight", Engine::LightType::Directional);
            sun->Transform.Position = glm::vec3(0.0f, 20.0f, 0.0f);
            if (auto light = sun->GetComponent<Engine::LightComponent>()) {
                light->Intensity = 3.5f;
            }

            TEST_ASSERT(scene.GetActors().size() == 3, "Setup scene must have 3 actors");
            TEST_ASSERT(scene.SaveToFile(testScenePath.string()), "SaveToFile failed");
        }

        // 2. Load Scene back and verify all coordinates and properties persist
        {
            Engine::Scene loadedScene;
            TEST_ASSERT(loadedScene.LoadFromFile(testScenePath.string()), "LoadFromFile failed");
            TEST_ASSERT(loadedScene.GetActors().size() == 3, "Loaded scene must have 3 actors");

            Engine::Actor* foundHero = nullptr;
            Engine::Actor* foundCube = nullptr;
            Engine::Actor* foundSun = nullptr;

            for (const auto& a : loadedScene.GetActors()) {
                if (a->GetName() == "PlayerHero") foundHero = a.get();
                else if (a->GetName() == "PhysicsCube") foundCube = a.get();
                else if (a->GetName() == "SunLight") foundSun = a.get();
            }

            TEST_ASSERT(foundHero != nullptr, "Hero actor was not restored");
            TEST_ASSERT(foundCube != nullptr, "Cube actor was not restored");
            TEST_ASSERT(foundSun != nullptr, "Sun light actor was not restored");

            // Verify Coordinates
            TEST_ASSERT(std::abs(foundHero->Transform.Position.x - 12.5f) < 0.001f, "Hero X coordinate mismatch");
            TEST_ASSERT(std::abs(foundHero->Transform.Position.y - 3.2f) < 0.001f, "Hero Y coordinate mismatch");
            TEST_ASSERT(std::abs(foundHero->Transform.Position.z - (-7.8f)) < 0.001f, "Hero Z coordinate mismatch");
            TEST_ASSERT(std::abs(foundHero->Transform.Scale.x - 1.2f) < 0.001f, "Hero Scale mismatch");

            auto chr = dynamic_cast<Engine::Character*>(foundHero);
            TEST_ASSERT(chr != nullptr, "Hero actor is not Character type");
            TEST_ASSERT(std::abs(chr->WalkSpeed - 8.5f) < 0.001f, "Hero WalkSpeed mismatch");

            // Verify Cube
            TEST_ASSERT(std::abs(foundCube->Transform.Position.x - (-4.0f)) < 0.001f, "Cube X coordinate mismatch");
            TEST_ASSERT(std::abs(foundCube->Transform.Position.y - 6.0f) < 0.001f, "Cube Y coordinate mismatch");
            TEST_ASSERT(std::abs(foundCube->Transform.Position.z - 2.0f) < 0.001f, "Cube Z coordinate mismatch");

            auto cubeMesh = foundCube->GetComponent<Engine::StaticMeshComponent>();
            TEST_ASSERT(cubeMesh != nullptr, "Cube missing StaticMeshComponent");
            TEST_ASSERT(std::abs(cubeMesh->BaseColor.r - 1.0f) < 0.001f, "Cube color R mismatch");

            auto cubeRb = foundCube->GetComponent<Engine::RigidBodyComponent>();
            TEST_ASSERT(cubeRb != nullptr, "Cube missing RigidBodyComponent");
            TEST_ASSERT(std::abs(cubeRb->Mass - 5.0f) < 0.001f, "Cube mass mismatch");

            // Verify Light
            auto lightComp = foundSun->GetComponent<Engine::LightComponent>();
            TEST_ASSERT(lightComp != nullptr, "Sun missing LightComponent");
            TEST_ASSERT(std::abs(lightComp->Intensity - 3.5f) < 0.001f, "Sun intensity mismatch");
        }

        std::filesystem::remove(testScenePath);
        return true;
    }

    bool TestActorToActorCollision() {
        Engine::Scene scene;
        scene.ClearAllActors();

        Engine::PhysicsWorld world;
        world.Gravity = glm::vec3(0.0f, -9.81f, 0.0f);

        // Ground box collider (large plane 20x0.2x20)
        auto ground = scene.SpawnActor("GroundObstacle");
        ground->Transform.Position = glm::vec3(0.0f, 0.0f, 0.0f);
        auto groundRb = ground->AddComponent<Engine::RigidBodyComponent>();
        groundRb->bIsStatic = true;
        groundRb->bUseGravity = false;
        groundRb->Shape = Engine::ColliderType::Box;
        groundRb->ColliderDimensions = glm::vec3(10.0f, 0.1f, 10.0f);

        // Dynamic falling box (spawned above ground at X=3, Y=1.5, Z=3)
        auto cube = scene.SpawnActor("FallingBox");
        cube->Transform.Position = glm::vec3(3.0f, 1.5f, 3.0f);
        auto cubeRb = cube->AddComponent<Engine::RigidBodyComponent>();
        cubeRb->Mass = 1.0f;
        cubeRb->Shape = Engine::ColliderType::Box;
        cubeRb->ColliderDimensions = glm::vec3(0.5f, 0.5f, 0.5f);

        // Step physics world multiple times
        for (int i = 0; i < 60; ++i) {
            world.Step(scene, 0.016f);
        }

        // Cube should land on the ground (around Y = 0.1 + 0.5 = 0.6) and NOT fall through
        TEST_ASSERT(cube->Transform.Position.y >= 0.55f, "Falling box penetrated through ground floor collider!");
        TEST_ASSERT(cube->Transform.Position.y <= 0.75f, "Falling box failed to settle on ground surface");

        return true;
    }

} // namespace EngineTests

int main() {
    std::cout << "========================================\n";
    std::cout << "  RUNNING ENGINE AUTOMATED TEST SUITE   \n";
    std::cout << "========================================\n";

    int passed = 0;
    int total = 13;

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
    runTest("Character Locomotion, UI & Custom Components", EngineTests::TestCharacterAndUIComponents);
    runTest("Scene Persistence, Coordinates & Component State", EngineTests::TestSceneSerializationAndCoordinates);
    runTest("Actor-to-Actor 3D Collision Resolution", EngineTests::TestActorToActorCollision);

    std::cout << "========================================\n";
    std::cout << "Tests Passed: " << passed << " / " << total << "\n";
    std::cout << "========================================\n";

    return (passed == total) ? 0 : 1;
}
