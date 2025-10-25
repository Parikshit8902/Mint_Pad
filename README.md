# Mint_Pad: A Minimalist Code Editor for Linux

[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen)](https://github.com/your-username/mint-pad) [![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT) 

Mint_Pad is a lightweight, clean, and simple code editor built specifically for Linux systems using C++ and the GTKmm 3.0 toolkit. It provides essential features for writing and running C, C++, and Python code without the bloat of a full-fledged IDE. It is a personal project, created to ease the process of writing and running code from the same user space.

---

## ✨ Features

* **Multi-Language Support:** Write and run code in **C**, **C++**, and **Python**. The editor automatically detects language from file extensions (`.c`, `.cpp`, `.py`) when opening files and applies appropriate syntax highlighting.
* **Syntax Highlighting:** Leverages `GtkSourceView` for clear and accurate highlighting of keywords, strings, comments, and more for supported languages.
* **Multiple Tabs:** Open and manage multiple files simultaneously using a tabbed interface.
* **Core Editor Functionality:**
    * Line Numbers
    * Auto Indentation 
    * Bracket Matching Highlight
* **File Management:**
    * **New:** Create new, empty files in separate tabs.
    * **Open:** Open existing code files.
    * **Save:** Save changes to the current file.
    * **Save As:** Save the current file to a new location or with a new name (suggests appropriate file extension).
    * **Unsaved Changes Indicator:** An asterisk (`*`) appears on tabs and the window title for modified files.
    * **Save Prompts:** The editor prompts you to save unsaved changes before closing a tab or quitting the application.
* **Build & Run:**
    * Simple "Run" button to compile (C/C++) or interpret (Python) the code in the current tab.
    * Execution occurs in a **separate `gnome-terminal` window**.
    * The terminal automatically pauses after execution until you press Enter.
* **Customizable Interface:**
    * **Light/Dark Theme:** Toggle between a default light theme and a custom dark theme via the File menu.
    * **Font Preferences:** Choose your preferred editor font and size via the Preferences dialogue.
    * **Status Bar:** Displays the current cursor line and column number.
* **Minimalist Design:** Built with `GTKmm 3.0` for a native Linux look and feel.

---
## 📁 Files

* **main.cpp** - Main program file.
* **dark.css** - CSS for enabling dark theme.
* **Makefile** - For building the project.
---
## 🔧 Dependencies

Mint_Pad requires the following libraries and tools to be installed:

* **GTKmm 3.0** (C++ bindings for GTK 3) - `libgtkmm-3.0-dev`
* **GtkSourceViewmm 3.0** (C++ bindings for GtkSourceView 3) - `libgtksourceviewmm-3.0-dev`
* **GCC/G++** (C/C++ Compiler) - `build-essential` or `gcc-c++`
* **Python 3** (Interpreter) - `python3`
* **pkg-config** (Library helper) - `pkg-config` or `pkgconf-pkg-config`

---

## 🔨 Currently working on...
* **Dark Theme** - The current line remains white even in dark theme, most probably due to GTK native theme overriding CSS rules.
* **Multiple Themes** - Various colours depending upon the type of project (cool, warm, energetic, etc.)
* **Adding support for other languages** - Rust, Java, Javascript, HTML...
---


## ⚙️ Installation & Building

1.  **Clone the repository.**

2.  **Install Dependencies:**

    * **Debian / Ubuntu / Linux Mint:**
        ```bash
        sudo apt update
        sudo apt install libgtkmm-3.0-dev libgtksourceviewmm-3.0-dev build-essential python3 pkg-config
        ```
    * **Fedora / CentOS / RHEL:**
        ```bash
        sudo dnf install gtkmm30-devel gtksourceviewmm3-devel gcc-c++ python3 pkgconf-pkg-config
        ```
    * **Arch Linux:**
        ```bash
        sudo pacman -Syu gtkmm3 gtksourceviewmm3 base-devel python pkgconf
        ```

3.  **Build the IDE:**
    Navigate to the project directory in your terminal and run `make`:
    ```bash
    make
    ```
    This will compile the `main.cpp` file and create an executable named `mint_pad`.
    If you've made any changes in the main.cpp, dark.css or Makefile after running it initially, make sure to run `make clean` first, and then run `make`.
    

---

## ▶️ Running

After building, run the IDE from the project directory:

```bash
./mint_pad
