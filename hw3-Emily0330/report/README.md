# hw3 report

|      |                 |
| ---: | :-------------- |
| Name | your full name |
|   ID |    your student id    |

## How much time did you spend on this project

> This is the assignment that I've spent the most time on in this semester.

* Doing research on C++ object-oriented programming & visitor pattern: 16 hours
* Coding & Debugging: more than 30 hours
* Report: 4 hours


## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way.
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here.

### Visitor Pattern
**I use Visitor Pattern in this assignment.**
#### How it works
In base class `AstNode`, there are two virtual member functions defined: `accept(AstNodeVisitor &p_visitor)` and `visitChildNodes(AstNodeVisitor &p_visitor)`. 

For each class inherited from `AstNode`, I overrided these functions.  
`accept` function will call the node's corresponding `visit()` function defined in `AstDumper.cpp` and print out the information of that Ast node, and then visit its child nodes; `visitChildNodes` function defines the order in which the node's child nodes are visited.

### Modifications in `parser.y`
* In declaration section:
    * Include header files of classes of AST nodes  
        ```c++
        %{
            #include "AST/BinaryOperator.hpp"
            #include "AST/CompoundStatement.hpp"
            #include "AST/ConstantValue.hpp"
            #include "AST/FunctionInvocation.hpp"
            #include "AST/UnaryOperator.hpp"
            ...
            #include "AST/AstDumper.hpp"
            #include "AST/ast.hpp"
            #include "AST/statement.hpp"
            #include "AST/expr.hpp"
            ...
        ```
    * Add classes in `%code` section  
        ```c++
        %code requires {
            #include "AST/ast.hpp"
            #include "AST/statement.hpp"
            class AstNode;
            class CompoundStatementNode;
            class DeclNode;
            ...
            class WhileNode;
            class ForNode;
            class ReturnNode;
        }
        ```
    * Define data types that terminals and nonterminals can return in `%union` section 
        ```c++
        %union {
            /* basic semantic value */
            char *identifier;
            char* str_literal;
            char* type;
            bool bool_literal;
            int int_literal;
            double real_literal;

            AstNode *node;
            CompoundStatementNode *compound_stmt_ptr;
            DeclNode* decl_node;
            std::vector<DeclNode *>* decl_list;
            ...
            IfNode* if_ptr;
            WhileNode* while_ptr;
            ForNode* for_ptr;
            ReturnNode* return_ptr;
        };
        ```

    * Set return data types for each nonterminal and some terminals  
        
        ```c++
        %type <identifier> ProgramName FunctionName
        %type <compound_stmt_ptr> CompoundStatement
        %type <decl_node> Declaration
        %type <decl_list> Declarations DeclarationList FormalArgList
        ...
        %type <if_ptr> Condition
        %type <while_ptr> While
        %type <for_ptr> For
        %type <return_ptr> Return

        ...
            /* Literal */
        %token <int_literal> INT_LITERAL
        %token <real_literal> REAL_LITERAL
        %token <str_literal> STRING_LITERAL
        ```
    * Set operator precedence and left-associativity  
        ```c++
            /* Operator */
        %token ASSIGN
            /* TODO: specify the precedence of the following operators */
        %token MULTIPLY DIVIDE MOD
        %token LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
        %token OR AND NOT
        %token PLUS MINUS
        %token NEG

        %left OR AND NOT
        %left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
        %left MINUS
        %left PLUS
        %left DIVIDE MOD
        %left MULTIPLY
        %left NEG
        ```

* In grammar rule section
    * Add action for each grammar rule  
    Take grammar rules of the nonterminal `ReturnType` for example.
        
        ```c++
        ReturnType: // should return TypeInfo*
            COLON ScalarType{
                $$ = $2; // ScalarType also returns TypeInfo*
            }
            |
            Epsilon{
                // return nothing
                $$ = new TypeInfo;
                $$->type = NULLTYPE;
            }
        ;
        ```

### Modifications in `scanner.l`
* Add `yylval` for some terminals so that it can pass attribute information to `parser.y`  
    ```c++
        /* Identifier */
    [a-zA-Z][a-zA-Z0-9]* {
        listLiteral("id", yytext);
        yylval.identifier = strndup(yytext, MAX_ID_LEN);
        return TOK_ID;
    }
        /* Type keyword */
    "boolean" { listToken("KWboolean"); yylval.type = yytext; return TOK_BOOLEAN; }
    "integer" { listToken("KWinteger"); yylval.type = yytext; return TOK_INTEGER; }
    "real"    { listToken("KWreal"); yylval.type = yytext; return TOK_REAL; }
    "string"  { listToken("KWstring"); yylval.type = yytext; return TOK_STRING; }

    "true"    { listToken("KWtrue"); yylval.bool_literal = true; return TOK_TRUE; }
    "false"   { listToken("KWfalse"); yylval.bool_literal = false; return TOK_FALSE; }

            /* Integer (decimal/octal) */
    {integer} { listLiteral("integer", yytext); yylval.int_literal = std::atoi(yytext); return TOK_INT_LITERAL; } 
    0[0-7]+   { listLiteral("oct_integer", yytext); yylval.int_literal = std::stoi(yytext, 0, 8); return TOK_INT_LITERAL; }

        /* Floating-Point */
    {float} { listLiteral("float", yytext); yylval.real_literal = std::atof(yytext); return TOK_REAL_LITERAL; }

        /* Scientific Notation [Ee][+-]?[0-9]+ */
    ({nonzero_integer}|{nonzero_float})[Ee][+-]?({integer}) { listLiteral("scientific", yytext); yylval.real_literal = std::atof(yytext); return TOK_REAL_LITERAL; }

        /* String */
    \"([^"\n]|\"\")*\" {
        ...
        yylval.str_literal = string_literal;
        ...
    }
    ```
* `yylval` will have the following types defined in `%union` section in `parser.y`:  
    
    ```c++
        /* For yylval */
    %union {
        /* basic semantic value */
        char *identifier;
        char* str_literal;
        char* type; // for some keyword terminals
        bool bool_literal;
        int int_literal;
        double real_literal;
        ...
    }
    ```

### Abilities of my AST
> By constructin AST, the parser builds a detailed structure of the program.
#### Self-defined structs/enums
* `typedef struct _Variable Variable`  
This struct is to store the constant value and its type. It is used in `class ConstantValueNode`.
    ```c++
    typedef struct _Variable{
        EnumType var_type; // INTEGER,REAL,STRING,BOOLEAN
        bool bool_val;
        int int_val;
        std::string str_val;
        double real_val;
    }Variable;
    ```

    * `enum EnumType`  
    Specify the type of a constant value or a variable.
        ```c++
        enum EnumType{
            ARRAY = 300,
            INTEGER,
            REAL,
            STRING,
            BOOLEAN,
            NULLTYPE // for ReturnType (when returning nothing)
        };
        ```
* `typedef struct _type TypeInfo`  
    This struct stores the information of the variable type. If the variable is `EnumType::ARRAY` type, its array dimenison will be recorded in `std::vector<int>array_dim`.
    ```c++
    typedef struct _type{
        EnumType type; // record the type (ARRAY,INTEGER,REAL,STRING,BOOLEAN)
        std::vector<int>array_dim;
        std::string scalar_type; // for array data type because it  has ARRAY in enum
        std::string to_return; // a trivial trick to allow the function getTypeCString() of some class to return a string.
    }TypeInfo;
    ```
    
    
* `typedef struct _id_with_loc IdLoc`   
    This struct contains the location information (line, col) of a identifier. This is used in the nonterminal `IdList`, which returns the type `std::vector<IdLoc*>*`.
    ```c++
    typedef struct _id_with_loc{
        uint32_t line;
        uint32_t col;
        char* identifier;
    }IdLoc;
    ```
* `typedef struct _ExprNode ExprNode`  
    Due to the unfamiliarity with OOP, I could not figure out how to leverage 
    the `class ExpressionNode` defined in the template. As a result, I defined another
    struct `_ExprNode` to store the information about the type of expression node.
    ```c++
    typedef struct _ExprNode{ 
        EnumExprType expr_type;

        // Only one of the pointer is valid in each `_ExprNode` (specified in `expr_type`)
        BinaryOperatorNode* binary_op_node;
        UnaryOperatorNode* unary_op_node;
        ConstantValueNode* constant_value_node;
        VariableReferenceNode* variable_reference_node;
        FunctionInvocationNode* function_call_node;
    }ExprNode;
    ```

    * `enum EnumExprType`  
    Specify the type of the node of the expression.

        ```c++
        enum EnumExprType{
            TYPE_BINARY_OP,
            TYPE_UNARY_OP,
            TYPE_CONSTANT_VALUE,
            TYPE_VARIABLE_REFER,
            TYPE_FUNC_CALL
        };
        ```

* `typedef struct _StatementNode StatementNode`  
    Similar to `struct _ExprNode`, only one of the pointer is valid in each `_StatementNode` (specified in `stmt_type`). This is used in the nonterminal `Statement`, whose return type is `StatementNode*`.
    ```c++
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
    ```
    * `enum EnumStmtType`  
    To specify the type of a statement.   
        ```c++
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
        ```
* `enum EnumDeclType`  
Specify whether the declaration is a variable declaration or a constant declaration.
    ```c++
    enum EnumDeclType{
        TYPE_VARIABLE = 501,
        TYPE_CONSTANT
    };
    ```
* `enum EnumOpType`  
Specify the type of operator. It is used in function of `class BinaryOperatorNode` and `class UnaryOperatorNode`. It helps to construct the condition of the function `getOpCstring()`.
    ```c++
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
    ```

## What is the hardest you think in this project

> Not required, but bonus point may be given.
1. 學 OOP，花了很多時間了解這些 classes 和 member functions 怎麼使用
2. 最難的是一開始寫小零件的時候: ExprNode, StatementNode, DeclarationNode, CompoundNode，因為有些後面還沒寫的 Nodes 也會是構成這些 Nodes 的一部分，這種 Cyclic Dependency 的關係當下處理會覺得比較混亂。這些小零件都定義好之後，後面寫起來就比較輕鬆。
3. 常常忘記自己定義好的 classes 需要哪些參數 (畢竟有 20 幾個 .hpp 檔)，也很容易忘記各個 nonterminals 的回傳型態是什麼，所以後期我就把回傳型態或是class的參數註解在我需要用的地方提醒自己。
4. 因為我是照著測資的順序一個一個寫那些 Classes (Nodes)，在前期還沒記清楚流程時，常常同樣的錯誤一犯再犯，後來我就把看過的錯誤記錄下來，才不會下次遇到又要 debug 很久。(一開始常常有這種情況: 「欸我剛剛看過這個 error，但是我剛剛是改什麼修好的啊?」)

## Feedback to T.A.s

> Not required, but bonus point may be given.  
寫完很有成就感，但過程好辛苦 XD  
沒想到自己竟然是在編譯器設計概論學會OOP... (之前資料結構與物件導向教了 90% 資結、 10% OOP)  
謝謝助教們設計的提示以及非常詳細的 spec，以及實作建議，感恩再感恩...
