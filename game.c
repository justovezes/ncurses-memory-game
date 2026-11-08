#include "game.h"

// game config.
const GameModeSettings game_mode_settings[TOTAL_MODES] =
	{ { .map_size = {2, 8}, .timer = 120 },
	  { .map_size = {3, 8}, .timer = 150 },
	  { .map_size = {4, 8}, .timer = 180 } };
GameModeSettings const *current_setting;

static const int const symbols[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '!', '@', '#',
			            '$', '%', '&', '*', '/', '|', 'X', '~', '[', ']', '+', '-', ';', };

const SymbolList symbols_list = {
	.available_symbols = (int) (sizeof(symbols) / sizeof(symbols[0])),
	.symbols = symbols,
};

static void generate_map(WINDOW *win, int map_rows, int map_cols, Card (*card_map)[map_cols]);
static int get_symbol(void);
static bool number_is_used(int number, int list_size, int *used_list);
static void refresh_map(WINDOW *win, Card *selected_card, int map_rows, int map_cols, Card (*card_map)[map_cols]);
static void draw_box(WINDOW *win, Position position, int height, int width);
static int *shuffle_array(int new_array_size, int array_size, int *array);

void game(MODE mode)
{
	WINDOW *stats_win;
	WINDOW *game_win, *game_contents_win;
	WINDOW *credits_win;

	current_setting = game_mode_settings + mode;
	Card *selected_card;
	Card *shown_cards[2] = { NULL, NULL };
	
	stats_win = newwin(1, COLS, 0, 0);

	game_win = newwin(LINES - 2, COLS, 1, 0);
	box(game_win, 0, 0);
	game_contents_win = derwin(game_win, LINES-4, COLS-2, 1, 1);

	credits_win = newwin(1, COLS, LINES-1, 0);

	// pointer to array of x elements.
	Card (*card_map)[current_setting->map_size.x];
	
	card_map = malloc(sizeof(*card_map) * current_setting->map_size.y);
	generate_map(game_contents_win, current_setting->map_size.y, current_setting->map_size.x, card_map);
	selected_card = card_map[0];

	refresh();
	refresh_map(game_contents_win, selected_card, current_setting->map_size.y, current_setting->map_size.x, card_map);
	touchwin(game_win);
	wrefresh(game_win);
	getch();
}


static void generate_map(WINDOW *win, int map_rows, int map_cols, Card (*card_map)[map_cols])
{
	int rows, cols;
	getmaxyx(win, rows, cols);

	int map_height = map_rows * 4 - 1;
	int map_width = map_cols * 4 - 1;
	
	int start_row_pos = (rows - map_height) / 2;
	int start_col_pos = (cols - map_width) / 2;

	for (int current_row = 0; current_row < map_rows; current_row++) {
		for (int current_col = 0; current_col < map_cols; current_col++) {
			card_map[current_row][current_col].symbol = get_symbol();
			card_map[current_row][current_col].position = (Position) { start_row_pos + current_row * 4, start_col_pos + current_col * 5 };
			card_map[current_row][current_col].is_available = true;
		}
	}
}

static int get_symbol(void)
{
	// criar um array do tamanho do mapa com simbolos onde cada simbolo tem uma copia posicionados de forma aleatoria

	static int *map_symbols_list = NULL;
	static int total_map_symbols;
	static int index = 0;

	if (map_symbols_list == NULL) {
		total_map_symbols = current_setting->map_size.y * current_setting->map_size.x / 2;
		map_symbols_list = shuffle_array(total_map_symbols, symbols_list.available_symbols, symbols_list.symbols);

	}
	
	if (index == total_map_symbols - 1) {
		int tmp_value = map_symbols_list[index];
		free(map_symbols_list);
		index = 0;
		map_symbols_list = NULL;
		return tmp_value;
	}
	return map_symbols_list[index++];
}

static bool number_is_used(int number, int list_size, int *used_list)
{
	for (int i = 0; i < list_size; i++) {
		if (number == used_list[i])
			return true;
	}
	return false;
}

static void refresh_map(WINDOW *win, Card *selected_card, int map_rows, int map_cols, Card (*card_map)[map_cols])
{
	werase(win);
	wattrset(win, COLOR_PAIR(CARD_COLOR));
	for (int current_row = 0; current_row < map_rows; current_row++) {
		for (int current_col = 0; current_col < map_cols; current_col++) {
			for (int y = 0; y < 3; y++) {
				for (int x = 0; x < 3; x++) {
					if (y == 1 && x == 1)
						mvwaddch(win, card_map[current_row][current_col].position.y + y, card_map[current_row][current_col].position.x + x, card_map[current_row][current_col].symbol);
					else
						mvwaddch(win, card_map[current_row][current_col].position.y + y, card_map[current_row][current_col].position.x + x, ' ');
				}
			}
		}

	}
	wattrset(win, A_NORMAL);
	draw_box(win, selected_card->position, 5, 5);	
}

static void draw_box(WINDOW *win, Position position, int height, int width)
{
	int y = position.y - 1,
	    x = position.x - 1;
	
	mvwaddch(win, y, x, ACS_ULCORNER);
	mvwaddch(win, y, x + width - 1, ACS_URCORNER);
	mvwaddch(win, y + height - 1, x, ACS_LLCORNER);
	mvwaddch(win, y + height - 1, x + width - 1, ACS_LRCORNER);

	mvwhline(win, y, x + 1, ACS_HLINE, width - 2);
	mvwhline(win, y + height - 1, x + 1, ACS_HLINE, width - 2);

	mvwvline(win, y + 1, x, ACS_VLINE, height - 2);
	mvwvline(win, y + 1, x + width - 1, ACS_VLINE, height - 2);
}

static int *shuffle_array(int new_array_size, int array_size, int *array)
{
	int *new_array;
	int tmp_num;
	int index = 0;

	if (new_array_size > array_size)
		return NULL;
	
	new_array = malloc(sizeof(int) * new_array_size);

	for (int i = 0; i < new_array_size; i++) {
		do {
			getrandom(&tmp_num, sizeof(int), 0);
			tmp_num %= array_size;
		} while (number_is_used(tmp_num, index, new_array));
		new_array[index++] = array[tmp_num];
	}

	return new_array;
}	
