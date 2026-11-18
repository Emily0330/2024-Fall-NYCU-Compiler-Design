#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/CompoundStatement.hpp"
#include <string>
#include <iostream>

class FunctionNode : public AstNode {
  private:
    // TODO: name, declarations, return type, compound statement
    std::string name;
    std::vector<DeclNode*> decl_list;
    TypeInfo* return_type;
    CompoundStatementNode* body = nullptr;
    std::string funcPrototype;
  public:
    FunctionNode(const uint32_t line, const uint32_t col, std::string name, 
                  std::vector<DeclNode*> decl_list, TypeInfo* return_type, 
                  CompoundStatementNode* body
                 /* TODO: name, declarations, return type,
                  *       compound statement (optional) */);
    ~FunctionNode() = default;
    
    const char *getNameCString() const {
      return name.c_str();
    }
    const char *getReturnTypeCString() const {
      if(return_type->type == NULLTYPE){
        std::string tmp = "void";
        return tmp.c_str();
      }
      else if(return_type->type == ARRAY){
        return_type->to_return = return_type->scalar_type;
        int co = 0;
        for(int& dim: return_type->array_dim){
          if(co == 0){
            return_type->to_return += " ";
            co++;
          }
          return_type->to_return += "[";
          return_type->to_return += std::to_string(dim);
          return_type->to_return += "]";
        }
        return return_type->to_return.c_str();
      }
      else if(return_type->type == INTEGER) {
        std::string tmp = "integer";
        return tmp.c_str();
      }
      else if(return_type->type == REAL) {
        std::string tmp = "real";
        return tmp.c_str();
      }
      else if(return_type->type == STRING) {
        std::string tmp = "string";
        return tmp.c_str();
      }
      else if(return_type->type == BOOLEAN) {
        std::string tmp = "boolean";
        return tmp.c_str();
      }
    }
    const char *getFuncPrototypeCString() {
      std::string ret_type(getReturnTypeCString());
      funcPrototype = ret_type + " (";
      int co_decl = 0, co_var = 0;
      for(auto& decl_ptr: decl_list){
        co_var = 0;
        co_decl++;
        for(auto& var_ptr: decl_ptr->var_list){
          const char *c_string = var_ptr->getTypeCString();
          std::string tmp(c_string); // convert c-style string to std::string
          //std::string tmp = "";
          funcPrototype += tmp;
          co_var++;
          //std::cout<<"vector size: "<<co_var == decl_ptr->var_list.size()<<'\n';
          if(!(co_var == decl_ptr->var_list.size() && co_decl == decl_list.size())){
            funcPrototype += ", ";
          }
        }
      }
      funcPrototype += ")";
      return funcPrototype.c_str();
    }
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void print() override;
};

#endif
