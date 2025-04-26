## Nintendo Switch Homebrew 3‑D Game Engine

Lightweight, component‑based engine that targets **Nintendo Switch homebrew** using devkitPro & libnx.

---
### Quick Build (Switch only)
```bash
# Make sure devkitPro is installed and DEVKITPRO is exported
export DEVKITPRO=/opt/devkitpro   # adjust to your path

# Build the .nro
make -j$(nproc)
```
The build produces `GameEngine2.nro`, `GameEngine2.elf`, and supporting files in the project root.

---
### Requirements
* **devkitPro tool‑chain** (devkitA64, libnx, switch‑rules) – install via pacman: `sudo dkp-pacman -S switch-dev`
* A modern C++17 compiler (provided by devkitPro)



## Acknowledgements
* **devkitPro & libnx teams** – for the Switch SDK, pacman repositories, and the invaluable *switch‑examples* sample code.
* **switchbrew community** – documentation and continual reverse‑engineering efforts.
* **Homebrew testers & contributors** – pull requests, feedback, and bug reports.

Thank you all for making Switch homebrew possible!

