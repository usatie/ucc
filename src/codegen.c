/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codegen.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/10 11:32:05 by susami            #+#    #+#             */
/*   Updated: 2022/12/08 15:45:33 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "ucc.h"

int			depth = 0;

static void	assign_lvar_offset(Function *prog);
static void	gen_func(Function *func);
static void	gen_block(Node *node);
static void	gen_stmt(Node *node);
static void	gen_expr(Node *node);
static char	*argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static char	*argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

// codegen.c
void	codegen(Function *func)
{
	// code gen first part
	printf(".intel_syntax noprefix\n");
	while (func)
	{
		printf("# %s\n", func->name);
		assign_lvar_offset(func);
		gen_func(func);
		func = func->next;
	}
}

static void	push(void)
{
	printf("  push rax\n");
	depth++;
}

static void	pop(char *arg)
{
	printf("  pop %s\n", arg);
	depth--;
}

static void	load(Type *ty)
{
	if (ty->kind == TY_ARRAY)
	{
		// If it is an array, do not attempt to load a value to the
		// register because in general we can't load an entire array to a
		// register. As a result, the result of an evaluation of an array
		// becomes not the array itself but the address of the array.
		// This is where "array is automatically converted to a pointer to
		// the first element of the array in C" occurs.
		return ;
	}
	if (ty->size == 4)
		printf("  movsxd rax, [rax]\n");
	else if (ty->size == 8)
		printf("  mov rax, [rax]\n");
}

static void	store(Type *ty)
{
	pop("rdi");
	if (ty->size == 4)
		printf("  mov [rdi], eax\n");
	else if (ty->size == 8)
		printf("  mov [rdi], rax\n");
}

static void	setup_args(Function *func)
{
	LVar	*arg;
	int		i;

	arg = func->args;
	i = 0;
	arg = func->args;
	while (i < func->nargs)
	{
		printf("  mov rax, rbp\n");
		printf("  sub rax, %d\n", arg->offset);
		if (arg->type->size == 8)
			printf("  mov [rax], %s\n", argreg64[i]);
		else if (arg->type->size == 4)
			printf("  mov [rax], %s\n", argreg32[i]);
		else
			error("Unknown type size");
		i++;
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

// Round up `n` to the nearest multiple of `align`. For instance,
// align_to(5, 8) returns 8 and align_to(11, 8) returns 16.
static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

static void	assign_lvar_offset(Function *prog)
{
	int	offset;

	offset = 0;
	for (LVar *var = prog->locals; var; var = var->next)
	{
		offset += var->type->size;
		var->offset = offset;
	}
	prog->stack_size = align_to(offset, 16);
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
	printf("  sub rsp, %d\n", func->stack_size);
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

static int	count(void)
{
	static int	i = 1;

	return (i++);
}

static void	gen_stmt(Node *node)
{
	const int	c = count();

	if (node->kind == ND_RETURN_STMT)
	{
		gen_expr(node->lhs);
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
	if (node->kind == ND_IF_STMT)
	{
		gen_expr(node->cond);
		printf("  cmp rax, 0\n");
		printf("  je .Lelse%d\n", c);
		gen_stmt(node->then);
		printf("  jmp .Lend%d\n", c);
		printf(".Lelse%d:\n", c);
		if (node->els)
			gen_stmt(node->els);
		printf(".Lend%d:\n", c);
		return ;
	}
	if (node->kind == ND_WHILE_STMT)
	{
		printf(".Lstart%d:\n", c);
		gen_expr(node->cond);
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", c);
		gen_stmt(node->then);
		printf("  jmp .Lstart%d\n", c);
		printf(".Lend%d:\n", c);
		return ;
	}
	if (node->kind == ND_FOR_STMT)
	{
		printf("# for.init\n");
		if (node->init)
			gen_expr(node->init);
		printf(".Lstart%d:\n", c);
		printf("# for.cond\n");
		if (node->cond)
		{
			gen_expr(node->cond);
			printf("  cmp rax, 0\n");
			printf("  je .Lend%d\n", c);
		}
		printf("# for.then\n");
		gen_stmt(node->then);
		printf("# for.inc\n");
		if (node->inc)
			gen_expr(node->inc);
		printf("  jmp .Lstart%d\n", c);
		printf(".Lend%d:\n", c);
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
		return ;
	}
	error_tok(node->tok, "Expected local variable.");
}

static void	gen_binary_expr(Node *node)
{
	//printf("# gen(%s)\n", stringize(node));
	gen_expr(node->lhs);
	push();
	gen_expr(node->rhs);
	printf("  mov rdi, rax\n");
	pop("rax");
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
}

static void	gen_funcall(Node *node)
{
	int		i;
	Node	*arg;

	i = 0;
	printf("# TODO: allign sp to 16\n");
	printf("# func call\n");
	arg = node->args;
	while (arg)
	{
		gen_expr(arg);
		printf("  mov %s, rax\n", argreg64[i]);
		i++;
		arg = arg->next;
	}
	printf("  call %s\n", node->funcname);

}

static void	gen_expr(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("  push %d\n", node->val);
		printf("  pop rax\n");
		return ;
	}
	if (node->kind == ND_NEG)
	{
		gen_expr(node->lhs);
		printf("  neg rax\n");
		return ;
	}
	else if (node->kind == ND_LVAR)
	{
		printf("# local variable\n");
		gen_lval(node);
		load(node->ty);
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
		push();
		printf("# assign rhs\n");
		gen_expr(node->rhs);
		store(node->ty);
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
		load(node->ty);
	}
	else if (node->lhs && node->rhs)
		gen_binary_expr(node);
	else
		error_tok(node->tok, "Invalid expression node\n");
}
