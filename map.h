#ifndef MAP_H
 #define MAP_H

#include <ncurses.h>

typedef struct map_ Map;


Map *map_create(size_t height, size_t width);
void map_destroy(Map *m);

void map_dig(Map *m);

cchar_t *map_glyph_at(Map *m, int y, int x);

void map_render(const Map *m);


#endif
