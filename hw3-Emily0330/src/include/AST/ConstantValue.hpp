#ifndef __AST_CONSTANT_VALUE_NODE_H
#define __AST_CONSTANT_VALUE_NODE_H

#include "AST/expression.hpp"
#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <memory>
#include <iostream>

class ConstantValueNode : public ExpressionNode {
  public:
    // TODO: constant value
    Variable *constant_value;

  public:
    ConstantValueNode(const uint32_t line, const uint32_t col, Variable *const constant_value);/* TODO: constant value */
    ~ConstantValueNode() = default;

    const char *getValueCString() const { 
        std::string tmp;
        
        if(constant_value->var_type == INTEGER) {
          // std::cout<<"int_val: "<<std::to_string(constant_value->int_val).c_str()<<'\n';
          constant_value->str_val = std::to_string(constant_value->int_val);
          return constant_value->str_val.c_str();
        }
        else if(constant_value->var_type == REAL) return std::to_string(constant_value->real_val).c_str();
        else if(constant_value->var_type == STRING) return constant_value->str_val.c_str();
        else if(constant_value->var_type == BOOLEAN) return (constant_value->bool_val)? "true": "false";
      //return tmp.c_str(); 
    }
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void print() override;
};

#endif
