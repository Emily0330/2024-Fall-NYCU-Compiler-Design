#ifndef AST_AST_NODE_H
#define AST_AST_NODE_H

#include <cstdint>
#include <string>
#include <vector>

class AstNodeVisitor;

struct Location {
    uint32_t line;
    uint32_t col;

    ~Location() = default;
    Location(const uint32_t line, const uint32_t col) : line(line), col(col) {}
};

class AstNode {
  public: // protected:
    Location location;

  public:
    virtual ~AstNode() = 0;
    AstNode(const uint32_t line, const uint32_t col);

    // Delete copy/move operations to avoid slicing. [1]
    // And "You almost never want to copy or move polymorphic objects. They
    // [1]
    // generally live on the heap, and are accessed via (smart) pointers." [2]
    // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-copy-virtual
    // [2] https://stackoverflow.com/a/54792149

    AstNode(const AstNode &) = delete;
    AstNode(AstNode &&) = delete;
    AstNode &operator=(const AstNode &) = delete;
    AstNode &operator=(AstNode &&) = delete;

    virtual void accept(AstNodeVisitor &p_visitor) = 0; // added
    virtual void visitChildNodes(AstNodeVisitor &p_visitor){}; // added

    const Location &getLocation() const;

    virtual void print() = 0;
};

enum EnumVarType{
  TYPE_BOOL_TRUE,
  TYPE_BOOL_FALSE,
  TYPE_STRING,
  TYPE_INT,
  TYPE_REAL
};
enum EnumType{
  ARRAY = 300,
  INTEGER,
  REAL,
  STRING,
  BOOLEAN,
  NULLTYPE // for ReturnType (when returning nothing)
};

typedef struct _Variable{
  EnumType var_type; // EnumVarType
  bool bool_val;
  int int_val;
  std::string str_val;
  double real_val;
}Variable;

typedef struct _type{
  EnumType type;
  std::vector<int>array_dim;
  std::string scalar_type; // for array data type because it has ARRAY in enum
  std::string to_return;
}TypeInfo;

typedef struct _id_with_loc{
  uint32_t line;
  uint32_t col;
  char* identifier;
}IdLoc;

enum EnumDeclType{
  TYPE_VARIABLE = 501,
  TYPE_CONSTANT
};

enum EnumStmtType{
  TYPE_COMPOUND = 701,
  TYPE_ASSIGNMENT,
  TYPE_PRINT,
  TYPE_READ,
  TYPE_IF,
  TYPE_WHILE,
  TYPE_FOR,
  TYPE_RETURN,
  TYPE_FUNCTION_CALL,
  TYPE_NULL // for the nonterminal ElseOrNot
};

enum EnumOpType{
  TYPE_PLUS = 801,
  TYPE_SUBTRACT, // binary minus
  TYPE_NEGATIVE, // unary minus
  TYPE_OR,
  TYPE_AND,
  TYPE_LESS,
  TYPE_LESS_OR_EQUAL,
  TYPE_EQUAL,
  TYPE_GREATER,
  TYPE_GREATER_OR_EQUAL,
  TYPE_NOT_EQUAL,
  TYPE_MULTIPLY,
  TYPE_DIVIDE,
  TYPE_MOD,
  TYPE_NOT
};

enum EnumExprType{
  TYPE_BINARY_OP,
  TYPE_UNARY_OP,
  TYPE_CONSTANT_VALUE,
  TYPE_VARIABLE_REFER,
  TYPE_FUNC_CALL
};

#endif