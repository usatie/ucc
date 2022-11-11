/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codegen.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:32:05 by susami            #+#    #+#             */
/*   Updated: 2022/11/11 11:08:04 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "ucc.h"

static void	gen_stmt(Node *node);
static void	gen_expr(Node *node);

// codegen.c
void	codegen(Node *node)
{
	// code gen first part
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	while (node)
	{
		gen_stmt(node);
		// Evaluated result of the code is on the top of stack.
		printf("  pop rax\n");
		node = node->next;
	}

	// Pop stack top to RAX to make it return value.
	printf("# return from main\n");
	printf("  ret\n");
}

static void	gen_stmt(Node *node)
{
	if (node->kind == ND_STMT)
	{
		gen_expr(node->lhs);
		return ;
	}
	error_at(NULL, "Invalid kind");
}

static void	gen_expr(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("  push %d\n", node->val);
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
	printf("  push rax\n");
}
