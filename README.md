# 3D Scene Manager

## Overview

This project is a 3D scene manager developed for the CS-330 Computational Graphics and Visualization course. It includes functionalities for loading textures, managing shaders, setting up lighting, and rendering various 3D objects.

## Features

- **Texture Management**: Load and bind textures to OpenGL texture memory slots.
- **Shader Management**: Set shader parameters for rendering.
- **Lighting Setup**: Configure multiple light sources for the 3D scene.
- **Object Rendering**: Render various 3D objects like tables, balls, walls, windows, laptops, and coffee mugs.
- **Camera Control**: Interact with the 3D scene using keyboard and mouse inputs.

## Dependencies

- **GLFW**: For creating windows and handling input.
- **GLM**: For mathematical operations on vectors and matrices.
- **stb_image**: For loading images.

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/AlanChumsawang/3D-Scene-Manager.git
    ```
2. Navigate to the project directory:
    ```sh
    cd 3D-Scene-Manager
    ```
3. Build the project using your preferred IDE (e.g., CLion).

## Usage

1. Run the executable generated after building the project.
2. Use the following controls to interact with the 3D scene:
    - `W`, `A`, `S`, `D`: Move the camera forward, left, backward, and right.
    - `Q`, `E`: Move the camera up and down.
    - Mouse: Look around.
    - `1`, `2`, `3`: Switch between different orthographic views.
    - `4`: Switch to perspective view.

## File Structure

- `Source/SceneManager.h` and `Source/SceneManager.cpp`: Manage the preparation and rendering of 3D scenes.
- `Source/ViewManager.h` and `Source/ViewManager.cpp`: Handle the creation of the display window and camera controls.
- `Source/ShaderManager.h` and `Source/ShaderManager.cpp`: Manage the loading and setting of shader code.
- `Source/ShapeMeshes.h` and `Source/ShapeMeshes.cpp`: Load and draw basic 3D shapes.

## License

This project is created for educational purposes and is not intended for commercial use.

## Author

- **Alan Chumsawang** - Created for CS-330 Computational Graphics and Visualization, Oct. 21st, 2024.