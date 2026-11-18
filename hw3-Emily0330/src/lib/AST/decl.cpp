#include "AST/decl.hpp"

// TODO
DeclNode::DeclNode(const uint32_t line, const uint32_t col, std::vector<VariableNode*> var_list)
    : AstNode{line, col}, var_list(var_list) {}

// TODO
// DeclNode::DeclNode(const uint32_t line, const uint32_t col)
//    : AstNode{line, col}{}

// TODO: You may use code snippets in AstDumper.cpp
void DeclNode::print() {}

void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    for(auto& var_ptr: var_list){ // check
        var_ptr->accept(p_visitor);
    }
    
}
