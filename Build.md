# Build Guide

## Requirements

### Build Requirements

* **CMake**
* **C++20-compatible compiler**
* Supported platforms:

  * Windows 10 / Windows 11 (64-bit x86)
  * Linux Mint (64-bit x86)

### Runtime Requirements

* **Vulkan-compatible GPU or integrated GPU**
* **Vulkan 1.0 drivers**

---

## Build Instructions

1. **Clone the repository (including submodules):**

   ```bash
   git clone --recursive https://github.com/Julian45398/SimpleVulkanEngine.git
   ```

2. **Navigate to the repository directory:**

   ```bash
   cd SimpleVulkanEngine/
   ```

3. **Create a build directory:**

   ```bash
   mkdir build
   cd build
   ```

4. **Generate build files using CMake:**

   ```bash
   cmake ..
   ```

5. **Build the project:**

   ```bash
   cmake --build .
   ```

---

## Running the Application

After a successful build, navigate to the directory containing the generated executable and run it:

```bash
./Level-Editor
```

(On Windows, run the `.exe` file directly.)

---

## Notes

* Ensure your system has up-to-date Vulkan drivers installed.
* If you encounter compiler issues, verify that your compiler fully supports the C++20 standard.
* Ensure you are running the application from the executable directory else it may fail to load the required shader files
