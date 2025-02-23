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

struct T {
	unsigned unsigned_member;
	unsigned char unsigned_char_member;
	unsigned short unsigned_short_member;
	unsigned short int unsigned_short_int_member;
	unsigned int unsigned_int_member;
	unsigned long unsigned_long_member;
	unsigned long int unsigned_long_int_member;
	unsigned long long unsigned_long_long_member;
	unsigned long long int unsigned_long_long_int_member;

	signed signed_member;
	signed char signed_char_member;
	signed short signed_short_member;
	signed short int signed_short_int_member;
	signed int signed_int_member;
	signed long signed_long_member;
	signed long int signed_long_int_member;
	signed long long signed_long_long_member;
	signed long long int signed_long_long_int_member;

	short short_member;
	short int short_int_member;

	long long_member;
	long int long_int_member;

	long long long_long_member;
	long long int long_long_int_member;
};

unsigned unsigned_global;
unsigned char unsigned_char_global;
unsigned short unsigned_short_global;
unsigned short int unsigned_short_int_global;
unsigned int unsigned_int_global;
unsigned long unsigned_long_global;
unsigned long int unsigned_long_int_global;
unsigned long long unsigned_long_long_global;
unsigned long long int unsigned_long_long_int_global;

signed signed_global;
signed char signed_char_global;
signed short signed_short_global;
signed short int signed_short_int_global;
signed int signed_int_global;
signed long signed_long_global;
signed long int signed_long_int_global;
signed long long signed_long_long_global;
signed long long int signed_long_long_int_global;

short short_global;
short int short_int_global;

long long_global;
long int long_int_global;

long long long_long_global;
long long int long_long_int_global;

int main(void) {
	unsigned unsigned_local;
	unsigned char unsigned_char_local;
	unsigned short unsigned_short_local;
	unsigned short int unsigned_short_int_local;
	unsigned int unsigned_int_local;
	unsigned long unsigned_long_local;
	unsigned long int unsigned_long_int_local;
	unsigned long long unsigned_long_long_local;
	unsigned long long int unsigned_long_long_int_local;

	signed signed_local;
	signed char signed_char_local;
	signed short signed_short_local;
	signed short int signed_short_int_local;
	signed int signed_int_local;
	signed long signed_long_local;
	signed long int signed_long_int_local;
	signed long long signed_long_long_local;
	signed long long int signed_long_long_int_local;

	short short_local;
	short int short_int_local;

	long long_local;
	long int long_int_local;

	long long long_long_local;
	long long int long_long_int_local;
}
