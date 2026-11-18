#ifndef __AST_EXPR_NODE_H
#define __AST_EXPR_NODE_H

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include "AST/BinaryOperator.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/VariableReference.hpp"
#include "AST/FunctionInvocation.hpp"

typedef struct _ExprNode{
    EnumExprType expr_type;

    BinaryOperatorNode* binary_op_node;
    UnaryOperatorNode* unary_op_node;
    ConstantValueNode* constant_value_node;
    VariableReferenceNode* variable_reference_node;
    FunctionInvocationNode* function_call_node;
}ExprNode;

#endif