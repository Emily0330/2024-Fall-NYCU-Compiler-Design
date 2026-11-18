#include "AST/CompoundStatement.hpp"

// TODO
CompoundStatementNode::CompoundStatementNode(const uint32_t line, const uint32_t col,std::vector<DeclNode*> decl_list, std::vector<StatementNode*> stmt_list)
    : AstNode{line, col}, decl_list(decl_list), stmt_list(stmt_list) {}

// TODO: You may use code snippets in AstDumper.cpp
void CompoundStatementNode::print() {}

void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    
    // declaration nodes
    for(auto& decl_node_ptr: decl_list){
        decl_node_ptr->accept(p_visitor);
    }
    // statement nodes
    for(auto& stmt_node_ptr: stmt_list){
        if(stmt_node_ptr->stmt_type == TYPE_COMPOUND){
            stmt_node_ptr->compound_node->accept(p_visitor);
        }
        else if(stmt_node_ptr->stmt_type == TYPE_PRINT){
            stmt_node_ptr->print_node->accept(p_visitor);
        }
        else if(stmt_node_ptr->stmt_type == TYPE_ASSIGNMENT){
            stmt_node_ptr->assignment_node->accept(p_visitor);
        }
        else if(stmt_node_ptr->stmt_type == TYPE_READ){
            stmt_node_ptr->read_node->accept(p_visitor);
        }
        else if(stmt_node_ptr->stmt_type == TYPE_IF){
            stmt_node_ptr->if_node->accept(p_visitor);
        }
        else if(stmt_node_ptr->stmt_type == TYPE_WHILE){
            stmt_node_ptr->while_node->accept(p_visitor);
        }
        else if(stmt_node_ptr->stmt_type == TYPE_FOR){
            stmt_node_ptr->for_node->accept(p_visitor);
        }
        else if(stmt_node_ptr->stmt_type == TYPE_RETURN){
            stmt_node_ptr->return_node->accept(p_visitor);
        }
        else if(stmt_node_ptr->stmt_type == TYPE_FUNCTION_CALL){
            stmt_node_ptr->function_call_node->accept(p_visitor);
        }
    }
}
