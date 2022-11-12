/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenize.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 12:01:38 by susami            #+#    #+#             */
/*   Updated: 2022/11/12 13:27:33 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ucc.h"

// tokenizer.c
static Token	*new_token(TokenKind kind, char *str)
{
	Token	*tok;

	tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	return (tok);
}

bool	startswith(char *p, char *q)
{
	return (memcmp(p, q, strlen(q)) == 0);
}

static bool	is_ident_first(char c)
{
	return (isalpha(c) || c == '_');
}

static bool	is_ident_non_first(char c)
{
	return (is_ident_first(c) || isdigit(c));
}

#include <stdio.h>

Token	*tokenize(char *p)
{
	Token	head;
	Token	*cur;
	char	*q;

	head.next = NULL;
	cur = &head;
	while (*p)
	{
		if (isspace(*p))
		{
			p++;
			continue ;
		}
		// Multi-letter punctuator
		if (startswith(p, "==") || startswith(p, "!=")
			|| startswith(p, "<=") || startswith(p, ">="))
		{
			cur = cur->next = new_token(TK_RESERVED, p);
			cur->len = 2;
			p += 2;
			continue ;
		}
		// Single-letter punctuator
		if (strchr("+-*/()<>;=", *p))
		{
			cur = cur->next = new_token(TK_RESERVED, p++);
			cur->len = 1;
			continue ;
		}
		if (isdigit(*p))
		{
			cur = cur->next = new_token(TK_NUM, p);
			q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue ;
		}
		if (is_ident_first(*p))
		{
			cur = cur->next = new_token(TK_IDENT, p);
			q = p;
			p++;
			while (is_ident_non_first(*p))
				p++;
			cur->len = p - q;
			continue ;
		}
		error_at(p, "Invalid Token.");
	}
	cur->next = new_token(TK_EOF, p);
	return (head.next);
}
