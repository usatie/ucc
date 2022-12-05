/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codegen.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:32:05 by susami            #+#    #+#             */
/*   Updated: 2022/12/05 23:18:12 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "ucc.h"

static void	gen_func(Function *func);
static void	gen_block(Node *node);
static void	gen_stmt(Node *node);
static void	gen_expr(Node *node);
static int	stack_size(void);
static char	*argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// codegen.c
void	codegen(Function *func)
{
	// code gen first part
	printf(".intel_syntax noprefix\n");
	while (func)
	{
		gen_func(func);
		func = func->next;
	}
}

static void	setup_args(Function *func)
{
	LVar	*arg;

	arg = func->args;
	while (arg)
	{
		printf("  mov rax, rbp\n");
		printf("  sub rax, %d\n", arg->offset);
		printf("  mov [rax], %s\n", argreg[(arg->offset / 8) - 1]);
		arg = arg->next;
	}
	/*
	  If needed to zero initialize all local variables
	  This is also ok?
	for (int i = 0; i < stack_size() / 8; i++)
	{
		printf("  push 0\n");
	}
	*/
}

static void	gen_func(Function *func)
{
	printf(".globl %s\n", func->name);
	printf("%s:\n", func->name);

	// prologue
	printf("# Prologue\n");
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	// Allocate local variables
	printf("# Allocate local variables\n");
	printf("  push rbp\n");
	printf("  sub rsp, %d\n", stack_size());
	// Setup args
	setup_args(func);

	printf("# Program\n");
	gen_block(func->body);

	// Epilogue
	// The last result is on rax
	printf("# Epilogue\n");
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
}

static void	gen_block(Node *node)
{
	while (node)
	{
		gen_stmt(node);
		node = node->next;
	}
}

static void	gen_stmt(Node *node)
{
	static int	label;
	const int	label_at_gen = label;

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
		// Evaluated result of the code is on the top of the stack.
		// Pop to RAX to make it return value,
		// and clean up the top of the stack.
		printf("  pop rax\n");
		return ;
	}
	if (node->kind == ND_IF_STMT)
	{
		label++;
		gen_expr(node->cond);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lelse%d\n", label_at_gen);
		gen_stmt(node->then);
		printf("  jmp .Lend%d\n", label_at_gen);
		printf(".Lelse%d:\n", label_at_gen);
		if (node->els)
			gen_stmt(node->els);
		printf(".Lend%d:\n", label_at_gen);
		return ;
	}
	if (node->kind == ND_WHILE_STMT)
	{
		label++;
		printf(".Lstart%d:\n", label_at_gen);
		gen_expr(node->cond);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", label_at_gen);
		gen_stmt(node->then);
		printf("  jmp .Lstart%d\n", label_at_gen);
		printf(".Lend%d:\n", label_at_gen);
		return ;
	}
	if (node->kind == ND_FOR_STMT)
	{
		label++;
		printf("# for.init\n");
		if (node->init)
		{
			gen_expr(node->init);
			printf("  pop rax\n");
		}
		printf(".Lstart%d:\n", label_at_gen);
		printf("# for.cond\n");
		if (node->cond)
		{
			gen_expr(node->cond);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je .Lend%d\n", label_at_gen);
		}
		printf("# for.then\n");
		gen_stmt(node->then);
		printf("# for.inc\n");
		if (node->inc)
		{
			gen_expr(node->inc);
			printf("  pop rax\n");
		}
		printf("  jmp .Lstart%d\n", label_at_gen);
		printf(".Lend%d:\n", label_at_gen);
		return ;
	}
	if (node->kind == ND_BLOCK)
	{
		printf("# code block start\n");
		if (node->body)
			gen_block(node->body);
		printf("# code block end\n");
		return ;
	}
	error_tok(node->tok, "Invalid kind");
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
	error_tok(node->tok, "Expected local variable.");
}

static void	gen_binary_expr(Node *node)
{
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
		error_tok(node->tok, "Invalid binary expression node\n");
	printf("  push rax\n");
}

static void	gen_funcall(Node *node)
{
	int		i;
	int		nargs;
	Node	*arg;

	nargs = 0;
	printf("# TODO: allign sp to 16\n");
	printf("# func call\n");
	arg = node->args;
	while (arg)
	{
		gen_expr(arg);
		nargs++;
		arg = arg->next;
	}
	i = nargs - 1;
	while (i >= 0)
	{
		printf("  pop %s\n", argreg[i]);
		i--;
	}
	printf("  call %s\n", node->funcname);
	printf("  push rax\n");

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
		printf("# local variable\n");
		gen_lval(node);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return ;
	}
	else if (node->kind == ND_ASSIGN)
	{
		printf("# assign start\n");
		printf("# assign lhs\n");
		if (node->lhs->kind == ND_DEREF)
			gen_expr(node->lhs->lhs);
		else
			gen_lval(node->lhs);
		printf("# assign rhs\n");
		gen_expr(node->rhs);
		printf("  pop rdi # rvalue\n");
		printf("  pop rax # lvalue\n");
		printf("  mov [rax], rdi # lvalue = rvalue\n");
		printf("  push rdi\n");
		return ;
	}
	else if (node->kind == ND_FUNC_CALL)
	{
		gen_funcall(node);
	}
	else if (node->kind == ND_ADDR)
		gen_lval(node->lhs);
	else if (node->kind == ND_DEREF)
	{
		gen_expr(node->lhs);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
	}
	else if (node->lhs && node->rhs)
		gen_binary_expr(node);
	else
		error_tok(node->tok, "Invalid expression node\n");
}

static int	stack_size(void)
{
	if (ctx.lvars == NULL)
		return (0);
	return (ctx.lvars->offset);
}
