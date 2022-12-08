/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   type.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/02 06:52:32 by susami            #+#    #+#             */
/*   Updated: 2022/12/08 15:49:34 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "ucc.h"

Type	*ty_int = &(Type){TY_INT, NULL, 4, 0};

bool	is_integer(Type *ty)
{
	return (ty->kind == TY_INT);
}

void	add_type(Node *node)
{
	if (node == NULL || node->ty != NULL)
		return ;
	add_type(node->lhs);
	add_type(node->rhs);
	add_type(node->cond);
	add_type(node->then);
	add_type(node->els);
	add_type(node->init);
	add_type(node->inc);
	for (Node *n = node->body; n; n = n->next)
		add_type(n);
	for (Node *n = node->args; n; n = n->next)
		add_type(n);
	switch (node->kind)
	{
	case ND_ADD:
	case ND_SUB:
	case ND_MUL:
	case ND_DIV:
	case ND_NEG:
		node->ty = node->lhs->ty;
		return ;
	case ND_ASSIGN:
		if (node->lhs->ty->kind == TY_ARRAY)
			error_tok(node->lhs->tok, "not an lvalue");
		node->ty = node->lhs->ty;
		return ;
	case ND_EQ:
	case ND_NEQ:
	case ND_LT:
	case ND_LTE:
	case ND_GT:
	case ND_GTE:
	case ND_NUM:
		node->ty = ty_int;
		return ;
	case ND_LVAR:
		node->ty = node->lvar->type;
		return ;
	case ND_ADDR:
		if (node->lhs->ty->kind == TY_ARRAY)
			node->ty = ptr_to(node->lhs->ty->ptr_to);
		else
			node->ty = ptr_to(node->lhs->ty);
		return ;
	case ND_DEREF:
		if (node->lhs->ty->ptr_to == NULL)
			error_tok(node->lhs->tok, "invalid pointer dereference");
		node->ty = node->lhs->ty->ptr_to;
		return ;
	case ND_FUNC_CALL:
		node->ty = ty_int;
		return ;
	case ND_RETURN_STMT:
		node->ty = node->lhs->ty;
		return ;
	case ND_IF_STMT:
	case ND_FOR_STMT:
	case ND_EXPR_STMT:
	case ND_WHILE_STMT:
	case ND_FUNC_DECL:
	case ND_BLOCK:
		return ;
	}
}

Type	*ptr_to(Type *type)
{
	Type	*new_type;

	new_type = calloc(sizeof(Type), 1);
	new_type->kind = TY_PTR;
	new_type->ptr_to = type;
	new_type->size = 8;
	return (new_type);
}

Type	*ary_of(Type *type, size_t array_size)
{
	Type	*new_type;

	new_type = calloc(sizeof(Type), 1);
	new_type->kind = TY_ARRAY;
	new_type->ptr_to = type;
	new_type->size = type->size * array_size;
	new_type->array_size = array_size;
	return (new_type);
}
