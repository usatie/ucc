/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:28:47 by susami            #+#    #+#             */
/*   Updated: 2022/11/11 11:01:21 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ucc.h"

// parser.c
// utility
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

	(void)tok; // TODO: use this instead of ctx->tok
	node = stmt();
	cur = node;
	while (!at_eof())
	{
		cur->next = stmt();
		cur = cur->next;
	}
	return (node);
}

Node	*stmt(void)
{
	Node	*node;

	node = new_node_unary(ND_STMT, expr());
	expect(";");
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
			node = new_node_binary(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node_binary(ND_NEQ, node, relational());
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
			node = new_node_binary(ND_LT, node, add());
		else if (consume("<="))
			node = new_node_binary(ND_LTE, node, add());
		else if (consume(">"))
			node = new_node_binary(ND_GT, node, add());
		else if (consume(">="))
			node = new_node_binary(ND_GTE, node, add());
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
			node = new_node_binary(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node_binary(ND_SUB, node, mul());
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
			node = new_node_binary(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node_binary(ND_DIV, node, unary());
		else
			return (node);
	}
}

Node	*unary(void)
{
	if (consume("+"))
		return (primary());
	if (consume("-"))
		return (new_node_binary(ND_SUB, new_node_num(0), primary()));
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
