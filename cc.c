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
#include <stdint.h>

/* Globals */
struct token_list* output_list;
struct token_list* global_symbol_list;
struct token_list* global_stack;

/* Imported functions */
int asprintf(char **strp, const char *fmt, ...);
void read_all_tokens(char* source_file);

void emit(char *s, bool hands_off)
{
	struct token_list* t = calloc(1, sizeof(struct token_list));
	t->next = output_list;
	t->hands_off = hands_off;
	output_list = t;
	t->s = s;
}

void pull_value_off_stack(int register_number)
{
	global_stack = global_stack->next;

	if(0 == register_number) emit("POP_eax\n", true);
	else if(1 == register_number) emit("POP_ebx\n", true);
	else exit(EXIT_FAILURE);
}

void add_to_frame(struct token_list* a)
{
	struct token_list* new = calloc(1, sizeof(struct token_list));
	new->entry = a;
	new->next = global_stack;
	global_stack = new;
}

void put_value_on_stack(int register_number, struct token_list* a)
{
	add_to_frame(a);

	if(0 == register_number) emit("PUSH_eax\n", true);
	else if(1 == register_number) emit("PUSH_ebx\n", true);
	else exit(EXIT_FAILURE);
}

void be_pop_type(struct token_list* floor, int type)
{
	bool flag = false;
	for( struct token_list* i = global_stack; ((!flag) && (floor != i) && ((NULL == i->entry) || (i->entry->type == type))); i = global_stack)
	{
		if((NULL != i->entry) && (i->entry->type == FUNCTION)) flag = true;
		pull_value_off_stack(1);
	}
}

struct token_list* sym_declare(char *s, int type)
{
	char* label;
	asprintf(&label, "# Defining_%s\n", s);
	emit(label, true);

	struct token_list* a = calloc(1, sizeof(struct token_list));
	a->next = global_symbol_list;
	global_symbol_list = a;

	a->type = type;
	a->s = s;
	return a;
}

struct token_list* sym_lookup(char *s, struct token_list* symbol_list)
{
	for(struct token_list* i = symbol_list; NULL != i; i = i->next)
	{
		if(0 == strcmp(s,i->s)) return i;
	}
	return NULL;
}

int stack_index(struct token_list* a)
{
	int index = 0;
	for(struct token_list* b = global_stack; NULL != b; (b = b->next) && (index = index + 1))
	{
		if(b->entry == a) return index;
	}
	return index;
}

void sym_get_value(char *s)
{
	char* label;
	struct token_list* a = sym_lookup(s, global_symbol_list);
	if(a == NULL) exit(EXIT_FAILURE);

	switch(a->type)
	{
		case GLOBAL: asprintf(&label, "LOAD_IMMEDIATE_eax &GLOBAL_%s\n", s); break;
		case LOCAL_VARIABLE: asprintf(&label, "LOAD_EFFECTIVE_ADDRESS %c%d\n", 37, 4 * stack_index(a)); break;
		case ARGUEMENT: asprintf(&label, "LOAD_EFFECTIVE_ADDRESS %c%d\n", 37, 4 * (stack_index(a) + 1)); break;
		case FUNCTION:
		{
			asprintf(&label, "LOAD_IMMEDIATE_eax &FUNCTION_%s\n", s);
			emit(label, true);
			put_value_on_stack(0, a);
			return;
		}
		default: exit(EXIT_FAILURE);
	}
	emit(label, true);
	if(strcmp(global_token->next->s, "=")) emit("LOAD_INTEGER\n", true);
}

void require_char(char* message, char required)
{
	if(global_token->s[0] != required)
	{
		fprintf(stderr, "%s", message);
		exit(EXIT_FAILURE);
	}
	global_token = global_token->next;
}

void expression();
void parse_string();

/*
 * primary-expr:
 *     identifier
 *     constant
 *     ( expression )
 */
void primary_expr()
{
	char* label;

	if(('0' <= global_token->s[0]) & (global_token->s[0] <= '9'))
	{
		asprintf(&label, "LOAD_IMMEDIATE_eax %c%s\n", 37, global_token->s);
		emit(label, true);
		global_token = global_token->next;
	}
	else if(('a' <= global_token->s[0]) & (global_token->s[0] <= 'z'))
	{
		sym_get_value(global_token->s);
		global_token = global_token->next;
	}
	else if(global_token->s[0] == '(')
	{
		global_token = global_token->next;
		expression();
		require_char("Error in Primary expression\nDidn't get )\n", ')');
	}
	else if((global_token->s[0] == 39) & (global_token->s[1] != 0) & (global_token->s[2] == 39) & (global_token->s[3] == 0))
	{
		asprintf(&label, "LOAD_IMMEDIATE_eax %c%d\n", 37, global_token->s[1]);
		emit(label, true);
		global_token = global_token->next;
	}
	else if(global_token->s[0] == '"')
	{
		parse_string();
		global_token = global_token->next;
	}
	else exit(EXIT_FAILURE);
}

/* Deal with Expression lists */
void process_expression_list()
{
	global_token = global_token->next;
	struct token_list* function = global_stack->entry;

	if(global_token->s[0] != ')')
	{
		expression();
		put_value_on_stack(0, NULL);

		while(global_token->s[0] == ',')
		{
			global_token = global_token->next;
			expression();
			put_value_on_stack(0, NULL);
		}
		require_char("ERROR in process_expression_list\nNo ) was found\n", ')');
	}
	else global_token = global_token->next;

	char* label;
	asprintf(&label, "LOAD_ESP_IMMEDIATE_into_eax %c%d\nCALL_eax\n", 37, 4 * stack_index(function));
	emit(label, true);
	be_pop_type(NULL, FUNCTION);
}

void common_recursion(void (*function) (void))
{
	global_token = global_token->next;
	put_value_on_stack(0, NULL);
	function();
	pull_value_off_stack(1);
}

/*
 * postfix-expr:
 *         primary-expr
 *         postfix-expr [ expression ]
 *         postfix-expr ( expression-list-opt )
 */
void postfix_expr()
{
	primary_expr();

	if(global_token->s[0] == '[')
	{
		common_recursion(expression);
		emit("ADD_ebx_to_eax\n", true);

		if(strcmp(global_token->next->s, "=")) emit("LOAD_BYTE\n", true);
		require_char("ERROR in postfix_expr\nMissing ]\n", ']');
	}
	else if(global_token->s[0] == '(') process_expression_list();
}

/*
 * additive-expr:
 *         postfix-expr
 *         additive-expr + postfix-expr
 *         additive-expr - postfix-expr
 */
void additive_expr()
{
	postfix_expr();

	while(1)
	{
		if(global_token->s[0] == '+')
		{
			common_recursion(postfix_expr);
			emit("ADD_ebx_to_eax\n", true);
		}
		else if(global_token->s[0] == '-')
		{
			common_recursion(postfix_expr);
			emit("SUBTRACT_eax_from_ebx_into_ebx\nMOVE_ebx_to_eax\n", true);
		}
		else return;
	}
}

/*
 * shift-expr:
 *         additive-expr
 *         shift-expr << additive-expr
 *         shift-expr >> additive-expr
 */
void shift_expr()
{
	additive_expr();

	while(1)
	{
		if(!strcmp(global_token->s, "<<"))
		{
			global_token = global_token->next;
			put_value_on_stack(0, NULL);
			additive_expr();
			emit("COPY_eax_to_ecx\n", true);
			pull_value_off_stack(0);
			emit("SAL_eax_cl\n", true);
		}
		else if(!strcmp(global_token->s, ">>"))
		{
			global_token = global_token->next;
			put_value_on_stack(0, NULL);
			additive_expr();
			emit("COPY_eax_to_ecx\n", true);
			pull_value_off_stack(0);
			emit("SAR_eax_cl\n", true);
		}
		else
		{
			return;
		}
	}
}

/*
 * relational-expr:
 *         shift-expr
 *         relational-expr <= shift-expr
 */
void relational_expr()
{
	shift_expr();

	while(!strcmp(global_token->s, "<="))
	{
		common_recursion(shift_expr);
		emit("CMP\nSETLE\nMOVEZBL\n", true);
	}
}

/*
 * equality-expr:
 *         relational-expr
 *         equality-expr == relational-expr
 *         equality-expr != relational-expr
 */
void equality_expr()
{
	relational_expr();

	while(1)
	{
		if(!strcmp(global_token->s, "=="))
		{
			common_recursion(relational_expr);
			emit("CMP\nSETE\nMOVEZBL\n", true);
		}
		else if(!strcmp(global_token->s, "!="))
		{
			common_recursion(relational_expr);
			emit("CMP\nSETNE\nMOVEZBL\n", true);
		}
		else return;
	}
}

/*
 * bitwise-and-expr:
 *         equality-expr
 *         bitwise-and-expr & equality-expr
 */
void bitwise_and_expr()
{
	equality_expr();

	while(global_token->s[0] == '&')
	{
		common_recursion(equality_expr);
		emit("AND_eax_ebx\n", true);
	}
}

/*
 * bitwise-or-expr:
 *         bitwise-and-expr
 *         bitwise-and-expr | bitwise-or-expr
 */
void bitwise_or_expr()
{
	bitwise_and_expr();

	while(global_token->s[0] == '|')
	{
		common_recursion(bitwise_and_expr);
		emit("OR_eax_ebx\n", true);
	}
}

/*
 * expression:
 *         bitwise-or-expr
 *         bitwise-or-expr = expression
 */
void expression()
{
	bitwise_or_expr();

	if(global_token->s[0] == '=')
	{
		bool byte = !strcmp(global_token->prev->s, "]");
		common_recursion(expression);
		if(!byte) emit("STORE_INTEGER\n", true);
		else emit("STORE_CHAR\n", true);
	}
}

/*
 * type-name:
 *     char *
 *     int
 */
void type_name()
{
	global_token = global_token->next;

	while(global_token->s[0] == '*')
	{
		global_token = global_token->next;
	}
}

void statement();

/* Process local variable */
void collect_local()
{
	type_name();
	struct token_list* a = sym_declare(global_token->s, LOCAL_VARIABLE);
	global_token = global_token->next;

	if(global_token->s[0] == '=')
	{
		global_token = global_token->next;
		expression();
	}

	require_char("ERROR in collect_local\nMissing ;\n", ';');
	put_value_on_stack(0, a);
}

/* Evaluate if statements */
void process_if()
{
	char* label;
	static int if_count;
	int number = if_count;
	if_count = if_count + 1;

	asprintf(&label, "# IF_%d\n", number);
	emit(label, true);

	global_token = global_token->next;
	require_char("ERROR in process_if\nMISSING (\n", '(');
	expression();

	asprintf(&label, "TEST\nJUMP_EQ %c%s_%d\n", 37, "ELSE",  number);
	emit(label, true);

	require_char("ERROR in process_if\nMISSING )\n", ')');
	statement();

	asprintf(&label, "JUMP %c_END_IF_%d\n:ELSE_%d\n", 37, number, number);
	emit(label, true);

	if(!strcmp(global_token->s, "else"))
	{
		global_token = global_token->next;
		statement();
	}
	asprintf(&label, ":_END_IF_%d\n", number);
	emit(label, true);
}

/* Process while loops */
void process_while()
{
	char* label;
	static int while_count;
	int number = while_count;
	while_count = while_count + 1;

	asprintf(&label, ":WHILE_%d\n", number);
	emit(label, true);

	global_token = global_token->next;

	require_char("ERROR in process_while\nMISSING (\n", '(');
	expression();

	asprintf(&label, "TEST\nJUMP_EQ %c%s_WHILE_%d\n# THEN_while_%d\n", 37, "END", number, number);
	emit(label, true);

	require_char("ERROR in process_while\nMISSING )\n", ')');
	statement();

	asprintf(&label, "JUMP %c%s_%d\n:END_WHILE_%d\n", 37, "WHILE", number, number);
	emit(label, true);
}

/* Ensure that functions return */
void return_result()
{
	global_token = global_token->next;
	if(global_token->s[0] != ';') expression();

	require_char("ERROR in return_result\nMISSING ;\n", ';');
	be_pop_type(NULL, LOCAL_VARIABLE);
	emit("RETURN\n", true);
}

void recursive_statement()
{
	global_token = global_token->next;
	struct token_list* frame = global_stack;

	while(strcmp(global_token->s, "}"))
	{
		statement();
	}
	global_token = global_token->next;

	be_pop_type(frame, LOCAL_VARIABLE);
}

/*
 * statement:
 *     { statement-list-opt }
 *     type-name identifier ;
 *     type-name identifier = expression;
 *     if ( expression ) statement
 *     if ( expression ) statement else statement
 *     while ( expression ) statement
 *     return ;
 *     expr ;
 */
void statement()
{
	if(global_token->s[0] == '{') recursive_statement();
	else if((!strcmp(global_token->s, "char")) | (!strcmp(global_token->s, "int"))) collect_local();
	else if(!strcmp(global_token->s, "if")) process_if();
	else if(!strcmp(global_token->s, "while")) process_while();
	else if(!strcmp(global_token->s, "return")) return_result();
	else
	{
		expression();
		require_char("ERROR in statement\nMISSING ;\n", ';');
	}
}

/* Collect function arguments */
void collect_arguments()
{
	global_token = global_token->next;

	while(strcmp(global_token->s, ")"))
	{
		type_name();

		if(global_token->s[0] != ')')
		{
			struct token_list* a = sym_declare(global_token->s, ARGUEMENT);
			add_to_frame(a);
			global_token = global_token->next;
		}

		/* ignore trailing comma (needed for foo(bar(), 1); expressions*/
		if(global_token->s[0] == ',') global_token = global_token->next;
	}
	global_token = global_token->next;
}

void declare_global()
{
	char* label;
	struct token_list* symbol = sym_declare(global_token->prev->s, GLOBAL);
	asprintf(&label, "\n:GLOBAL_%s\n", symbol->s);
	emit(label, true);

	global_token = global_token->next;
	emit("NOP\n", true);
}

void declare_function()
{
	char* label;
	asprintf(&label, "\n:FUNCTION_%s\n", global_token->prev->s);
	sym_declare(global_token->prev->s, FUNCTION);

	struct token_list* current = global_symbol_list;
	collect_arguments();

	if(global_token->s[0] != ';')
	{
		emit(label, true);
		statement();
		emit("RETURN\n", true);
	}
	else
	{
		global_token = global_token->next;
	}

	for( struct token_list* i = global_symbol_list; current != i; i = i->next)
	{
		global_symbol_list = i;
	}
}

/*
 * program:
 *     declaration
 *     declaration program
 *
 * declaration:
 *     type-name identifier ;
 *     type-name identifier ( parameter-list ) ;
 *     type-name identifier ( parameter-list ) statement
 *
 * parameter-list:
 *     parameter-declaration
 *     parameter-list, parameter-declaration
 *
 * parameter-declaration:
 *     type-name identifier-opt
 */
void program()
{
	while(NULL != global_token->next)
	{
		type_name();
		global_token = global_token->next;
		if(global_token->s[0] == ';') declare_global();
		else if(global_token->s[0] == '(') declare_function();
		else exit(EXIT_FAILURE);
	}
}

void recursive_output(FILE* out, struct token_list* i)
{
	if(NULL == i) return;

	recursive_output(out, i->next);

	if(i->hands_off) fprintf(out, "%s", i->s);
	else
	{
		fprintf(out, "'");
		for(int j = 0; 0 != i->s[j]; j = j + 1)
		{
			fputc(i->s[j], out);
		}
		fprintf(out, "'\t# NONE\n");
	}
}

/* Our essential organizer */
int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "We require more arguments\n");
		exit(EXIT_FAILURE);
	}

	read_all_tokens(argv[1]);
	program();
	FILE* output = fopen(argv[2], "w");
	recursive_output(output, output_list);
	fclose(output);
	return 0;
}
