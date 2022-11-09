/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ucc.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 17:33:50 by susami            #+#    #+#             */
/*   Updated: 2022/11/09 14:34:04 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UCC_H
# define UCC_H

typedef enum {
	TK_RESERVED,
	TK_NUM,
	TK_EOF,
}	TokenKind;

typedef struct Token	Token;
struct Token {
	TokenKind	kind;
	Token		*next;
	int			val;
	char		*str;
};

typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // integer
}	NodeKind;

typedef struct Node		Node;
struct Node {
	NodeKind	kind;
	Node		*lhs;
	Node		*rhs;
	int			val;
};

extern Token			*token;
extern char				*user_input;

// parser.c
Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);
Node	*new_node_num(int val);
Node	*expr(void);
Node	*mul(void);
Node	*primary(void);
#endif