# FKS (File Koding System)

**FKS** is a lightweight, dual-purpose utility for Linux (Debian/Ubuntu/Kali) that combines a stylish system information display (fetch) with an enhanced wrapper for the `nano` text editor.

![FKS Screenshot]([https://via.placeholder.com/800x400?text=FKS+Screenshot+Here](https://i.postimg.cc/hvLnTjJy/Capture-d-ecran-du-2026-01-10-19-45-37.png))  
*(Replace with a real screenshot of your terminal!)*

## Features

### 🖥️ System Fetch
When run without arguments, `fks` displays detailed system information alongside custom ASCII art, similar to `neofetch` or `fastfetch`.

**Displayed Information:**
-   **User & Hostname**
-   **OS & Kernel**
-   **Uptime**
-   **Package Counts** (dpkg, flatpak, snap)
-   **Shell** (with version)
-   **Desktop Environment & Window Manager**
-   **Hardware Stats**: CPU (Model, Cores, Frequency), GPU, RAM Usage, Disk Usage
-   **Network**: Local IP & Interface

### 📝 Enhanced Editor
When run with a file argument (`fks myfile.cpp`), it launches `nano` with pre-configured developer-friendly flags:
-   **Line Numbers** (`-l`)
-   **Auto-Indentation** (`-i`)
-   **Mouse Support** (`-m`)
-   **Constant Cursor Position** (`-c`)
-   **Syntax Highlighting** (Inherits your system's `nano` config)

---

## Installation

### From .deb Package
Download the latest `fks.deb` release and install it:

```bash
sudo dpkg -i fks.deb
sudo apt-get install -f  # Fix dependencies if needed
```

### Build from Source

**Dependencies:**
-   `g++`
-   `make`
-   `nano`

**Build & Install:**
```bash
# Clone the repository
git clone https://github.com/yourusername/fks.git
cd fks

# Compile
make

# Install (Standard)
sudo make install

# Or Build .deb Package
mkdir -p package/DEBIAN
# ... ensure control file is in package/DEBIAN/ ...
dpkg-deb --build package fks.deb
```

## Usage

**Show System Info:**
```bash
fks
```

**Edit a File:**
```bash
fks main.cpp
```

## Configuration
The ASCII art is stored in `/usr/share/fks/ascii.txt`. You can modify this file to change the splash logo.

