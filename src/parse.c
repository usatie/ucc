/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:28:47 by susami            #+#    #+#             */
/*   Updated: 2022/11/12 11:14:32 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ucc.h"

// parser.c
// utility
/*
bool	consume(Token *token, char *op)
{
	if (token->kind != TK_RESERVED
		|| (int)strlen(op) != token->len
		|| memcmp(token->str, op, token->len))
		return (false);
	ctx->token = token->next;
	return (true);
}

void	expect(Token *token, char *op)
{
	if (token->kind != TK_RESERVED
		|| (int)strlen(op) != token->len
		|| memcmp(token->str, op, token->len))
		error_at(token->str, "expected '%s', but not.", op);
	ctx->token = token->next;
}

int	expect_number(Token *token)
{
	const int	val = token->val;

	if (token->kind != TK_NUM)
		error_at(token->str, "expected number, but not.");
	ctx->token = token->next;
	return (val);
}
*/

// Returns if `token` matches `op`.
static bool	isequal(const Token *token, const char *op)
{
	return (token->kind == TK_RESERVED
		&& memcmp(token->str, op, token->len) == 0
		&& op[token->len] == '\0');
}

// Ensure that `token` matches `op`.
// Returns the next token if it does.
static Token	*expect(const Token *token, const char *op)
{
	if (!isequal(token, op))
		error_at(token->str, "expected '%s', but not.", op);
	return (token->next);
}

bool	at_eof(Token *token)
{
	return (token->kind == TK_EOF);
}

Node	*new_node(NodeKind kind)
{
	Node	*node;

	node = calloc(1, sizeof(Node));
	node->kind = kind;
	return (node);
}

Node	*new_node_binary(NodeKind kind, Node *lhs, Node *rhs)
{
	Node	*node;

	node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return (node);
}

Node	*new_node_unary(NodeKind kind, Node *expr)
{
	Node	*node;

	node = new_node(kind);
	node->lhs = expr;
	return (node);
}

Node	*new_node_num(int val)
{
	Node	*node;

	node = new_node(ND_NUM);
	node->val = val;
	return (node);
}

// syntax parser

Node	*parse(Token *tok)
{
	Node	*node;
	Node	*cur;

	node = stmt(&tok, tok);
	cur = node;
	while (!at_eof(tok))
	{
		cur->next = stmt(&tok, tok);
		cur = cur->next;
	}
	return (node);
}

Node	*stmt(Token **rest, Token *tok)
{
	Node	*node;

	node = new_node_unary(ND_STMT, expr(&tok, tok));
	*rest = expect(tok, ";");
	return (node);
}

Node	*expr(Token **rest, Token *tok)
{
	return (equality(rest, tok));
}

Node	*equality(Token **rest, Token *tok)
{
	Node	*node;

	node = relational(&tok, tok);
	while (1)
	{
		if (isequal(tok, "=="))
			node = new_node_binary(ND_EQ, node, relational(&tok, tok->next));
		else if (isequal(tok, "!="))
			node = new_node_binary(ND_NEQ, node, relational(&tok, tok->next));
		else
		{
			*rest = tok;
			return (node);
		}
	}
}

Node	*relational(Token **rest, Token *tok)
{
	Node	*node;

	node = add(&tok, tok);
	while (1)
	{
		if (isequal(tok, "<"))
			node = new_node_binary(ND_LT, node, add(&tok, tok->next));
		else if (isequal(tok, "<="))
			node = new_node_binary(ND_LTE, node, add(&tok, tok->next));
		else if (isequal(tok, ">"))
			node = new_node_binary(ND_GT, node, add(&tok, tok->next));
		else if (isequal(tok, ">="))
			node = new_node_binary(ND_GTE, node, add(&tok, tok->next));
		else
		{
			*rest = tok;
			return (node);
		}
	}
}

Node	*add(Token **rest, Token *tok)
{
	Node	*node;

	node = mul(&tok, tok);
	while (1)
	{
		if (isequal(tok, "+"))
			node = new_node_binary(ND_ADD, node, mul(&tok, tok->next));
		else if (isequal(tok, "-"))
			node = new_node_binary(ND_SUB, node, mul(&tok, tok->next));
		else
		{
			*rest = tok;
			return (node);
		}
	}
}

Node	*mul(Token **rest, Token *tok)
{
	Node	*node;

	node = unary(&tok, tok);
	while (1)
	{
		if (isequal(tok, "*"))
			node = new_node_binary(ND_MUL, node, unary(&tok, tok->next));
		else if (isequal(tok, "/"))
			node = new_node_binary(ND_DIV, node, unary(&tok, tok->next));
		else
		{
			*rest = tok;
			return (node);
		}
	}
}

Node	*unary(Token **rest, Token *tok)
{
	if (isequal(tok, "+"))
		return (primary(rest, tok->next));
	if (isequal(tok, "-"))
		return (new_node_binary(
				ND_SUB,
				new_node_num(0),
				primary(rest, tok->next)));
	return (primary(rest, tok));
}

Node	*primary(Token **rest, Token *tok)
{
	Node	*node;

	if (isequal(tok, "("))
	{
		node = expr(&tok, tok->next);
		*rest = expect(tok, ")");
		return (node);
	}
	else if (tok->kind == TK_NUM)
	{
		node = new_node_num(tok->val);
		*rest = tok->next;
		return (node);
	}
	error_at(tok->str, "Invalid token.");
}
