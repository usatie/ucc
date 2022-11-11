/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenize.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 12:01:38 by susami            #+#    #+#             */
/*   Updated: 2022/11/11 09:44:07 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ucc.h"

// tokenizer.c
Token	*new_token(TokenKind kind, Token *cur, char *str, int len)
{
	Token	*tok;

	tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return (tok);
}

bool	startswith(char *p, char *q)
{
	return (memcmp(p, q, strlen(q)) == 0);
}

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
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue ;
		}
		// Single-letter punctuator
		if (strchr("+-*/()<>;", *p))
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}
		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p, 0);
			q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue ;
		}
		if ('a' <= *p && *p <= 'z')
		{
			cur = new_token(TK_IDENT, cur, p++, 0);
			cur->len = 1;
			continue ;
		}
		error_at(p, "Invalid Token.");
	}
	new_token(TK_EOF, cur, p, 0);
	return (head.next);
}
