#include "AST/ConstantValue.hpp"
#include "AST/ast.hpp"

// TODO
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col, Variable *_constant_value)
    : ExpressionNode{line, col}, constant_value(_constant_value) {}

// TODO: You may use code snippets in AstDumper.cpp
void ConstantValueNode::print() {}

void ConstantValueNode::visitChildNodes(AstNodeVisitor &p_visitor){
    return; // constant value is a leaf
}