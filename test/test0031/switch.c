/* Copyright (C) 2023 Jeremiah Orians
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

#include <bootstrappable.h>

int match(char* a, char* b);

enum {
	ELEVEN = 11,
	SIX = 6,
	SEVEN,
};

int test_should_choose_expression_character() {
	int a = 0x4B;

	switch (a) {
		case 'A' + 10: return 0;
		default: return 1;
	}

	return 1;
}

int test_should_choose_character() {
	int a = 0x41;

	switch (a) {
		case 'A': return 0;
		default: return 1;
	}

	return 1;
}


int test_should_choose_expression_ending_with_integer() {
	int a = 10;

	switch (a) {
		case SIX + 4: return 0;
		default: return 1;
	}

	return 1;
}

int test_should_choose_expression() {
	int a = 10;

	switch (a) {
		case 4 + SIX: return 0;
		default: return 1;
	}

	return 1;
}

int test_should_choose_integer() {
	int a = 10;

	switch (a) {
		case 10: return 0;
		default: return 1;
	}

	return 1;
}

int test_should_choose_enum() {
	int a = 6;

	switch (a) {
		case SIX: return 0;
		default: return 1;
	}

	return 1;
}

#define EIGHTEEN 18

int test_should_choose_define() {
	int a = 18;

	switch (a) {
		case EIGHTEEN: return 0;
		default: return 1;
	}

	return 1;
}

#define NINETEEN 19    /* This is my comment */

int test_should_choose_define_with_comment() {
	int a = 19;

	switch (a) {
		case NINETEEN: return 0;
		default: return 1;
	}

	return 1;
}

int test_should_allow_no_default() {
	int a = 3;

	switch (a) {
		case 0: return 1;
		case 1: return 1;
	}

	return 0;
}

int main(int argc, char** argv)
{
	char* a = argv[1];
	int b = 0;

	if(match("test1", a))
	{
		b = 1;
	}
	else if(match("test2", a))
	{
		b = 3;
	}
	else if(match("test3", a))
	{
		b = 4;
	}
	else if(match("test4", a))
	{
		b = 6;
	}
	else if(match("test5", a))
	{
		b = 9;
	}
	else if(match("test6", a))
	{
		b = 8;
	}
	else if(match("test7", a))
	{
		b = 7;
	}

	int i = 31;
	switch(b)
	{
		case 0: return 111;
		case 1:
		case 2: return 122;
		case 3: break;
		case 4: i = 42;
		case 5: i = i + 1;
		        break;
		case 7: i = i + 2;
		case 8: i = i + 1;
		case 9:
			switch(i)
			{
				case 31: return 133;
				case 32: i = 77;
				case 33: break;
				default: i = 144;
			}
			i = i + 7;
			break;
		default: return 155;
	}

	switch(10)
	{
		case 1 + 8: return 100;
		case 'A' /* 0x41 */: return 101;
		case 'A' + 16 /* 0x51 */: return 102;
		case 'a': return 103;
		case SEVEN: return 104;
		case ELEVEN: return 105;
		case SIX: return 106;
		default: break;
	}

	if(test_should_choose_enum() != 0) return 107;
	if(test_should_choose_integer() != 0) return 108;
	if(test_should_choose_expression() != 0) return 109;
	if(test_should_choose_expression_ending_with_integer() != 0) return 110;
	if(test_should_choose_character() != 0) return 111;
	if(test_should_choose_expression_character() != 0) return 112;
	if(test_should_choose_define() != 0) return 113;
	if(test_should_choose_define_with_comment() != 0) return 114;
	if(test_should_allow_no_default() != 0) return 115;

	return i;
}
