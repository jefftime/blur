/* Copyright 2020, Jeffery Stager
 *
 * This file is part of Tortuga
 *
 * Tortuga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tortuga is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tortuga.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "trig.h"
#include <string.h>

#define m4xy(m, x, y) m->data[(x * 4) + y]

void v3addv(struct vec3 *lhs, struct vec3 *rhs, struct vec3 *out) {
  out->x = lhs->x + rhs->x;
  out->y = lhs->y + rhs->y;
  out->z = lhs->z + rhs->z;
}

void v3subv(struct vec3 *lhs, struct vec3 *rhs, struct vec3 *out) {
  out->x = lhs->x - rhs->x;
  out->y = lhs->y - rhs->y;
  out->z = lhs->z - rhs->z;
}

void v3adds(struct vec3 *v, float n, struct vec3 *out) {
  out->x = v->x + n;
  out->y = v->y + n;
  out->z = v->z + n;
}

void v3subs(struct vec3 *v, float n, struct vec3 *out) {
  out->x = v->x - n;
  out->y = v->y - n;
  out->z = v->z - n;
}

void v3muls(struct vec3 *v, float n, struct vec3 *out) {
  out->x = v->x * n;
  out->y = v->y * n;
  out->z = v->z * n;
}

float v3dot(struct vec3 *lhs, struct vec3 *rhs) {
  float out = 0.0f;

  out += lhs->x * rhs->x;
  out += lhs->y * rhs->y;
  out += lhs->z * rhs->z;
  return out;
}

void v3cross(struct vec3 *lhs, struct vec3 *rhs, struct vec3 *out) {
  float a, b, c;

  /* cross product = ai - bj + ck */
  a = (lhs->y * rhs->z) - (lhs->z * rhs->y);
  b = (lhs->x * rhs->z) - (lhs->z * rhs->x);
  c = (lhs->x * rhs->y) - (lhs->y * rhs->x);
  out->x = a;
  out->y = -b;
  out->z = c;
}

void m4new(float f, struct mat4 *out) {
  size_t i;

  for (i = 0; i < 16; ++i) {
    out->data[i] = f;
  }
}

void m4ident(struct mat4 *out) {
  memset(out->data, 0, sizeof(float) * 16);
  m4xy(out, 0, 0) = 1.0f;
  m4xy(out, 1, 1) = 1.0f;
  m4xy(out, 2, 2) = 1.0f;
  m4xy(out, 3, 3) = 1.0f;
}

void m4mulv(struct mat4 *lhs, struct vec4 *rhs, struct vec4 *out) {
  float *row1, *row2, *row3, *row4;
  float x, y, z, w;

  row1 = lhs->data;
  row2 = lhs->data + 4;
  row3 = lhs->data + 8;
  row4 = lhs->data + 12;
  x =
    (rhs->x * row1[0])
    + (rhs->y * row1[1])
    + (rhs->z * row1[2])
    + (rhs->w * row1[3]);
  y =
    (rhs->x * row2[0])
    + (rhs->y * row2[1])
    + (rhs->z * row2[2])
    + (rhs->w * row2[3]);
  z =
    (rhs->x * row3[0])
    + (rhs->y * row3[1])
    + (rhs->z * row3[2])
    + (rhs->w * row3[3]);
  w =
    (rhs->x * row4[0])
    + (rhs->y * row4[1])
    + (rhs->z * row4[2])
    + (rhs->w * row4[3]);
  out->x = x;
  out->y = y;
  out->z = z;
  out->w = w;
}

