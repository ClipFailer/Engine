/**
 * @file ActorDetailWindow.cpp
 * @brief Implementation of full-screen Actor detail window with 3D scene preview and collision bounds.
 */

#include "ActorDetailWindow.hpp"
#include "scene/Scene.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "scene/CollisionComponent.hpp"
#include "scene/PhysicsComponent.hpp"
#include "scene/PositionComponent.hpp"
#include "scene/LightComponent.hpp"
#include "scene/AudioComponent.hpp"
#include "scene/WidgetComponent.hpp"
#include "reflection/Reflection.hpp"
#include "core/Log.hpp"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include <vector>
#include <array>
#include <cmath>

namespace Engine {

    void ActorDetailWindow::Open(Actor* actor, const std::string& assetPath) {
        m_TargetActor = actor;
        m_bIsOpen = (actor != nullptr);
        m_bJustOpened = true;
        m_bDirty = false;
        m_SaveNotificationTimer = 0.0f;

        if (!assetPath.empty()) {
            m_AssociatedAssetPath = assetPath;
            if (m_TargetActor) m_TargetActor->AssetPath = assetPath;
        } else if (m_TargetActor && !m_TargetActor->AssetPath.empty()) {
            m_AssociatedAssetPath = m_TargetActor->AssetPath;
        } else if (m_TargetActor) {
            std::filesystem::path dir = !m_ProjectContentPath.empty() ? m_ProjectContentPath : (std::filesystem::current_path() / "Content");
            m_AssociatedAssetPath = (dir / (m_TargetActor->GetName() + ".actor")).string();
            m_TargetActor->AssetPath = m_AssociatedAssetPath;
        }
    }

    void ActorDetailWindow::Close() noexcept {
        if (m_TargetActor && m_bDirty) {
            SaveCurrentAsset();
        }
        m_TargetActor = nullptr;
        m_bIsOpen = false;
    }

    bool ActorDetailWindow::SaveCurrentAsset() {
        if (!m_TargetActor) return false;
        if (m_AssociatedAssetPath.empty()) {
            std::filesystem::path dir = !m_ProjectContentPath.empty() ? m_ProjectContentPath : (std::filesystem::current_path() / "Content");
            m_AssociatedAssetPath = (dir / (m_TargetActor->GetName() + ".actor")).string();
        }
        bool ok = m_TargetActor->SaveToAssetFile(m_AssociatedAssetPath);
        if (ok) {
            m_bDirty = false;
            m_SaveNotificationTimer = 3.0f;
            LOG_INFO("ActorDetailWindow: Saved actor properties to '{}'", m_AssociatedAssetPath);
        }
        return ok;
    }

    void ActorDetailWindow::Render(const Scene& scene) {
        if (!m_bIsOpen || !m_TargetActor) return;

        // Verify that target actor still exists in scene
        bool bFound = false;
        for (const auto& a : scene.GetActors()) {
            if (a.get() == m_TargetActor) {
                bFound = true;
                break;
            }
        }
        if (!bFound) {
            m_TargetActor = nullptr;
            m_bIsOpen = false;
            return;
        }

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(vp->WorkSize, ImGuiCond_Always);

        if (m_bJustOpened) {
            ImGui::SetNextWindowFocus();
            m_bJustOpened = false;
        }

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_MenuBar |
                                 ImGuiWindowFlags_NoDocking;

        std::string title = "Actor Blueprint & 3D Inspector - " + m_TargetActor->GetName() + "###ActorDetailWindow";

        if (ImGui::Begin(title.c_str(), &m_bIsOpen, flags)) {
            // 1. Menu Bar
            if (ImGui::BeginMenuBar()) {
                if (ImGui::MenuItem("<- Back to Main Editor")) {
                    m_bIsOpen = false;
                }

                if (m_TargetActor) {
                    ImGui::Separator();
                    ImGui::Text("Editing: %s (%s)", m_TargetActor->GetName().c_str(), m_TargetActor->Tag.c_str());

                    ImGui::Separator();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.52f, 0.28f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.65f, 0.35f, 1.0f));
                    if (ImGui::Button("Save .actor Asset (Ctrl+S)", ImVec2(170, 22))) {
                        SaveCurrentAsset();
                    }
                    ImGui::PopStyleColor(2);

                    std::filesystem::path p(m_AssociatedAssetPath);
                    ImGui::TextDisabled("File: %s", p.filename().string().c_str());

                    if (m_SaveNotificationTimer > 0.0f) {
                        ImGui::TextColored(ImVec4(0.3f, 0.95f, 0.45f, 1.0f), "[Saved to .actor!]");
                    } else if (m_bDirty) {
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.25f, 1.0f), "[Unsaved changes]");
                    }

                    if (auto coll = m_TargetActor->GetComponent<CollisionComponent>()) {
                        if (auto mesh = m_TargetActor->GetComponent<StaticMeshComponent>()) {
                            ImGui::Separator();
                            if (ImGui::MenuItem("Fit Collision to Mesh")) {
                                coll->FitToMesh(*mesh);
                                SaveCurrentAsset();
                            }
                        }
                    }
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Reset View")) {
                    m_CamYaw = 35.0f;
                    m_CamPitch = 20.0f;
                    m_CamDistance = 3.5f;
                }

                ImGui::EndMenuBar();
            }

            ImGuiIO& io = ImGui::GetIO();
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
                SaveCurrentAsset();
            }
            if (m_SaveNotificationTimer > 0.0f) {
                m_SaveNotificationTimer -= io.DeltaTime;
            }

            // 2. Fullscreen split layout: Left = 3D Scene Viewport, Right = Inspector
            ImGui::Columns(2, "ActorDetailSplit", true);
            ImGui::SetColumnWidth(0, vp->WorkSize.x * 0.65f);

            // Left Column: Interactive 3D Scene Viewport
            ImGui::BeginChild("3DViewportPanel", ImVec2(0, 0), true);
            Draw3DPreview(ImGui::GetContentRegionAvail());
            ImGui::EndChild();

            ImGui::NextColumn();

            // Right Column: Details Inspector
            ImGui::BeginChild("InspectorPanel", ImVec2(0, 0), true);
            DrawInspectorPanel();
            ImGui::EndChild();

            ImGui::Columns(1);
        }
        ImGui::End();

        if (!m_bIsOpen) {
            Close();
        }
    }

    void ActorDetailWindow::Draw3DPreview(ImVec2 canvasSize) {
        if (!m_TargetActor || canvasSize.x < 10.0f || canvasSize.y < 10.0f) return;

        ImGuiIO& io = ImGui::GetIO();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 1. Background slate gradient
        drawList->AddRectFilledMultiColor(
            canvasPos,
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
            IM_COL32(24, 26, 30, 255),
            IM_COL32(24, 26, 30, 255),
            IM_COL32(14, 15, 18, 255),
            IM_COL32(14, 15, 18, 255)
        );

        // 2. Orbit Camera Interaction
        ImGui::InvisibleButton("CanvasInteraction", canvasSize);
        bool isHovered = ImGui::IsItemHovered();
        bool isActive = ImGui::IsItemActive();

        if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            m_CamYaw += io.MouseDelta.x * 0.45f;
            m_CamPitch = std::clamp(m_CamPitch + io.MouseDelta.y * 0.45f, -85.0f, 85.0f);
        }
        if (isHovered && io.MouseWheel != 0.0f) {
            m_CamDistance = std::clamp(m_CamDistance - io.MouseWheel * 0.4f, 1.2f, 20.0f);
        }

        // 3. Setup Camera Matrices
        float radYaw = glm::radians(m_CamYaw);
        float radPitch = glm::radians(m_CamPitch);
        glm::vec3 eye(
            m_CamDistance * std::cos(radPitch) * std::sin(radYaw),
            m_CamDistance * std::sin(radPitch),
            m_CamDistance * std::cos(radPitch) * std::cos(radYaw)
        );

        glm::mat4 view = glm::lookAt(eye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), canvasSize.x / canvasSize.y, 0.1f, 100.0f);
        glm::mat4 vp = proj * view;

        auto project = [&](const glm::vec3& p3, ImVec2& out2) -> bool {
            glm::vec4 clip = vp * glm::vec4(p3, 1.0f);
            if (clip.w <= 0.01f) return false;
            glm::vec3 ndc = glm::vec3(clip) / clip.w;
            if (ndc.z < -1.0f || ndc.z > 1.0f) return false;
            out2.x = canvasPos.x + (ndc.x * 0.5f + 0.5f) * canvasSize.x;
            out2.y = canvasPos.y + (0.5f - ndc.y * 0.5f) * canvasSize.y;
            return true;
        };

        // 4. Draw Ground Grid
        const int gridHalf = 4;
        const float gridStep = 0.5f;
        for (int i = -gridHalf; i <= gridHalf; ++i) {
            ImVec2 p0, p1;
            if (project(glm::vec3(i * gridStep, -0.6f, -gridHalf * gridStep), p0) &&
                project(glm::vec3(i * gridStep, -0.6f,  gridHalf * gridStep), p1)) {
                drawList->AddLine(p0, p1, IM_COL32(50, 55, 62, 160), 1.0f);
            }
            if (project(glm::vec3(-gridHalf * gridStep, -0.6f, i * gridStep), p0) &&
                project(glm::vec3( gridHalf * gridStep, -0.6f, i * gridStep), p1)) {
                drawList->AddLine(p0, p1, IM_COL32(50, 55, 62, 160), 1.0f);
            }
        }

        // 5. Draw 3D Static Mesh
        auto meshComp = m_TargetActor->GetComponent<StaticMeshComponent>();
        if (meshComp) {
            PrimitiveType pType = meshComp->MeshType;
            ImU32 faceCol = IM_COL32(
                static_cast<int>(meshComp->BaseColor.r * 180),
                static_cast<int>(meshComp->BaseColor.g * 180),
                static_cast<int>(meshComp->BaseColor.b * 180),
                220
            );
            ImU32 edgeCol = IM_COL32(230, 230, 240, 240);

            if (pType == PrimitiveType::Cube) {
                glm::vec3 h(0.5f);
                std::array<glm::vec3, 8> corners = {
                    glm::vec3(-h.x, -h.y, -h.z), glm::vec3( h.x, -h.y, -h.z),
                    glm::vec3( h.x,  h.y, -h.z), glm::vec3(-h.x,  h.y, -h.z),
                    glm::vec3(-h.x, -h.y,  h.z), glm::vec3( h.x, -h.y,  h.z),
                    glm::vec3( h.x,  h.y,  h.z), glm::vec3(-h.x,  h.y,  h.z)
                };

                std::array<ImVec2, 8> sc{};
                bool ok = true;
                for (int i = 0; i < 8; ++i) {
                    if (!project(corners[i], sc[i])) ok = false;
                }

                if (ok) {
                    // Faces
                    auto drawQuad = [&](int a, int b, int c, int d) {
                        drawList->AddQuadFilled(sc[a], sc[b], sc[c], sc[d], faceCol);
                        drawList->AddQuad(sc[a], sc[b], sc[c], sc[d], edgeCol, 1.2f);
                    };
                    drawQuad(0, 1, 2, 3);
                    drawQuad(4, 5, 6, 7);
                    drawQuad(0, 1, 5, 4);
                    drawQuad(2, 3, 7, 6);
                    drawQuad(0, 3, 7, 4);
                    drawQuad(1, 2, 6, 5);
                }
            } else if (pType == PrimitiveType::Sphere) {
                const int segments = 16;
                float r = 0.5f;
                // Latitude and longitude rings
                for (float lat = -0.3f; lat <= 0.35f; lat += 0.3f) {
                    float ringR = r * std::cos(lat);
                    float ringY = r * std::sin(lat);
                    ImVec2 prev;
                    bool prevOk = false;
                    for (int i = 0; i <= segments; ++i) {
                        float theta = (static_cast<float>(i) / segments) * 6.28318f;
                        glm::vec3 p(ringR * std::cos(theta), ringY, ringR * std::sin(theta));
                        ImVec2 cur;
                        if (project(p, cur)) {
                            if (prevOk) drawList->AddLine(prev, cur, edgeCol, 1.2f);
                            prev = cur;
                            prevOk = true;
                        } else {
                            prevOk = false;
                        }
                    }
                }
            } else {
                // Plane or other primitive
                ImVec2 p0, p1, p2, p3;
                if (project(glm::vec3(-1, 0, -1), p0) && project(glm::vec3(1, 0, -1), p1) &&
                    project(glm::vec3(1, 0, 1), p2) && project(glm::vec3(-1, 0, 1), p3)) {
                    drawList->AddQuadFilled(p0, p1, p2, p3, faceCol);
                    drawList->AddQuad(p0, p1, p2, p3, edgeCol, 1.5f);
                }
            }
        }

        // 6. Draw Collision Bounds (Neon UE5 Green Wireframe)
        auto collComp = m_TargetActor->GetComponent<CollisionComponent>();
        if (collComp) {
            ImU32 collColor = collComp->bIsTrigger ? IM_COL32(255, 180, 0, 255) : IM_COL32(0, 255, 120, 255);
            float lineThick = 2.5f;

            if (collComp->Shape == CollisionShape::Box) {
                glm::vec3 ext = collComp->BoxHalfExtents;
                glm::vec3 off = collComp->Offset;

                std::array<glm::vec3, 8> boxPts = {
                    off + glm::vec3(-ext.x, -ext.y, -ext.z), off + glm::vec3( ext.x, -ext.y, -ext.z),
                    off + glm::vec3( ext.x,  ext.y, -ext.z), off + glm::vec3(-ext.x,  ext.y, -ext.z),
                    off + glm::vec3(-ext.x, -ext.y,  ext.z), off + glm::vec3( ext.x, -ext.y,  ext.z),
                    off + glm::vec3( ext.x,  ext.y,  ext.z), off + glm::vec3(-ext.x,  ext.y,  ext.z)
                };

                std::array<ImVec2, 8> sc{};
                bool ok = true;
                for (int i = 0; i < 8; ++i) {
                    if (!project(boxPts[i], sc[i])) ok = false;
                }

                if (ok) {
                    int edges[12][2] = {
                        {0,1}, {1,2}, {2,3}, {3,0},
                        {4,5}, {5,6}, {6,7}, {7,4},
                        {0,4}, {1,5}, {2,6}, {3,7}
                    };
                    for (auto& edge : edges) {
                        drawList->AddLine(sc[edge[0]], sc[edge[1]], collColor, lineThick);
                    }
                }
            } else {
                // Sphere Collider (3 orthogonal rings: XY, XZ, YZ)
                float r = collComp->SphereRadius;
                glm::vec3 off = collComp->Offset;
                const int segs = 32;

                auto drawRing = [&](int axis) {
                    ImVec2 prev;
                    bool prevOk = false;
                    for (int i = 0; i <= segs; ++i) {
                        float theta = (static_cast<float>(i) / segs) * 6.28318f;
                        glm::vec3 p = off;
                        if (axis == 0) p += glm::vec3(0.0f, r * std::cos(theta), r * std::sin(theta)); // YZ
                        if (axis == 1) p += glm::vec3(r * std::cos(theta), 0.0f, r * std::sin(theta)); // XZ
                        if (axis == 2) p += glm::vec3(r * std::cos(theta), r * std::sin(theta), 0.0f); // XY

                        ImVec2 cur;
                        if (project(p, cur)) {
                            if (prevOk) drawList->AddLine(prev, cur, collColor, lineThick);
                            prev = cur;
                            prevOk = true;
                        } else {
                            prevOk = false;
                        }
                    }
                };

                drawRing(0);
                drawRing(1);
                drawRing(2);
            }
        }

        // 7. Overlay HUD Info
        drawList->AddText(
            ImVec2(canvasPos.x + 15.0f, canvasPos.y + 15.0f),
            IM_COL32(220, 225, 230, 240),
            "3D Viewport: Orbit (Hold LMB & Drag) | Zoom (Mouse Wheel)"
        );

        if (collComp) {
            char collInfo[128];
            if (collComp->Shape == CollisionShape::Box) {
                std::snprintf(collInfo, sizeof(collInfo), "Collision: BOX (HalfExtents: %.2f, %.2f, %.2f)",
                    collComp->BoxHalfExtents.x, collComp->BoxHalfExtents.y, collComp->BoxHalfExtents.z);
            } else {
                std::snprintf(collInfo, sizeof(collInfo), "Collision: SPHERE (Radius: %.2f)", collComp->SphereRadius);
            }
            drawList->AddText(
                ImVec2(canvasPos.x + 15.0f, canvasPos.y + 35.0f),
                IM_COL32(0, 255, 140, 255),
                collInfo
            );
        }
    }

    void ActorDetailWindow::DrawInspectorPanel() {
        if (!m_TargetActor) return;
        bool bChanged = false;

        // 1. General Info & Tag
        if (ImGui::CollapsingHeader("General & Tag", ImGuiTreeNodeFlags_DefaultOpen)) {
            char nameBuf[128];
            std::strncpy(nameBuf, m_TargetActor->GetName().c_str(), sizeof(nameBuf) - 1);
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            if (ImGui::InputText("Actor Name", nameBuf, sizeof(nameBuf))) {
                m_TargetActor->SetName(nameBuf);
                bChanged = true;
            }

            char tagBuf[64];
            std::strncpy(tagBuf, m_TargetActor->Tag.c_str(), sizeof(tagBuf) - 1);
            tagBuf[sizeof(tagBuf) - 1] = '\0';
            if (ImGui::InputText("Tag", tagBuf, sizeof(tagBuf))) {
                m_TargetActor->Tag = tagBuf;
                bChanged = true;
            }

            ImGui::SameLine();
            if (ImGui::BeginCombo("##TagPresets", "Presets")) {
                const char* presets[] = {"Player", "Enemy", "Obstacle", "Pickup", "Trigger", "Environment"};
                for (const char* p : presets) {
                    if (ImGui::Selectable(p)) {
                        m_TargetActor->Tag = p;
                        bChanged = true;
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::Checkbox("Visible in World", &m_TargetActor->bIsVisible)) {
                bChanged = true;
            }
        }

        // 2. Transform (Location, Rotation, Scale)
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::DragFloat3("Location", &m_TargetActor->Transform.Position.x, 0.05f)) bChanged = true;
            if (ImGui::DragFloat3("Rotation", &m_TargetActor->Transform.Rotation.x, 0.5f))  bChanged = true;
            if (ImGui::DragFloat3("Scale",    &m_TargetActor->Transform.Scale.x, 0.05f, 0.01f, 100.0f)) bChanged = true;

            if (auto posComp = m_TargetActor->GetComponent<PositionComponent>()) {
                posComp->Transform = m_TargetActor->Transform;
            }
        }

        // 3. Static Mesh Component
        if (auto meshComp = m_TargetActor->GetComponent<StaticMeshComponent>()) {
            if (ImGui::CollapsingHeader("Static Mesh Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                const char* meshNames[] = {"Cube", "Sphere", "Plane", "Custom"};
                int currentMeshIndex = static_cast<int>(meshComp->MeshType);
                if (ImGui::Combo("Mesh Primitive", &currentMeshIndex, meshNames, IM_ARRAYSIZE(meshNames))) {
                    meshComp->MeshType = static_cast<PrimitiveType>(currentMeshIndex);
                    bChanged = true;
                }

                if (ImGui::ColorEdit4("Base Color", &meshComp->BaseColor.x)) bChanged = true;
                if (ImGui::SliderFloat("Metallic", &meshComp->Metallic, 0.0f, 1.0f)) bChanged = true;
                if (ImGui::SliderFloat("Roughness", &meshComp->Roughness, 0.0f, 1.0f)) bChanged = true;
                if (ImGui::Checkbox("Cast Dynamic Shadows", &meshComp->bCastShadows)) bChanged = true;
            }
        }

        // 4. Collision Component (with real-time sliders & FitToMesh)
        if (auto collComp = m_TargetActor->GetComponent<CollisionComponent>()) {
            if (ImGui::CollapsingHeader("Collision Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                const char* shapeNames[] = {"Box Collider", "Sphere Collider"};
                int currentShape = static_cast<int>(collComp->Shape);
                if (ImGui::Combo("Collider Shape", &currentShape, shapeNames, IM_ARRAYSIZE(shapeNames))) {
                    collComp->Shape = static_cast<CollisionShape>(currentShape);
                    bChanged = true;
                }

                if (collComp->Shape == CollisionShape::Box) {
                    if (ImGui::DragFloat3("Box Half Extents", &collComp->BoxHalfExtents.x, 0.05f, 0.05f, 50.0f)) bChanged = true;
                } else {
                    if (ImGui::DragFloat("Sphere Radius", &collComp->SphereRadius, 0.05f, 0.05f, 50.0f)) bChanged = true;
                }

                if (ImGui::DragFloat3("Center Offset", &collComp->Offset.x, 0.05f)) bChanged = true;
                if (ImGui::SliderFloat("Restitution (Bounce)", &collComp->Restitution, 0.0f, 1.0f)) bChanged = true;
                if (ImGui::SliderFloat("Friction", &collComp->Friction, 0.0f, 1.0f)) bChanged = true;
                if (ImGui::Checkbox("Is Trigger (Overlap Only)", &collComp->bIsTrigger)) bChanged = true;

                ImGui::Spacing();
                if (auto mesh = m_TargetActor->GetComponent<StaticMeshComponent>()) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.55f, 0.35f, 1.0f));
                    if (ImGui::Button("Fit Collision to Mesh", ImVec2(-1, 30))) {
                        collComp->FitToMesh(*mesh);
                        bChanged = true;
                    }
                    ImGui::PopStyleColor();
                }
            }
        }

        // 5. Physics Component
        if (auto physComp = m_TargetActor->GetComponent<PhysicsComponent>()) {
            if (ImGui::CollapsingHeader("Physics Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::DragFloat("Mass (kg)", &physComp->Mass, 0.1f, 0.01f, 10000.0f)) bChanged = true;
                if (ImGui::Checkbox("Is Static (Immovable)", &physComp->bIsStatic)) bChanged = true;
                if (ImGui::Checkbox("Enable Gravity", &physComp->bUseGravity)) bChanged = true;
                if (ImGui::DragFloat3("Linear Velocity", &physComp->LinearVelocity.x, 0.1f)) bChanged = true;
                if (ImGui::DragFloat3("Angular Velocity", &physComp->AngularVelocity.x, 0.1f)) bChanged = true;
            }
        }

        // Auto-save any modified settings immediately to the .actor file
        if (bChanged) {
            m_bDirty = true;
            SaveCurrentAsset();
        }
    }

} // namespace Engine
