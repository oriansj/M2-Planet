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
#include <getopt.h>

/* The core functions */
void initialize_types();
struct token_list* read_all_tokens(FILE* a, struct token_list* current);
struct token_list* reverse_list(struct token_list* head);
struct token_list* program(struct token_list* out);
void recursive_output(FILE* out, struct token_list* i);

#if !__MESC__
static
#endif
struct option long_options[] = {
	{"file", required_argument, 0, 'f'},
	{"output", required_argument, 0, 'o'},
	{"help", no_argument, 0, 'h'},
	{"version", no_argument, 0, 'V'},
	{0, 0, 0, 0}
};


/* Our essential organizer */
int main(int argc, char **argv)
{
	global_token = NULL;

	int c;
	FILE* source_file;
	FILE* destination_file;
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "f:h:o:V", long_options, &option_index)) != -1)
	{
		switch(c)
		{
			case 0: break;
			case 'h':
			{
				fprintf(stderr, "Usage: %s -f FILENAME1 {-f FILENAME2} -o OUTPUT\n", argv[0]);
				exit(EXIT_SUCCESS);
			}
			case 'f':
			{
				#if __MESC__
					source_file = open(optarg, O_RDONLY);
				#else
					source_file = fopen(optarg, "r");
				#endif

				if(NULL == source_file)
				{
					fprintf(stderr, "The file: %s can not be opened!\n", optarg);
					exit(EXIT_FAILURE);
				}

				global_token = read_all_tokens(source_file, global_token);
				break;
			}
			case 'o':
			{
				#if __MESC__
					destination_file = open(optarg, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
				#else
					destination_file = fopen(optarg, "w");
				#endif

				if(NULL == destination_file)
				{
					fprintf(stderr, "The file: %s can not be opened!\n", optarg);
					exit(EXIT_FAILURE);
				}
				break;
			}
			case 'V':
			{
				fprintf(stdout, "M2-Planet 0.1\n");
				exit(EXIT_SUCCESS);
			}
			default:
			{
				fprintf(stderr, "Unknown option\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	if(NULL == global_token)
	{
		fprintf(stderr, "Either no input files were given or they were empty\n");
		exit(EXIT_FAILURE);
	}
	global_token = reverse_list(global_token);

	initialize_types();
	struct token_list* output_list = program(NULL);

	/* Output the program we have compiled */
	fprintf(destination_file, "\n# Core program\n\n");
	recursive_output(destination_file, output_list);
	fprintf(destination_file, "\n# Program global variables\n\n");
	recursive_output(destination_file, globals_list);
	fprintf(destination_file, "\n# Program strings\n\n");
	recursive_output(destination_file, strings_list);
	return EXIT_SUCCESS;
}
