#include "AST/variable.hpp"

// TODO
VariableNode::VariableNode(const uint32_t line, const uint32_t col, const char *const  name, TypeInfo* type, ConstantValueNode* constant_value_node)
    : AstNode{line, col}, name(name), type(type), constant_value_node(constant_value_node) {}

// TODO: You may use code snippets in AstDumper.cpp
void VariableNode::print() {}

void VariableNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO

    if(constant_value_node != nullptr){
        constant_value_node->accept(p_visitor);
    }

}
