/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codegen.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:32:05 by susami            #+#    #+#             */
/*   Updated: 2022/11/12 15:03:19 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "ucc.h"

static void	gen_stmt(Node *node);
static void	gen_expr(Node *node);
static int	stack_size(void);

// codegen.c
void	codegen(Node *node)
{
	// code gen first part
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// prologue
	// Allocate local variables
	printf("# Prologue\n");
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, %d\n", stack_size());
	/*
	  If needed to zero initialize all local variables
	  This is also ok?
	for (int i = 0; i < stack_size() / 8; i++)
	{
		printf("  push 0\n");
	}
	*/

	printf("# Program\n");
	while (node)
	{
		gen_stmt(node);
		// Evaluated result of the code is on the top of stack.
		// Pop stack top to RAX to make it return value,
		// and clean up the top of the stack.
		printf("  pop rax\n");
		node = node->next;
	}

	// Epilogue
	// The last result is on rax
	printf("# Epilogue\n");
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
}

static void	gen_stmt(Node *node)
{
	if (node->kind == ND_RETURN_STMT)
	{
		gen_expr(node->lhs);
		printf("  pop rax\n");
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
		return ;
	}
	if (node->kind == ND_EXPR_STMT)
	{
		gen_expr(node->lhs);
		return ;
	}
	error("Invalid kind");
}

static void	gen_lval(Node *node)
{
	int	offset;

	if (node->kind == ND_LVAR)
	{
		offset = node->lvar->offset;
		printf("  mov rax, rbp\n");
		printf("  sub rax, %d\n", offset);
		printf("  push rax\n");
		return ;
	}
	error("Expected local variable.");
}

static void	gen_expr(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("  push %d\n", node->val);
		return ;
	}
	else if (node->kind == ND_LVAR)
	{
		gen_lval(node);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return ;
	}
	else if (node->kind == ND_ASSIGN)
	{
		gen_lval(node->lhs);
		gen_expr(node->rhs);
		printf("  pop rdi # rvalue\n");
		printf("  pop rax # lvalue\n");
		printf("  mov [rax], rdi # lvalue = rvalue\n");
		printf("  push rdi\n");
		return ;
	}
	//printf("# gen(%s)\n", stringize(node));
	gen_expr(node->lhs);
	gen_expr(node->rhs);
	printf("  pop rdi\n");
	printf("  pop rax\n");
	if (node->kind == ND_ADD)
	{
		printf("# ADD\n");
		printf("  add rax, rdi\n");
	}
	else if (node->kind == ND_SUB)
	{
		printf("# SUB\n");
		printf("  sub rax, rdi\n");
	}
	else if (node->kind == ND_MUL)
	{
		printf("# MUL\n");
		printf("  imul rax, rdi\n");
	}
	else if (node->kind == ND_DIV)
	{
		printf("# DIV\n");
		printf("  cqo\n");
		printf("  idiv rdi\n");
	}
	else if (node->kind == ND_EQ)
	{
		printf("# EQ\n");
		printf("  cmp rax, rdi\n");
		printf("  sete al\n");
		printf("  movzb rax, al\n");
	}
	else if (node->kind == ND_NEQ)
	{
		printf("# NEQ\n");
		printf("  cmp rax, rdi\n");
		printf("  setne al\n");
		printf("  movzb rax, al\n");
	}
	else if (node->kind == ND_LT)
	{
		printf("# LT\n");
		printf("  cmp rax, rdi\n");
		printf("  setl al\n");
		printf("  movzb rax, al\n");
	}
	else if (node->kind == ND_LTE)
	{
		printf("# LTE\n");
		printf("  cmp rax, rdi\n");
		printf("  setle al\n");
		printf("  movzb rax, al\n");
	}
	else if (node->kind == ND_GT)
	{
		printf("# GT\n");
		printf("  cmp rax, rdi\n");
		printf("  setg al\n");
		printf("  movzb rax, al\n");
	}
	else if (node->kind == ND_GTE)
	{
		printf("# GTE\n");
		printf("  cmp rax, rdi\n");
		printf("  setge al\n");
		printf("  movzb rax, al\n");
	}
	else
		error("Invalid node\n");
	printf("  push rax\n");
}

static int	stack_size(void)
{
	if (ctx.lvars == NULL)
		return (0);
	return (ctx.lvars->offset);
}
