/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codegen.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:32:05 by susami            #+#    #+#             */
/*   Updated: 2022/11/11 10:45:13 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "ucc.h"

// codegen.c
void	codegen(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("  push %d\n", node->val);
		return ;
	}
	//printf("# gen(%s)\n", stringize(node));
	codegen(node->lhs);
	codegen(node->rhs);
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
