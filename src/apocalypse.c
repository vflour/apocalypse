/// Apocalypse's behaviour is a state machine:
// This is dumb.
// 1. Wandering: Apoc will wander around the map (within a grid range) to a random position.
// 2. Summoned: Apoc will walk towards the front point of the camera
// 3. Freefall: Apoc will not move because they are being flung
// 4. Playing: Apoc is distracted and is playing with an object, i.e. a cat toy or whatever

////////////////////////////////
#include "apocalypse.h"
#include <stdio.h>

// Model data
#include "apoco_dsm_bin.h"
#include "apoco_apoco_dsa_bin.h"
#include "apoco_tex_bin.h"
#include <time.h>


#define VELOCITY_F 0.2f


void do_nothing(Apocalypse* apoco) {}


void on_enter_wandering(Apocalypse* apoco) {
    //apoco->v_x = 1;
    //apoco->v_z = 1;
    apoco->target_x = (rand() % (apoco->bounds_x*2)) - apoco->bounds_x;
    apoco->target_z = (rand() % (apoco->bounds_z*2)) - apoco->bounds_z;
    apoco->moving_to_target = 1;
} 


void on_update_wandering(Apocalypse* apoco) {
    // on every 10th frame, change target
    if (apoco->frame % 256 == 0 && !apoco->moving_to_target) {
        on_enter_wandering(apoco);
    } 
	printf("\x1b[11;0HCurrent frame:     %u ", apoco->frame);
} 


void (*state_enter_functions[5]) (Apocalypse* apoco) = {do_nothing, on_enter_wandering};
void (*state_update_functions[5]) (Apocalypse* apoco) = {do_nothing, on_update_wandering};


///////////////////////////////////////////////////////////////////////////

int set_binary_direction(int a, int b, int* out) {
    *out = a - b < 1 ? 0 : 1;
    *out = a - b < 0 ? -1 : *out;
}

float set_direction(float a, float b, float* out) {
    float diff = a - b;

    if (diff > VELOCITY_F)
        *out = VELOCITY_F;
    else if (diff < -VELOCITY_F)
        *out = -VELOCITY_F;
    else
        *out = 0.0;
    //*out = diff < VELOCITY_F ? 0 : VELOCITY_F;
    //*out = diff < 0 && diff > -VELOCITY_F ? *out : -VELOCITY_F;
    //*out = diff == 0 ? 0 : *out;

}

void set_rotation(float x, float z, float offset, float* out ) {
    float abs_x = x < 0.0f ? -x : x;
    float abs_z = z < 0.0f ? -z : z;
    float is_negative = z < 0.0f;

    if (abs_x == abs_z) {
        *out = x < 0.0f ? 135 : 90;
        *out = x > 0.0f ? 45 : *out;
    } else {
        *out = x == 0.0f ? 90 : 0;
        *out = x < 0.0f ? 180 : *out;
    }

    *out = is_negative ? *out : -*out;
    *out = *out + offset;

}

int is_near(float a, float b) {
    float diff = a - b;
    return diff <= 1.0 && diff >= -1.0;
}


void apocalypse_set_state(Apocalypse* apoco, ApocalypseState state) {

    apoco->state = state;
    state_enter_functions[state](apoco);

}

void apocalypse_print_debug(Apocalypse* apoco, int is_near_target) {
    
    printf("\x1b[15;0HV_X:     %f ", apoco->v_x);
	printf("\x1b[16;0HV_Z:     %f ", apoco->v_z);
    printf("\x1b[17;0HT_X:     %i ", apoco->target_x);
	printf("\x1b[18;0HT_Z:     %i ", apoco->target_z);
    printf("\x1b[19;0HX:       %f ", apoco->x);
	printf("\x1b[20;0HZ:       %f ", apoco->z);

	printf("\x1b[21;0HIS_MOVING:     %u ", apoco->moving_to_target);
	printf("\x1b[22;0HROT:     %f", apoco->rotation);

}

void apocalypse_update(Apocalypse* apoco) {
    
    // First, states are checked
    state_update_functions[apoco->state](apoco);

    // If apocalypse is moving, then update their velocity
    int is_near_target = is_near(apoco->target_x, apoco->x) && 
                         is_near(apoco->target_y, apoco->y) &&
                         is_near(apoco->target_z, apoco->z);
    apoco->moving_to_target = apoco->moving_to_target && !is_near_target;

    if (!is_near_target && apoco->moving_to_target) {
        set_direction(apoco->target_x, apoco->x, &apoco->v_x); 
        set_direction(apoco->target_y, apoco->y, &apoco->v_y);
        set_direction(apoco->target_z, apoco->z, &apoco->v_z);

        set_rotation(apoco->v_x, apoco->v_z, -90, &apoco->rotation);
        //glRotateY(apoco->v_x);
    }

    // Then, update their position
    apoco->x += apoco->v_x;
    apoco->y += apoco->v_y;
    apoco->z += apoco->v_z;

    apocalypse_print_debug(apoco, is_near_target);
}

const static void *dsa_file = apoco_apoco_dsa_bin;
const static void *dsm_file = apoco_dsm_bin;

void apocalypse_render(Apocalypse* apoco) {

    // Get pointers to the animated model files
    const int num_frames = DSMA_GetNumFrames(dsa_file);
	apoco->frame = apoco->frame+1;

    glLoadIdentity();
    gluLookAt(0.0, 5.0, 20.0,  // camera possition
                0.0, 3.0, 0.0,  // look at
                0.0, 1.0, 0.0); // up

	glPushMatrix();
    {   
        glTranslatef(apoco->x, apoco->y, apoco->z);
        glRotateY(apoco->rotation);
        glBindTexture(0, apoco->texture_id);
    	DSMA_DrawModel(dsm_file, dsa_file, apoco->frame% num_frames);
	}
	glPopMatrix(1);
    
}


void apocalypse_init(Apocalypse* apoco) {

    srand(time(NULL)*2);
    glGenTextures(1, &apoco->texture_id);
    glBindTexture(0, apoco->texture_id);
    glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_64, TEXTURE_SIZE_64, 0,
                 TEXGEN_TEXCOORD, (u8*)apoco_tex_bin);



	//printf("\x1b[11;12HCurrent frame:     %u ", apoco->frame);

    srand(time(NULL));
    
    apocalypse_set_state(apoco, Wandering);
    apoco->bounds_x = 5;
    apoco->bounds_z = 5;

}