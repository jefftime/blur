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

#include "xrand.h"
#include "sized_types.h"

uint64_t XRAND_SEED;

uint64_t xrand(void) {
  XRAND_SEED ^= XRAND_SEED << 1;
  XRAND_SEED ^= XRAND_SEED >> 13;
  XRAND_SEED ^= XRAND_SEED << 45;
  return XRAND_SEED;
}
