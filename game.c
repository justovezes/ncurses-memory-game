#include "game.h"
#include "color.h"
#include "main.h"
#include <ncurses.h>
#include <unistd.h>

// game config.
const GameModeSettings game_mode_settings[TOTAL_MODES] =
	{ { .map_size = {2, 8}, .timer = 120 },
	  { .map_size = {3, 8}, .timer = 150 },
	  { .map_size = {4, 8}, .timer = 180 } };
GameModeSettings const *current_setting;

static int symbols[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '!', '@', '#',
			            '$', '%', '&', '*', '/', '|', 'X', '~', '[', ']', '+', '-', ';', };

const SymbolList symbols_list = {
	.available_symbols = (int) (sizeof(symbols) / sizeof(symbols[0])),
	.symbols = symbols,
};

static void generate_map(WINDOW *win, int map_rows, int map_cols, Card (*card_map)[map_cols]);
static int get_symbol(void);
static bool number_is_used(int number, int list_size, int *used_list);
static void refresh_map(WINDOW *win, Position selected_card, int map_rows, int map_cols, Card (*card_map)[map_cols]);
static void draw_box(WINDOW *win, Position position, int height, int width);
static int *shuffle_array(int new_array_size, int array_size, int *array);
static bool cards_remain(int map_rows, int map_cols, Card (*card_map)[map_cols]);
static Position select_neighbor(Position current_card, DIRECTION direction, int map_rows, int map_cols, Card (*card_map)[map_cols]);
static void show_card(Card **shown_cards, Position current_card, int map_rows, int map_cols, Card (*card_map)[map_cols]);
static bool check_card(WINDOW *win, Card **shown_cards);
static Position select_any_neighbor(int map_rows, int map_cols, Card (*card_map)[map_cols]);

void game(MODE mode)
{
	current_setting = game_mode_settings + mode;

	WINDOW *stats_win;
	WINDOW *game_win, *game_contents_win;
	WINDOW *credits_win;
  int current_timer = current_setting->timer;
  int tick_count = 0;
  
	Position selected_card;
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
	selected_card = (Position) { 0, 0 };

	keypad(game_win, true);
  curs_set(0);
  
	refresh();
  refresh_map(game_contents_win, selected_card, current_setting->map_size.y, current_setting->map_size.x, card_map);
  touchwin(game_win);
  wrefresh(game_win);

  while (current_timer >= 0) {
    
    int input = wgetch(game_win);
    DIRECTION direction = NO_DIRECTION;
    bool moved = false;
    //if (!cards_remain(current_setting->map_size.x, current_setting->map_size.y, card_map))
      // you won!!!!!
    
    switch (input) {
      case 'w': case KEY_UP:
        direction = UP;
        moved = true;
        break;
      case 'a': case KEY_LEFT:
        direction = LEFT;
        moved = true;
        break;
      case 'd': case KEY_RIGHT:
        direction = RIGHT;
        moved = true;
        break;
      case 's': case KEY_DOWN:
        direction = DOWN;
        moved = true;
        break;
      case ' ': case '\n':
        show_card(shown_cards, selected_card, current_setting->map_size.y, current_setting->map_size.x, card_map);
        if (shown_cards[1] != NULL) {
          if (check_card(game_win, shown_cards))
            shown_cards[0]->is_available = shown_cards[1]->is_available = false;
          shown_cards[0]->is_shown = shown_cards[1]->is_shown = false;
          shown_cards[0] = shown_cards[1] = NULL;
        }
        break;
      case 'q':
        goto QUIT;
        break;
      default:
        break;
    }
    
      if (moved)
        selected_card = select_neighbor(selected_card, direction, current_setting->map_size.y, current_setting->map_size.x, card_map);
      refresh_map(game_contents_win, selected_card, current_setting->map_size.y, current_setting->map_size.x, card_map);
      touchwin(game_win);
      wrefresh(game_win);
    }
    
    usleep(50000);
    tick_count = (tick_count + 1) % 20;
    if (!tick_count)
      current_timer--;

  QUIT:
    return;

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
			card_map[current_row][current_col].is_shown = false;
		}
	}
}

static int get_symbol(void)
{
	static int *map_symbols_list = NULL;
	static int total_map_symbols;
	static int index = 0;

	if (map_symbols_list == NULL) {
    int *random_symbols_list;
    int *old_pointer;
    total_map_symbols = (current_setting->map_size.y * current_setting->map_size.x) / 2;
    map_symbols_list = malloc(total_map_symbols * sizeof(int) * 2);

    random_symbols_list = shuffle_array(total_map_symbols, symbols_list.available_symbols, symbols_list.symbols);
    for (int i = 0; i < total_map_symbols; i++) {
      map_symbols_list[i * 2] = random_symbols_list[i];
      map_symbols_list[i * 2 + 1] = random_symbols_list[i];
    }

    old_pointer = map_symbols_list;

    map_symbols_list = shuffle_array(
        total_map_symbols * 2,
        total_map_symbols * 2,
        old_pointer
    );

    free(old_pointer);
	}
	
	if (index == total_map_symbols * 2 - 1) {
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
  // N TO USANDO
	for (int i = 0; i < list_size; i++) {
		if (number == used_list[i])
			return true;
	}
	return false;
}

static void refresh_map(WINDOW *win, Position selected_card, int map_rows, int map_cols, Card (*card_map)[map_cols])
{
	werase(win);
	wattrset(win, COLOR_PAIR(CARD_COLOR));
	for (int current_row = 0; current_row < map_rows; current_row++) {
		for (int current_col = 0; current_col < map_cols; current_col++) {
			for (int y = 0; y < 3; y++) {
				for (int x = 0; x < 3; x++) {
          if (card_map[current_row][current_col].is_available) {
            if (y == 1 && x == 1 && card_map[current_row][current_col].is_shown)
              mvwaddch(win, card_map[current_row][current_col].position.y + y, card_map[current_row][current_col].position.x + x, card_map[current_row][current_col].symbol);
            else
              mvwaddch(win, card_map[current_row][current_col].position.y + y, card_map[current_row][current_col].position.x + x, ' ');
          }
				}
			}
		}
	}
	wattrset(win, A_NORMAL);
	draw_box(win, card_map[selected_card.y][selected_card.x].position, 5, 5);	
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

    if (new_array_size > array_size || new_array_size < 0)
        return NULL;

    new_array = malloc(sizeof(int) * array_size);

    if (!new_array)
        return NULL;

    memcpy(new_array, array, sizeof(int) * array_size);

    for (int i = array_size - 1; i > 0; i--) {
      unsigned int rand_num; 
      int j;
      getrandom(&rand_num, sizeof(int), 0);
      j = rand_num % (i + 1);

      int tmp = new_array[i];
      new_array[i] = new_array[j];
      new_array[j] = tmp;
    }

    return new_array;
}

static bool cards_remain(int map_rows, int map_cols, Card (*card_map)[map_cols])
{
  for (int i = 0; i < map_rows; i++)
    for (int n = 0; n < map_cols; n++)
      if (card_map[i][n].is_available)
        return false;
  return true;
}

static Position select_neighbor(Position current_card, DIRECTION direction, int map_rows, int map_cols, Card (*card_map)[map_cols])
{
  Position old_position = current_card;

  switch (direction) {
      case UP:
        current_card.y -= 1;
        break;
      case LEFT:
        current_card.x -= 1;
        break;
      case RIGHT:
        current_card.x += 1;
        break;
      case DOWN:
        current_card.y += 1;
        break;
      case NO_DIRECTION: printf("what");
  }
  
  if (current_card.y < 0 || current_card.x < 0 || current_card.y >= map_rows || current_card.x >= map_cols) {
    return old_position;
  }

  return current_card;
}

static void show_card(Card **shown_cards, Position current_card, int map_rows, int map_cols, Card (*card_map)[map_cols])
{
  if (!card_map[current_card.y][current_card.x].is_available || card_map[current_card.y][current_card.x].is_shown)
    return;
  card_map[current_card.y][current_card.x].is_shown = true;
  
  int i = 0;
  while (i < 2 && shown_cards[i] != NULL)
    i++;
  
  if (i == 2)
    return;
  else
    shown_cards[i] = &card_map[current_card.y][current_card.x];
}

static bool check_card(WINDOW *win, Card **shown_cards)
{ // 1, 1 middle 
  bool val;
  if (shown_cards[0]->symbol == shown_cards[1]->symbol) {
    wattrset(win, COLOR_PAIR(SUCCESS_COLOR));
    val = true;
  } else {
    wattrset(win, COLOR_PAIR(FAIL_COLOR));
    val = false;
  }

  for (int i = 0; i < 2; i++)
    mvwaddch(win, shown_cards[i]->position.y + 2, shown_cards[i]->position.x + 2, shown_cards[i]->symbol);
  
  touchwin(win);
  wrefresh(win);
  usleep(600000);
	wattrset(win, A_NORMAL);

  return val;
}
