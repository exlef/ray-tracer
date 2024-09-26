# Simple Raytracer
This project is a simple raytracer implemented in C using the Raylib library. It demonstrates basic ray tracing techniques, including sphere intersection, lighting calculations, shadows, and reflections.

<img width="793" alt="screenshot_1" src="https://github.com/user-attachments/assets/58bd888f-0a09-45bc-ba2e-12f2dc220e12">


## Features

- Ray-sphere intersection
- Multiple light types (ambient, point, and directional)
- Phong shading model (diffuse and specular reflections)
- Shadow casting
- Recursive reflections
- Basic scene setup with multiple spheres

## Prerequisites

To build and run this project, you'll need:

- A C compiler (e.g., GCC, Clang)
- [Raylib](https://www.raylib.com/) library installed on your system

## Building the Project

1. Clone this repository:
   ```
   git clone https://github.com/amedduman/simple-raytracer.git
   cd simple-raytracer
   ```

2. Compile the project (adjust the command based on your setup):
   ```
   gcc -o raytracer main.c -lraylib -lm
   ```

## Running the Raytracer

After building the project, run the executable:

```
./raytracer
```

This will open a window displaying the rendered scene.

## Scene Configuration

The scene is configured in the `main` function. You can modify the following elements:

- Spheres: position, radius, color, specularity, and reflectiveness
- Lights: type (ambient, point, directional), intensity, and position/direction

## Implementation Details

The raytracer uses the following key components:

- `ray_sphere_intersection`: Calculates the intersection of a ray with a sphere
- `ray_closest_intersection`: Finds the closest intersection point for a ray in the scene
- `compute_light`: Calculates lighting based on the Phong shading model
- `trace_ray`: The main raytracing function, handling reflections and recursive calls

## Future Improvements

- Add support for more complex shapes (e.g., planes, triangles)
- Transparency
- Constructive Solid Geometry
- Optimize performance with spatial data structures, multithreading and subsampling
