#include "mwRandom.h"
#include "mwFxcg.h"
#include <limits.h>

static unsigned int number = 0;

static unsigned int rand() {
	number = number * 48271 % 2147483647;
	return number;
}

void initRandom(void) {
	number = (unsigned int)RTC_GetTicks();
}

int getRandomNum(int min, int max) {
	return rand() % (max - min) + min;
}
