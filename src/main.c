
#include <nds.h>
#include <stdio.h>
#include "apocalypse.h"

Apocalypse apoco;

//
void video_init() {
	videoSetMode(MODE_0_3D);

    vramSetBankA(VRAM_A_TEXTURE);
    
    consoleDemoInit();
}


void gl_init() {

	glInit();

    glEnable(GL_ANTIALIAS);
    glEnable(GL_TEXTURE_2D);
 
    // Setup the rear plane
    glClearColor(2, 2, 2, 31); // BG must be opaque for AA to work
    glClearPolyID(63); // BG must have a unique polygon ID for AA to work
    glClearDepth(0x7FFF);

 
	glViewport(0,0,255,191);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70, 256.0 / 192.0, 0.1, 40);
 
    glLight(0, RGB15(255, 255, 255),                 0,  floattov10(0.8), floattov10(-0.2));

    glMaterialf(GL_AMBIENT, RGB15(255, 255, 255));
    glMaterialf(GL_DIFFUSE, RGB15(255, 255, 255));
    glMaterialf(GL_SPECULAR, BIT(15) | RGB15(0, 0, 0));
    glMaterialf(GL_EMISSION, RGB15(0, 0, 0));
    glMaterialShinyness();

 
    glMatrixMode(GL_MODELVIEW);

    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 |
              POLY_FORMAT_LIGHT1 | POLY_FORMAT_LIGHT2 | POLY_FORMAT_LIGHT3);

}


void sub_sprites_init(void) {
 
	oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);
 
	int x = 0;
	int y = 0;
 
	int id = 0;

	//set up a 4x3 grid of 64x64 sprites to cover the screen
	for(y = 0; y < 3; y++)
	for(x = 0; x < 4; x++)
	{
	/*	u16 *offset = &SPRITE_GFX_SUB[(x * 64) + (y * 64 * 256)];
 
		oamSet(&oamSub, x + y * 4, x * 64, y * 64, 0, 15, SpriteSize_64x64, 
			SpriteColorFormat_Bmp, offset, -1, false,false,false,false,false);
	*/
		oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
		oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
		id++;
	}
 
	swiWaitForVBlank();
 
	oamUpdate(&oamSub);
}


void init() {

	video_init();
	gl_init();
	apocalypse_init(&apoco);

}


void update() {
	apocalypse_update(&apoco);
}


void render() {

	apocalypse_render(&apoco);

}


int main() {
    init();
	
    while (true) {

		// wait for capture unit to be ready
		while(REG_DISPCAPCNT & DCAP_ENABLE);

		// Handle exit
		scanKeys();
		int keys = keysDown();
		if (keys & KEY_START) break;

        //lcdMainOnBottom();
        REG_DISPCAPCNT = DCAP_BANK(2) | DCAP_ENABLE | DCAP_SIZE(3);
		
		update();
        render();
		glFlush(0);
		swiWaitForVBlank();

    }

    return 0;
}