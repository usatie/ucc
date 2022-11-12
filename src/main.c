/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 16:52:42 by susami            #+#    #+#             */
/*   Updated: 2022/11/12 13:23:13 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ucc.h"

static char	*user_input;

// error.c
void	error(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void	error_at(const char *loc, const char *fmt, ...)
{
	va_list		ap;
	const int	pos = loc - user_input;

	va_start(ap, fmt);
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " "); // print spaces for pos
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// main
int	main(int argc, char *argv[])
{
	Token	*token;
	Node	*node;

	// argparse
	if (argc != 2)
	{
		fprintf(stderr, "Invalid number of args\n");
		return (1);
	}
	user_input = argv[1];

	// tokenize
	token = tokenize(user_input);
	printf("# tokenize finished.\n");

	// parse
	node = parse(token);
	printf("# parse finished.\n");

	// code gen
	codegen(node);
	printf("# codegen finished.\n");

	return (0);
}
