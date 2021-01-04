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

/* point where we are currently modifying the global_token list */
struct token_list* macro_token;

void eat_current_token()
{
    struct token_list* tmp;

    if (NULL != macro_token->prev)
        macro_token->prev->next = macro_token->next;
    else
        global_token = macro_token->next;

    /* update backlinks */
    if (NULL != macro_token->next)
        macro_token->next->prev = macro_token->prev;

    tmp = macro_token;
    macro_token = macro_token->next;
    free(tmp);
}

void eat_newline_tokens()
{
    macro_token = global_token;

    while (TRUE)
    {
        if (NULL == macro_token)
            return;

        if(match("\n", macro_token->s))
            eat_current_token();
        else
            macro_token = macro_token->next;
    }
}

