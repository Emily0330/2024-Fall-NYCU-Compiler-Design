#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "AST/expr.hpp"
#include <vector>

#include <memory>

class BinaryOperatorNode : public ExpressionNode {
  public:
    BinaryOperatorNode(const uint32_t line, const uint32_t col,EnumOpType op, std::vector<struct _ExprNode*> expr_list
                       /* TODO: operator, expressions */); // expr_list should contain only two nodes
    ~BinaryOperatorNode() = default;

    const char *getOpCString() {
        if(op == EnumOpType::TYPE_AND) op_to_return = "and";
        else if(op == EnumOpType::TYPE_OR) op_to_return = "or";
        else if(op == EnumOpType::TYPE_DIVIDE) op_to_return = "/";
        else if(op == EnumOpType::TYPE_EQUAL) op_to_return = "=";
        else if(op == EnumOpType::TYPE_GREATER) op_to_return = ">";
        else if(op == EnumOpType::TYPE_GREATER_OR_EQUAL) op_to_return = ">=";
        else if(op == EnumOpType::TYPE_LESS) op_to_return = "<";
        else if(op == EnumOpType::TYPE_LESS_OR_EQUAL) op_to_return = "<=";
        else if(op == EnumOpType::TYPE_MOD) op_to_return = "mod";
        else if(op == EnumOpType::TYPE_MULTIPLY) op_to_return = "*";
        else if(op == EnumOpType::TYPE_NEGATIVE) op_to_return = "-";
        else if(op == EnumOpType::TYPE_NOT_EQUAL) op_to_return = "<>";
        else if(op == EnumOpType::TYPE_PLUS) op_to_return = "+";
        else if(op == EnumOpType::TYPE_SUBTRACT) op_to_return = "-";
        return op_to_return.c_str();
    }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void print() override;

  private:
    // TODO: operator, expressions
    EnumOpType op;
    std::vector<struct _ExprNode*> expr_list;
    std::string op_to_return;
};

#endif
