/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:28:47 by susami            #+#    #+#             */
/*   Updated: 2022/12/08 13:34:23 by susami           ###   ########.fr       */
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
static Token	*skip_op(const Token *tok, const char *op)
{
	if (!isequal(tok, op))
		error_tok(tok, "expected '%s', but not.", op);
	return (tok->next);
}

// Ensure that `token` matches `kind`.
// and returns the next token.
static Token	*skip_kind(const Token *tok, TokenKind kind)
{
	if (tok->kind != kind)
		error_tok(tok, "expected '%d', but not.", kind);
	return (tok->next);
}

// Returns true if `tok` matches `op`
// and update `rest`.
static bool	consume_op(Token **rest, Token *tok, char *op)
{
	if (isequal(tok, op))
	{
		*rest = tok->next;
		return (true);
	}
	*rest = tok;
	return (false);
}

static bool	at_eof(Token *token)
{
	return (token->kind == TK_EOF);
}

static Node	*new_node(NodeKind kind, Token *tok)
{
	Node	*node;

	node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->tok = tok;
	return (node);
}

static Node	*new_node_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
	Node	*node;

	node = new_node(kind, tok);
	node->lhs = lhs;
	node->rhs = rhs;
	return (node);
}

static Node	*new_node_unary(NodeKind kind, Node *expr, Token *tok)
{
	Node	*node;

	node = new_node(kind, tok);
	node->lhs = expr;
	return (node);
}

static Node	*new_node_num(int val, Token *tok)
{
	Node	*node;

	node = new_node(ND_NUM, tok);
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

static LVar	*new_lvar(const Token *tok, Type *ty)
{
	LVar	*lvar;

	lvar = find_lvar(tok);
	if (lvar)
		error_tok(tok, "Already declared identifier");
	else
	{
		lvar = calloc(1, sizeof(LVar));
		lvar->name = tok->str;
		lvar->len = tok->len;
		lvar->type = ty;
		if (ctx.lvars == NULL)
			ctx.lvars = lvar;
		else
		{
			LVar *last = ctx.lvars;
			while (last && last->next)
				last = last->next;
			last->next = lvar;
		}
	}
	return (lvar);
}

static Node	*new_node_lvar(Token *tok)
{
	LVar	*lvar;
	Node	*node;

	node = new_node(ND_LVAR, tok);
	lvar = find_lvar(tok);
	if (lvar == NULL)
		error_tok(tok, "Undeclared identifier");
	node->lvar = lvar;
	return (node);
}

// syntax parser

/*
EBNF syntax
(Exetnded Backus-Naur form)

program      = funcdecl*
funcdecl     = declspec ident "(" vardecl* ")" block
block        = "{" stmt* "}"
stmt         = expr-stmt
             | "int" "*"* ident ";"
             | "return" expr ";"
			 | "if" "(" expr ")" stmt ("else" stmt)?
			 | "while" "(" expr ")" stmt
             | "for" "(" expr? ";" expr? ";" expr? ")" stmt
             | block
expr-stmt    = expr ";"
expr         = assign
assign       = equality ("=" assign)?
equality     = relational ("==" relational | "!=" relational)*
relational   = add ("<" add | "<=" add | ">" add | ">=" add)*
add          = mul ("+" mul | "-" mul)*
mul          = unary ("*" unary | "/" unary)*
unary        = ("+" | "-" | "&")? primary
             | "*"* primary
primary      = num 
             | funcall
             | ident
			 | "(" expr ")"
funcall      = ident "(" expr? ")"
             | ident "(" expr ( "," expr )* ")" 
*/

// program = funcdecl*
Function	*parse(Token *tok)
{
	Function	*head;
	Function	*cur;

	head = funcdecl(&tok, tok);
	cur = head;
	while (!at_eof(tok))
	{
		cur->next = funcdecl(&tok, tok);
		cur = cur->next;
	}
	return (head);
}

// declspec = "int" "*"*
Type	*declspec(Token **rest, Token *tok)
{
	Type	*type;

	tok = skip_op(tok, "int");
	type = ty_int;
	while (consume_op(&tok, tok, "*"))
		type = ptr_to(type);
	*rest = tok;
	return (type);
}

// vardecl = declspec ident
Node	*vardecl(Token **rest, Token *tok)
{
	Node	*node;
	Type	*type;

	node = new_node(ND_BLOCK, tok); // this is empty block
	type = declspec(&tok, tok);
	new_lvar(tok, type); // this is only for allocate local variable
	*rest = tok->next;
	return (node);
}

// funcdecl = declspec ident "(" vardecl? ("," vardecl)* ")" block
Function	*funcdecl(Token **rest, Token *tok)
{
	Function	*func;

	ctx.lvars = NULL;
	func = calloc(sizeof(Function), 1);
	func->type = declspec(&tok, tok);
	skip_kind(tok, TK_IDENT);
	func->name = strndup(tok->str, tok->len);
	tok = skip_op(tok->next, "(");
	if (!isequal(tok, ")"))
	{
		do {
			vardecl(&tok, tok);
			func->nargs++;
		} while (consume_op(&tok, tok, ","));
	}
	tok = skip_op(tok, ")");
	func->args = ctx.lvars;
	func->body = block(rest, tok);
	func->locals = ctx.lvars;
	return (func);
}

// block = "{" stmt* "}"
Node	*block(Token **rest, Token *tok)
{
	Node	*node;
	Node	head = {};
	Node	*cur = &head;

	node = new_node(ND_BLOCK, tok);
	tok = skip_op(tok, "{");
	while (!isequal(tok, "}"))
	{
		cur = cur->next = stmt(&tok, tok);
		add_type(cur);
	}
	node->body = head.next;
	*rest = skip_op(tok, "}");
	return (node);
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | block
//      | "int" "*"* ident ";"
//      | expr-stmt
Node	*stmt(Token **rest, Token *tok)
{
	Node	*node;

	if (isequal(tok, "return"))
	{
		node = new_node_unary(ND_RETURN_STMT, expr(&tok, tok->next), tok);
		*rest = skip_op(tok, ";");
		return (node);
	}
	else if (isequal(tok, "for"))
	{
		node = new_node(ND_FOR_STMT, tok);
		tok = skip_op(tok->next, "(");
		// init? ;
		if (!isequal(tok, ";"))
			node->init = expr(&tok, tok);
		tok = skip_op(tok, ";");
		// cond? ;
		if (!isequal(tok, ";"))
			node->cond = expr(&tok, tok);
		tok = skip_op(tok, ";");
		// inc?
		if (!isequal(tok, ")"))
			node->inc = expr(&tok, tok);
		tok = skip_op(tok, ")");
		// then
		node->then = stmt(rest, tok);
		return (node);
	}
	else if (isequal(tok, "if"))
	{
		node = new_node(ND_IF_STMT, tok);
		tok = skip_op(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip_op(tok, ")");
		node->then = stmt(&tok, tok);
		if (isequal(tok, "else"))
			node->els = stmt(&tok, tok->next);
		*rest = tok;
		return (node);
	}
	else if (isequal(tok, "while"))
	{
		node = new_node(ND_WHILE_STMT, tok);
		tok = skip_op(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip_op(tok, ")");
		node->then = stmt(&tok, tok);
		*rest = tok;
		return (node);
	}
	else if (isequal(tok, "{"))
		return (block(rest, tok));
	else if (isequal(tok, "int"))
		return (vardecl(rest, tok));
	else
		return (expr_stmt(rest, tok));
}

// expr-stmt = expr? ";"
Node	*expr_stmt(Token **rest, Token *tok)
{
	Node	*node;

	if (consume_op(rest, tok, ";"))
	{
		return (new_node(ND_BLOCK, tok));
	}
	node = new_node_unary(ND_EXPR_STMT, expr(&tok, tok), tok);
	*rest = skip_op(tok, ";");
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
		node = new_node_binary(ND_ASSIGN, node, assign(&tok, tok->next), tok);
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
		Token	*start = tok;
		if (isequal(tok, "=="))
			node = new_node_binary(ND_EQ, node, relational(&tok, tok->next), start);
		else if (isequal(tok, "!="))
			node = new_node_binary(ND_NEQ, node, relational(&tok, tok->next), start);
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
		Token	*start = tok;
		if (isequal(tok, "<"))
			node = new_node_binary(ND_LT, node, add(&tok, tok->next), start);
		else if (isequal(tok, "<="))
			node = new_node_binary(ND_LTE, node, add(&tok, tok->next), start);
		else if (isequal(tok, ">"))
			node = new_node_binary(ND_GT, node, add(&tok, tok->next), start);
		else if (isequal(tok, ">="))
			node = new_node_binary(ND_GTE, node, add(&tok, tok->next), start);
		else
		{
			*rest = tok;
			return (node);
		}
	}
}

// In C, `+` operator is overloaded to perform the pointer arithmetic.
// If p is a pointer, p+n adds not n but sizeof(*p)*n to the value of p,
// so that p+n points to the location of n elements (not bytes) ahead of p.
// In other words, we need to scale an integer value before adding to a
// pointer value. This function take care of the scaling.
static Node	*new_add(Node *lhs, Node *rhs, Token *tok)
{
	add_type(lhs);
	add_type(rhs);
	// num + num
	if (is_integer(lhs->ty) && is_integer(rhs->ty))
		return new_node_binary(ND_ADD, lhs, rhs, tok);
	// ptr + ptr is invalid
	if (lhs->ty->ptr_to && rhs->ty->ptr_to)
		error_tok(tok, "Invalid operands");
	// num + ptr -> ptr + num
	if (lhs->ty->ptr_to == NULL && rhs->ty->ptr_to)
	{
		Node	*tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}
	// ptr + num
	rhs = new_node_binary(ND_MUL, rhs, new_node_num(rhs->ty->size, tok), tok);
	return new_node_binary(ND_ADD, lhs, rhs, tok);
}

// `-` is overloaded for the pointer type.
static Node	*new_sub(Node *lhs, Node *rhs, Token *tok)
{
	add_type(lhs);
	add_type(rhs);
	// num - num
	if (is_integer(lhs->ty) && is_integer(rhs->ty))
		return new_node_binary(ND_SUB, lhs, rhs, tok);
	// ptr - num
	if (lhs->ty->ptr_to && rhs->ty->ptr_to == NULL)
	{
		rhs = new_node_binary(ND_MUL, rhs, new_node_num(rhs->ty->size, tok), tok);
		add_type(rhs);
		Node	*node = new_node_binary(ND_SUB, lhs, rhs, tok);
		node->ty = lhs->ty;
		return node;
	}
	// ptr - ptr, which means how many elements are between the two.
	if (lhs->ty->ptr_to && rhs->ty->ptr_to)
	{
		Node	*node = new_node_binary(ND_SUB, lhs, rhs, tok);
		node->ty = ty_int;
		return new_node_binary(ND_DIV, node, new_node_num(rhs->ty->size, tok), tok);
	}
	error_tok(tok, "Invalid operands");
}

// add = mul ("+" mul | "-" mul)*
Node	*add(Token **rest, Token *tok)
{
	Node	*node;

	node = mul(&tok, tok);
	while (1)
	{
		Token	*start = tok;
		if (isequal(tok, "+"))
			node = new_add(node, mul(&tok, tok->next), start);
		else if (isequal(tok, "-"))
			node = new_sub(node, mul(&tok, tok->next), start);
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
		Token	*start = tok;
		if (isequal(tok, "*"))
			node = new_node_binary(ND_MUL, node, unary(&tok, tok->next), start);
		else if (isequal(tok, "/"))
			node = new_node_binary(ND_DIV, node, unary(&tok, tok->next), start);
		else
		{
			*rest = tok;
			return (node);
		}
	}
}

// unary = ("+" | "-" | "&")? primary
//       | "*"* primary
Node	*unary(Token **rest, Token *tok)
{
	Node	*node;
	int		num_deref;

	if (isequal(tok, "+"))
		return (primary(rest, tok->next));
	if (isequal(tok, "-"))
		return (new_node_unary(
				ND_NEG,
				primary(rest, tok->next),
				tok));
	if (isequal(tok, "*"))
	{
		Token	*start = tok;
		num_deref = 0;
		while (consume_op(&tok, tok, "*"))
			num_deref++;
		node = primary(rest, tok);
		while (num_deref > 0)
		{
			node = new_node_unary(ND_DEREF, node, start);
			num_deref--;
		}
		return (node);
	}
	if (isequal(tok, "&"))
		return (new_node_unary(ND_ADDR, primary(rest, tok->next), tok));
	return (primary(rest, tok));
}

// args = expr?
//      | expr ( "," expr )*
Node	*parse_args(Token **rest, Token *tok)
{
	Node	*head;
	Node	*cur;

	head = expr(&tok, tok);
	cur = head;
	while (!isequal(tok, ")"))
	{
		tok = skip_op(tok, ",");
		cur->next = expr(&tok, tok);
		cur = cur->next;
	}
	*rest = tok;
	return (head);
}

// funcall = ident "(" expr? ")"
//         | ident "(" expr ( "," expr )* ")" 
Node	*funcall(Token **rest, Token *tok)
{
	Node	*node;

	node = new_node(ND_FUNC_CALL, tok);
	node->funcname = strndup(tok->str, tok->len);
	tok = tok->next->next;
	if (!isequal(tok, ")"))
		node->args = parse_args(&tok, tok);
	*rest = skip_op(tok, ")");
	return (node);
}

// primary = num 
//         | funcall
//         | ident
//         | "(" expr ")"
//         | "sizeof" primary
Node	*primary(Token **rest, Token *tok)
{
	Node	*node;

	if (isequal(tok, "("))
	{
		node = expr(&tok, tok->next);
		*rest = skip_op(tok, ")");
		return (node);
	}
	if (tok->kind == TK_NUM)
	{
		node = new_node_num(tok->val, tok);
		*rest = tok->next;
		return (node);
	}
	if (tok->kind == TK_SIZEOF)
	{
		node = unary(rest, tok->next);
		add_type(node);
		return new_node_num(node->ty->size, tok);
	}
	if (tok->kind == TK_IDENT)
	{
		// Function call
		if (isequal(tok->next, "("))
			return (funcall(rest, tok));
		// Variable
		*rest = tok->next;
		return (new_node_lvar(tok));
	}
	error_tok(tok, "Invalid token.");
}
