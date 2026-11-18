#ifndef __AST_IF_NODE_H
#define __AST_IF_NODE_H

#include "AST/ast.hpp"
#include "AST/expr.hpp"
#include "AST/CompoundStatement.hpp"

class IfNode : public AstNode {
  public:
    IfNode(const uint32_t line, const uint32_t col, ExprNode* expr, std::vector<CompoundStatementNode*> compound_list
           /* TODO: expression, compound statement, compound statement */);
    ~IfNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void print() override;

  private:
    // TODO: expression, compound statement, compound statement
    ExprNode* expr;
    std::vector<CompoundStatementNode*> compound_list;
};

#endif
