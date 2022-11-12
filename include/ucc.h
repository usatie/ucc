/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ucc.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 17:33:50 by susami            #+#    #+#             */
/*   Updated: 2022/11/12 11:27:35 by susami           ###   ########.fr       */
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
	ND_STMT, // statement
}	NodeKind;

typedef struct Node		Node;
struct Node {
	NodeKind	kind;
	Node		*next; // only for ND_STMT
	Node		*lhs;
	Node		*rhs;
	int			val; // only for ND_NUM
};

typedef struct context	context;
struct context {
	Node	*node;
	Token	*token;
};

// error.c
void	error(const char *fmt, ...) __attribute__((noreturn));
void	error_at(const char *loc, const char *fmt, ...)\
			__attribute__((noreturn));

// tokenize.c
Token	*tokenize(char *p);
// parser.c
// syntax parser
Node	*parse(Token *tok);
Node	*stmt(Token **rest, Token *tok);
Node	*expr(Token **rest, Token *tok);
Node	*equality(Token **rest, Token *tok);
Node	*relational(Token **rest, Token *tok);
Node	*add(Token **rest, Token *tok);
Node	*mul(Token **rest, Token *tok);
Node	*unary(Token **rest, Token *tok);
Node	*primary(Token **rest, Token *tok);

// codegen.c
void	codegen(Node *node);
#endif
