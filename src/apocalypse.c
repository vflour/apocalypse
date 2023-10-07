/// Apocalypse's behaviour is a state machine:
// This is dumb.
// 1. Wandering: Apoc will wander around the map (within a grid range) to a random position.
// 2. Summoned: Apoc will walk towards the front point of the camera
// 3. Freefall: Apoc will not move because they are being flung
// 4. Playing: Apoc is distracted and is playing with an object, i.e. a cat toy or whatever

////////////////////////////////
#include "apocalypse.h"
#include <stdio.h>
#include <time.h>
#include <nds/ndstypes.h>
#include "tangent.h"

// Model data
#include "apoco_dsm_bin.h"
#include "apoco_apoco_dsa_bin.h"
#include "apoco_tex_bin.h"


#define M_VELOCITY floattof32(0.1)
#define NEAR_BOUNDS floattof32(0.5)
#define R_VELOCITY degreesToAngle(3)

#define ROOM_BOUNDS inttof32(20)

void do_nothing(Apocalypse* apoco) {}


void on_enter_wandering(Apocalypse* apoco) {

    apoco->target_x = inttof32(mod32(rand(), apoco->bounds_x<<1)  - apoco->bounds_x);
    apoco->target_z = inttof32(mod32(rand(), apoco->bounds_z<<1)  - apoco->bounds_z);
    apoco->moving_to_target = 1;

} 


void on_enter_summoning(Apocalypse* apoco) {

    // move to camera, assuming cam pos is 0,0 for now
    apoco->target_x = 0;
    apoco->target_z = 0;
    apoco->moving_to_target = 1;

}


void on_update_wandering(Apocalypse* apoco) {
    
    // on every 10th frame, change target
    if (mod32(apoco->frame, 128) == 0 && !apoco->moving_to_target) {
        on_enter_wandering(apoco);
    } 
	printf("\x1b[11;0HCurrent frame:     %u ", apoco->frame);

} 


void (*state_enter_functions[5]) (Apocalypse* apoco) = {do_nothing, on_enter_wandering, on_enter_summoning};
void (*state_update_functions[5]) (Apocalypse* apoco) = {do_nothing, on_update_wandering, do_nothing};


///////////////////////////////////////////////////////////////////////////

void set_direction(Apocalypse* apoco) {
    
    apoco->v_x = mulf32(cosLerp(apoco->rotation), apoco->v);
    apoco->v_z = mulf32(sinLerp(apoco->rotation), apoco->v);

}

int clamp(int a, int min, int max) {
    if (a < min)
        return min;
    if (a > max) 
        return max;

    return a;
}

static inline void set_rotation(f32 x, f32 z, int* out ) {

    int result = atan2Lookup(x, z);
    if (*out != result) {
        
        int delta = mod32(result - *out, DEGREES_IN_CIRCLE);
        int dist = mod32(delta << 1, DEGREES_IN_CIRCLE) - delta;
        *out = *out + div32(dist, 12);

    }

}


int is_near(Apocalypse* apoco) {

    f32 x = apoco->target_x - apoco->x;
    f32 z = apoco->target_z - apoco->z;
    f32 root = sqrtf32(mulf32(x,x) + mulf32(z,z));
    return root <= NEAR_BOUNDS;

}

void apocalypse_set_state(Apocalypse* apoco, ApocalypseState state) {

    apoco->state = state;
    state_enter_functions[state](apoco);

}

void apocalypse_print_debug(Apocalypse* apoco, int is_near_target) {
    
    printf("\x1b[15;0HV_X:     %f ", f32tofloat(apoco->v_x));
	printf("\x1b[16;0HV_Z:     %f ", f32tofloat(apoco->v_z));
    printf("\x1b[17;0HT_X:     %f ", f32tofloat(apoco->target_x));
	printf("\x1b[18;0HT_Z:     %f ", f32tofloat(apoco->target_z));
    printf("\x1b[19;0HX:       %f ", f32tofloat(apoco->x));
	printf("\x1b[20;0HZ:       %f ", f32tofloat(apoco->z));
	printf("\x1b[21;0HEXPECTED:     %i ", angleToDegrees(atan2Lookup(apoco->target_x - apoco->x, apoco->target_z - apoco->z)));

	printf("\x1b[22;0HROT:     %i      ", angleToDegrees(apoco->rotation));

}

void apocalypse_update(Apocalypse* apoco) {
    
    // First, states are checked
    state_update_functions[apoco->state](apoco);

    // If apocalypse is moving, then update their velocity
    int is_near_target = is_near(apoco);
    apoco->moving_to_target = apoco->moving_to_target && !is_near_target;
    apoco->v = 0;
    if (!is_near_target && apoco->moving_to_target) {
        
        set_rotation(apoco->target_x - apoco->x, apoco->target_z - apoco->z, &apoco->rotation);
        apoco->v = M_VELOCITY;

    }
    set_direction(apoco);

    // Then, update their position
    apoco->x += apoco->v_x;
    apoco->y += apoco->v_y;
    apoco->z += apoco->v_z;

    apocalypse_print_debug(apoco, is_near_target);
    
}

f32 cam_x = 0;
f32 cam_z = 0;
const static void *dsa_file = apoco_apoco_dsa_bin;
const static void *dsm_file = apoco_dsm_bin;

//display list
u32 floor[] = 
{
	17,
	FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_COLOR, FIFO_VERTEX16, FIFO_COLOR),
	GL_QUADS,
	RGB15(3,3,6),
	VERTEX_PACK(f32tov16(-ROOM_BOUNDS),inttov16(0)), VERTEX_PACK(f32tov16(-ROOM_BOUNDS),inttov16(0)),
	RGB15(5,5,8),
	FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_COLOR, FIFO_VERTEX16, FIFO_COLOR),
	VERTEX_PACK(f32tov16(-ROOM_BOUNDS),inttov16(0)), VERTEX_PACK(f32tov16(ROOM_BOUNDS),inttov16(0)),
	RGB15(5,5,8),
	VERTEX_PACK(f32tov16(ROOM_BOUNDS),inttov16(0)), VERTEX_PACK(f32tov16(ROOM_BOUNDS),inttov16(0)),
	RGB15(3,3,6),
    // what now?
	FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_END, FIFO_NOP, FIFO_NOP),
	VERTEX_PACK(f32tov16(ROOM_BOUNDS),inttov16(0)), VERTEX_PACK(f32tov16(-ROOM_BOUNDS),inttov16(0)),
    0,
    0,
};

u32 wall[] = 
{
	17,
	FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_COLOR, FIFO_VERTEX16, FIFO_COLOR),
	GL_QUADS,
	RGB15(4,4,8),
	VERTEX_PACK(f32tov16(-ROOM_BOUNDS),f32tov16(-ROOM_BOUNDS)), VERTEX_PACK(0,0),
	RGB15(0,0,4),
	FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_COLOR, FIFO_VERTEX16, FIFO_COLOR),
	VERTEX_PACK(f32tov16(-ROOM_BOUNDS),f32tov16(ROOM_BOUNDS)), VERTEX_PACK(0,0),
	RGB15(0,0,4),
	VERTEX_PACK(f32tov16(ROOM_BOUNDS),f32tov16(ROOM_BOUNDS)), VERTEX_PACK(0,0),
	RGB15(4,4,8),
    // what now?
	FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_END, FIFO_NOP, FIFO_NOP),
	VERTEX_PACK(f32tov16(ROOM_BOUNDS),f32tov16(-ROOM_BOUNDS)), VERTEX_PACK(0,0),
    0,
    0,
};

void room_render() {
    // this is probably the worst way to render a model that doesnt exist yet
    // however i am lazy
    glPushMatrix();
    {
        glScalef32(inttof32(5),inttof32(2),inttof32(5));
        glTranslatef32(0,inttof32(-1),0);
        glCallList(floor);
    }
	glPopMatrix(1);

    // left
    glPushMatrix();
    {
        glTranslatef32(-ROOM_BOUNDS,0,0);
        glScalef32(inttof32(5),inttof32(2),inttof32(5));
        glRotateYi( degreesToAngle(-90) );
        glCallList(wall);
    }
	glPopMatrix(1);
    // right
    glPushMatrix();
    {
        glTranslatef32(ROOM_BOUNDS,0,0);
        glScalef32(inttof32(5),inttof32(2),inttof32(5));
        glRotateYi( degreesToAngle(90) );
        glCallList(wall);
    }
	glPopMatrix(1);
    // front
    glPushMatrix();
    {
        glTranslatef32(0,0,ROOM_BOUNDS);
        glScalef32(inttof32(5),inttof32(2),inttof32(5));
        glCallList(wall);
    }
	glPopMatrix(1);
    // back
    glPushMatrix();
    {
        glTranslatef32(0,0,-ROOM_BOUNDS);
        glScalef32(inttof32(5),inttof32(2),inttof32(5));
        glRotateYi( degreesToAngle(180) );
        glCallList(wall);
    }
	glPopMatrix(1);

} 


void apocalypse_render(Apocalypse* apoco) {

    // Get pointers to the animated model files
    const int num_frames = DSMA_GetNumFrames(dsa_file);
	apoco->frame = apoco->frame+1;

    glLoadIdentity();

    //camera repelling mechanism
    f32 d = sqrtf32(mulf32(apoco->x, apoco->x) + mulf32(apoco->z, apoco->z));
    f32 v = sqrtf32(mulf32(apoco->v_x, apoco->v_x) + mulf32(apoco->v_z, apoco->v_z));
    f32 c_cam_x = 0;
    f32 c_cam_z = 0;

    if (d <= inttof32(8)) {
        f32 d_repel = inttof32(8);
        if (d > inttof32(1)) {
            d_repel = inttof32(8) - d;
        }
        c_cam_x = -mulf32(divf32(apoco->x, d), d_repel);  
        c_cam_z = -mulf32(divf32(apoco->z, d), d_repel); 
    }
    
    cam_x = cam_x + mulf32(c_cam_x - cam_x, floattof32(0.2f));
    cam_z = cam_z + mulf32(c_cam_z - cam_z, floattof32(0.2f));

    printf("\x1b[2;0HD:     %f ", f32tofloat(d));
    printf("\x1b[3;0HCAM_X:     %f ", f32tofloat(cam_x));
	printf("\x1b[4;0HCAM_Z:     %f ", f32tofloat(cam_z));

    gluLookAtf32(
        cam_x,   inttof32(2), cam_z,  // camera possition
        apoco->x, apoco->y, apoco->z,  // look at
        0, inttof32(1), 0); // up

    // TODO: MOVE THIS TO MAP
    room_render();
    
    // actually draw the bastard
    glPushMatrix();
    {   
        glScalef32(floattof32(0.5f),floattof32(0.5f),floattof32(0.5f));
        glTranslatef32(apoco->x, apoco->y, apoco->z);
        
        glRotateYi( mod32(-apoco->rotation - DEGREES_IN_CIRCLE - degreesToAngle(90), DEGREES_IN_CIRCLE) );

        glBindTexture(0, apoco->texture_id);
     	DSMA_DrawModel(dsm_file, dsa_file, apoco->frame% num_frames);
	}
	glPopMatrix(1);

    
}


void apocalypse_init(Apocalypse* apoco) {

    srand(time(NULL));
    glGenTextures(1, &apoco->texture_id);
    glBindTexture(0, apoco->texture_id);
    glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_64, TEXTURE_SIZE_64, 0,
                 TEXGEN_TEXCOORD, (u8*)apoco_tex_bin);

    apoco->bounds_x = f32toint(ROOM_BOUNDS) - 2;
    apoco->bounds_z = f32toint(ROOM_BOUNDS) - 2;
    apocalypse_set_state(apoco, Wandering);

}