#ifndef AST_EXPRESSION_NODE_H
#define AST_EXPRESSION_NODE_H

#include "AST/ast.hpp"
#include "AST/PType.hpp"

class ExpressionNode : public AstNode {
  public:
    ~ExpressionNode() = default;
    ExpressionNode(const uint32_t line, const uint32_t col)
        : AstNode{line, col} {}
    const PTypeSharedPtr getReturnType() const { return type_ptr; }
    void setRetType(PTypeSharedPtr type){
      type_ptr = type;
    }
    void setExprErrorTrue() { ExprError = true; }
    bool getExprError() { return ExprError; }
  protected:
    // for carrying type of result of an expression
    // TODO: for next assignment
    PTypeSharedPtr type_ptr = nullptr;
    bool ExprError = false;
};

#endif
