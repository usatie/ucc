/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:28:47 by susami            #+#    #+#             */
/*   Updated: 2022/11/10 11:51:31 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucc.h"

// tokenizer.c
bool	consume(char *op)
{
	if (ctx->token->kind != TK_RESERVED
		|| (int)strlen(op) != ctx->token->len
		|| memcmp(ctx->token->str, op, ctx->token->len))
		return (false);
	ctx->token = ctx->token->next;
	return (true);
}

void	expect(char *op)
{
	if (ctx->token->kind != TK_RESERVED
		|| (int)strlen(op) != ctx->token->len
		|| memcmp(ctx->token->str, op, ctx->token->len))
		error_at(ctx->token->str, "expected '%s', but not.", op);
	ctx->token = ctx->token->next;
}

int	expect_number(void)
{
	const int	val = ctx->token->val;

	if (ctx->token->kind != TK_NUM)
		error_at(ctx->token->str, "expected number, but not.");
	ctx->token = ctx->token->next;
	return (val);
}

bool	at_eof(void)
{
	return (ctx->token->kind == TK_EOF);
}

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
		if (strchr("+-*/()<>", *p))
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
		error_at(p, "Invalid Token.");
	}
	new_token(TK_EOF, cur, p, 0);
	return (head.next);
}

// parser.c
Node	*new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node	*node;

	node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return (node);
}

Node	*new_node_num(int val)
{
	Node	*node;

	node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return (node);
}

Node	*expr(void)
{
	return (equality());
}

Node	*equality(void)
{
	Node	*node;

	node = relational();
	while (1)
	{
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NEQ, node, relational());
		else
			return (node);
	}
}

Node	*relational(void)
{
	Node	*node;

	node = add();
	while (1)
	{
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LTE, node, add());
		else if (consume(">"))
			node = new_node(ND_GT, node, add());
		else if (consume(">="))
			node = new_node(ND_GTE, node, add());
		else
			return (node);
	}
}

Node	*add(void)
{
	Node	*node;

	node = mul();
	while (1)
	{
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return (node);
	}
}

Node	*mul(void)
{
	Node	*node;

	node = unary();
	while (1)
	{
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return (node);
	}
}

Node	*unary(void)
{
	if (consume("+"))
		return (primary());
	if (consume("-"))
		return (new_node(ND_SUB, new_node_num(0), primary()));
	return (primary());
}

Node	*primary(void)
{
	Node	*node;

	if (consume("("))
	{
		node = expr();
		expect(")");
		return (node);
	}
	node = new_node_num(expect_number());
	return (node);
}
