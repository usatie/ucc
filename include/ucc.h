/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ucc.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 17:33:50 by susami            #+#    #+#             */
/*   Updated: 2022/11/12 16:17:42 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UCC_H
# define UCC_H

typedef enum {
	TK_RESERVED,
	TK_KEYWORD,
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

typedef struct LVar		LVar;
struct LVar {
	LVar	*next;
	char	*name;
	int		len; // length of the name
	int		offset; // offset from rbp
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
	ND_ASSIGN, // = assignment
	ND_LVAR, // local variable
	ND_EXPR_STMT, // expression statement
	ND_RETURN_STMT, // return
	ND_IF_STMT, // if
}	NodeKind;

typedef struct Node		Node;
struct Node {
	NodeKind	kind;
	Node		*next; // Next node (for ND_*_STMT)

	Node		*lhs;
	Node		*rhs;

	// "if" statement
	Node		*cond;
	Node		*then;
	Node		*els;

	int			val; // only for ND_NUM
	LVar		*lvar; // only for ND_LVAR
};

typedef struct context	context;
struct context {
	char	*user_input;
	LVar	*lvars;
};

extern context			ctx;

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
Node	*assign(Token **rest, Token *tok);
Node	*equality(Token **rest, Token *tok);
Node	*relational(Token **rest, Token *tok);
Node	*add(Token **rest, Token *tok);
Node	*mul(Token **rest, Token *tok);
Node	*unary(Token **rest, Token *tok);
Node	*primary(Token **rest, Token *tok);

// codegen.c
void	codegen(Node *node);
#endif
