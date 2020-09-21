#include "map.h"

#include <stdlib.h>

struct map_ {
	cchar_t ***glyphs;
	size_t height;
	size_t width;
};

Map *map_create(size_t height, size_t width)
{
	Map *map = malloc(sizeof(*map));
	if (!map) {
		return NULL;
	}

	map->height = height;
	map->width = width;
	map->glyphs = malloc(map->height * sizeof(*map->glyphs));
	if (!map->glyphs) {
		free(map);
		return NULL;
	}

	for (size_t n=0; n < map->height; ++n) {
		map->glyphs[n] = calloc(map->width, sizeof(map->glyphs[n]));
		if (!map->glyphs[n]) {
			while (n--) {
				free(map->glyphs[n]);
			}
			free(map);
			return NULL;
		}
	}

	return map;
}

void map_destroy(Map *m)
{
	for (size_t n=0; n < m->height; ++n) {
		free(m->glyphs[n]);
	}
	free(m->glyphs);

	free(m);
}

void map_render(const Map *m)
{
	for (size_t y=0; y < m->height; ++y) {
		move((int)y, 0);
		for (size_t x=0; x < m->width; ++x) {
			if (m->glyphs[y][x]) {
				add_wch(m->glyphs[y][x]);
			} else {
				addwstr(L"█");
			}
		}
	}
}

cchar_t *map_glyph_at(Map *m, int y, int x)
{
	if (!m) {
		return NULL;
	}

	if (y < 0 || m->height < (unsigned)y || x < 0 || m->width < (unsigned)x) {
		return NULL;
	}

	return m->glyphs[y][x];
}

// Will always path along x before y
static void write_passage(Map *m, int y1, int x1, int y2, int x2)
{
	while (x1 != x2) {
		m->glyphs[y1][x1] = WACS_BULLET;
		if (x1 < x2) {
			++x1;
		} else {
			--x1;
		}
	}

	while (y1 != y2) {
		m->glyphs[y1][x1] = WACS_BULLET;
		if (y1 < y2) {
			++y1;
		} else {
			--y1;
		}
	}
}

static void write_room(Map *m, int y, int x, int height, int width)
{
	m->glyphs[y][x] = WACS_ULCORNER;
	for (int n=1; n < width-1; ++n) {
		m->glyphs[y][x+n] = WACS_HLINE;
	}
	m->glyphs[y][x+width-1] = WACS_URCORNER;

	for (int n=1; n < height-1; ++n) {
		m->glyphs[y+n][x] = WACS_VLINE;
		m->glyphs[y+n][x+width-1] = WACS_VLINE;
	}

	m->glyphs[y+height-1][x] = WACS_LLCORNER;
	for (int n=1; n < width-1; ++n) {
		m->glyphs[y+height-1][x+n] = WACS_HLINE;
	}
	m->glyphs[y+height-1][x+width-1] = WACS_LRCORNER;
}

void map_dig(Map *m)
{
	int last_y, last_x;

	for (int n=0; n < 3; ++n) {
		// 3, from 2 walls separated by at least one walkable middle
		// 10, max of 2 wall plus 8 middle tiles
		int height = 3 + rand() % 10;
		int width = 3 + rand() % 10;

		int y = rand() % ((int)m->height - height);
		int x = rand() % ((int)m->width - width);
		write_room(m, y, x, height, width);
		if (n != 0) {
			int end_y = y + rand() % height;
			int end_x = x + rand() % width;
			write_passage(m, last_y, last_x, end_y, end_x);
		}

		last_y = y + rand() % height;
		last_x = x + rand() % width;
	}
}

