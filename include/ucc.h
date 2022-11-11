/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ucc.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 17:33:50 by susami            #+#    #+#             */
/*   Updated: 2022/11/11 10:05:48 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UCC_H
# define UCC_H

typedef enum {
	TK_RESERVED,
	TK_IDENT,
	TK_NUM,
	TK_EOF,
}	TokenKind;

typedef struct Token	Token;
struct Token {
	TokenKind	kind;
	Token		*next;
	int			val;
	char		*str;
	int			len;
};

typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // integer
	ND_EQ, // ==
	ND_NEQ, // !=
	ND_LT, // <
	ND_LTE, // <=
	ND_GT, // >
	ND_GTE, // >=
}	NodeKind;

typedef struct Node		Node;
struct Node {
	NodeKind	kind;
	Node		*lhs;
	Node		*rhs;
	int			val;
};

typedef struct context	context;
struct context {
	Node	*code[100];
	Token	*token;
	char	*user_input;
};

extern context			*ctx;

// error.c
void	error_at(char *loc, char *fmt, ...);

// tokenize.c
Token	*tokenize(char *p);
// parser.c
Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);
Node	*new_node_num(int val);

// syntax parser
void	program(void);
Node	*stmt(void);
Node	*expr(void);
Node	*equality(void);
Node	*relational(void);
Node	*add(void);
Node	*mul(void);
Node	*unary(void);
Node	*primary(void);

// codegen.c
void	gen(Node *node);
#endif
