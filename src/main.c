/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ucc.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 16:52:42 by susami            #+#    #+#             */
/*   Updated: 2022/11/10 12:03:25 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ucc.h"

context	*ctx;

// error.c
void	error_at(char *loc, char *fmt, ...)
{
	va_list		ap;
	const int	pos = loc - ctx->user_input;

	printf("ERROR!\n");
	va_start(ap, fmt);
	fprintf(stderr, "%s\n", ctx->user_input);
	fprintf(stderr, "%*s", pos, " "); // print spaces for pos
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
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
	ctx = calloc(1, sizeof(context));

	ctx->user_input = argv[1];
	// tokenize
	ctx->token = tokenize(argv[1]);

	// parse
	node = expr();

	// code gen first part
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// code gen last part
	gen(node);

	// Pop stack top to RAX to make it return value.
	printf("# return from main\n");
	printf("  pop rax\n");
	printf("  ret\n");
	return (0);
}
