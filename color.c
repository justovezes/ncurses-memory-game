#include "color.h"

void setup_color(void)
{
	start_color();
	
	init_pair(DEFAULT_COLOR, COLOR_WHITE, COLOR_BLACK);
	init_pair(CARD_COLOR, COLOR_WHITE, COLOR_BLUE);
	init_pair(SHOW_COLOR, COLOR_WHITE, COLOR_CYAN);
}
