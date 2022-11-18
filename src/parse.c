/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:28:47 by susami            #+#    #+#             */
/*   Updated: 2022/11/18 12:20:04 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ucc.h"

// parser.c
// utility
// Returns if `token` matches `op`.
static bool	isequal(const Token *tok, const char *op)
{
	return (memcmp(tok->str, op, tok->len) == 0
		&& op[tok->len] == '\0');
}

// Ensure that `token` matches `op`,
// and returns the next token.
static Token	*expect_and_skip(const Token *tok, const char *op)
{
	if (!isequal(tok, op))
		error_at(tok->str, "expected '%s', but not.", op);
	return (tok->next);
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
stmt         = expr-stmt
             | "return" expr ";"
			 | "if" "(" expr ")" stmt ("else" stmt)?
			 | "while" "(" expr ")" stmt
             | "for" "(" expr? ";" expr? ";" expr? ")" stmt
expr-stmt    = expr ";"
expr         = assign
assign       = equality ("=" assign)?
equality     = relational ("==" relational | "!=" relational)*
relational   = add ("<" add | "<=" add | ">" add | ">=" add)*
add          = mul ("+" mul | "-" mul)*
mul          = unary ("*" unary | "/" unary)*
unary        = ("+" | "-")? primary
primary      = num 
             | ident ( "(" ")" )?
			 | "(" expr ")"

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
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "{" stmt* "}"
//      | expr-stmt
Node	*stmt(Token **rest, Token *tok)
{
	Node	*node;
	Node	*cur;

	if (isequal(tok, "return"))
	{
		node = new_node_unary(ND_RETURN_STMT, expr(&tok, tok->next));
		*rest = expect_and_skip(tok, ";");
		return (node);
	}
	else if (isequal(tok, "for"))
	{
		node = new_node(ND_FOR_STMT);
		tok = expect_and_skip(tok->next, "(");
		// init? ;
		if (!isequal(tok, ";"))
			node->init = expr(&tok, tok);
		tok = expect_and_skip(tok, ";");
		// cond? ;
		if (!isequal(tok, ";"))
			node->cond = expr(&tok, tok);
		tok = expect_and_skip(tok, ";");
		// inc?
		if (!isequal(tok, ")"))
			node->inc = expr(&tok, tok);
		tok = expect_and_skip(tok, ")");
		// then
		node->then = stmt(rest, tok);
		return (node);
	}
	else if (isequal(tok, "if"))
	{
		node = new_node(ND_IF_STMT);
		tok = expect_and_skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = expect_and_skip(tok, ")");
		node->then = stmt(&tok, tok);
		if (isequal(tok, "else"))
			node->els = stmt(&tok, tok->next);
		*rest = tok;
		return (node);
	}
	else if (isequal(tok, "while"))
	{
		node = new_node(ND_WHILE_STMT);
		tok = expect_and_skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = expect_and_skip(tok, ")");
		node->then = stmt(&tok, tok);
		*rest = tok;
		return (node);
	}
	else if (isequal(tok, "{"))
	{
		node = new_node(ND_BLOCK);
		tok = tok->next;
		if (!isequal(tok, "}"))
			node->body = stmt(&tok, tok);
		cur = node->body;
		while (!isequal(tok, "}"))
			cur = cur->next = stmt(&tok, tok);
		*rest = expect_and_skip(tok, "}");
		return (node);
	}
	else
		return (expr_stmt(rest, tok));
}

// expr-stmt = expr? ";"
Node	*expr_stmt(Token **rest, Token *tok)
{
	Node	*node;

	if (isequal(tok, ";"))
	{
		*rest = tok->next;
		return (new_node(ND_BLOCK));
	}
	node = new_node_unary(ND_EXPR_STMT, expr(&tok, tok));
	*rest = expect_and_skip(tok, ";");
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

Node	*parse_args(Token **rest, Token *tok)
{
	Node	*head;
	Node	*cur;

	head = expr(&tok, tok);
	cur = head;
	while (!isequal(tok, ")"))
	{
		tok = expect_and_skip(tok, ",");
		cur->next = expr(&tok, tok);
		cur = cur->next;
	}
	*rest = tok;
	return (head);
}

Node	*funcall(Token **rest, Token *tok)
{
	Node	*node;

	node = new_node(ND_FUNC_CALL);
	node->funcname = strndup(tok->str, tok->len);
	tok = tok->next->next;
	if (!isequal(tok, ")"))
		node->args = parse_args(&tok, tok);
	*rest = expect_and_skip(tok, ")");
	return (node);
}

// primary = num 
//         | ident ( "(" ")" )?
//         | "(" expr ")"
Node	*primary(Token **rest, Token *tok)
{
	Node	*node;

	if (isequal(tok, "("))
	{
		node = expr(&tok, tok->next);
		*rest = expect_and_skip(tok, ")");
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
		// Function call
		if (isequal(tok->next, "("))
			return (funcall(rest, tok));
		// Variable
		node = new_node_lvar(tok);
		*rest = tok->next;
		return (node);
	}
	error_at(tok->str, "Invalid token.");
}
