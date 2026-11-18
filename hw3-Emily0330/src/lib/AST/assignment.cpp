#include "AST/assignment.hpp"

// TODO
AssignmentNode::AssignmentNode(const uint32_t line, const uint32_t col, VariableReferenceNode* variable_reference, struct _ExprNode* expr)
    : AstNode{line, col}, variable_reference(variable_reference), expr(expr) {}

// TODO: You may use code snippets in AstDumper.cpp
void AssignmentNode::print() {}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    
    // variable reference
    variable_reference->accept(p_visitor);
    
    // expression
    if(expr->expr_type == EnumExprType::TYPE_CONSTANT_VALUE) expr->constant_value_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_BINARY_OP) expr->binary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_UNARY_OP) expr->unary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_VARIABLE_REFER) expr->variable_reference_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_FUNC_CALL) expr->function_call_node->accept(p_visitor);
}
