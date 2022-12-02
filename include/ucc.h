/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ucc.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: susami <susami@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 17:33:50 by susami            #+#    #+#             */
/*   Updated: 2022/12/02 21:22:11 by susami           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UCC_H
# define UCC_H

typedef enum {
	TK_PUNCT, // Punctuators
	TK_KEYWORD, // Keywords
	TK_IDENT, // Identifiers
	TK_NUM, // Numeric literals
	TK_EOF, // End-of-file markers
}	TokenKind;

typedef struct Token	Token;
struct Token {
	TokenKind	kind;
	Token		*next;
	int			val;
	char		*str;
	int			len;
};

typedef struct Type		Type;
struct Type {
	typedef enum {INT, PTR} type_kind;
	type_kind	ty;
	Type		*ptr_to;
};

typedef struct LVar		LVar;
struct LVar {
	LVar	*next;
	char	*name;
	Type	*type;
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
	ND_WHILE_STMT, // while
	ND_FOR_STMT, // for
	ND_BLOCK, // {...}
	ND_FUNC_CALL, // func()
	ND_FUNC_DECL, // func() {}
	ND_ADDR, // &
	ND_DEREF, // *
}	NodeKind;

typedef struct Node		Node;
struct Node {
	NodeKind	kind;
	Node		*next; // Next node (for ND_*_STMT)

	Node		*lhs;
	Node		*rhs;

	// "if", "while" or "for" statement
	Node		*init;
	Node		*inc;
	Node		*cond;
	Node		*then;
	Node		*els;

	// Block
	Node		*body;

	// Function call or decl
	char		*funcname;
	Node		*args;

	int			val; // only for ND_NUM
	LVar		*lvar; // only for ND_LVAR
};

typedef struct Function	Function;
struct Function {
	Function	*next;

	char		*name;
	Type		*type;
	LVar		*args;

	Node		*body;
	LVar		*locals;
	int			stack_size;
};

typedef struct context	context;
struct context {
	char	*user_input;
	LVar	*lvars;
};

extern context			ctx;
extern Type				*ty_int;

// error.c
void		error(const char *fmt, ...) __attribute__((noreturn));
void		error_at(const char *loc, const char *fmt, ...)\
				__attribute__((noreturn));
void		error_tok(const Token *tok, const char *fmt, ...)\
				__attribute__((noreturn));

// tokenize.c
Token		*tokenize(char *p);

// type.c
Type		*ptr_to(Type *type);

// parser.c
// syntax parser
Function	*parse(Token *tok);
Function	*funcdecl(Token **rest, Token *tok);
Node		*block(Token **rest, Token *tok);
Node		*stmt(Token **rest, Token *tok);
Node		*expr_stmt(Token **rest, Token *tok);
Node		*expr(Token **rest, Token *tok);
Node		*assign(Token **rest, Token *tok);
Node		*equality(Token **rest, Token *tok);
Node		*relational(Token **rest, Token *tok);
Node		*add(Token **rest, Token *tok);
Node		*mul(Token **rest, Token *tok);
Node		*unary(Token **rest, Token *tok);
Node		*primary(Token **rest, Token *tok);
Node		*lvar(Token **rest, Token *tok);

// codegen.c
void		codegen(Function *func);
#endif
