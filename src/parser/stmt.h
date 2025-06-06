#ifndef __NODE_H
#define __NODE_H

#include "../utils/str.h"

// NOTE TO MYSELF: ADD STMT_EXPR FOR EXPRESSIONS DONT DO ANYTHING BUT JUST EXISTS
typedef enum {
    STMT_COMPOUND, // Block
    STMT_DECL,
    STMT_EXPR,
    STMT_WHILE,
    STMT_IF,
    STMT_FUNCTION_DECL,
    STMT_RETURN,
    STMT_BREAK,
    STMT_CONTINUE,
} StmtKind;

typedef enum {
    BIN_ASSIGN,
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
    BIN_MOD,

    BIN_EQ,
    BIN_NEQ,
    BIN_LT,
    BIN_LTE,
    BIN_GT,
    BIN_GTE,

    BIN_AND,
    BIN_OR,

    BIN_BIT_AND,
    BIN_BIT_OR,
    BIN_BIT_XOR,
    BIN_BIT_NOT,
    BIN_BIT_LSHIFT,
    BIN_BIT_RSHIFT,
} BinaryOp;

typedef enum {
    UNARY_LOGICAL_NOT,
    UNARY_BIT_NOT,
    UNARY_POSITIVE,
    UNARY_NEGATIVE,

    UNARY_PRE_INC,
    UNARY_PRE_DEC,
    UNARY_POST_INC,
    UNARY_POST_DEC,
} UnaryOp;

typedef enum {
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_CALL,
} ExprKind;

typedef struct Expr {
    ExprKind kind;

    union {
        struct LiteralExpr* literal;
        struct VariableExpr* variable;
        struct UnaryExpr* unary;
        struct BinaryExpr* binary;
        struct CallExpr* call;
    };

    int line;
    int col;
} Expr;

typedef enum LiteralType {
    LITERAL_STRING,
    LITERAL_FLOAT,
    LITERAL_INT,
    LITERAL_NONE,
} LiteralType;

typedef struct LiteralExpr {
    LiteralType type;
    StringView value;
} LiteralExpr;

typedef struct VariableExpr {
    StringView name;
} VariableExpr;

typedef struct BinaryExpr {
    BinaryOp op;
    Expr* lhs;
    Expr* rhs;
} BinaryExpr;

typedef struct CallExpr {
    StringView name; // Name of calle
    Expr** args; // Darray
} CallExpr;

typedef struct UnaryExpr {
    UnaryOp op;
    Expr* operand;
} UnaryExpr;

typedef struct Stmt {
    StmtKind kind;

    union {
        struct CompoundStmt* compound;
        struct DeclStmt* decl_stmt;
        struct Expr* expr_stmt;
        struct WhileStmt* while_stmt;
        struct IfStmt* if_stmt;
        struct FunctionDeclStmt* function_decl;
        struct ReturnStmt* return_stmt;
        void* dummy; // For statements like break or continue
    };

    int line;
    int col;
} Stmt;

typedef struct CompoundStmt {
    Stmt** stmts; // Darray for statements
} CompoundStmt;

typedef struct DeclStmt {
    StringView name;
    Expr* initializer;
    int is_const;
} DeclStmt;

typedef struct WhileStmt {
    Expr* condition;
    Stmt** body; // Darray
} WhileStmt;

typedef struct IfStmt {
    Expr* condition;
    Stmt** then_branch; // Darray
    Stmt* else_branch;
} IfStmt;

typedef struct FunctionDeclStmt {
    StringView name;
    Expr** params;
    Stmt** body;
} FunctionDeclStmt;

typedef struct ReturnStmt {
    Expr* value;
} ReturnStmt;

Stmt* stmt_new(StmtKind kind, int line, int col);
Stmt* stmt_new_compound(Stmt** stmts, int line, int col);
Stmt* stmt_new_decl(StringView name, Expr* initializer, int is_const, int line, int col);
Stmt* stmt_new_expr(Expr* expr, int line, int col);
Stmt* stmt_new_while(Expr* condition, Stmt** body, int line, int col);
Stmt* stmt_new_if(Expr* condition, Stmt** then_branch, Stmt* else_branch, int line, int col);
Stmt* stmt_new_function_decl(StringView name, Expr** args, Stmt** body, int line, int col);
Stmt* stmt_new_return(Expr* value, int line, int col);
Stmt* stmt_new_break(int line, int col);
Stmt* stmt_new_continue(int line, int col);
void stmt_free(Stmt* s);
void stmt_dump(Stmt* s);

Expr* expr_new(ExprKind kind, int line, int col);
Expr* expr_new_literal(LiteralType type, StringView value, int line, int col);
Expr* expr_new_variable(StringView name, int line, int col);
Expr* expr_new_binary(BinaryOp op, Expr* lhs, Expr* rhs, int line, int col);
Expr* expr_new_call(StringView name, Expr** args, int line, int col);
Expr* expr_new_unary(UnaryOp op, Expr* operand, int line, int col);
void expr_free(Expr* e);
void expr_dump(Expr* e);

#endif