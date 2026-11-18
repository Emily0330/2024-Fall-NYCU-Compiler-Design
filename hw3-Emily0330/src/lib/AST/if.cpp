#include "AST/if.hpp"

// TODO
IfNode::IfNode(const uint32_t line, const uint32_t col, ExprNode* expr, std::vector<CompoundStatementNode*> compound_list)
    : AstNode{line, col}, expr(expr), compound_list(compound_list) {}

// TODO: You may use code snippets in AstDumper.cpp
void IfNode::print() {}

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    // an expression
    if(expr->expr_type == EnumExprType::TYPE_BINARY_OP) expr->binary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_CONSTANT_VALUE) expr->constant_value_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_UNARY_OP) expr->unary_op_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_VARIABLE_REFER) expr->variable_reference_node->accept(p_visitor);
    else if(expr->expr_type == EnumExprType::TYPE_FUNC_CALL) expr->function_call_node->accept(p_visitor);

    // compound statements
    for(auto& compound_ptr: compound_list){
        compound_ptr->accept(p_visitor);
    }
}
