GPLv3# 3D Game Engine – Project Scope Document

## 1  Project Title
**Cross‑Platform Homebrew 3D Game Engine** (Nintendo Switch & Linux)

---
## 2  Purpose & Background
The project will deliver a lightweight, component‑based 3‑D game engine that targets **Nintendo Switch homebrew** first and desktop **Linux** second.  It bridges modern C++17, Bullet Physics, OpenGL 4.x, SDL2 audio/input, and ROMFS packaging so developers can prototype or ship small‑to‑medium‑scale 3‑D titles on Switch without port‑by‑port rewrites.

---
## 3  Goals & Objectives
| ID | Goal | Measurable Objective |
|----|------|----------------------|
| G1 | Functional core | Render 1k textured cubes at 60 fps on retail Switch with vsync off < 33 ms ‑ CPU frame time |
| G2 | Physics layer | Integrate Bullet so rigid‑body demo runs deterministically on both platforms |
| G3 | Audio | Play & pan 32 simultaneous OGG sounds with positional falloff |
| G4 | Tooling | `make` ⇒ `.nro` (incl. ROMFS) in ≤ 45 s on 8‑thread host ; `make PLATFORM=linux` runs same demo |
| G5 | Documentation | 100 % public headers have doxygen comments ; quick‑start guide builds in 10 min |

---
## 4  Major Deliverables
1. **Source tree** in `source/` (see structure below) with build scripts for both targets.
2. **Static library** `libengine.a` plus sample game `demo/`
3. **Switch .nro + Linux ELF** binaries for the sample game
4. **Developer documentation** – wiki & generated API docs
5. **Getting‑started PDF** (≤ 10 pages)
6. **Automated CI pipeline** (GitHub Actions) building both targets on push

---
## 5  In‑Scope Features
* Scene‑graph & component system (transform, camera, script component)
* OpenGL renderer supporting:
  * Static meshes (.obj) & textures (PNG, DDS, KTX)
  * Phong lighting (ambient, directional, point, spot)
  * Basic material system with uniform parameters
* Bullet Physics – Rigid bodies, convex/concave colliders, collision callbacks
* SDL2‑based spatial audio (mono/stereo wav/ogg), channel mixing, distance attenuation
* Unified input abstraction (Joy‑Con, Pro Controller, keyboard, mouse, standard gamepads)
* Asset pipeline & packaging: ROMFS for Switch, file‑system for Linux
* Build + cross‑compile workflows (Makefile, optional CMake for Linux)

---
## 6  Out‑of‑Scope Items
* Networking / multiplayer functionality
* Editor or GUI tools beyond console test harness
* Scripting languages (Lua, Python, etc.) – future extensions only
* Post‑processing effects beyond basic gamma‑correct HDR tonemap
* Windows or macOS platform ports

---
## 7  Assumptions
* End‑users possess a legally obtained Nintendo Switch capable of running homebrew (.nro)
* devkitPro toolchain is installed on contributor machines
* Project will remain open‑source under the GPLv3 License
* Target frame budget on Switch: CPU ≤ 10 ms, GPU ≤ 16 ms (60 fps base)

---
## 8  Constraints
* Switch homebrew runs in userland; cannot access NVN APIs – limited to OpenGL wrapper
* Memory budgets: 3 GB shared RAM; engine must be usable with ≤ 512 MB working set
* Project timeline limited to two part‑time developers and one designer (~25 h/week total)

---
## 9  Stakeholders & Roles
| Stakeholder | Role | Responsibility |
|-------------|------|----------------|
| Product Owner | Sets vision & priority, accepts milestones |
| Tech Lead | Architecture, code reviews, merges |
| Developer(s) | Feature implementation, unit tests |
| QA / Tester | Smoke tests on retail Switch & Linux distros |
| Community | Provide feedback, pull requests |

---
## 10  High‑Level Timeline & Milestones
| Phase | Duration | Key Outputs |
|-------|----------|-------------|
| P0 – Toolchain Setup | 1 wk | devkitPro & CI scripts operational |
| P1 – Core & Renderer | 3 wks | Scene graph, render triangle on both platforms |
| P2 – Physics | 2 wks | Bullet integrated; physics testbed |
| P3 – Audio & Input | 2 wks | Play 3‑D sounds; bind Joy‑Con & keyboard |
| P4 – Sample Game | 3 wks | Demo level with physics, audio, lighting |
| P5 – Docs & Packaging | 1 wk | Quick‑start guide, auto‑ROMFS build |
| P6 – Buffer / Hardening | 2 wks | Performance pass, bug fixes |

_Total planned effort: 14 weeks (April → July 2025)_

---
## 11  Success / Acceptance Criteria
* **Build reproducibility**: `git clone && ./build.sh switch` succeeds on fresh Ubuntu 22 image
* **Performance**: Demo level ≥ 60 fps on overclocked retail Switch, ≥ 120 fps on mid‑tier PC
* **Portability**: Same source compiles for both targets without `#ifdef` outside `platform/`
* **Docs**: New dev can render the sample within 1 hour of reading the guide

---
## 12  Key Risks & Mitigations
| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|-----------|
| Bullet compile issues on Switch | Medium | Schedule slip | Use devkitPro port; pin version early |
| OpenGL driver quirks | High | Rendering blockers | Maintain minimal shader path; fallback debug renderer |
| Audio latency on SDL2 | Low | User experience | Pre‑mix common sounds; tune buffer sizes |
| Scarce tester hardware | Medium | Missed regressions | Recruit community testers, set up nightly beta builds |

---
## 13  Approval
*Sign‑off indicates agreement that the scope, deliverables, and constraints are understood and accepted.*

| Name | Position | Signature | Date |
|------|----------|-----------|------|
|  |  |  |  |


