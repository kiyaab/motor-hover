# PROJECT REDLINE — Realistic Open-World Motorcycle Game

A high-performance, realistic 3D open-world motorcycle simulation and street racing game built for **Unreal Engine 5.4 and Native C++**, set in the fictional coastal metropolis of **REDHAVEN**.

---

## 1. Game Identity & Vision

- **Title**: PROJECT REDLINE
- **Genre**: Open-World Motorcycle Action, Street Racing, Exploration & Driving Simulation
- **Platform**: Windows PC (x64)
- **Engine**: Unreal Engine 5 (UE 5.4) + Native C++ Core
- **World**: **Redhaven Metropolis** (15 Navigable Regions: Downtown Financial District, Commercial Strips, Shipping Docks, Coastal Highway, Highland Mountain Pass, Industrial Warehouse District, Desert Valley, and International Racing Circuit)
- **Campaign**: **100 Designed Story Missions** across 10 Narrative Chapters

---

## 2. Unreal Engine 5 C++ Architecture

The game architecture is built around modular Unreal Engine 5 C++ subsystems, actors, and components:

```
Source/ProjectRedline/
├── ProjectRedline.Build.cs              # UE5.4 Module Rules (ChaosVehicles, EnhancedInput, AIModule, UMG)
├── ProjectRedline.h / .cpp              # Primary Game Module Definition
├── Core/
│   └── RedlineGameMode.h / .cpp         # GameModeBase establishing Player Pawn & Controller
├── Vehicle/
│   ├── RedlineMotorcycleTypes.h         # Powertrain tuning, Pacejka tire constants, telemetry structs
│   ├── RedlineMotorcyclePhysicsComponent.h / .cpp # 2-Wheel Chaos physics, 58° lean, weight shift, wheelies & stoppies
│   ├── RedlineMotorcyclePawn.h / .cpp   # Motorcycle actor with 4-camera rig, lighting, nitro, audio triggers
│   └── RedlineRiderIKComponent.h / .cpp # Articulated rider hang-off, knee-puck touchdown sparks, handlebar IK
├── AI/
│   ├── RedlineTrafficTypes.h            # Traffic vehicle configurations (Sedan, SUV, Sports Coupe, Semi-Truck)
│   ├── RedlineTrafficVehicle.h / .cpp   # Highway lane following, safe distance, collision avoidance, honk reaction
│   └── RedlinePolicePursuitSubsystem.h / .cpp # 5-Level Heat escalation, LOS tracking, roadblock placement, search cooldown
├── Missions/
│   ├── RedlineMissionTypes.h            # Mission metadata, objectives, route checkpoints, targets, rewards
│   ├── RedlineMissionRegistry.h / .cpp  # Complete 100 Missions Database across 10 Chapters
│   └── RedlineMissionSubsystem.h / .cpp # Active mission state, split times, star ratings (1-3★), credit rewards
├── Garage/
│   └── RedlineGarageSubsystem.h / .cpp  # Motorcycle upgrades (Engine, Brakes, Tires, Nitro) & cosmetic paint/liveries
├── Environment/
│   └── RedlineWeatherManager.h / .cpp   # Day/Night 24hr cycle, dynamic sun, Lumen/Nanite presets, wet road grip
└── Save/
    └── RedlineSaveGame.h / .cpp         # Persistent career saves: credits, stars, best times, owned bikes
```

---

## 3. The 100-Missions Campaign (10 Chapters x 10 Missions)

| Chapter | Title & Setting | Permitted Class | Highlight Missions |
| :--- | :--- | :--- | :--- |
| **Chapter 1** | *The Asphalt Rookie* (Downtown District) | Commuter (250cc) | First Ignition, Redline Delivery, Rookie License Exam |
| **Chapter 2** | *Redhaven Underground* (Commercial Tunnels) | Naked (650cc) | Subway Tunnel Dash, Courier Run, Underground Crown |
| **Chapter 3** | *Harbor Speed Trials* (Shipping Container Docks) | Naked (900cc) | Salt Air Quarter Mile, Cargo Run Heist, King of the Port |
| **Chapter 4** | *Neon Syndicate* (Tokyo Neon Expressway) | Supersport (600cc) | High Voltage Squeeze, Shadow Duel: Katsu, Neon Master |
| **Chapter 5** | *Mountain Hairpins & Drift* (Highland Pass) | Supersport (600cc) | Hairpin 7, Knee Scraper, Mountain Summit Crown |
| **Chapter 6** | *High Stakes Highway Outlaws* (Coastal Highway) | Superbike (1000cc) | Coastal 300, Semi-Truck Squeeze, Highway Outlaw Legend |
| **Chapter 7** | *Desert Endurance* (Dust Valley & Farmland) | Adventure / Dual-Sport | Baja Ridge Sprint, Sunstroke Rally, Desert King |
| **Chapter 8** | *The Syndicate Betrayal* (Industrial District) | Superbike (1000cc) | Warehouse Infiltration, The Stolen Prototype, Retribution |
| **Chapter 9** | *Police Lockdown Gauntlet* (Citywide Barricades) | Superbike (1000cc) | Code Red Alert, Five Star Pursuit, Free of the Net |
| **Chapter 10** | *Grand Apex Championship* (International Circuit) | Superbike (1000cc) | Apex Qualifiers, The 200 MPH Barrier, **The 100th Milestone** |

---

## 4. 2-Wheel Physics & Real-World Kinematics

### Longitudinal Weight Transfer & Stunts
$$\Delta F_z = m \cdot a_x \cdot \left(\frac{h_{\text{CoM}}}{L_{\text{wheelbase}}}\right)$$
- **Wheelie Initiation**: When forward acceleration exceeds $1.28\,\text{G}$, front tire normal load drops to $0\,\text{N}$, lifting the front fork.
- **Stoppie Initiation**: Hard dual-disc front braking ($>0.85\,\text{G}$ deceleration) transfers total normal load to front forks, lifting the rear wheel.

### Centripetal Lean & MotoGP Cornering
$$\theta_{\text{lean}} = \arctan\left(\frac{a_{\text{lateral}}}{g}\right)$$
- Clamped dynamically to a maximum knee-down angle of **$58^\circ$**.
- Over-leaning beyond $61^\circ$ at speeds $>40\,\text{km/h}$ triggers low-side crash physics and slide recovery.

---

## 5. Law Enforcement Pursuit System (Heat 1 to 5)

- **Heat 1 (Traffic Warning)**: Single patrol unit, low aggression.
- **Heat 2 (Local Patrol Pursuit)**: Two squad cars, aggressive pursuit, radio dispatch.
- **Heat 3 (Multi-Unit Interceptors)**: High-speed police interceptors, tactical boxing.
- **Heat 4 (Roadblocks & Spike Strips)**: Tactical barriers across highway bridges and intersections.
- **Heat 5 (Citywide Lockdown)**: Helicopter spotlight, tactical interceptors, full cordon.

---

## 6. How to Build & Run

### A. Run Standalone Automated C++ Verification Suite
The project contains an automated verification harness that compiles and validates all physics math, the 100 missions database, and police escalation logic without booting Unreal Engine:

```powershell
# Compile with C++20
& g++.exe -std=c++20 -O2 .\Tests\ProjectRedlineHarness.cpp -o .\Tests\ProjectRedlineHarness.exe

# Execute tests (12 / 12 validation tests)
.\Tests\ProjectRedlineHarness.exe
```

### B. Play Instant 3D Interactive Web Simulation
A complete, real-time 3D simulation with procedural highway, articulated rider, 100 missions career selector, 18-wheeler traffic, and audio synthesizer is ready to run immediately:

```powershell
python -m http.server 8080 --directory ".\Simulation"
```
Then visit: **[http://localhost:8080](http://localhost:8080)**

### C. Launch in Unreal Engine 5.4 Editor
1. Right-click [ProjectRedline.uproject](file:///d:/New%20project's/GAME%20DEV/ProjectRedline.uproject) and click **"Generate Visual Studio project files"**.
2. Open `ProjectRedline.sln` in Visual Studio 2022 (with *Game Development with C++* workload).
3. Set build configuration to **Development Editor** / **Win64**.
4. Press **F5** to build and launch into the Unreal Engine 5.4 Editor.
5. In the viewport, press **Play in Editor (Alt + P)**.

---

## 7. Controls Summary

| Input | Action |
| :--- | :--- |
| **`W` / `Up Arrow`** | Throttle (Accelerate / Wheelie) |
| **`S` / `Down Arrow`** | Front Brake (Dual Brembo Discs / Stoppie) |
| **`A` / `D`** | Counter-Steer & Dynamic Lean (Up to 58°) |
| **`Shift` / `N`** | Nitro Boost (Exhaust Flames & Speed Blur) |
| **`C`** | Cycle 4 Cameras (Chase / Cockpit FPV / Knee-Down / Drone) |
| **`E`** | Cycle Environment (Sunny Highway / Sunset / Tokyo Night) |
| **`H`** | Honk Horn at Traffic Cars |
| **`V`** | Garage / Switch Motorcycle |
| **`P` / `Esc`** | 100 Missions Career Menu |
| **`R`** | Recover Motorcycle / Reset |
