#include <stdio.h>
#include <string.h>

#include "mwFxcg.h"
#include "mwGame.h"

unsigned short* vram;

int main() {
	vram = (unsigned short*)GetVRAMAddress();
	Bdisp_EnableColor(1);

	init();
	firstStart();

	int key;
	while (1) {
		GetKey(&key);
		processInput(key);
		Bdisp_PutDisp_DD();
	}
	return 0;
}