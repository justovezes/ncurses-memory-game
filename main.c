#include "main.h"
#include "menu.h"
#include "game.h"

int main(void)
{
	MODE mode;

	initscr();
	setup_color();
	noecho();
	keypad(stdscr, true);

	while ((mode = menu()) != EXIT_CODE) {
		game(mode);
	}

	endwin();
}
