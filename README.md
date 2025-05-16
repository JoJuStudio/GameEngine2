## Nintendo Switch Homebrew 3‑D Game Engine

Lightweight, component-based 3D engine targeting **Nintendo Switch homebrew**, built with devkitPro and libnx.
Supports OpenGL rendering, GLTF asset loading, and modular components.

---

### Step 1 – Get Your Switch's IP Address

Before running `make setup`, you need to get your Switch's **local IP**:

1. Boot your Switch and launch the **Homebrew Menu in app mode** (not via Album!)

   > Tip: Use title override with a game like YouTube or a dummy NSP to launch full app mode.
2. In the Homebrew Menu, press **`Y`** to activate **NetLoader**
3. The screen will show your IP address (e.g. `192.168.0.42`)

**Write this IP down** — you’ll need it in the next step.

---

### Step 2 – Set Up the Environment

Run the setup process once:

```bash
make setup
```

This will:

* Prompt you for your **Switch IP** and store it in `.env`
* Install all **Switch libraries** via `dkp-pacman`
* Initialize required submodules like `tinygltf`, `stb`

> `.env` file will look like:
>
> ```
> IP=192.168.0.42
> ```

---

### 🚀 Step 3 – Build and Send to the Switch

```bash
make -j$(nproc)   # build the project
make send         # send it via nxlink to the IP in .env
```

The `.nro` will be transferred over the network using the stored IP.

---

### Requirements

* devkitPro toolchain:

  ```bash
  sudo dkp-pacman -Syu
  sudo dkp-pacman -S switch-dev
  ```
* C++17 compiler (included in devkitA64)
* `make setup` installs all remaining dependencies

---

## Thanks

* **devkitPro & libnx** — toolchain and Switch SDK
* **switchbrew** — for documentation and hardware knowledge
* **tinygltf** — GLTF loading
* **stb** — header-only image loaders
* **Homebrew contributors** — for testing and improvements

Thanks to everyone making Nintendo Switch homebrew possible!


