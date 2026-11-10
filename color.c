#include "main.h"
#include "color.h"

void setup_color(void)
{
	start_color();
	
	init_pair(DEFAULT_COLOR, COLOR_WHITE, COLOR_BLACK);
	init_pair(CARD_COLOR, COLOR_WHITE, COLOR_BLUE);
	init_pair(SUCCESS_COLOR, COLOR_GREEN, COLOR_BLUE);
	init_pair(FAIL_COLOR, COLOR_RED, COLOR_BLUE);
}
