#ifndef MW_IMAGES_H
#define MW_IMAGES_H

#define BLOCK_SIZE 16
#define IMAGE_OFFSET 24

enum Image {
	BLOCK_0, BLOCK_1, BLOCK_2, BLOCK_3, BLOCK_4, BLOCK_5, BLOCK_6, BLOCK_7, BLOCK_8, BLOCK_UNKNOWN,
	BLOCK_KNOWN_MINE, BLOCK_QUESTION, BLOCK_MINE, BLOCK_MINE_EXPLODED, BLOCK_WRONG_MINE
};

void putToVRAM(
	enum Image image,
	short x, short y,
	unsigned short border
);

#endif