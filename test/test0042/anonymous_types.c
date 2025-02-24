/* Copyright (C) 2024 Gtker
 * This file is part of M2-Planet.
 *
 * M2-Planet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * M2-Planet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with M2-Planet.  If not, see <http://www.gnu.org/licenses/>.
 */

struct {
    int a;
    int b;
    int c;
} global_struct;

enum {
    GE_A,
    GE_B = 1,
    GE_C = 2,
} global_enum;

int main(void) {
    global_struct.a = 10;
    global_struct.b = 11;
    global_struct.c = 12;

    if(global_struct.a != 10) return 1;
    if(global_struct.b != 11) return 2;
    if(global_struct.c != 12) return 3;

    global_enum = GE_A;

    if(global_enum != GE_A) return 4;
    if(GE_B != 1) return 5;
    if(GE_C != 2) return 6;

    struct {
        int d;
        int e;
        int f;
    } local_struct;

    local_struct.d = 13;
    local_struct.e = 14;
    local_struct.f = 15;

    if(local_struct.d != 13) return 7;
    if(local_struct.e != 14) return 8;
    if(local_struct.f != 15) return 9;

    enum {
        LE_D,
        LE_E,
    } local_enum = LE_E;

    if(local_enum != 1) return 10;
    if(LE_D != 0) return 11;
    if(LE_E != 1) return 12;
}
