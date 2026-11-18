#ifndef __AST_STATEMENT_NODE_H
#define __AST_STATEMENT_NODE_H

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include "AST/CompoundStatement.hpp"
#include "AST/print.hpp"
#include "AST/assignment.hpp"
#include "AST/read.hpp"
#include "AST/if.hpp"
#include "AST/while.hpp"
#include "AST/for.hpp"
#include "AST/return.hpp"
#include "AST/FunctionInvocation.hpp"

typedef struct _StatementNode{
  EnumStmtType stmt_type;

  CompoundStatementNode* compound_node;
  AssignmentNode* assignment_node;
  PrintNode* print_node;
  ReadNode* read_node;
  IfNode* if_node;
  WhileNode* while_node;
  ForNode* for_node;
  ReturnNode* return_node;
  FunctionInvocationNode* function_call_node;
}StatementNode;

#endif