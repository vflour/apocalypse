#ifndef APOCALYPSE_H
#define APOCALYPSE_H

#include <nds.h>
#include <dsma.h>

typedef enum ApocalypseState {
    Idle,
    Wandering,
    Summoned,
    Freefall,
    Playing
} ApocalypseState;

typedef struct Apocalypse {
    // position
    float x;
    float y;
    float z;
    // velocity
    float v_x;
    float v_y;
    float v_z;
    // move towards target
    int moving_to_target;
    // target
    int target_x;
    int target_y;
    int target_z;
    // bounds (+- n, where n is either in the x or z directions)
    int bounds_x;
    int bounds_z;
    // rotation around the y axis
    float rotation;

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