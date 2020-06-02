#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>

#include "walls.h"

struct coord {
	int y, x;
};

struct {
	struct coord max;
} config;

struct {
	cchar_t ***glyphs;
	size_t height;
	size_t width;
} map;

void init_map(void)
{
	map.height = (unsigned)config.max.y;
	map.width = (unsigned)config.max.x;
	map.glyphs = malloc(map.height * sizeof(*map.glyphs));
	for (size_t n=0; n < map.height; ++n) {
		map.glyphs[n] = calloc(map.width, sizeof(map.glyphs[n]));
	}
}

void destroy_map(void)
{
	for (size_t n=0; n < map.height; ++n) {
		free(map.glyphs[n]);
	}
	free(map.glyphs);
}

void draw_map(void)
{
	for (size_t y=0; y < map.height; ++y) {
		move((int)y, 0);
		for (size_t x=0; x < map.width; ++x) {
			if (map.glyphs[y][x]) {
				add_wch(map.glyphs[y][x]);
			} else {
				addwstr(L"█");
			}
		}
	}
}

void write_room(int y, int x, int height, int width)
{
	map.glyphs[y][x] = WACS_ULCORNER;
	for (int n=1; n < width-1; ++n) {
		map.glyphs[y][x+n] = WACS_HLINE;
	}
	map.glyphs[y][x+width-1] = WACS_URCORNER;

	for (int n=1; n < height-1; ++n) {
		map.glyphs[y+n][x] = WACS_VLINE;
		map.glyphs[y+n][x+width-1] = WACS_VLINE;
	}

	map.glyphs[y+height-1][x] = WACS_LLCORNER;
	for (int n=1; n < width-1; ++n) {
		map.glyphs[y+height-1][x+n] = WACS_HLINE;
	}
	map.glyphs[y+height-1][x+width-1] = WACS_LRCORNER;
}

void write_map(void)
{

	for (int n=0; n < 3; ++n) {
		// 3, from 2 walls separated by at least one walkable middle
		// 10, max of 2 wall plus 8 middle tiles
		int height = 3 + rand() % 10;
		int width = 3 + rand() % 10;

		int y = rand() % ((int)map.height-height);
		int x = rand() % ((int)map.width-width);
		write_room(y, x, height, width);
	}
}

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
	init_map();
	write_map();
	cchar_t paintbrush = {WA_BOLD, {'@'}, 1};

	clear();
	draw_map();
	while (input != 'q') {
		mvadd_wch(player.y, player.x, &paintbrush);
		move(player.y, player.x);
		refresh();

		input = getch();
		if (map.glyphs[player.y][player.x]) {
			add_wch(map.glyphs[player.y][player.x]);
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
				if (player.y > 0 && !map.glyphs[player.y-1][player.x]) {
					--player.y;
				}
				break;
			case KEY_RIGHT:
				if (player.x < config.max.x-1 && !map.glyphs[player.y][player.x+1]) {
					++player.x;
				}
				break;
			case KEY_DOWN:
				if (player.y < config.max.y-1 && !map.glyphs[player.y+1][player.x]) {
					++player.y;
				}
				break;
			case KEY_LEFT:
				if (player.x > 0 && !map.glyphs[player.y][player.x-1]) {
					--player.x;
				}
				break;

			case 12: // CTRL_L
				clear();
				draw_map();
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
	destroy_map();

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
