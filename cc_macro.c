/* Copyright (C) 2021 Sanne Wouda
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
#include "cc.h"
#include "gcc_req.h"

void require(int bool, char* error);
int numerate_string(char* a);
void line_error_token(struct token_list* list);
struct token_list* eat_token(struct token_list* head);

struct conditional_inclusion
{
	struct conditional_inclusion* prev;
	int include; /* 1 == include, 0 == skip */
	int previous_condition_matched; /* 1 == all subsequent conditions treated as FALSE */
};

struct macro_list
{
	struct macro_list* next;
	char* symbol;
	struct token_list* expansion;
};

struct macro_list* macro_env;
struct conditional_inclusion* conditional_inclusion_top;

/* point where we are currently modifying the global_token list */
struct token_list* macro_token;

void eat_current_token()
{
	int update_global_token = FALSE;
	if (macro_token == global_token)
		update_global_token = TRUE;

	macro_token = eat_token(macro_token);

	if(update_global_token)
		global_token = macro_token;
}

void eat_newline_tokens()
{
	macro_token = global_token;

	while(TRUE)
	{
		if(NULL == macro_token) return;

		if(match("\n", macro_token->s))
		{
			eat_current_token();
		}
		else
		{
			macro_token = macro_token->next;
		}
	}
}

/* returns the first token inserted; inserts *before* point */
struct token_list* insert_tokens(struct token_list* point, struct token_list* token)
{
	struct token_list* copy;
	struct token_list* first = NULL;

	while (NULL != token)
	{
		copy = calloc(1, sizeof(struct token_list));
		copy->s = token->s;
		copy->filename = token->filename;
		copy->linenumber = token->linenumber;

		if(NULL == first)
		{
			first = copy;
		}

		copy->next = point;

		if (NULL != point)
		{
			copy->prev = point->prev;

			if(NULL != point->prev)
			{
				point->prev->next = copy;
			}

			point->prev = copy;
		}

		token = token->next;
	}

	return first;
}

struct macro_list* lookup_macro(struct token_list* token)
{
	struct macro_list* hold = macro_env;

	while (NULL != hold)
	{
		if (match(token->s, hold->symbol))
		{
			/* found! */
			return hold;
		}

		hold = hold->next;
	}

	/* not found! */
	return NULL;
}

int macro_expression();
int macro_variable()
{
	eat_current_token();
	return 0;
}
int macro_number()
{
	int result = numerate_string(macro_token->s);
	eat_current_token();
	return result;
}

int macro_primary_expr()
{
	int defined_has_paren = FALSE;
	int hold;
	require(NULL != macro_token, "got an EOF terminated macro primary expression\n");

	if('-' == macro_token->s[0])
	{
		eat_current_token();
		return -macro_primary_expr();
	}
	else if('!' == macro_token->s[0])
	{
		eat_current_token();
		return !macro_primary_expr();
	}
	else if('(' == macro_token->s[0])
	{
		eat_current_token();
		return macro_expression();
	}
	else if(match("defined", macro_token->s))
	{
		eat_current_token();

		require(NULL != macro_token, "got an EOF terminated macro defined expression\n");

		if('(' == macro_token->s[0])
		{
			defined_has_paren = TRUE;
			eat_current_token();
		}

		if (NULL != lookup_macro(macro_token))
		{
			hold = TRUE;
		}
		else
		{
			hold = FALSE;
		}
		eat_current_token();

		if(TRUE == defined_has_paren)
		{
			require(')' == macro_token->s[0], "missing close parenthesis for defined()\n");
			eat_current_token();
		}

		return hold;
	}
	else if(in_set(macro_token->s[0], "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"))
	{
		return macro_variable();
	}
	else if(in_set(macro_token->s[0], "0123456789"))
	{
		return macro_number();
	}
	else
	{
		return 0;    /* FIXME: error handling */
	}
}

int macro_additive_expr()
{
	int lhs = macro_primary_expr();
	int hold;

	require(NULL != macro_token, "got an EOF terminated macro additive expression\n");
	if(match("+", macro_token->s))
	{
		eat_current_token();
		return lhs + macro_additive_expr();
	}
	else if(match("-", macro_token->s))
	{
		eat_current_token();
		return lhs - macro_additive_expr();
	}
	else if(match("*", macro_token->s))
	{
		eat_current_token();
		return lhs * macro_additive_expr();
	}
	else if(match("/", macro_token->s))
	{
		eat_current_token();
		hold = macro_additive_expr();
		require(0 != hold, "divide by zero not valid even in C macros\n");
		return lhs / hold;
	}
	else if(match("%", macro_token->s))
	{
		eat_current_token();
		hold = macro_additive_expr();
		require(0 != hold, "modulus by zero not valid even in C macros\n");
		return lhs % hold;
	}
	else if(match(">>", macro_token->s))
	{
		eat_current_token();
		return lhs >> macro_additive_expr();
	}
	else if(match("<<", macro_token->s))
	{
		eat_current_token();
		return lhs << macro_additive_expr();
	}
	else
	{
		return lhs;
	}
}

int macro_relational_expr()
{
	int lhs = macro_additive_expr();

	if(match("<", macro_token->s))
	{
		eat_current_token();
		return lhs < macro_relational_expr();
	}
	else if(match("<=", macro_token->s))
	{
		eat_current_token();
		return lhs <= macro_relational_expr();
	}
	else if(match(">=", macro_token->s))
	{
		eat_current_token();
		return lhs >= macro_relational_expr();
	}
	else if(match(">", macro_token->s))
	{
		eat_current_token();
		return lhs > macro_relational_expr();
	}
	else if(match("==", macro_token->s))
	{
		eat_current_token();
		return lhs == macro_relational_expr();
	}
	else if(match("!=", macro_token->s))
	{
		eat_current_token();
		return lhs != macro_relational_expr();
	}
	else
	{
		return lhs;
	}
}

int macro_bitwise_expr()
{
	int rhs;
	int lhs = macro_relational_expr();

	if(match("&", macro_token->s))
	{
		eat_current_token();
		return lhs & macro_bitwise_expr();
	}
	else if(match("&&", macro_token->s))
	{
		eat_current_token();
		rhs = macro_bitwise_expr();
		return lhs && rhs;
	}
	else if(match("|", macro_token->s))
	{
		eat_current_token();
		rhs = macro_bitwise_expr();
		return lhs | rhs;
	}
	else if(match("||", macro_token->s))
	{
		eat_current_token();
		rhs = macro_bitwise_expr();
		return lhs || rhs;
	}
	else if(match("^", macro_token->s))
	{
		eat_current_token();
		rhs = macro_bitwise_expr();
		return lhs ^ rhs;
	}
	else
	{
		return lhs;
	}
}

int macro_expression()
{
	return macro_bitwise_expr();
}

void handle_define()
{
	struct macro_list* hold;
	struct token_list* expansion_end;

	eat_current_token();

	require(NULL != macro_token, "got an EOF terminated #define\n");
	require('\n' != macro_token->s[0], "unexpected newline after #define\n");

	/* insert new macro */
	hold = calloc(1, sizeof(struct macro_list));
	hold->symbol = macro_token->s;
	hold->next = macro_env;
	macro_env = hold;

	/* discard the macro name */
	eat_current_token();

	while (TRUE)
	{
		require(NULL != macro_token, "got an EOF terminated #define\n");

		if ('\n' == macro_token->s[0])
		{
			expansion_end->next = NULL;
			return;
		}

		expansion_end = macro_token;

		/* in the first iteration, we set the first token of the expansion, if
		   it exists */
		if (NULL == hold->expansion)
		{
			hold->expansion = macro_token;
		}

		eat_current_token();
	}

}

void macro_directive()
{
	struct conditional_inclusion *t;
	int result;

	/* FIXME: whitespace is allowed between "#"" and "if" */
	if(match("#if", macro_token->s))
	{
		eat_current_token();
		/* evaluate constant integer expression */
		result = macro_expression();
		/* push conditional inclusion */
		t = calloc(1, sizeof(struct conditional_inclusion));
		t->prev = conditional_inclusion_top;
		conditional_inclusion_top = t;
		t->include = TRUE;

		if(FALSE == result)
		{
			t->include = FALSE;
		}

		t->previous_condition_matched = t->include;
	}
	else if(match("#elif", macro_token->s))
	{
		eat_current_token();
		result = macro_expression();
		require(NULL != conditional_inclusion_top, "#elif without leading #if\n");
		conditional_inclusion_top->include = result && !conditional_inclusion_top->previous_condition_matched;
		conditional_inclusion_top->previous_condition_matched =
		    conditional_inclusion_top->previous_condition_matched || conditional_inclusion_top->include;
	}
	else if(match("#else", macro_token->s))
	{
		eat_current_token();
		require(NULL != conditional_inclusion_top, "#else without leading #if\n");
		conditional_inclusion_top->include = !conditional_inclusion_top->previous_condition_matched;
	}
	else if(match("#endif", macro_token->s))
	{
		if(NULL == conditional_inclusion_top)
		{
			line_error_token(macro_token);
			file_print("unexpected #endif\n", stderr);
			exit(EXIT_FAILURE);
		}

		eat_current_token();
		/* pop conditional inclusion */
		t = conditional_inclusion_top;
		conditional_inclusion_top = conditional_inclusion_top->prev;
		free(t);
	}
	else if(match("#define", macro_token->s))
	{
		handle_define();
	}
	else
	{
		/* unhandled macro directive; let's eat until a newline; om nom nom */
		while(TRUE)
		{
			if(NULL == macro_token)
			{
				return;
			}

			if('\n' == macro_token->s[0])
			{
				return;
			}

			eat_current_token();
		}
	}
}

struct token_list* maybe_expand(struct token_list* token)
{
	struct macro_list* hold = lookup_macro(token);
	struct token_list* hold2;
	if (NULL == hold)
	{
		return token->next;
	}

	token = eat_token(token);
	hold2 = insert_tokens(token, hold->expansion);

	return hold2->next;
}

void preprocess()
{
	int start_of_line = TRUE;
	macro_token = global_token;

	while(NULL != macro_token)
	{
		if(start_of_line && '#' == macro_token->s[0])
		{
			macro_directive();

			if(macro_token)
			{
				if('\n' != macro_token->s[0])
				{
					line_error_token(macro_token);
					file_print("newline expected at end of macro directive\n", stderr);
					file_print("found: '", stderr);
					file_print(macro_token->s, stderr);
					file_print("'\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
		}
		else if('\n' == macro_token->s[0])
		{
			start_of_line = TRUE;
			macro_token = macro_token->next;
		}
		else
		{
			start_of_line = FALSE;
			if(NULL == conditional_inclusion_top)
			{
				macro_token = maybe_expand(macro_token);
			}
			else if(!conditional_inclusion_top->include)
			{
				/* rewrite the token stream to exclude the current token */
				eat_current_token();
			}
			else
			{
				macro_token = maybe_expand(macro_token);
			}
		}
	}
}
