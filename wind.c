#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>

#include "map.h"
#include "walls.h"

struct coord {
	int y, x;
};

struct {
	struct coord max;
} config;

int main(int argc, char *argv[])
{
	time_t seed;
	if (argc == 1) {
		seed = time(NULL);
	} else if (argc == 2) {
		seed = strtol(argv[1], NULL, 10);
	} else {
		fprintf(stderr, "Usage: %s [seed]\n", argv[0]);
		return 1;
	}

	printf("%ld\n", seed);
	srand((unsigned)seed);
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();

	nonl();
	intrflush(stdscr, false);
	keypad(stdscr, true);
	start_color();

	for (short n=1; n < 25; ++n) {
		init_pair(n, n, 0);
	}

	struct coord player = { 15, 15 };
	int input = '\0';
	getmaxyx(stdscr, config.max.y, config.max.x);
	Map *map = map_create((unsigned)config.max.y, (unsigned)config.max.x);
	map_dig(map);
	cchar_t paintbrush = {WA_BOLD, {'@'}, 1};

	clear();
	map_render(map);
	while (input != 'q') {
		mvadd_wch(player.y, player.x, &paintbrush);
		move(player.y, player.x);
		refresh();

		input = getch();
		cchar_t *pos = map_glyph_at(map, player.y, player.x);
		if (pos) {
			add_wch(pos);
		} else {
			add_wch(&paintbrush);
		}

		switch (input) {
			case 'b':
				paintbrush.ext_color++;

				break;

			case 337: // KEY_SUP
				paintbrush.chars[0] = next_up(paintbrush.chars[0]);
				break;
			case KEY_SRIGHT:
				paintbrush.chars[0] = next_right(paintbrush.chars[0]);
				break;
			case 336: // KEY_SDOWN
				paintbrush.chars[0] = next_down(paintbrush.chars[0]);
				break;
			case KEY_SLEFT:
				paintbrush.chars[0] = next_left(paintbrush.chars[0]);
				break;

			case KEY_UP:
				if (player.y > 0 && !map_glyph_at(map, player.y-1, player.x)) {
					--player.y;
				}
				break;
			case KEY_RIGHT:
				if (player.x < config.max.x-1 && !map_glyph_at(map, player.y, player.x+1)) {
					++player.x;
				}
				break;
			case KEY_DOWN:
				if (player.y < config.max.y-1 && !map_glyph_at(map, player.y+1, player.x)) {
					++player.y;
				}
				break;
			case KEY_LEFT:
				if (player.x > 0 && !map_glyph_at(map, player.y, player.x-1)) {
					--player.x;
				}
				break;

			case 12: // CTRL_L
				clear();
				map_render(map);
				break;

				// Sufficient buffer for formatting the int 'input'
				// sizeof(n) = log_256(n) = K
				// sign + log_10(256) * K + nul
				char buf[1 + 3*sizeof(input) + 1];
			default:
				snprintf(buf, sizeof(buf), "%d ", input);
				mvaddstr(config.max.y-1, 0, buf);
		}
	}
	map_destroy(map);

	endwin();

	for (short n=0; n < 25; ++n) {
		short fg, bg;
		short r, g, b;
		pair_content(n, &fg, &bg);
		color_content(fg, &r, &g, &b);
		printf("%hd: (%hd, %hd, %hd)\n", fg, r, g, b);
	}

	printf("%d/%d\n", COLORS, COLOR_PAIRS);
	//XXX Uncomment for valgrind tests (will still leak a little from curses)
	// _nc_free_and_exit(0);
}
