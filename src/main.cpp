/**
 * @file main.cpp
 * @brief Entry point for the Vulkan 3D Game & Simulation Engine Editor.
 */

#include "editor/EditorApp.hpp"
#include "core/Log.hpp"
#include "scene/Component.hpp"
#include "scene/Actor.hpp"
#include "scene/Character.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/RigidBodyComponent.hpp"
#include "scene/AudioComponent.hpp"
#include "scene/PositionComponent.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/MovementComponent.hpp"
#include "scene/WidgetComponent.hpp"
#include "scene/CustomComponent.hpp"
#include "animation/SkeletalMeshComponent.hpp"
#include <fstream>
#include <sstream>
#include <chrono>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

static void LogAndShowCrash(const std::string& crashDetails) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&t);
    if (!timeStr.empty() && timeStr.back() == '\n') timeStr.pop_back();

    std::stringstream ss;
    ss << "========================================\n"
       << "       ENGINE CRASH REPORT              \n"
       << "========================================\n"
       << "Time: " << timeStr << "\n\n"
       << "Reason / Diagnosis:\n" << crashDetails << "\n\n"
       << "A copy of this diagnostic log has been written to 'crash_log.txt'.\n";

    std::string report = ss.str();

    std::ofstream file("crash_log.txt", std::ios::app);
    if (file.is_open()) {
        file << report << "\n----------------------------------------\n\n";
        file.close();
    }

    MessageBoxA(NULL, report.c_str(), "Engine Crash Reporter", MB_OK | MB_ICONERROR);
}

static LONG WINAPI EngineUnhandledExceptionFilter(EXCEPTION_POINTERS* ep) {
    std::stringstream ss;
    DWORD code = ep ? ep->ExceptionRecord->ExceptionCode : 0;
    void* addr = ep ? ep->ExceptionRecord->ExceptionAddress : nullptr;

    const char* desc = "Unknown Hardware Exception";
    switch (code) {
        case EXCEPTION_ACCESS_VIOLATION: desc = "Access Violation (Invalid Memory Read/Write)"; break;
        case EXCEPTION_ILLEGAL_INSTRUCTION: desc = "Illegal CPU Instruction"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO: desc = "Integer Division by Zero"; break;
        case EXCEPTION_STACK_OVERFLOW: desc = "Stack Overflow"; break;
        case 0xE06D7363: desc = "Unhandled C++ Exception (MSVC/Clang CRT)"; break;
        default: break;
    }

    ss << desc << " (Code: 0x" << std::hex << code << ") at address: 0x" << addr;
    LogAndShowCrash(ss.str());
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

/**
 * @brief Main engine application entry point.
 * @param argc Argument count.
 * @param argv Argument vectors.
 * @return Process exit code.
 */
int main(int argc, char* argv[]) {
#if defined(_WIN32)
    SetUnhandledExceptionFilter(EngineUnhandledExceptionFilter);
#endif

    std::set_terminate([]() {
        try {
            auto ex = std::current_exception();
            if (ex) {
                std::rethrow_exception(ex);
            }
        } catch (const std::exception& e) {
#if defined(_WIN32)
            LogAndShowCrash(std::string("Fatal unhandled exception: ") + e.what());
#endif
        } catch (...) {
#if defined(_WIN32)
            LogAndShowCrash("Fatal unhandled non-standard exception");
#endif
        }
        std::abort();
    });

    // 1. Register Engine Reflection metadata
    Engine::Component::RegisterReflection();
    Engine::Actor::RegisterReflection();
    Engine::Character::RegisterReflection();
    Engine::StaticMeshComponent::RegisterReflection();
    Engine::LightComponent::RegisterReflection();
    Engine::RigidBodyComponent::RegisterReflection();
    Engine::AudioComponent::RegisterReflection();
    Engine::PositionComponent::RegisterReflection();
    Engine::PhysicsComponent::RegisterReflection();
    Engine::CollisionComponent::RegisterReflection();
    Engine::MovementComponent::RegisterReflection();
    Engine::WidgetComponent::RegisterReflection();
    Engine::CustomComponent::RegisterReflection();
    Engine::SkeletalMeshComponent::RegisterReflection();

    LOG_INFO("Vulkan 3D Engine Editor starting up...");

    // 2. Instantiate and launch Editor
    try {
        Engine::EditorApp app;
        if (!app.Initialize()) {
            LOG_ERROR("Engine Editor failed to initialize");
            return -1;
        }

        LOG_INFO("Entering engine main loop");
        app.Run();
        LOG_INFO("Engine Editor exited successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("Unhandled engine exception: {}", e.what());
        return -1;
    }

    return 0;
}

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/**
 * @brief Windows WinMain entry point for GUI subsystem without an OS console window.
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    FreeConsole();
    return main(__argc, __argv);
}
#endif
