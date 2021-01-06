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

struct conditional_inclusion
{
	struct conditional_inclusion* prev;
	int include; /* 1 == include, 0 == skip */
	int previous_condition_matched; /* 1 == all subsequent conditions treated as FALSE */
};

struct conditional_inclusion* conditional_inclusion_top;

/* point where we are currently modifying the global_token list */
struct token_list* macro_token;

void eat_current_token()
{
	struct token_list* tmp;

	if(NULL != macro_token->prev)
	{
		macro_token->prev->next = macro_token->next;
	}
	else
	{
		global_token = macro_token->next;
	}

	/* update backlinks */
	if(NULL != macro_token->next)
	{
		macro_token->next->prev = macro_token->prev;
	}

	tmp = macro_token;
	macro_token = macro_token->next;
	free(tmp);
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
	require(NULL != macro_token, "got an EOF terminated macro expression\n");

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

	require(NULL != macro_token, "got an EOF terminated macro expression\n");
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
				macro_token = macro_token->next;
			}
			else if(!conditional_inclusion_top->include)
			{
				/* rewrite the token stream to exclude the current token */
				eat_current_token();
			}
			else
			{
				macro_token = macro_token->next;
			}
		}
	}
}
