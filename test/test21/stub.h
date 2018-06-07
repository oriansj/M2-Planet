/* -*- c-file-style: "linux";indent-tabs-mode:t -*- */
/* Copyright (C) 2016 Jeremiah Orians
 * Copyright (C) 2017 Jan Nieuwenhuizen <janneke@gnu.org>
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

CONSTANT NULL 0
CONSTANT stdout 1
CONSTANT stderr 2
CONSTANT EOF 0xFFFFFFFF
CONSTANT EXIT_SUCCESS 0
CONSTANT EXIT_FAILURE 1
void* calloc(int count, int size);
void exit(int value);
int fgetc(FILE* f);
void fputc(char s, FILE* f);
FILE* fopen(char* filename, char* mode);
int fclose(FILE* stream);
