#ifndef GAME_HEADER
#define GAME_HEADER

#include "main.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/random.h>

typedef enum {UP, DOWN, LEFT, RIGHT, NO_DIRECTION} DIRECTION;

typedef Vector2 MapSize;
typedef Vector2 Position;

typedef struct gameModeSettings {
	MapSize map_size;
	int timer;
} GameModeSettings;

typedef struct card {
	bool is_available;
  bool is_shown;
	int symbol;
	Position position;
} Card;

typedef struct symbolList {
	int available_symbols;
	int *symbols;
} SymbolList;

extern const GameModeSettings game_mode_settings[TOTAL_MODES];
extern const SymbolList symbols_list;

void game(MODE mode);

#endif
