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
void recursive_output(FILE* out, struct token_list* i);

/* Our essential organizer */
int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "We require more arguments\n");
		exit(EXIT_FAILURE);
	}

	initialize_types();
	FILE* input = fopen(argv[1], "r");
	global_token = reverse_list(read_all_tokens(input, NULL));
	struct token_list* output_list = program(NULL);
	FILE* output = fopen(argv[2], "w");
	fprintf(output, "\n# Core program\n\n");
	recursive_output(output, output_list);
	fprintf(output, "\n# Program global variables\n\n");
	recursive_output(output, globals_list);
	fprintf(output, "\n# Program strings\n\n");
	recursive_output(output, strings_list);
	fclose(output);
	return 0;
}
