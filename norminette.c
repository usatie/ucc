/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   norminette.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 17:16:01 by susami            #+#    #+#             */
/*   Updated: 2022/11/12 13:16:51 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>

int	main(int argc, char *argv[])
{
	int		i;
	char	*cmd;

	i = 1;
	cmd = "norminette";
	while (i < argc)
	{
		asprintf(&cmd, "%s %s", cmd, argv[i]);
		i++;
	}
	asprintf(&cmd, "%s | grep -v -E "
		"'"
		"WRONG_SCOPE_COMMENT"
		"|EMPTY_LINE_FUNCTION"
		"|TOO_MANY_FUNCS"
		"|FORBIDDEN_CHAR_NAME"
		"|GLOBAL_VAR_NAMING"
		"|GLOBAL_VAR_DETECTED"
		"|STRUCT_TYPE_NAMING"
		"|USER_DEFINED_TYPEDEF"
		"|MISSING_TYPEDEF_ID"
		"|TOO_MANY_LINES"
		"|MULT_ASSIGN_LINE"
		"'",
		cmd);
	system(cmd);
	return (0);
}
