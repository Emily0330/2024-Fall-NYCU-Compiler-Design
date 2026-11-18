#ifndef __AST_FOR_NODE_H
#define __AST_FOR_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/assignment.hpp"
#include "AST/expr.hpp"
#include "AST/CompoundStatement.hpp"

class ForNode : public AstNode {
  public:
    ForNode(const uint32_t line, const uint32_t col, DeclNode* decl, AssignmentNode* assignment, struct _ExprNode* expr, CompoundStatementNode* compound
            /* TODO: declaration, assignment, expression,
             *       compound statement */);
    ~ForNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void print() override;

  private:
    // TODO: declaration, assignment, expression, compound statement
    DeclNode* decl;
    AssignmentNode* assignment;
    struct _ExprNode* expr;
    CompoundStatementNode* compound;
};

#endif
