/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ucc.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 16:52:42 by susami            #+#    #+#             */
/*   Updated: 2022/11/09 14:31:39 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucc.h"

Token	*token;
char	*user_input;

// error.c
void	error_at(char *loc, char *fmt, ...)
{
	va_list		ap;
	const int	pos = loc - user_input;

	printf("ERROR!\n");
	va_start(ap, fmt);
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " "); // print spaces for pos
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// tokenizer.c
bool	consume(char op)
{
	if (token->kind != TK_RESERVED || token->str[0] != op)
		return (false);
	token = token->next;
	return (true);
}

void	expect(char op)
{
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error_at(token->str, "expected '%c', but not.", op);
	token = token->next;
}

int	expect_number(void)
{
	const int	val = token->val;

	if (token->kind != TK_NUM)
		error_at(token->str, "expected number, but not.");
	token = token->next;
	return (val);
}

bool	at_eof(void)
{
	return (token->kind == TK_EOF);
}

Token	*new_token(TokenKind kind, Token *cur, char *str)
{
	Token	*tok;

	tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return (tok);
}

Token	*tokenize(char *p)
{
	Token	head;
	Token	*cur;

	head.next = NULL;
	cur = &head;
	while (*p)
	{
		if (isspace(*p))
		{
			p++;
			continue ;
		}
		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
		{
			cur = new_token(TK_RESERVED, cur, p++);
			continue ;
		}
		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue ;
		}
		error_at(p, "Cannot tokenize.");
	}
	new_token(TK_EOF, cur, p);
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
	Node	*node;

	node = mul();
	while (1)
	{
		if (consume('+'))
			node = new_node(ND_ADD, node, mul());
		else if (consume('-'))
			node = new_node(ND_SUB, node, mul());
		else
			return (node);
	}
}

Node	*mul(void)
{
	Node	*node;

	node = primary();
	while (1)
	{
		if (consume('*'))
			node = new_node(ND_MUL, node, primary());
		else if (consume('/'))
			node = new_node(ND_DIV, node, primary());
		else
			return (node);
	}
}

Node	*primary(void)
{
	Node	*node;

	if (consume('('))
	{
		node = expr();
		expect(')');
		return (node);
	}
	node = new_node_num(expect_number());
	return (node);
}

// codegen.c
void	gen(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("  push %d\n", node->val);
		return ;
	}
	gen(node->lhs);
	gen(node->rhs);
	printf("  pop rdi\n");
	printf("  pop rax\n");
	if (node->kind == ND_ADD)
		printf("  add rax, rdi\n");
	else if (node->kind == ND_SUB)
		printf("  sub rax, rdi\n");
	else if (node->kind == ND_MUL)
		printf("  imul rax, rdi\n");
	else if (node->kind == ND_DIV)
	{
		printf("  cqo\n");
		printf("  idiv rdi\n");
	}
	printf("  push rax\n");
}

// main
int	main(int argc, char *argv[])
{
	Node	*node;

	if (argc != 2)
	{
		fprintf(stderr, "Invalid number of args\n");
		return (1);
	}

	user_input = argv[1];
	// tokenize
	token = tokenize(argv[1]);

	// parse
	node = expr();

	// code gen first part
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// code gen last part
	gen(node);

	// Pop stack top to RAX to make it return value.
	printf("  pop rax\n");
	printf("  ret\n");
	return (0);
}
