#ifndef __AST_UNARY_OPERATOR_NODE_H
#define __AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "AST/expr.hpp"
#include <string>

class UnaryOperatorNode : public ExpressionNode {
  public:
    UnaryOperatorNode(const uint32_t line, const uint32_t col, EnumOpType op, struct _ExprNode* expr
                      /* TODO: operator, expression */);
    ~UnaryOperatorNode() = default;

    const char *getOpCString() {
        if(op == EnumOpType::TYPE_NOT) op_to_return = "not";
        else if(op == EnumOpType::TYPE_NEGATIVE) op_to_return = "neg";
        return op_to_return.c_str();
    }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void print() override;

  private:
    // TODO: operator, expression
    EnumOpType op;
    struct _ExprNode* expr;
    std::string op_to_return;
};

#endif
