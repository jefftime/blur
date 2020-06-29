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

#ifndef VECTOR_H
#define VECTOR_H

struct vec3 {
  float x, y, z;
};

struct vec4 {
  float x, y, z, w;
};

void v3addv(struct vec3 *result, struct vec3 *first, struct vec3 *second);
void v3subv(struct vec3 *result, struct vec3 *first, struct vec3 *second);
void v3adds(struct vec3 *result, struct vec3 *v, float n);
void v3subs(struct vec3 *result, struct vec3 *v, float n);
void v3muls(struct vec3 *result, struct vec3 *v, float n);
float v3dot(struct vec3 *first, struct vec3 *second);
void v3cross(struct vec3 *result, struct vec3 *first, struct vec3 *second);

#endif
