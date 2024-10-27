#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
Vector2 g_playerspeed;
Vector2 g_playerpos;

#define PLAYER_FRICTION (0.005f)
#define PLAYER_ACCEL (0.05f)
#define PLAYER_MAX_SPEED (2.0f)

#define HITBOX_SIZE (30)

#define FIGHTERS_COUNT (20)
#define FIGHTERS_MAX_SPEED (1.5f)
typedef struct FIGHTER
{
  int isinthefight;
  int respawn_no_place;
  Vector2 pos;
  Vector2 speed;
} FIGHTER;

void KillFighter(FIGHTER *f);
void PlaceFighter(FIGHTER *f);

FIGHTER g_fighter[FIGHTERS_COUNT];

int main()
{
  InitWindow(600, 400, "Star Fight");
  SetRandomSeed(5432);
  SetTargetFPS(60);

  for (int i = 0; i < FIGHTERS_COUNT; ++i)
  {
    g_fighter[i].isinthefight = 0;
    g_fighter[i].speed = (Vector2){0, 0};
    g_fighter[i].pos = (Vector2){0, 0};
    g_fighter[i].respawn_no_place = 0;
  }

  PlaceFighter(&g_fighter[0]);
  PlaceFighter(&g_fighter[1]);

  while (!WindowShouldClose())
  {
    // player update
    if (IsKeyDown(KEY_UP))
      g_playerspeed.y -= PLAYER_ACCEL;
    if (IsKeyDown(KEY_DOWN))
      g_playerspeed.y += PLAYER_ACCEL;
    if (IsKeyDown(KEY_LEFT))
      g_playerspeed.x -= PLAYER_ACCEL;
    if (IsKeyDown(KEY_RIGHT))
      g_playerspeed.x += PLAYER_ACCEL;

    if (0 > g_playerspeed.x)
      g_playerspeed.x += PLAYER_FRICTION;
    else
      g_playerspeed.x -= PLAYER_FRICTION;
    if (0 > g_playerspeed.y)
      g_playerspeed.y += PLAYER_FRICTION;
    else
      g_playerspeed.y -= PLAYER_FRICTION;

    g_playerspeed = Vector2ClampValue(g_playerspeed, 0.0f, PLAYER_MAX_SPEED);
    g_playerpos = Vector2Add(g_playerpos, g_playerspeed);

    // fighter update
    int player_hit = 0;
    Rectangle player_hitbox = {.x = g_playerpos.x,
                               .y = g_playerpos.y + 4, // feels better
                               .width = HITBOX_SIZE,
                               .height = HITBOX_SIZE};
    Rectangle debug_hitboxes[FIGHTERS_COUNT];

    for (int i = 0; i < FIGHTERS_COUNT; ++i)
    {
      if (!g_fighter[i].isinthefight)
        continue;

      if (g_fighter[i].pos.x > GetScreenWidth() || 0 > g_fighter[i].pos.x)
        PlaceFighter(&g_fighter[i]);
      if (g_fighter[i].pos.y > GetScreenHeight() || 0 > g_fighter[i].pos.y)
        PlaceFighter(&g_fighter[i]);

      g_fighter[i].pos = Vector2Add(g_fighter[i].pos, g_fighter[i].speed);
      debug_hitboxes[i] =
          (Rectangle){.x = g_fighter[i].pos.x,
                      .y = g_fighter[i].pos.y + 4, // feels better
                      .width = HITBOX_SIZE,
                      .height = HITBOX_SIZE};
      if (CheckCollisionRecs(debug_hitboxes[i], player_hitbox))
      {
        player_hit += 1;
      }
    }

    // draw
    BeginDrawing();
    ClearBackground(WHITE);

    for (int i = 0; i < FIGHTERS_COUNT; ++i)
    {
      if (g_fighter[i].isinthefight)
      {
#ifdef DEBUG_HITBOX
        DrawRectangleRec(debug_hitboxes[i], RED);
#endif // DEBUG_HITBOX
        DrawText("O", g_fighter[i].pos.x, g_fighter[i].pos.y, 48, BLACK);
      }
    }
#ifdef DEBUG_HITBOX
    DrawRectangleRec(player_hitbox, YELLOW);
#endif // DEBUG_HITBOX
    DrawText("X", g_playerpos.x, g_playerpos.y, 48, BLACK);
    const char *debug = TextFormat("");
    DrawText(debug, 10, 10, 12, BLACK);
    EndDrawing();
  }

  DrawFPS(10, 10);

  CloseWindow();
  return 0;
}

void KillFighter(FIGHTER *f)
{
  f->isinthefight = 0;
  f->respawn_no_place = 0;
}

void PlaceFighter(FIGHTER *f)
{

  if (f->isinthefight && f->respawn_no_place)
  {
    f->respawn_no_place -= 1;
    if (0 > f->respawn_no_place)
      f->respawn_no_place = 0;
    return;
  }

  f->isinthefight = 1;
  f->respawn_no_place = 120; // ~2 sec

  Vector2 screen_pt = (Vector2){.x = GetRandomValue(0, GetScreenWidth()),
                                .y = GetRandomValue(0, GetScreenHeight())};
  float pt_thata = rand();
  Vector2 unit_pt_thata = (Vector2){.x = cos(pt_thata), .y = sin(pt_thata)};
  float r = (sqrt(pow(GetScreenWidth(), 2) + pow(GetScreenHeight(), 2)) / 2.0f) + 1;
  Vector2 spawnpoint = Vector2Scale(unit_pt_thata, r);

  f->pos = (Vector2){.x = spawnpoint.x + (GetScreenWidth() / 2),
                     .y = spawnpoint.y + (GetScreenHeight() / 2)};

  Vector2 dhat = Vector2Normalize((Vector2){
      .x = screen_pt.x - spawnpoint.x,
      .y = screen_pt.y - spawnpoint.y});

  f->speed = Vector2Scale(dhat, FIGHTERS_MAX_SPEED);

  printf("x=%.2f, y=%.2f, pt_th=%.2f\n", spawnpoint.x, spawnpoint.y, pt_thata);
}
