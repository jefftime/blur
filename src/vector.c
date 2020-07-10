#include "vector.h"

void v3addv(struct vec3 *first, struct vec3 *second, struct vec3 *out) {
  out->x = first->x + second->x;
  out->y = first->y + second->y;
  out->z = first->z + second->z;
}

void v3subv(struct vec3 *first, struct vec3 *second, struct vec3 *out) {
  out->x = first->x - second->x;
  out->y = first->y - second->y;
  out->z = first->z - second->z;
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

float v3dot(struct vec3 *first, struct vec3 *second) {
  float out = 0.0f;

  out += first->x * second->x;
  out += first->y * second->y;
  out += first->z * second->z;
  return out;
}

void v3cross(struct vec3 *first, struct vec3 *second, struct vec3 *out) {
  float a, b, c;

  /* cross product = ai - bj + ck */
  a = (first->y * second->z) - (first->z * second->y);
  b = (first->x * second->z) - (first->z * second->x);
  c = (first->x * second->y) - (first->y * second->x);
  out->x = a;
  out->y = -b;
  out->z = c;
}

