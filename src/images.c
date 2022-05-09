#include "mwImages.h"
#include "mwFxcg.h"

extern unsigned short* vram;

struct ImageType {
	unsigned short width, height;
	const color_t* data;
};

extern const color_t DATA_0[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_1[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_2[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_3[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_4[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_5[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_6[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_7[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_8[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_UNKNOWN[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_KNOWN_MINE[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_QUESTION[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_MINE[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_MINE_EXPLODED[BLOCK_SIZE * BLOCK_SIZE];
extern const color_t DATA_WRONG_MINE[BLOCK_SIZE * BLOCK_SIZE];

static const struct ImageType IMAGES[15] = {
	{BLOCK_SIZE, BLOCK_SIZE, DATA_0},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_1},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_2},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_3},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_4},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_5},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_6},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_7},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_8},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_UNKNOWN},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_KNOWN_MINE},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_QUESTION},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_MINE},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_MINE_EXPLODED},
	{BLOCK_SIZE, BLOCK_SIZE, DATA_WRONG_MINE}
};

static const color_t BORDER_COLOR = COLOR_BLACK;

void putToVRAM(
	enum Image image,
	short x, short y,
	unsigned short border
) {
	const struct ImageType* imagePtr = IMAGES + image;
	int xPixelCoord, yPixelCoord;
	for (int i = 0; i < imagePtr->width * imagePtr->height; i++) {
		xPixelCoord = i % imagePtr->width;
		yPixelCoord = i / imagePtr->width;
		*(vram + (yPixelCoord + y) * LCD_WIDTH_PX + xPixelCoord + x)
			= (xPixelCoord < border || xPixelCoord >= imagePtr->width - border
				|| yPixelCoord < border || yPixelCoord >= imagePtr->height - border)
				? BORDER_COLOR : imagePtr->data[i];
	}
}
