#include "vector.h"

void v3addv(struct vec3 *result, struct vec3 *first, struct vec3 *second) {
  result->x = first->x + second->x;
  result->y = first->y + second->y;
  result->z = first->z + second->z;
}

void v3subv(struct vec3 *result, struct vec3 *first, struct vec3 *second) {
  result->x = first->x - second->x;
  result->y = first->y - second->y;
  result->z = first->z - second->z;
}

void v3adds(struct vec3 *result, struct vec3 *v, float n) {
  result->x = v->x + n;
  result->y = v->y + n;
  result->z = v->z + n;
}

void v3subs(struct vec3 *result, struct vec3 *v, float n) {
  result->x = v->x - n;
  result->y = v->y - n;
  result->z = v->z - n;
}

void v3muls(struct vec3 *result, struct vec3 *v, float n) {
  result->x = v->x * n;
  result->y = v->y * n;
  result->z = v->z * n;
}

float v3dot(struct vec3 *first, struct vec3 *second) {
  float result = 0.0f;

  result += first->x * second->x;
  result += first->y * second->y;
  result += first->z * second->z;
  return result;
}

void v3cross(struct vec3 *result, struct vec3 *first, struct vec3 *second) {
  float a, b, c;

  /* cross product = ai - bj + ck */
  a = (first->y * second->z) - (first->z * second->y);
  b = (first->x * second->z) - (first->z * second->x);
  c = (first->x * second->y) - (first->y * second->x);
  result->x = a;
  result->y = -b;
  result->z = c;
}

