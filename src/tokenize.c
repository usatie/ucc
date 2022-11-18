/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenize.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 12:01:38 by susami            #+#    #+#             */
/*   Updated: 2022/11/18 10:33:57 by susami           ###   ########.fr       */
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

static bool	is_match_keyword(char *p, char *kw)
{
	const size_t	len = strlen(kw);

	return (strncmp(p, kw, len) == 0 && !is_ident_non_first(p[len]));
}

static bool	is_keyword(char *p)
{
	static char		*kw[] = {"return", "if", "else", "while", "for"};
	unsigned long	i;

	i = 0;
	while (i < sizeof(kw) / sizeof(*kw))
	{
		if (is_match_keyword(p, kw[i]))
			return (true);
		i++;
	}
	return (false);
}

static void	covnert_keywords(Token *tok)
{
	Token	*t;

	t = tok;
	while (t->kind != TK_EOF)
	{
		if (t->kind == TK_IDENT && is_keyword(t->str))
			t->kind = TK_KEYWORD;
		t = t->next;
	}
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
			cur = cur->next = new_token(TK_RESERVED, p);
			cur->len = 2;
			p += 2;
			continue ;
		}
		// Single-letter punctuator
		if (strchr("+-*/()<>;={},", *p))
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
		// identifier or keyword
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
		error_at(p, "Invalid character.");
	}
	cur->next = new_token(TK_EOF, p);
	covnert_keywords(head.next);
	return (head.next);
}
