#include "AST/print.hpp"
#include <iostream>

// TODO
PrintNode::PrintNode(const uint32_t line, const uint32_t col, struct _ExprNode* expr)
    : AstNode{line, col}, expr(expr) {}

// TODO: You may use code snippets in AstDumper.cpp
void PrintNode::print() {}

void PrintNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    // std::cout<<"expr->expr_type: "<< expr->expr_type<<'\n'; // test
    if(expr->expr_type == EnumExprType::TYPE_BINARY_OP) expr->binary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_CONSTANT_VALUE) expr->constant_value_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_UNARY_OP) expr->unary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_VARIABLE_REFER) expr->variable_reference_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_FUNC_CALL) expr->function_call_node->accept(p_visitor);
    // std::cout<<"PrintNode visitChildNodes successfully.\n"; // test
}
