#ifndef __AST_WHILE_NODE_H
#define __AST_WHILE_NODE_H

#include "AST/ast.hpp"
#include "AST/expr.hpp"
#include "AST/CompoundStatement.hpp"

class WhileNode : public AstNode {
  public:
    WhileNode(const uint32_t line, const uint32_t col, struct _ExprNode* expr, CompoundStatementNode* compound
              /* TODO: expression, compound statement */);
    ~WhileNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void print() override;

  private:
    // TODO: expression, compound statement
    struct _ExprNode* expr;
    CompoundStatementNode* compound;
};

#endif
