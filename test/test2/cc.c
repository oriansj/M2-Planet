/*
 * Copyright (C) 2006 Edmund GRIMLEY EVANS <edmundo@rano.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "cc.h"

/* Imported functions */
void emit(int n, char *s);
void sym_define_global(int current_symbol);
int sym_declare_global(char *s);
void save_int(char *p, int n);
void get_token();
void program();

void be_start()
{
  emit(16, "\x7f\x45\x4c\x46\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00");
  emit(16, "\x02\x00\x03\x00\x01\x00\x00\x00\x54\x80\x04\x08\x34\x00\x00\x00");
  emit(16, "\x00\x00\x00\x00\x00\x00\x00\x00\x34\x00\x20\x00\x01\x00\x00\x00");
  emit(16, "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x80\x04\x08");
  emit(16, "\x00\x80\x04\x08\x10\x4b\x00\x00\x10\x4b\x00\x00\x07\x00\x00\x00");
  emit(16, "\x00\x10\x00\x00\xe8\x00\x00\x00\x00\x89\xc3\x31\xc0\x40\xcd\x80");

  sym_define_global(sym_declare_global("exit"));
  /* pop %ebx ; pop %ebx ; xor %eax,%eax ; inc %eax ; int $0x80 */
  emit(7, "\x5b\x5b\x31\xc0\x40\xcd\x80");

  sym_define_global(sym_declare_global("getchar"));
  /* mov $3,%eax ; xor %ebx,%ebx ; push %ebx ; mov %esp,%ecx */
  emit(10, "\xb8\x03\x00\x00\x00\x31\xdb\x53\x89\xe1");
  /* xor %edx,%edx ; inc %edx ; int $0x80 */
  /* test %eax,%eax ; pop %eax ; jne . + 7 */
  emit(10, "\x31\xd2\x42\xcd\x80\x85\xc0\x58\x75\x05");
  /* mov $-1,%eax ; ret */
  emit(6, "\xb8\xff\xff\xff\xff\xc3");

  sym_define_global(sym_declare_global("malloc"));
  /* mov 4(%esp),%eax */
  emit(4, "\x8b\x44\x24\x04");
  /* push %eax ; xor %ebx,%ebx ; mov $45,%eax ; int $0x80 */
  emit(10, "\x50\x31\xdb\xb8\x2d\x00\x00\x00\xcd\x80");
  /* pop %ebx ; add %eax,%ebx ; push %eax ; push %ebx ; mov $45,%eax */
  emit(10, "\x5b\x01\xc3\x50\x53\xb8\x2d\x00\x00\x00");
  /* int $0x80 ; pop %ebx ; cmp %eax,%ebx ; pop %eax ; je . + 7 */
  emit(8, "\xcd\x80\x5b\x39\xc3\x58\x74\x05");
  /* mov $-1,%eax ; ret */
  emit(6, "\xb8\xff\xff\xff\xff\xc3");

  sym_define_global(sym_declare_global("putchar"));
  /* mov $4,%eax ; xor %ebx,%ebx ; inc %ebx */
  emit(8, "\xb8\x04\x00\x00\x00\x31\xdb\x43");
  /*  lea 4(%esp),%ecx ; mov %ebx,%edx ; int $0x80 ; ret */
  emit(9, "\x8d\x4c\x24\x04\x89\xda\xcd\x80\xc3");

  save_int(code + 85, codepos - 89); /* entry set to first thing in file */
}

void be_finish()
{
  save_int(code + 68, codepos);
  save_int(code + 72, codepos);
  i = 0;
  while (i <= codepos - 1) {
    putchar(code[i]);
    i = i + 1;
  }
}


int main()
{
	code_offset = 134512640; /* 0x08048000 */
	be_start();
	nextc = getchar();
	get_token();
	program();
	be_finish();
	return 0;
}
