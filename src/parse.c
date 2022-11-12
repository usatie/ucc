/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:28:47 by susami            #+#    #+#             */
/*   Updated: 2022/11/12 15:03:05 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ucc.h"

// parser.c
// utility
// Returns if `token` matches `op`.
static bool	isequal(const Token *token, const char *op)
{
	return (token->kind == TK_RESERVED
		&& memcmp(token->str, op, token->len) == 0
		&& op[token->len] == '\0');
}

// Ensure that `token` matches `op`.
static void	expect(const Token *token, const char *op)
{
	if (!isequal(token, op))
		error_at(token->str, "expected '%s', but not.", op);
}

static bool	at_eof(Token *token)
{
	return (token->kind == TK_EOF);
}

static Node	*new_node(NodeKind kind)
{
	Node	*node;

	node = calloc(1, sizeof(Node));
	node->kind = kind;
	return (node);
}

static Node	*new_node_binary(NodeKind kind, Node *lhs, Node *rhs)
{
	Node	*node;

	node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return (node);
}

static Node	*new_node_unary(NodeKind kind, Node *expr)
{
	Node	*node;

	node = new_node(kind);
	node->lhs = expr;
	return (node);
}

static Node	*new_node_num(int val)
{
	Node	*node;

	node = new_node(ND_NUM);
	node->val = val;
	return (node);
}

static LVar	*find_lvar(const Token *tok)
{
	LVar	*var;

	var = ctx.lvars;
	while (var)
	{
		if (var->len == tok->len && memcmp(tok->str, var->name, var->len) == 0)
			return (var);
		var = var->next;
	}
	return (NULL);
}

static Node	*new_node_lvar(Token *tok)
{
	LVar	*lvar;
	Node	*node;

	node = new_node(ND_LVAR);
	lvar = find_lvar(tok);
	if (lvar)
		node->lvar = lvar;
	else
	{
		lvar = calloc(1, sizeof(LVar));
		lvar->next = ctx.lvars;
		lvar->name = tok->str;
		lvar->len = tok->len;
		if (ctx.lvars == NULL)
			lvar->offset = 8;
		else
			lvar->offset = ctx.lvars->offset + 8;
		node->lvar = lvar;
		ctx.lvars = lvar;
	}
	return (node);
}

// syntax parser

/*
EBNF syntax
(Exetnded Backus-Naur form)

program      = stmt*
stmt         = expr ";"
             | "return" expr ";"
expr         = assign
assign       = equality ("=" assign)?
equality     = relational ("==" relational | "!=" relational)*
relational   = add ("<" add | "<=" add | ">" add | ">=" add)*
add          = mul ("+" mul | "-" mul)*
mul          = unary ("*" unary | "/" unary)*
unary        = ("+" | "-")? primary
primary      = num | ident | "(" expr ")"

*/

// program = stmt*
Node	*parse(Token *tok)
{
	Node	*head;
	Node	*cur;

	head = stmt(&tok, tok);
	cur = head;
	while (!at_eof(tok))
	{
		cur->next = stmt(&tok, tok);
		cur = cur->next;
	}
	return (head);
}

// stmt = "return" expr ";"
//      | expr ";"
Node	*stmt(Token **rest, Token *tok)
{
	Node	*node;

	if (tok->kind == TK_RETURN)
		node = new_node_unary(ND_RETURN_STMT, expr(&tok, tok->next));
	else
		node = new_node_unary(ND_EXPR_STMT, expr(&tok, tok));
	expect(tok, ";");
	*rest = tok->next;
	return (node);
}

// expr = assign
Node	*expr(Token **rest, Token *tok)
{
	return (assign(rest, tok));
}

// assign = equality ("=" assign)?
Node	*assign(Token **rest, Token *tok)
{
	Node	*node;

	node = equality(&tok, tok);
	if (isequal(tok, "="))
		node = new_node_binary(ND_ASSIGN, node, assign(&tok, tok->next));
	*rest = tok;
	return (node);
}

// equality = relational ("==" relational || "!=" relational)*
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

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
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

// add = mul ("+" mul | "-" mul)*
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

// mul = unary ("*" unary | "/" unary)*
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

// unary = ("+" | "-")? primary
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

// primary = num | ident | "(" expr ")"
Node	*primary(Token **rest, Token *tok)
{
	Node	*node;

	if (isequal(tok, "("))
	{
		node = expr(&tok, tok->next);
		expect(tok, ")");
		*rest = tok->next;
		return (node);
	}
	if (tok->kind == TK_NUM)
	{
		node = new_node_num(tok->val);
		*rest = tok->next;
		return (node);
	}
	if (tok->kind == TK_IDENT)
	{
		node = new_node_lvar(tok);
		*rest = tok->next;
		return (node);
	}
	error_at(tok->str, "Invalid token.");
}
