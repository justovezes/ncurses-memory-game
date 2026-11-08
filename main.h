#ifndef MAIN_HEADER
#define MAIN_HEADER

#include <ncurses.h>
#include "color.h"

#ifndef ENTER_KEY 
#define ENTER_KEY '\n'
#endif

#define EXIT_CODE 999

typedef enum {EASY, MEDIUM, HARD, TOTAL_MODES} MODE;

typedef struct vector2 {
	int y;
	int x;
} Vector2;

#endif
