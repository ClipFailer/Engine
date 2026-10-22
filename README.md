# Vulkan 3D Game Engine & Editor

Современный высокопроизводительный трёхмерный игровой движок и полноценная среда разработки (IDE/Editor) на **C++20** и **Vulkan 1.4**, спроектированные в архитектурном стиле **Unreal Engine 5**.

Движок включает в себя компонентную архитектуру экторов (ECS), физический мир с коллизиями, рендерер с микрофасетным PBR и ACES Filmic Tone Mapping, программируемые шейдерные триггер-зоны (Shader Volumes), систему загрузки 3D моделей (.obj) и текстур, трёхмерное позиционированное аудио (miniaudio), движок HTML+CSS UI для интерфейсов, встроенный редактор кода, Project Hub и панель настроек графики.

---

## 🌟 Ключевые возможности движка

### 1. Рендеринг и Графика (Vulkan 1.4)
- **Физически корректный рендеринг (PBR)**:
  - Микрофасетное распределение нормалей **Cook-Torrance GGX**.
  - Геометрическое затенение **Schlick-GGX** и аппроксимация Френеля **Fresnel-Schlick**.
  - Полусферическое атмосферное рассеяние (Sky & Ground irradiance).
- **Динамическое освещение**:
  - **Directional Light** — солнце с настраиваемым направлением и интенсивностью.
  - **Point Light** — точечные источники света с квадратичным затуханием по радиусу.
  - **Spot Light** — прожекторы с внутренним (`InnerConeAngle`) и внешним (`OuterConeAngle`) углами конуса.
- **Пост-процессинг и Tone Mapping**:
  - Тонокомпенсация: **ACES Filmic** (стандарт UE5), **Reinhard**, **Neutral**, **AgX**.
  - **Screen-Space Ambient Occlusion (SSAO)** с настраиваемой силой затенения.
  - **HDR Bloom** с регулировкой порога яркости и интенсивности свечения.
  - Управление экспозицией (`Exposure`), гаммой (`Gamma`) и масштабом разрешения (`ResolutionScale`).
- **Сглаживание (Anti-Aliasing)**:
  - Режимы: Off, FXAA, MSAA 2x, MSAA 4x.
- **Оффскрин-рендеринг во вьюпорт**:
  - Кадры рендерятся в `VkFramebuffer` с `VK_FORMAT_R8G8B8A8_UNORM` и глубиной `VK_FORMAT_D32_SFLOAT`, после чего выводятся во вьюпорт ImGui с нулевой задержкой.

### 2. Шейдерные триггер-зоны (Shader Volumes)
- **Shader Volume Actor** — невидимый на уровне объект, содержащий `ShaderComponent`.
- **Зоны активации**:
  - Сферический триггер (`TriggerRadius`) или параллелепипед (`TriggerHalfExtents`) со смещением центра.
  - При входе камеры в зону шейдерный эффект плавно активируется.
  - Тогл **`Active Everywhere (Scene-Wide)`** распространяет действие шейдера на всю сцену.
- **Встроенная библиотека пресетов**:
  - `Vignette` — кинематографическое затемнение краев экрана.
  - `NightVision` — прибор ночного видения с зеленым фосфором, усилением яркости и строчной разверткой.
  - `Grayscale` — черно-белое стилизованное изображение.
  - `Sepia` — теплое ретро-тонирование.
  - `Invert` — инверсия цветов (негатив).
  - `Custom / Cyberpunk Tint` — смещение цветовой палитры.
- Редактирование GLSL фрагментного кода прямо в панели **Details** с кнопкой возврата к эталонному шейдеру.

### 3. Загрузка 3D моделей (.obj) и текстур
- Встроенный оптимизированный парсер **Wavefront OBJ** (`MeshLoader`):
  - Автоматическая триангуляция граней полигонов (triangle fan).
  - Вычисление нормалей вершин при их отсутствии в файле.
  - Поддержка текстурных координат (UV) и сглаживания.
- Динамическая загрузка и кэширование GPU-буферов (`GPUMesh`) для импортированных моделей.
- Поддержка форматов изображений (`.png`, `.jpg`, `.jpeg`, `.bmp`, `.tga`).
- Drag-and-Drop текстур и 3D моделей:
  - Из Content Browser прямо на объекты во вьюпорте.
  - В слоты `ModelPath` и `TexturePath` панели Details.

### 4. Архитектура Actor & Component (ECS)
Иерархия игровых сущностей:
- **`Actor`** — базовый объект сцены с пространственной трансформацией (`Transform`: Position, Rotation, Scale), тегами и уникальным UUID.
- **Компоненты**:
  - **`PositionComponent`** — привязка к координатам в мире.
  - **`StaticMeshComponent`** — процедурные примитивы (Cube, Sphere, Plane) или загруженные 3D модели (.obj) с материалами (BaseColor, Metallic, Roughness).
  - **`CollisionComponent`** — физические коллайдеры (Box, Sphere, Trigger), редактирование размеров в стиле Unity, кнопка «Fit to Mesh Geometry».
  - **`PhysicsComponent`** / **`RigidBodyComponent`** — масса, гравитация, аэродинамическое демпфирование, коэффициент упругости (Restitution) и трения (Friction).
  - **`LightComponent`** — источники света (Directional, Point, Spot).
  - **`ShaderComponent`** — триггерные объемы и полноэкранный GLSL пост-процессинг.
  - **`MovementComponent`** — кинематическое перемещение, ускорение, торможение.
  - **`WidgetComponent`** — встроенный HTML+CSS интерфейс (3D в пространстве мира или 2D Screen-Space HUD).
  - **`AudioComponent`** — трёхмерные звуки с пространственным затуханием.
  - **`SkeletalMeshComponent`** — скелетная анимация костей и таймлайн ключевых кадров.
  - **`CustomComponent`** — пользовательские компоненты геймплея на C++.

### 5. Физический движок (Physics World)
- Пространственная дискретизация и обработка столкновений тел «Actor-to-Actor».
- Алгоритм SAT / AABB / Sphere-Sphere пересечений с выталкиванием по вектору минимального проникновения.
- Аэродинамическое сопротивление воздуха (Linear & Angular Damping).
- Механизм стабилизации покоящихся объектов (Sleep Threshold) для устранения микро-дрожания при контактах нескольких тел.
- Трассировка лучей (Raycasting) для выбора объектов кликом мыши во вьюпорте.

### 6. Редактор (Editor IDE)
- **3D Вьюпорт**:
  - Навигация Fly-Camera: зажатая ПКМ + WASD + QE.
  - Фокусировка камеры по клавише **`F`** на выбранном объекте (Unreal-style).
  - Сброс выделения кликом ЛКМ в пустое пространство.
  - Горячие клавиши трансформации: **`W`** (Translate), **`E`** (Rotate), **`R`** (Scale).
  - Дублирование объектов по **`Ctrl+D`** со смещением и сохранением всех компонентов.
  - Удаление объекта по клавише **`Delete`**.
- **3D Визуализаторы во вьюпорте (Wireframes)**:
  - Зеленый каркас коллайдеров (`CollisionComponent`).
  - Оранжевый каркас триггерных объемов (`ShaderComponent`).
  - Золотые сферы и конусы радиуса действия источников света (`LightComponent`).
- **Play in Editor (PIE)**:
  - Кнопки **Play**, **Pause**, **Stop** на верхней панели.
  - При нажатии Play запускается симуляция физики и вызывается `BeginPlay()` / `Tick(dt)`.
  - При нажатии Stop сцена автоматически возвращается в исходное состояние с откатом координат и скоростей.
- **Content Browser**:
  - Дерево каталогов и плиточная сетка файлов.
  - Перетаскивание файлов (Drag & Drop) в папки и на уровень выше.
  - Цветовая дифференциация типов файлов (.actor, .cpp, .hpp, .obj, .png, .ui, .scene).
  - Контекстное меню ПКМ: создание блюпринтов (.actor), C++ классов, UI-виджетов, папок.
- **Встроенный редактор кода и интеграция с VS Code**:
  - Открытие файлов .cpp/.hpp/.h двойным кликом во внутреннем редакторе или в VS Code.
- **Project Hub**:
  - Создание новых проектов, выбор шаблонов, история недавних проектов.

---

## 📁 Структура проекта

```
Engine/
├── cmake/                      # Модули CMake (Dependencies.cmake)
├── external/                   # Сторонние библиотеки (miniaudio, stb_image)
├── shaders/                    # Исходные GLSL и скомпилированные SPIR-V шейдеры
│   ├── mesh.vert               # Вершинный шейдер трансформаций и света
│   └── mesh.frag               # PBR фрагментный шейдер + ToneMapping + ShaderVolumes
├── src/
│   ├── animation/              # Скелетная анимация и ключевые кадры
│   ├── audio/                  # Звуковой движок (miniaudio wrapper)
│   ├── core/                   # Базовые типы, UUID, логирование, математика
│   ├── editor/                 # Панели редактора (Viewport, Outliner, Details, ContentBrowser)
│   ├── module/                 # Модульная подсистема (IModule, ModuleManager)
│   ├── physics/                # Физический мир, гравитация, импульсы, коллизии
│   ├── project/                # Управление проектами и генератор C++ классов
│   ├── reflection/             # Система C++ метаданных и рантайм-рефлексии
│   ├── rhi/                    # Vulkan 1.4 Hardware Interface (Renderer, Buffer, Context)
│   ├── scene/                  # Экторы, компоненты, камера, загрузчик OBJ
│   └── ui/                     # Движок HTML+CSS UI документов
├── tests/                      # Автоматизированный тестовый набор (EngineTests.cpp)
├── CMakeLists.txt              # Конфигурация сборки CMake
├── Doxyfile                    # Конфигурация документации Doxygen
└── README.md                   # Главная документация проекта
```

---

## 🛠️ Сборка и Запуск

### Системные требования:
- **ОС**: Windows 10/11 x64
- **Компилятор**: Clang 18+ (LLVM) или MSVC 2022 с поддержкой **C++20**
- **Vulkan SDK**: 1.3+ или 1.4+ (установлен в систему)
- **Инструменты**: CMake 3.22+, Ninja

### Команды для сборки:

1. **Генерация проекта через CMake и Ninja**:
   ```powershell
   cmake -B build -G Ninja
   ```

2. **Компиляция движка и тестов**:
   ```powershell
   cmake --build build
   ```

3. **Запуск автоматических тестов**:
   ```powershell
   .\build\bin\EngineTests.exe
   ```
   *(Все 18 юнит- и интеграционных тестов должны пройти успешно)*.

4. **Запуск Редактора (Engine Editor)**:
   ```powershell
   .\build\bin\EngineEditor.exe
   ```

---

## 🎮 Руководство по разработке (Quick Start)

### 1. Создание движения для Куба (C++ Компонент)

1. В **Content Browser** нажмите ПКМ -> **`Create Class...`** -> выберите **`Component`**, имя: `MoveUpComponent`.
2. В созданных файлах реализуйте логику в методе `Tick`:

**`MoveUpComponent.hpp`:**
```cpp
#pragma once
#include "scene/Component.hpp"

class MoveUpComponent : public Engine::Component {
    GENERATED_BODY()
public:
    MoveUpComponent();
    void BeginPlay() override;
    void Tick(float deltaTime) override;

    float Speed{3.0f};      ///< Скорость подъёма (м/с)
    float MaxHeight{12.0f}; ///< Предел высоты
    bool bIsEnabled{true};
};
```

**`MoveUpComponent.cpp`:**
```cpp
#include "MoveUpComponent.hpp"
#include "scene/Actor.hpp"
#include "core/Log.hpp"
#include <cmath>

MoveUpComponent::MoveUpComponent() : Engine::Component("MoveUpComponent") {}

void MoveUpComponent::BeginPlay() {
    Engine::Component::BeginPlay();
    LOG_INFO("MoveUpComponent активирован на {}", GetOwner() ? GetOwner()->GetName() : "None");
}

void MoveUpComponent::Tick(float deltaTime) {
    Engine::Component::Tick(deltaTime);
    if (!bIsEnabled || !GetOwner()) return;

    // Подъём по оси Y:
    GetOwner()->Transform.Position.y += Speed * deltaTime;

    // Реверс при достижении максимальной высоты:
    if (GetOwner()->Transform.Position.y >= MaxHeight) {
        Speed = -std::abs(Speed);
    } else if (GetOwner()->Transform.Position.y <= 1.0f) {
        Speed = std::abs(Speed);
    }
}

ENGINE_CLASS_BEGIN(MoveUpComponent, Component)
    ENGINE_PROPERTY(MoveUpComponent, Speed, Float, "Movement", "Скорость подъёма")
    ENGINE_PROPERTY(MoveUpComponent, MaxHeight, Float, "Movement", "Максимальная высота")
    ENGINE_PROPERTY(MoveUpComponent, bIsEnabled, Bool, "Movement", "Активность компонента")
ENGINE_CLASS_END()
```

3. Выделите куб на сцене, в панели **Details** нажмите **`+ Add Component`** -> **`Custom Components`** -> **`MoveUpComponent`**.
4. Нажмите **Play** на панели инструментов — куб начнёт двигаться вверх.

---

### 2. Использование шейдерного объекта (Shader Volume)

1. В панели **Outliner** нажмите ПКМ -> **`Add Actor`** -> **`Shader Volume (Trigger / Post-Process)`**.
2. В панели **Details**:
   - Выберите пресет в списке **Visual Effect** (`NightVision`, `Vignette`, `Sepia`, `Invert` и др.).
   - Поставьте галочку **`Active Everywhere (Scene-Wide)`** — эффект мгновенно включится для всего вьюпорта!
   - Или отключите её и настройте радиус триггера `Trigger Radius`: эффект будет активироваться только при приближении камеры.

---

### 3. Горячие клавиши в 3D Viewport

| Горячая клавиша | Действие |
| :--- | :--- |
| **ПКМ + WASD** | Полет камеры во вьюпорте (Fly Camera) |
| **ПКМ + Q / E** | Спуск / Подъём камеры по вертикали |
| **F** | Фокусировка камеры на выбранном объекте (Unreal Engine style) |
| **ЛКМ по пустому месту** | Сброс выделения всех объектов |
| **W** | Режим перемещения (Translate Gizmo) |
| **E** | Режим вращения (Rotate Gizmo) |
| **R** | Режим масштабирования (Scale Gizmo) |
| **Ctrl + D** | Дублировать выбранный объект со всеми компонентами |
| **Delete** | Удалить выбранный объект со сцены |
| **Колесо мыши** | Приближение / удаление камеры (Zoom) |

---

## 🧪 Автоматизированное тестирование

В состав проекта входит тестовый сьют `EngineTests.exe` (18 тестов), покрывающий все критические подсистемы:
- Уникальность UUID и хеширование
- Векторная алгебра, матрицы и пространственные преобразования
- Рантайм-рефлексия C++ и сериализация свойств
- Иерархия сцены, экторов и компонентов
- Физические коллизии и алгоритм подгонки границ под меш (Fit To Mesh)
- Модульная система и HTML+CSS UI
- Скелетная анимация костей
- Процедурные 3D примитивы
- Генератор C++ кода классов проектов
- Сохранение и восстановление координат сцены
- Физическое разрешение столкновений тел
- Сохранение и спавн блюпринтов `.actor`
- Парсинг 3D Wavefront OBJ моделей и привязка текстур
- Триггерные шейдерные объемы и режим `activeEverywhere`
- Фокусировка камеры и пресеты настроек рендера
- Прожекторы (Spot Light) и дублирование экторов (`Ctrl+D`)

Для запуска тестов выполните:
```powershell
.\build\bin\EngineTests.exe
```

---

## 📜 Лицензия
Движок разработан в образовательных и исследовательских целях. Свободен для модификации и создания коммерческих и некоммерческих игр.
