#ifndef __AST_DECL_NODE_H
#define __AST_DECL_NODE_H

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include "AST/variable.hpp"
#include "AST/ConstantValue.hpp"
#include <vector>

class DeclNode : public AstNode {
  public:
    // variable declaration
    DeclNode(const uint32_t line, const uint32_t col, std::vector<VariableNode*> var_list
             /* TODO: identifiers, type */);

    // constant variable declaration
    // DeclNode(const uint32_t, const uint32_t col, std::vector<ConstantValueNode*> const_list
    //         /* TODO: identifiers, constant */);

    ~DeclNode() = default;

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    void print() override;

  public:
    // TODO: variables
    std::vector<VariableNode*> var_list;
};

#endif
