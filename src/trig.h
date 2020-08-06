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

#ifndef TRIG_H
#define TRIG_H

struct vec3 {
  float x, y, z;
};

struct vec4 {
  float x;
  float y;
  float z;
  float w;
};

/* Column major */
struct mat4 {
  float data[16];
};

void v3addv(struct vec3 *lhs, struct vec3 *rhs, struct vec3 *out);
void v3subv(struct vec3 *lhs, struct vec3 *rhs, struct vec3 *out);
void v3adds(struct vec3 *v, float n, struct vec3 *out);
void v3subs(struct vec3 *v, float n, struct vec3 *out);
void v3muls(struct vec3 *v, float n, struct vec3 *out);
float v3dot(struct vec3 *rhs, struct vec3 *out);
void v3cross(struct vec3 *lhs, struct vec3 *rhs, struct vec3 *out);

void v4addv(struct vec4 *lhs, struct vec4 *rhs, struct vec4 *out);
void v4subv(struct vec4 *lhs, struct vec4 *rhs, struct vec4 *out);
void v4adds(struct vec4 *v, float n, struct vec4 *out);
void v4subs(struct vec4 *v, float n, struct vec4 *out);
void v4muls(struct vec4 *v, float n, struct vec4 *out);
float v4dot(struct vec4 *rhs, struct vec4 *out);
void v4cross(struct vec4 *lhs, struct vec4 *rhs, struct vec4 *out);

void m4new(float f, struct mat4 *out);
void m4ident(struct mat4 *out);
void m4add(struct mat4 *lhs, struct mat4 *rhs, struct mat4 *out);
void m4sub(struct mat4 *lhs, struct mat4 *rhs, struct mat4 *out);
void m4mulm(struct mat4 *lhs, struct mat4 *rhs, struct mat4 *out);
void m4mulv(struct mat4 *lhs, struct vec4 *rhs, struct vec4 *out);

#endif
