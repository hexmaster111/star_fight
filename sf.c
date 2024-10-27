#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

// #define DEBUG_HITBOX

#define PLAYER_FRICTION (0.005f)
#define PLAYER_ACCEL (0.05f)
#define PLAYER_MAX_SPEED (2.0f)

#define HITBOX_SIZE (30)

#define FIGHTERS_COUNT (20)
#define FIGHTERS_MAX_SPEED (1.5f)

#define PLAYER_MAX_SHOTS (20)
#define PLAYER_BULLET_SPEED (5.0f)

#define FIGHTER_POINTS (100)

typedef struct FIGHTER
{
  int isinthefight;
  int respawn_no_place;
  Vector2 pos;
  Vector2 speed;
} FIGHTER;

typedef struct BULLET
{
  int isinthefight;
  Vector2 pos, speed;
} BULLET;

void RemoveFighterFromGame(FIGHTER *f);
void PlaceFighter(FIGHTER *f);
void PlayerShoot();
void RemoveBulletFromGame(BULLET *b);
void HitFighter(FIGHTER *f, BULLET *b);
// ======================= GLOBALS =========================
FIGHTER g_fighter[FIGHTERS_COUNT];
BULLET g_playerbullets[PLAYER_MAX_SHOTS];

Vector2 g_playerspeed;
Vector2 g_playerpos;
int g_playerscore;

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

    if (IsKeyPressed(KEY_SPACE))
      PlayerShoot();

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
    int player_hit_a_ship_count = 0;
    Rectangle player_hitbox = {.x = g_playerpos.x,
                               .y = g_playerpos.y + 4, // feels better
                               .width = HITBOX_SIZE,
                               .height = HITBOX_SIZE};
    Rectangle ship_hitboxes[FIGHTERS_COUNT];
    Rectangle player_bullet_hitboxes[PLAYER_MAX_SHOTS];

    for (int i = 0; i < FIGHTERS_COUNT; ++i)
    {
      if (!g_fighter[i].isinthefight)
        continue;

      if (g_fighter[i].pos.x > GetScreenWidth() || 0 > g_fighter[i].pos.x)
        PlaceFighter(&g_fighter[i]);
      if (g_fighter[i].pos.y > GetScreenHeight() || 0 > g_fighter[i].pos.y)
        PlaceFighter(&g_fighter[i]);

      g_fighter[i].pos = Vector2Add(g_fighter[i].pos, g_fighter[i].speed);
      ship_hitboxes[i] =
          (Rectangle){.x = g_fighter[i].pos.x,
                      .y = g_fighter[i].pos.y + 4, // feels better
                      .width = HITBOX_SIZE,
                      .height = HITBOX_SIZE};
      if (CheckCollisionRecs(ship_hitboxes[i], player_hitbox))
      {
        player_hit_a_ship_count += 1;
      }
    }

    for (int b_idx = 0; b_idx < PLAYER_MAX_SHOTS; ++b_idx)
    {
      if (!g_playerbullets[b_idx].isinthefight)
        continue;

      if (g_playerbullets[b_idx].pos.x > GetScreenWidth() || 0 > g_playerbullets[b_idx].pos.x)
        RemoveBulletFromGame(&g_playerbullets[b_idx]);
      if (g_playerbullets[b_idx].pos.y > GetScreenHeight() || 0 > g_playerbullets[b_idx].pos.y)
        RemoveBulletFromGame(&g_playerbullets[b_idx]);

      g_playerbullets[b_idx].pos = Vector2Add(g_playerbullets[b_idx].pos, g_playerbullets[b_idx].speed);

      player_bullet_hitboxes[b_idx] = (Rectangle){
          .x = g_playerbullets[b_idx].pos.x,
          .y = g_playerbullets[b_idx].pos.y + 4,
          .width = HITBOX_SIZE,
          .height = HITBOX_SIZE};

      for (int e_idx = 0; e_idx < FIGHTERS_COUNT; e_idx++)
      {
        if (!g_fighter[e_idx].isinthefight)
          continue;

        if (CheckCollisionRecs(ship_hitboxes[e_idx], player_bullet_hitboxes[b_idx]))
        {
          HitFighter(&g_fighter[e_idx], &g_playerbullets[b_idx]);
        }
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
        DrawRectangleRec(ship_hitboxes[i], RED);
#endif // DEBUG_HITBOX
        DrawText("O", g_fighter[i].pos.x, g_fighter[i].pos.y, 48, BLACK);
      }
    }

    for (int i = 0; i < PLAYER_MAX_SHOTS; ++i)
    {
      if (g_playerbullets[i].isinthefight)
      {

#ifdef DEBUG_HITBOX
        DrawRectangleRec(player_bullet_hitboxes[i], PURPLE);
#endif // DEBUG_HITBOX
        DrawText("#", g_playerbullets[i].pos.x, g_playerbullets[i].pos.y, 48, BLACK);
      }
    }

#ifdef DEBUG_HITBOX
    DrawRectangleRec(player_hitbox, YELLOW);
#endif // DEBUG_HITBOX
    int bullet_in_count = 0;
    for (int i = 0; i < PLAYER_MAX_SHOTS; ++i)
    {
      if (g_playerbullets[i].isinthefight)
        bullet_in_count += 1;
    }

    DrawText("X", g_playerpos.x, g_playerpos.y, 48, BLACK);
    const char *debug = TextFormat("%d", bullet_in_count);
    DrawText(debug, 10, 10, 12, BLACK);

    if (player_hit_a_ship_count)
    {
      ClearBackground(RED);
      g_playerscore = 0;
    }
    const char *score_text = TextFormat("SCORE : %d", g_playerscore);
    const int score_text_size = 35;

    DrawText(score_text, (GetScreenWidth() / 2) - MeasureText(score_text, score_text_size),
             20, score_text_size, BLACK);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}

void RemoveFighterFromGame(FIGHTER *f)
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
  f->respawn_no_place = 150;

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
}

void PlayerShoot()
{
  // todo shoot timer

  for (int i = 0; i < PLAYER_MAX_SHOTS; i++)
  {
    if (g_playerbullets[i].isinthefight)
      continue; // bullet in use

    g_playerbullets[i].isinthefight = 1;
    g_playerbullets[i].speed = Vector2Scale(Vector2Normalize(g_playerspeed), PLAYER_BULLET_SPEED);
    g_playerbullets[i].pos = g_playerpos;
    break;
  }
}

void RemoveBulletFromGame(BULLET *b)
{
  b->isinthefight = 0;
  b->pos = (Vector2){.x = 0, .y = 0};
  b->speed = (Vector2){.x = 0, .y = 0};
}

void HitFighter(FIGHTER *f, BULLET *b)
{
  g_playerscore += FIGHTER_POINTS;
  f->isinthefight = 0;
  RemoveBulletFromGame(b);
  RemoveFighterFromGame(f);
}