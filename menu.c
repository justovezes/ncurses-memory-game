#include "menu.h"

static void menu_print(WINDOW *win, int row, int cols, char *str, bool is_selected);

MODE menu(void)
{
	WINDOW *menu_win, *menu_contents_win;

	char *select_str = "Select mode:";
	char *modes_str[] = {"Easy", "Medium", "Hard"};
	char *leave_str = "Press 'q' to leave.\n";
	char *credit_str = "Made by Justo Vezes, 11/5/2026";

	int input;
	int rows, cols;
	MODE selected = EASY;

	menu_win = newwin(LINES - 2, COLS, 1, 0);
	box(menu_win, 0, 0);
	menu_contents_win = derwin(menu_win, LINES - 4, COLS - 2, 1, 1);
	getmaxyx(menu_contents_win, rows, cols);
	
	menu_print(menu_contents_win, 2, cols, select_str, false);
	menu_print(menu_contents_win, rows - 2, cols, credit_str, false);
	menu_print(menu_contents_win, rows - 3, cols, leave_str, false);

	while (true) {
		refresh();
		menu_print(menu_contents_win, rows/2 - 2, cols, modes_str[EASY], selected == EASY ? true : false);
		menu_print(menu_contents_win, rows/2, cols, modes_str[MEDIUM], selected == MEDIUM ? true : false);
		menu_print(menu_contents_win, rows/2 + 2, cols, modes_str[HARD], selected == HARD ? true : false);
		touchwin(menu_win);
		wrefresh(menu_win);

		switch (getch()) {
			case KEY_UP: case 'w':
				selected = (TOTAL_MODES + selected - 1) % TOTAL_MODES;
				break;
			case KEY_DOWN: case 's':
				selected = (selected + 1) % TOTAL_MODES;
				break;
      case ENTER_KEY: case ' ':
				delwin(menu_contents_win);
				delwin(menu_win);
				return selected;
			case 'q':
				delwin(menu_contents_win);
				delwin(menu_win);
				return EXIT_CODE;
			default: break;
		}
	}
}

static void menu_print(WINDOW *win, int row, int cols, char *str, bool is_selected)
{
	mvwaddstr(win, row, cols/2 - strlen(str)/2 - 2, is_selected ? "> " : "  ");
	mvwaddstr(win, row, cols/2 - strlen(str)/2, str);
	mvwaddstr(win, row, cols/2 + strlen(str)/2, is_selected ? " <" : "  ");
}
