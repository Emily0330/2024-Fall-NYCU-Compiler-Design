#include "AST/program.hpp"

#include <vector>
#include <iostream>
// TODO
ProgramNode::ProgramNode(const uint32_t line, const uint32_t col,
                         const char *const p_name, CompoundStatementNode *const p_body, std::vector<DeclNode *>* p_var_decls, std::vector<FunctionNode*> func_list)
    : AstNode{line, col}, name(p_name), m_body(p_body), var_decls(p_var_decls), func_list(func_list) {}

// visitor pattern version: const char *ProgramNode::getNameCString() const { return name.c_str(); }

void ProgramNode::print() {
    // TODO
    // outputIndentationSpace();

    std::printf("program <line: %u, col: %u> %s %s\n",
                location.line, location.col,
                name.c_str(), "void");

    // TODO
    // incrementIndentation();
    // visitChildNodes();
    // decrementIndentation();
}


void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor) { // visitor pattern version
    /* TODO
     *
     * for (auto &decl : var_decls) {
     *     decl->accept(p_visitor);
     * }
     *
     * // functions
     *
     * body->accept(p_visitor);
     */
    
    // declarations
    if(var_decls != nullptr){
        // std::cout << var_decls<< std::endl;
        for(auto &decl : *var_decls){
            decl->accept(p_visitor);
        }
    }
    

    // functions
    for(auto& func_ptr: func_list){
        func_ptr->accept(p_visitor);
    }
    // compound statements
    m_body->accept(p_visitor);
}
