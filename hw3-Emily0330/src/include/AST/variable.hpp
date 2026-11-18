#ifndef __AST_VARIABLE_NODE_H
#define __AST_VARIABLE_NODE_H

#include "AST/ast.hpp"
#include "AST/ConstantValue.hpp"
#include <vector>
#include <cctype>
#include <cstring>
#include <iostream>

class VariableNode : public AstNode {
  public:
    VariableNode(const uint32_t line, const uint32_t col, const char *const name, TypeInfo* type, ConstantValueNode* constant_value_node);
                 ///* TODO: variable name, type, constant value */);
    ~VariableNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    const char *getNameCString() const { return name.c_str(); }
    const char *getTypeCString() const { 

      if(type->type == ARRAY){
        // std::string tmp = type->scalar_type;
        type->to_return = type->scalar_type;
        int co = 0;
        for(int& dim: type->array_dim){
          if(co == 0){
            type->to_return += " ";
            co++;
          }
          type->to_return += "[";
          type->to_return += std::to_string(dim);
          type->to_return += "]";
        }
        // std::cout<<"tmp: "<<std::endl;
        return type->to_return.c_str();
      }
      else{
        return type->scalar_type.c_str(); 
      }
    }

    void print() override;

  private:
    // TODO: variable name, type, constant value
    std::string name;
    TypeInfo* type;
    ConstantValueNode* constant_value_node = nullptr; //default is nullptr
};

#endif
