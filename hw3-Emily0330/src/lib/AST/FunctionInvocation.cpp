#include "AST/FunctionInvocation.hpp"

// TODO
FunctionInvocationNode::FunctionInvocationNode(const uint32_t line, const uint32_t col, std::string function_name, std::vector<ExprNode*> expr_list)
    : ExpressionNode{line, col}, function_name(function_name), expr_list(expr_list) {}

// TODO: You may use code snippets in AstDumper.cpp
void FunctionInvocationNode::print() {}

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    for(auto& expr_node_ptr: expr_list){ // expression children nodes
        if(expr_node_ptr->expr_type == EnumExprType::TYPE_CONSTANT_VALUE) expr_node_ptr->constant_value_node->accept(p_visitor);
        else if(expr_node_ptr->expr_type == EnumExprType::TYPE_BINARY_OP) expr_node_ptr->binary_op_node->accept(p_visitor);
        else if(expr_node_ptr->expr_type == EnumExprType::TYPE_UNARY_OP) expr_node_ptr->unary_op_node->accept(p_visitor);
        else if(expr_node_ptr->expr_type == EnumExprType::TYPE_VARIABLE_REFER) expr_node_ptr->variable_reference_node->accept(p_visitor);
        else if(expr_node_ptr->expr_type == EnumExprType::TYPE_FUNC_CALL) expr_node_ptr->function_call_node->accept(p_visitor);
    }
}
