#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  bool show_wireframe;
  bool show_grid;
  bool show_help;
  bool show_fps;
} Settings;

typedef struct {
  Vector3 position;
  float radius;
  Vector3 velocity;
  float mass;
} Particle;

typedef struct {
  Vector3 position;
  float radius;
  float mass;
} Sphere;

#define MAX_PARTICLES 100
#define GRAVITY_CONSTANT 1.0f
#define FONT_SIZE 20

void ApplyGravity(Particle *particle, Sphere sphere) {
  Vector3 direction = Vector3Subtract(sphere.position, particle->position);
  float distance = Vector3Length(direction);

  if (distance > 0) {
    float forceMagnitude = (GRAVITY_CONSTANT * sphere.mass * particle->mass) /
                           (distance * distance);
    Vector3 force = Vector3Scale(Vector3Normalize(direction), forceMagnitude);

    particle->velocity = Vector3Add(particle->velocity, force);
  }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------

  // Make window the size of 720p
  const int screenWidth = 1280;
  const int screenHeight = 720;

  InitWindow(screenWidth, screenHeight,
             "raylib [core] example - 3d camera free");

  Settings settings = {
      false,
      true,
      false,
      true,
  };

  Sphere black_hole = {
      (Vector3){0.0f, 0.0f, 0.0f},
      1.0f,
      1.0f,
  };

  // Initialize particles
  Particle particles[MAX_PARTICLES];
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].position = (Vector3){
        GetRandomValue(0, 10), GetRandomValue(0, 10), GetRandomValue(2, 10)};
    particles[i].radius = 0.3f;
    particles[i].velocity = (Vector3){0, 0, 0};
    particles[i].mass = 0.3f;
  }

  // Create a sphere mesh and model
  Mesh sphereMesh = GenMeshSphere(1.0f, 16, 16); // Create a sphere mesh
  Model sphereModel =
      LoadModelFromMesh(sphereMesh); // Load the mesh into a model

  // Define transforms to be uploaded to GPU for instances
  Matrix *transforms = (Matrix *)RL_CALLOC(MAX_PARTICLES, sizeof(Matrix));

  for (int i = 0; i < MAX_PARTICLES; i++) {
    Matrix translation = MatrixTranslate((float)GetRandomValue(-10, 10),
                                         (float)GetRandomValue(-10, 10),
                                         (float)GetRandomValue(-10, 10));
    Vector3 axis = Vector3Normalize((Vector3){0.0f, 1.0f, 0.0f});
    float angle = 0.0f * DEG2RAD;
    Matrix rotation = MatrixRotate(axis, angle);

    transforms[i] = MatrixMultiply(rotation, translation);
  }

  // Load shader
  Shader shader = LoadShader("instancing.vs", "instancing.fs");
  // Shader shader = LoadShader("lighting_instancing.vs", "lighting_instancing.fs");
  if (shader.id == 0) {
    TraceLog(LOG_ERROR, "Failed to load shader!");
  }

  // Get shader locations
  shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
  shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

  // NOTE: We are assigning the intancing shader to material.shader
  // to be used on mesh drawing with DrawMeshInstanced()
  Material matInstances = LoadMaterialDefault();
  matInstances.shader = shader;
  matInstances.maps[MATERIAL_MAP_DIFFUSE].color = RED;

  Material matDefault = LoadMaterialDefault();
  matDefault.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;

  // Set anti-aliasing to 4x MSAA
  // SetConfigFlags(FLAG_MSAA_4X_HINT);

  // Define the camera to look into our 3d world
  Camera3D camera = {0};
  camera.position = (Vector3){10.0f, 10.0f, 10.0f}; // Camera position
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};      // Camera looking at point
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};          // Camera up vector
  camera.fovy = 45.0f;                              // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE;           // Camera projection type

  DisableCursor(); // Limit cursor to relative movement inside the window

  // Initialize last fps as 0
  int lastFPS = 0;
  char fpsText[10];

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------

    int currentFPS = GetFPS();

    // Only update the FPS text if the FPS value has changed
    if (currentFPS != lastFPS) {
      TextFormat(fpsText, sizeof(fpsText), "FPS: %d", currentFPS);
      lastFPS = currentFPS;
    }

    UpdateCamera(&camera, CAMERA_FREE);

    SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW],
                   &camera.position, SHADER_UNIFORM_VEC3);

    if (IsKeyPressed('Z'))
      camera.target = (Vector3){0.0f, 0.0f, 0.0f};

    if (IsKeyPressed('V')) {
      settings.show_wireframe = !settings.show_wireframe;
    }

    if (IsKeyPressed('G')) {
      settings.show_grid = !settings.show_grid;
    }

    if (IsKeyPressed(KEY_F1)) {
      settings.show_help = !settings.show_help;
    }

    if (IsKeyPressed(KEY_F2)) {
      settings.show_fps = !settings.show_fps;
      printf("Show FPS: %s\n", settings.show_fps ? "ON" : "OFF");
    }

    // Apply gravity to particles
    // for (int i = 0; i < MAX_PARTICLES; i++) {
    //   ApplyGravity(&particles[i], black_hole);
    //   particles[i].position =
    //       Vector3Add(particles[i].position, particles[i].velocity);

    //   // Update the transform matrix for each particle
    //   transforms[i] =
    //       MatrixTranslate(particles[i].position.x, particles[i].position.y,
    //                       particles[i].position.z);
    // }

    //----------------------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(GRAY);

    BeginMode3D(camera);

    DrawSphere(black_hole.position, 0.2f, BLACK);
    // DrawSphere(black_hole.position, black_hole.radius, BLACK);
    // DrawMesh(sphereMesh, matDefault, MatrixIdentity());

    // Draw the instanced particles
    // DrawMeshInstanced(sphereModel.meshes[0], sphereModel.materials[0],
    //                   transforms, MAX_PARTICLES);

    DrawMeshInstanced(sphereMesh, matInstances, transforms, MAX_PARTICLES);

    if (settings.show_wireframe) {
      DrawSphereWires(black_hole.position, black_hole.radius, 16, 16, WHITE);
    }

    if (settings.show_grid) {
      DrawGrid(30, 1.0f);
    }

    EndMode3D();

    if (settings.show_fps) {
      DrawFPS(20, 20);
    }

    if (settings.show_help) {
      DrawText("Controls:", 20, 40, FONT_SIZE, BLACK);
      DrawText("- V to toggle wireframe", 40, 60, FONT_SIZE, DARKGRAY);
      DrawText("- G to toggle grid", 40, 80, FONT_SIZE, DARKGRAY);
      DrawText("- F1 to toggle this help", 40, 100, FONT_SIZE, DARKGRAY);
      DrawText("- F2 to toggle FPS", 40, 120, FONT_SIZE, DARKGRAY);
      DrawText("- ESC to exit", 40, 140, FONT_SIZE, DARKGRAY);
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  RL_FREE(transforms); // Free transforms
  CloseWindow();       // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
