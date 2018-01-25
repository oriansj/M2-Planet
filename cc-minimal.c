/* Copyright (C) 2016 Jeremiah Orians
 * This file is part of stage0.
 *
 * stage0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * stage0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with stage0.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cc.h"
/* The core functions */
void initialize_types();
struct token_list* read_all_tokens(FILE* a, struct token_list* current);
struct token_list* reverse_list(struct token_list* head);
struct token_list* program(struct token_list* out);
void recursive_output(struct token_list* i, FILE* out);

/* Our essential organizer */
int main()
{
	initialize_types();
	FILE* input = fopen("input.c", "r");
	global_token = reverse_list(read_all_tokens(input, NULL));
	struct token_list* output_list = program(NULL);
	FILE* output = fopen("input.M1", "w");
	file_print("\n# Core program\n\n", output);
	recursive_output(output_list, output);
	file_print("\n# Program global variables\n\n", output);
	recursive_output(globals_list, output);
	file_print("\n# Program strings\n\n", output);
	recursive_output(strings_list, output);
	fclose(output);
	return 0;
}
