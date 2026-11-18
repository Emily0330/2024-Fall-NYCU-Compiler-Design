#include "AST/UnaryOperator.hpp"

// TODO
UnaryOperatorNode::UnaryOperatorNode(const uint32_t line, const uint32_t col, EnumOpType op, struct _ExprNode* expr)
    : ExpressionNode{line, col}, op(op), expr(expr) {}

// TODO: You may use code snippets in AstDumper.cpp
void UnaryOperatorNode::print() {}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(expr->expr_type == EnumExprType::TYPE_BINARY_OP) expr->binary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_CONSTANT_VALUE) expr->constant_value_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_UNARY_OP) expr->unary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_VARIABLE_REFER) expr->variable_reference_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_FUNC_CALL) expr->function_call_node->accept(p_visitor);
}
