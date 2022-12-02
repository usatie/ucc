/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   type.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/02 06:52:32 by susami            #+#    #+#             */
/*   Updated: 2022/12/02 21:12:23 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "ucc.h"

Type	*ty_int = &(Type){INT, NULL};

Type	*ptr_to(Type *type)
{
	Type	*new_type;

	new_type = calloc(sizeof(Type), 1);
	new_type->ty = PTR;
	new_type->ptr_to = type;
	return (new_type);
}
