#include "include/raylib.h"
#include "include/raymath.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

// #region Sphere
typedef struct
{
  Vector3 center;
  float radius;
  Color color;
  float specular;
  float reflective;
} Sphere;
// #endregion

// #region Lights

typedef enum
{
  e_light_type_ambient = 0,
  e_light_type_point = 1,
  e_light_type_directional = 2
} light_type;

typedef struct
{
  float intensity;
} light_ambient;

typedef struct
{
  float intensity;
  Vector3 position;
} light_point;

typedef struct
{
  float intensity;
  Vector3 direction;
} light_directional;

typedef struct
{
  light_type my_type;
  union 
  {
    light_ambient ambient;
    light_point point;
    light_directional directional;
  };
} light;

// #endregion

// #region Raycasting

  typedef struct
  {
    bool is_intersecting;
    Vector2 intersections;
  } ray_sphere_intersection_info;

  // https://www.youtube.com/watch?v=OCZTVpfMSys
  ray_sphere_intersection_info ray_sphere_intersection(Vector3 so, float sr, Vector3 ro, Vector3 rd)
  {
    Vector3 CO = Vector3Subtract(ro, so);

    float a = Vector3DotProduct(rd, rd);
    float b = 2 * Vector3DotProduct(CO, rd);
    float c = Vector3DotProduct(CO, CO) - (sr * sr);

    float discriminant = (b * b) - (4 * a * c); 

    if (discriminant < 0)
    {
      return (ray_sphere_intersection_info) {false, (Vector2){-1,-1}};
    }

    float t1 = (-b + sqrt(discriminant)) / (2*a);
    float t2 = (-b - sqrt(discriminant)) / (2*a);

    return (ray_sphere_intersection_info) {true, (Vector2){t1,t2}};
  }

  typedef struct 
  {
    bool has_found;
    Sphere closest_sphere;
    float closest_t;
  } ray_closest_intersection_result;

  ray_closest_intersection_result ray_closest_intersection(const Sphere* spheres, int sphere_count, float tmin, float tmax, Vector3 ro, Vector3 rd)
  {
    float closest_t = INFINITY;
    bool haveFoundSphereToDraw = false;
    Sphere closestSphere;

    for (int i = 0; i < sphere_count; i++)
    {
      ray_sphere_intersection_info hit_info = ray_sphere_intersection(spheres[i].center, spheres[i].radius, ro, rd);

      float t1 = hit_info.intersections.x;
      float t2 = hit_info.intersections.y;

      if (hit_info.is_intersecting)
      { 
        if (t1 > tmin && t1 < tmax)
        {
          if (t1 < closest_t)
          {
            haveFoundSphereToDraw = true;
            closest_t = t1;
            closestSphere = spheres[i];
          }
        }
        
        if (t2 > tmin && t2 < tmax)
        {
          if (t2 < closest_t)
          {
            haveFoundSphereToDraw = true;
            closest_t = t2;
            closestSphere = spheres[i];
          }
        }
      }
    }

    ray_closest_intersection_result result = {
      .has_found = haveFoundSphereToDraw,
      .closest_sphere = closestSphere,
      .closest_t = closest_t
    };

    return result;
  }
// #endregion

// #region LightCalculations

Vector3 ReflectRay(Vector3 ray, Vector3 normal) 
{
    Vector3 reflection = Vector3Scale(normal, (Vector3DotProduct(normal, ray) * 2)); 
    reflection = Vector3Subtract(reflection, ray);
    return reflection;
}

float compute_light(const Sphere* spheres, int sphere_count, float tmax, const light* lights, int light_count, Vector3 point, Vector3 normal, Vector3 view_vector, float specular)
{
  assert(Vector3Length(normal) < 1 + EPSILON);
  float intensity = 0.0f; // intensity
  Vector3 L = {0,0,0}; // coming light vector
  for (int i = 0; i < light_count; i++)
  {
    light current_light = lights[i];

    if (current_light.my_type == e_light_type_ambient)
    {
      intensity += current_light.ambient.intensity;
    }
    else
    {
      float light_intensity = 0;

      if (current_light.my_type == e_light_type_point)
      {
        L = Vector3Subtract(current_light.point.position, point);
        light_intensity = current_light.point.intensity;
      }
      else if (lights[i].my_type == e_light_type_directional)
      {
        L = current_light.directional.direction;
        L = Vector3Add(L, point);
        light_intensity = current_light.directional.intensity;
      }

      // Shadow check
      ray_closest_intersection_result shadow_check_result = ray_closest_intersection(spheres, sphere_count, .001, tmax, point, L);
      if (shadow_check_result.has_found)
      {
        continue;
      }
      

      // diffuse
      float n_dot_l = Vector3DotProduct(normal, L);
      if (n_dot_l > 0)
      {
        intensity += light_intensity * n_dot_l / (Vector3Length(normal) * Vector3Length(L));
      }

      // specular
      if (specular != -1)
      {
        Vector3 reflection = ReflectRay(L, normal);
        float reflection_dot_view = Vector3DotProduct(reflection, view_vector);

        if (reflection_dot_view > 0)
        {
          intensity += light_intensity * powf(reflection_dot_view / (Vector3Length(reflection) * Vector3Length(view_vector)), specular);
        }
      }
    }
  }

  return intensity;
}
// #endregion

// #region Canvas

  void canvas_put_pixel(int Cx, int Cy, Color color)
  {
    int Sx = GetScreenWidth() / 2 + Cx;
    int Sy = GetScreenHeight() / 2 - Cy;

    DrawPixel(Sx, Sy, color);
  }

 
  Vector3 canvas_to_viewport(int Cx, int Cy)
  {
    float Vw = 1.0f; // the viewport height
    float Vh = 1.0f; // the viewport width
    float d = 1.0f; // the distance from the camera to the canvas

    float Vx = Cx * (Vw / GetScreenWidth());
    float Vy = Cy * (Vh / GetScreenHeight());
    float Vz = d;

    return (Vector3){ Vx, Vy, Vz };
  }

// #endregion

// #region Raytracing

Color ColorScaleFactor(Color col, float factor)
{
  Color result = {
    Clamp((col.r * factor), 0, 255),
    Clamp((col.g * factor), 0, 255),
    Clamp((col.b * factor), 0, 255),
    255
  };

  return result;
}

Color trace_ray(Sphere* spheres, int sphere_count, light* lights, int light_count, Vector3 ro, Vector3 rd, Vector3 cam_pos, float tmin, float tmax, int recursion_depth)
{
  ray_closest_intersection_result result = ray_closest_intersection(spheres, sphere_count, tmin, tmax, ro, rd);

  float closest_t = result.closest_t;
  bool haveFoundSphereToDraw = result.has_found;
  Sphere closestSphere = result.closest_sphere;

  if (haveFoundSphereToDraw)
  {
    Vector3 intersection_point = Vector3Add(ro, Vector3Scale(rd, closest_t));
    Vector3 surface_normal = Vector3Subtract(intersection_point, closestSphere.center);
    surface_normal = Vector3Normalize(surface_normal);
    
    Vector3 view_vector = Vector3Subtract(cam_pos, intersection_point);

    float light_intensity = compute_light(spheres, sphere_count, tmax, lights, light_count, intersection_point, surface_normal, view_vector, closestSphere.specular);

    Color resulted_color = {
      Clamp((closestSphere.color.r * light_intensity), 0, 255),
      Clamp((closestSphere.color.g * light_intensity), 0, 255),
      Clamp((closestSphere.color.b * light_intensity), 0, 255),
      255
    };

    // If we hit the recursion limit or the object is not reflective, we're done
    float reflective = closestSphere.reflective;
    if (reflective < (0 + EPSILON) || recursion_depth <= 0)
    {
    return resulted_color;
    }
    

    Vector3 reflection = ReflectRay(Vector3Scale(rd, -1), surface_normal);
    Color reflected_color = trace_ray(spheres, sphere_count, lights, light_count, intersection_point, reflection, cam_pos, 0.01f, tmax, recursion_depth - 1);

    resulted_color = ColorScaleFactor(resulted_color, (1 - reflective));
    reflected_color = ColorScaleFactor(reflected_color, reflective);
    resulted_color = (Color){
      Clamp((resulted_color.r + reflected_color.r), 0, 255),
      Clamp((resulted_color.g + reflected_color.g), 0, 255),
      Clamp((resulted_color.b + reflected_color.b), 0, 255),
      255
    };

    return resulted_color;
  }
  else
  {
    return (Color){24, 1, 97, 255};
  }
}
// #endregion

// #region Main

#define sphere_count 4
#define light_count 3

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Game");
    SetTargetFPS(30);

    Sphere spheres[sphere_count] = {
      {.center = {0, -1, 3}, .radius = 1, RED, .specular = 500, .reflective = 0.2f},
      {.center = {2, 0, 4}, .radius = 1, BLUE, .specular = 500, .reflective = 0.3f},
      {.center = {-2, 0, 4}, .radius = 1, GREEN, .specular = 10, .reflective = 0.4f},
      {.center = {0, -5001, 0}, .radius = 5000, YELLOW, .specular = 1000, .reflective = 0.5f}
    };

    light lights[light_count] = {
      {e_light_type_ambient, .ambient = {0.2f}},
      {e_light_type_point, .point = {0.6f, {2, 1, 0}}},
      {e_light_type_directional, .directional = {0.2f, {1, 4, 4}}}
    };



    while (!WindowShouldClose())
    {
      BeginDrawing();

      ClearBackground(MAGENTA);

      Vector3 cam_pos = {0,0,0};
      Vector3 ro = {0,0,0};
      int w = GetScreenWidth();
      int h = GetScreenWidth();
      for (int x = -w/2; x < w/2; x++)
      {
        for (int y = -h/2; y < h/2; y++)
        {
          Vector3 rd = canvas_to_viewport(x, y);
          rd = Vector3Subtract(rd, ro);
          Color color = trace_ray(spheres, sphere_count, lights, light_count, ro, rd, cam_pos, 1, INFINITY, 3);
          canvas_put_pixel(x, y, color);
        }
      }

      EndDrawing();
    }
    CloseWindow();
    return 0;
}

// #endregion