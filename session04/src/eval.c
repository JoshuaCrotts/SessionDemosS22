#include "eval.h"
#include "apply.h"
#include "lval.h"
#include "utils.h"

static lval *eval(mpc_ast_t *ast);
static lval *eval_number(mpc_ast_t *ast);
static lval *eval_symbol(mpc_ast_t *ast);
static lval *eval_application(mpc_ast_t *ast);

void eval_ast(const mpc_ast_t *ast) {
    for (int i = 0; i < ast->children_num; i++) {
        lval *val = eval(ast->children[i]);
        if (val != NULL) {
            lval_print(val);
            // We probably shouldn't delete the lval until later...
            lval_delete(val);
        }
    }
}

/**
 * Evaluates an abstract syntax tree node based on its tag.
 */
static lval *eval(mpc_ast_t *ast) {
    char *tag = ast->tag;
    if (strstr(tag, "number")) {
        return eval_number(ast);
    } else if (strstr(tag, "symbol")) {
        return eval_symbol(ast);
    } else if (strstr(tag, "application")) {
        return eval_application(ast);
    }

    return NULL;
}

/**
 * Evaluates a number. If the parse fails, we throw an error. 
 */
static lval *eval_number(mpc_ast_t *ast) {
    long double number;
    int ret = sscanf(ast->contents, "%Le", &number);
    if (ret == 0 || ret == EOF) {
        fprintf(stderr, "Failed to scan number %s\n", ast->contents);
        exit(1);
    }
    return lval_init_number(number);
}

/**
 * Evaluates a symbol by converting it into its corresponding lvalue.
 */
static lval *eval_symbol(mpc_ast_t *ast) {
    return lval_init_symbol(ast->contents);
}

/**
 * Evaluates an application. An application is (op ...) followed by operands
 * where ... is.
 */
static lval *eval_application(mpc_ast_t *ast) {
    // Evaluate the arguments first.
    size_t num_args = ast->children_num - 3;
    lval **evaluated_args = s_malloc(sizeof(lval *) * num_args);
    for (int i = 0; i < num_args; i++) {
        mpc_ast_t *curr = ast->children[i + 2];
        evaluated_args[i] = eval(curr);
    }

    // Evaluate the operator.
    lval *op = eval(ast->children[1]);

    // Apply the operator to its arguments.
    lval *ret_val = apply(op, evaluated_args, num_args);

    // Free the evaluated arguments.
    for (int i = 0; i < num_args; i++) {
        lval_delete(evaluated_args[i]);
    }

    free(evaluated_args);
    lval_delete(op);
    return ret_val;
}