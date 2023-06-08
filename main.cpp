#include <iostream>

#include "Bullet.h"

#include "raylib.h"
#include <vector>
#include "rcamera.h"
#include "raymath.h"

int main() {
    // Screen constants
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    SetConfigFlags(FLAG_MSAA_4X_HINT);

    // Initialise window
    InitWindow(screenWidth, screenHeight, "My FPS Game");

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };    // Camera position
    camera.target = Vector3Add(camera.position, (Vector3){0, camera.position.y - 1, 0});      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(144);                   // Set our game to run at 60 frames-per-second

    // Textures
    Texture2D HUD;

    // Models
    Model zombie = LoadModel("../assets/Zombie.glb");
    zombie.transform = MatrixRotateX(90*DEG2RAD);

    Model map = LoadModel("../assets/room.vox");

    Model gun = LoadModel("../assets/gun.vox");

    Model characterOne = LoadModel("../assets/boyCharacter.vox");
    Model characterTwo = LoadModel("../assets/girlCharacter.vox");

    Vector3 gunLocation;

    // Game variables
    float bearing = 0;
    float verticalBearing = 90; // Zero is floor, 180 is ceiling

    double sensitivityMult = 1;

    double speedConstant = 0.1;
    int shooting = 0;

    float playerHeight = 2.5f;
    float floorHeight = 0;
    float verticalVelocity = 0;
    int ammo = 10;

    int hit;

    // Vector or Bullets
    Ray ray;
    std::vector<Ray>rayList;

    // Map
    std::vector<Vector3> spheres;
    Vector3 sphereLocation;

    for (int i = 0; i < 5; i++) {
        sphereLocation = (Vector3){(float)(2*i-4), (float)(i), 1};
        spheres.push_back(sphereLocation);
    }


    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float xChange = speedConstant*sin(bearing*DEG2RAD);
        float zChange = speedConstant*cos(bearing*DEG2RAD);

        // Custom movement - ordinary movement too slow
        if (IsKeyDown(KEY_W)) {
            camera.position.x += xChange;
            camera.position.z += zChange;
        }
        if (IsKeyDown(KEY_S)) {
            camera.position.x -= xChange;
            camera.position.z -= zChange;
        }
        if (IsKeyDown(KEY_A)) {
            camera.position.x += zChange;
            camera.position.z -= xChange;
        }
        if (IsKeyDown(KEY_D)) {
            camera.position.x -= zChange;
            camera.position.z += xChange;
        }

        // Custom mouse input - new
        // Vertical mouse movement
        verticalBearing -= sensitivityMult*GetMouseDelta().y / 2;
        if (verticalBearing > 89) {
            verticalBearing = 89;
        }
        else if (verticalBearing < -89) {
            verticalBearing = -89;
        }
        // Horizontal mouse movement
        bearing -= sensitivityMult*GetMouseDelta().x/2;
        if (bearing > 360) {
            bearing -= 360;
        }
        else if (bearing < 0) {
            bearing += 360;
        }
        camera.target = Vector3Add(camera.position, (Vector3){sin(bearing*DEG2RAD)*abs(cos(verticalBearing*DEG2RAD)),  verticalBearing/100, cos(bearing*DEG2RAD)*abs(cos(verticalBearing*DEG2RAD))});


        // Jumping / Height calculations
        // Check for input
        if (IsKeyDown(KEY_SPACE) && camera.position.y - playerHeight == floorHeight) {
            verticalVelocity = 0.14f;
            camera.position.y += 0.01f;
        }
        // If not on floor then change velocity
        if (camera.position.y - playerHeight > floorHeight) {
            camera.position.y += verticalVelocity;
            verticalVelocity -= 0.002;
        }
        if (camera.position.y - playerHeight <= floorHeight) {
            camera.position.y = floorHeight + playerHeight;
            verticalVelocity = 0;
        }

        // Firing mechanism
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) & ammo > 0) {
            shooting = 50;
            ammo--;

            ray.position = camera.position;
            ray.direction = Vector3Subtract(camera.target, camera.position);

            // Check collisions
            for (auto & sLocation : spheres) {
                if (GetRayCollisionSphere(ray, sLocation, 0.1f).hit) {
                    hit = 10;
                    sLocation = camera.position;
                }
            }

            rayList.push_back(ray);
        }
        else {
            if (shooting > 0) {
                shooting--;
            }
        }

        // Reload -> add animations soon
        if (IsKeyPressed(KEY_R)) {
            rayList.clear();
            ammo = 10;
        }

        // Aim-Down-Sight mechanics
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            gunLocation = Vector3Add(camera.position, (Vector3){0.25f*(camera.target.x-camera.position.x), -0.16f, 0.25f*(camera.target.z-camera.position.z)});
            gunLocation = Vector3Add(gunLocation, Vector3RotateByAxisAngle((Vector3){0.2f*(gunLocation.x - camera.position.x),0.1f*(camera.target.y - gunLocation.y), 0.2f*(gunLocation.z - camera.position.z)}, (Vector3){0, 0.1f, 0}, (float)(-M_PI/2)));
            sensitivityMult = 0.35;
        }
        else {
            gunLocation = Vector3Add(camera.position, (Vector3){0.25f*(camera.target.x-camera.position.x), -0.16f, 0.25f*(camera.target.z-camera.position.z)});
            gunLocation = Vector3Add(gunLocation, Vector3RotateByAxisAngle((Vector3){0.8f*(gunLocation.x - camera.position.x),0.1f*(camera.target.y - gunLocation.y), 0.8f*(gunLocation.z - camera.position.z)}, (Vector3){0, 0.3f, 0}, (float)(-M_PI/2)));
            sensitivityMult = 0.60;
        }

        // Crouch mechanics
        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            playerHeight = 1.f;
        }
        else {
            playerHeight = 2.5f;
        }

        // Rotate gun
        gun.transform = MatrixRotateY(bearing*DEG2RAD);
        // gun.transform = MatrixRotateZYX((Vector3){cos(bearing*DEG2RAD)*verticalBearing*DEG2RAD, bearing*DEG2RAD, sin(bearing*DEG2RAD)*verticalBearing*DEG2RAD});


        UpdateCamera(&camera);                  // Update camera

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        // Background
        ClearBackground(RAYWHITE);

        // Enter 3D mode
        BeginMode3D(camera);

        // Draw Map
        DrawGrid(20, 1.f);
        DrawLine3D((Vector3){0.f,1.f,0.f}, (Vector3){0.f,30.f,0.f}, BLUE);
        // DrawModel(map, (Vector3){-10,0,-10}, 1.f, WHITE);

        // Draw enemy
        DrawModel(characterOne, (Vector3){2.f,0.f,2.f}, 0.5f, WHITE);
        DrawModel(characterTwo, (Vector3){-2.f,0.f,2.f}, 0.5f, WHITE);

        // Draw gun
        DrawModel(gun, gunLocation, 0.01f, WHITE);


        // Draw targets
        for (auto & sLocation : spheres) {
            DrawSphere(sLocation, 0.1f , RED);
        }

        for (auto & ray : rayList) {
            DrawRay(ray, GREEN);
        }


        EndMode3D();

        // HUD
        DrawTexture(HUD, screenWidth/2 - HUD.width/2, screenHeight/2 - HUD.height/2, WHITE);


        // TEST DATA
        DrawText(TextFormat("Player height: %02.02f", (double)camera.position.y), 10, 25, 20, BLACK);
        DrawText(TextFormat("Horizontal bearing: %02.02f", (double)bearing), 10, 45, 20, BLACK);
        DrawText(TextFormat("Vertical bearing: %02.02f", (double)verticalBearing), 10, 65, 20, BLACK);
        DrawText(TextFormat("Gun height: %02.02f", (double)gunLocation.y), 10, 85, 20, BLACK);

        // AMMO HUD
        DrawText(TextFormat("%02i/10", ammo), screenWidth - 280, screenHeight - 160, 70, BLACK);

        // Cross Hair
        DrawRectangle(screenWidth/2 - 9.f, screenHeight/2 - 1.5f, 18.f, 3.f, BLACK);
        DrawRectangle(screenWidth/2 - 1.5f, screenHeight/2 - 9.f, 3.f, 18.f, BLACK);

        // FPS
        DrawFPS(1800, 20);

        if (hit > 0) {
            DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
            hit--;
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------



    return 0;
}
