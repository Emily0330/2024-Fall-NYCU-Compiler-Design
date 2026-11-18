#include "AST/for.hpp"

// TODO
ForNode::ForNode(const uint32_t line, const uint32_t col, DeclNode* decl, AssignmentNode* assignment, struct _ExprNode* expr, CompoundStatementNode* compound)
    : AstNode{line, col}, decl(decl), assignment(assignment), expr(expr), compound(compound) {}

// TODO: You may use code snippets in AstDumper.cpp
void ForNode::print() {}

void ForNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    decl->accept(p_visitor);
    assignment->accept(p_visitor);
    
    // an expression
    if(expr->expr_type == EnumExprType::TYPE_BINARY_OP) expr->binary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_CONSTANT_VALUE) expr->constant_value_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_UNARY_OP) expr->unary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_VARIABLE_REFER) expr->variable_reference_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_FUNC_CALL) expr->function_call_node->accept(p_visitor);

    compound->accept(p_visitor);
}
