/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 16:52:42 by susami            #+#    #+#             */
/*   Updated: 2022/12/01 22:26:27 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ucc.h"

context	ctx;

// error.c
void	error(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

static void	verror_at(const char *loc, const char *fmt, va_list ap) \
	__attribute__((noreturn));

static void	verror_at(const char *loc, const char *fmt, va_list ap)
{
	const int	pos = loc - ctx.user_input;

	fprintf(stderr, "%s\n", ctx.user_input);
	fprintf(stderr, "%*s", pos, ""); // print pos spaces.
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void	error_at(const char *loc, const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	verror_at(loc, fmt, ap);
}

void	error_tok(const Token *tok, const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	verror_at(tok->str, fmt, ap);
}

// main
int	main(int argc, char *argv[])
{
	Token		*token;
	Function	*func;

	// argparse
	if (argc != 2)
	{
		fprintf(stderr, "Invalid number of args\n");
		return (1);
	}
	ctx.user_input = argv[1];

	// tokenize
	token = tokenize(ctx.user_input);
	printf("# tokenize finished.\n");

	// parse
	func = parse(token);
	printf("# parse finished.\n");

	// code gen
	codegen(func);
	printf("# codegen finished.\n");

	return (0);
}
