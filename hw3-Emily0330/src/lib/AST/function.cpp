#include "AST/function.hpp"

// TODO
FunctionNode::FunctionNode(const uint32_t line, const uint32_t col, std::string name, 
                  std::vector<DeclNode*> decl_list, TypeInfo* return_type, 
                  CompoundStatementNode* body)
    : AstNode{line, col}, name(name), decl_list(decl_list), return_type(return_type), body(body) {}

// TODO: You may use code snippets in AstDumper.cpp
void FunctionNode::print() {}

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    
    for(auto& decl_node_ptr: decl_list){
        decl_node_ptr->accept(p_visitor);
    }

    // optional compound statement node
    if(body != nullptr){
        body->accept(p_visitor);
    }
}
