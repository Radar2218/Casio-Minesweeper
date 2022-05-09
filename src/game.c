#include "mwGame.h"
#include "mwRandom.h"
#include "mwImages.h"
#include "mwFxcg.h"

#include <string.h>
#include <stdio.h>

#define GAME_WIDTH 24
#define GAME_HEIGHT 12

static unsigned char mineField[GAME_WIDTH * GAME_HEIGHT];
static enum Image images[GAME_WIDTH * GAME_HEIGHT];
static char counted[GAME_WIDTH * GAME_HEIGHT];
static unsigned int index = 0;
static unsigned char running = 0;
static int numMines, minesFound;

static void setImage(unsigned int index, enum Image image, unsigned char border) {
	images[index] = image;
	putToVRAM(
		images[index],
		index % GAME_WIDTH * BLOCK_SIZE,
		index / GAME_WIDTH * BLOCK_SIZE + IMAGE_OFFSET,
		border
	);
}

static unsigned char count(unsigned int index) {
	int base = index - GAME_WIDTH - 1;
	unsigned char counted = 0;
	for (unsigned char i = 0; i < 9; i++) {
		if (i == 4) { continue; }
		if ((index / GAME_WIDTH == 0 && i < 3) || (index / GAME_WIDTH == GAME_HEIGHT - 1 && i > 5)){ continue; }
		if ((index % GAME_WIDTH == 0 && i % 3 == 0) || (index % GAME_WIDTH == GAME_WIDTH - 1 && i % 3 == 2)) { continue; }

		if (mineField[base + i / 3 * GAME_WIDTH + i % 3] == 1) {
			counted = counted + 1;
		}
	}
	return counted;
}

static void changeMinesFound(int amount) {
	minesFound = minesFound + amount;
	char text[13];
	int x = 0, y = 3;
	int minesLeft = (numMines - minesFound < 0) ? 0 : numMines - minesFound;
	if (minesLeft >= 100) { sprintf(text, "%d", minesLeft); }
	else if (minesLeft >= 10) { sprintf(text, " %d", minesLeft); }
	else { sprintf(text, "  %d", minesLeft); }
	PrintMini(&x, &y, text, 0x0, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 0, 0);
	x = LCD_WIDTH_PX - x - 10;
	PrintMini(&x, &y, text, 0x40, 0xffffffff, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
}

static void printMsgGameEnd(const char* msg) {
	MsgBoxPush(6);
	PrintXY(3, 2, msg, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 3, "xxNew Game:", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 4, "xx [F1]easy", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 5, "xx [F2]medium", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 6, "xx [F3]hard", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	int key;
	GetKey(&key);
	MsgBoxPop();
	if (key == KEY_CTRL_F1) {
		newGame(30);
	} else if (key == KEY_CTRL_F2) {
		newGame(50);
	} else if (key == KEY_CTRL_F3) {
		newGame(70);
	} else if (key == KEY_CTRL_F4) {
		newGame(1);
	}
}

static void checkForWin(void) {
	if (minesFound == numMines && running) {
		unsigned short correctMines = 0;
		for (unsigned int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
			if (mineField[i] == 1 && images[i] == BLOCK_KNOWN_MINE) {
				correctMines = correctMines + 1;
			}
		}
		if (correctMines == numMines) {
			endGame();
			printMsgGameEnd("xxYou won!");
		}
	}
}

static void indexChange(int amount) {
	setImage(index, images[index], 0);
	index = index + amount;
	setImage(index, images[index], 1);
}

static void uncoverEmpty(unsigned int start) {
	unsigned int indices[GAME_WIDTH * GAME_HEIGHT];
	unsigned int newIndeces[GAME_WIDTH * GAME_HEIGHT];
	indices[0] = start;
	unsigned int length = 1, newLength = 0, tempIndex;
	int base;
	while (length > 0) {
		for (unsigned int j = 0; j < length; j++) {
			base = indices[j] - GAME_WIDTH - 1;
			for (unsigned char i = 0; i < 9; i++) {
				if (i == 4) { continue; }
				if ((indices[j] / GAME_WIDTH == 0 && i < 3) || (indices[j] / GAME_WIDTH == GAME_HEIGHT - 1 && i > 5)) { continue; }
				if ((indices[j] % GAME_WIDTH == 0 && i % 3 == 0) || (indices[j] % GAME_WIDTH == GAME_WIDTH - 1 && i % 3 == 2)) { continue; }

				tempIndex = base + i / 3 * GAME_WIDTH + i % 3;
				if (images[tempIndex] == BLOCK_UNKNOWN || images[tempIndex] == BLOCK_QUESTION) {
					setImage(tempIndex, counted[tempIndex], 0);
					if (counted[tempIndex] == 0) {
						newIndeces[newLength] = tempIndex;
						newLength = newLength + 1;
					}
				}
			}
		}
		length = newLength;
		newLength = 0;
		memcpy(indices, newIndeces, sizeof(unsigned int) * GAME_WIDTH * GAME_HEIGHT);
	}
}

static void uncover(void) {
	if (images[index] == BLOCK_UNKNOWN || images[index] == BLOCK_QUESTION) {
		if (mineField[index] == 1) {
			setImage(index, BLOCK_MINE_EXPLODED, 1);
			endGame();
			printMsgGameEnd("xxYou lose!");
		} else {
			setImage(index, counted[index], 1);
			if (counted[index] == 0) {
				uncoverEmpty(index);
			}
		}
	} else if (images[index] == BLOCK_KNOWN_MINE) {
		setImage(index, BLOCK_UNKNOWN, 1);
		changeMinesFound(-1);
	} else if (images[index] <= BLOCK_8) {
		if (images[index] == BLOCK_0) {
			uncoverEmpty(index);
		}

		unsigned char found = 0;
		unsigned int base = index - GAME_WIDTH - 1;
		unsigned int tempIndex;

		for (unsigned char i = 0; i < 9; i++) {
			if (i == 4) { continue; }
			if ((index / GAME_WIDTH == 0 && i < 3) || (index / GAME_WIDTH == GAME_HEIGHT - 1 && i > 5)) { continue; }
			if ((index % GAME_WIDTH == 0 && i % 3 == 0) || (index % GAME_WIDTH == GAME_WIDTH - 1 && i % 3 == 2)) { continue; }

			tempIndex = base + i / 3 * GAME_WIDTH + i % 3;
			if (images[tempIndex] == BLOCK_KNOWN_MINE || images[tempIndex] == BLOCK_MINE_EXPLODED) {
				found += 1;
			}
		}

		if (found >= counted[index]) {
			for (unsigned char i = 0; i < 9; i++) {
				if (i == 4) { continue; }
				if ((index / GAME_WIDTH == 0 && i < 3) || (index / GAME_WIDTH == GAME_HEIGHT - 1 && i > 5)) { continue; }
				if ((index % GAME_WIDTH == 0 && i % 3 == 0) || (index % GAME_WIDTH == GAME_WIDTH - 1 && i % 3 == 2)) { continue; }

				tempIndex = base + i / 3 * GAME_WIDTH + i % 3;
				if (images[tempIndex] == BLOCK_UNKNOWN) {
					if (counted[tempIndex] >= 0) {
						setImage(tempIndex, counted[tempIndex], 0);
						if (counted[tempIndex] == 0) {
							uncoverEmpty(tempIndex);
						}
					} else {
						setImage(tempIndex, BLOCK_MINE_EXPLODED, 0);
						endGame();
						printMsgGameEnd("xxYou lose!");
						break;
					}
				}
			}
		}
	}
}

static void toggleFlag(void) {
	if (images[index] == BLOCK_UNKNOWN) {
		setImage(index, BLOCK_KNOWN_MINE, 1);
		changeMinesFound(1);
	} else if (images[index] == BLOCK_KNOWN_MINE) {
		setImage(index, BLOCK_QUESTION, 1);
		changeMinesFound(-1);
	} else if (images[index] == BLOCK_QUESTION) {
		setImage(index, BLOCK_UNKNOWN, 1);
	}
}

void init(void) {
	static unsigned char initialized = 0;
	if (!initialized) {
		initRandom();
		memset(mineField, 0, GAME_WIDTH * GAME_HEIGHT);
		memset(images, 0, sizeof(enum Image) * GAME_WIDTH * GAME_HEIGHT);
		memset(counted, 0, GAME_WIDTH * GAME_HEIGHT);
		initialized = 1;
	}
}

void firstStart(void) {
	MsgBoxPush(6);
	PrintXY(3, 2, "xxNew Game:", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 3, "xx [F1]easy", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 4, "xx [F2]medium", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 5, "xx [F3]hard", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 6, "xx[F5]place flag", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	PrintXY(3, 7, "xx[F6]uncover", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
	int key;
	GetKey(&key);
	while (!(key == KEY_CTRL_F1 || key == KEY_CTRL_F2 || key == KEY_CTRL_F3 || key == KEY_CTRL_F4)) {
		GetKey(&key);
	}
	MsgBoxPop();
	if (key == KEY_CTRL_F1) {
		newGame(30);
	} else if (key == KEY_CTRL_F2) {
		newGame(50);
	} else if (key == KEY_CTRL_F3) {
		newGame(70);
	} else if (key == KEY_CTRL_F4) {
		newGame(1);
	}
}

void newGame(unsigned int mines) {
	index = 0;
	numMines = mines;
	minesFound = 0;
	changeMinesFound(0);

	int minesLeft = numMines;
	for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
		mineField[i] = getRandomNum(0, GAME_WIDTH * GAME_HEIGHT - i) < minesLeft ? 1 : 0;
		if (mineField[i] == 1) {
			minesLeft = minesLeft - 1;
		}
		setImage(i, BLOCK_UNKNOWN, (i == 0 ? 1 : 0));
	}

	for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
		if (mineField[i] == 1) {
			counted[i] = -1;
		} else {
			counted[i] = count(i);
		}
	}

	running = 1;
}

void endGame(void) {
	setImage(index, images[index], 0);
	for (unsigned int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
		if (images[i] == BLOCK_UNKNOWN || images[i] == BLOCK_QUESTION) {
			if (mineField[i]) {
				setImage(i, BLOCK_MINE, 0);
			} else {
				setImage(i, counted[i], 0);
			}
		} else if (images[i] == BLOCK_KNOWN_MINE) {
			if (mineField[i] == 0) {
				setImage(i, BLOCK_WRONG_MINE, 0);
			}
		}
	}

	running = 0;
}

void processInput(int key) {
	if (running) {
		if (key == KEY_CTRL_RIGHT && index % GAME_WIDTH < GAME_WIDTH - 1) {
			indexChange(1);
		} else if (key == KEY_CTRL_LEFT && index % GAME_WIDTH > 0) {
			indexChange(-1);
		} else if (key == KEY_CTRL_DOWN && index / GAME_WIDTH < GAME_HEIGHT - 1) {
			indexChange(GAME_WIDTH);
		} else if (key == KEY_CTRL_UP && index / GAME_WIDTH > 0) {
			indexChange(-GAME_WIDTH);
		} else if (key == KEY_CTRL_F5) {
			toggleFlag();
		} else if (key == KEY_CTRL_F6) {
			uncover();
		}
	} else {
		if (key == KEY_CTRL_F1) {
			newGame(30);
		} else if (key == KEY_CTRL_F2) {
			newGame(50);
		} else if (key == KEY_CTRL_F3) {
			newGame(70);
		} else if (key == KEY_CTRL_F4) {
			newGame(1);
		}
	}
	checkForWin();
}
