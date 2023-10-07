#ifndef APOCALYPSE_H
#define APOCALYPSE_H

#include <nds.h>
#include <dsma.h>

typedef int32 f32;

typedef enum ApocalypseState {
    Idle,
    Wandering,
    Summoned,
    Freefall,
    Playing
} ApocalypseState;

typedef struct Apocalypse {
    // position
    f32 x;
    f32 y;
    f32 z;
    // speed
    f32 v;
    // velocity
    f32 v_x;
    f32 v_y;
    f32 v_z;
    // target
    f32 target_x;
    f32 target_y;
    f32 target_z;
    // bounds (+- n, where n is either in the x or z directions)
    int bounds_x;
    int bounds_z;
    // rotation around the y axis
    int rotation;
    // move towards target
    int moving_to_target;
    // state
    ApocalypseState state;
    // current frame
    int frame;
    // textures
    int texture_id;

} Apocalypse;

void apocalypse_update(Apocalypse* apoco);
void apocalypse_render(Apocalypse* apoco);
void apocalypse_init(Apocalypse* apoco);

#endif