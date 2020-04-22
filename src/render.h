#ifndef RENDER_H
#define RENDER_H

#include "sized_types.h"
#include "window.h"

struct render;

/* **************************************** */
/* render_<backend>.c */
struct render *render_new(struct window *w);
void render_del(struct render *);
int render_configure(struct render *r, uint16_t width, uint16_t height);
void render_update(struct render *r);
void render_line(struct render *, int32_t, int32_t, int32_t, int32_t);
/* **************************************** */

#endif
