# hw4 report

|||
|-:|:-|
|Name|your full name |
|ID|your student id|

## How much time did you spend on this project

> 20 hours.

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way.

> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here.

This time I mainly modified these two files: `SemanticAnalyzer.cpp` and `SemanticAnalyzer.hpp`. (I've got a lot of things going on so I failed to finish the assignment and return parts. QwQ)

### Changes in `SemanticAnalyzer.hpp`

* SymbolEntry
In addition to storing the 5 basic components in the entry, I also store some other members. 
    * `array_dims`: a `std::vector<uint64_t>` to store the dimentions. (if the variable this entry represents is an array)
    PTypeSharedPtr ptype_ptr;
    * `ptypr_ptr`: a `PTypeSharedPtr` to store the type of this entry in the structure of PType pointer. (I have stored the type string in `std::string type`)
    * `bool isLoopVarlErr`: `true` if there is error in loop variable (`NonIncrementalLoopVariableError`).
    * `error`: `true` if there is declaration error (array dim <= 0)
    * `FunctionNode* function_node`: (I planned to use but did not use this) A FunctionNode pointer to keep track of the function node if the entry kind is a function.
```c++
class SymbolEntry {
  public:
    SymbolEntry(std::string name,std::string kind,int level,std::string type,std::string attr)
     : name(name),kind(kind),level(level),type(type),attr(attr){}
    bool isDeclErr = false;
    bool isLoopVarlErr = false; // set this to true when the for loop var is erroneous
    bool error = false; // turn on when declaration error (array dim <= 0)
    FunctionNode* function_node = nullptr;
    PTypeSharedPtr ptype_ptr;
    std::vector<uint64_t>array_dims;
    std::string name;
    std::string kind;
    int level;
    std::string type = "";
    std::string attr = ""; // value of constants or list of types of formal params of a function
};
```

* EntryTable
I implemented some member functions in this class.
    * `bool addSymbol(SymbolEntry *entry)`: to add symbol entry in the table
    * `ScopeFormingNodeTypeEnum getScopeFormingNodeType()`: get the type of node that forms this scope. The Enum may have the following values `TYPE_PROGRAM`,`TYPE_FUNCTION`,`TYPE_COMPOUND`,`TYPE_FOR`.
    * `std::vector<SymbolEntry*> getEntries()`: get all the entries (as a `std::vector<SymbolEntry*>`)

```c++
class SymbolTable {
  public:
    std::vector<const SymbolTable*> prev_for_loop_tables;
    ScopeFormingNodeTypeEnum scope_forming_node;
    bool isFunction = false;
    SymbolTable(int level, ScopeFormingNodeTypeEnum scope_forming_node) : level(level), scope_forming_node(scope_forming_node){}; 
    bool addSymbol(SymbolEntry *entry);
    ScopeFormingNodeTypeEnum getScopeFormingNodeType();
    std::vector<SymbolEntry*> getEntries() const;
    SymbolEntry* lookup(std::string name); // not used (because lookup should usually be global)
  private:
    // keep the order by simply using vector
    std::vector<SymbolEntry*> entries;
    int level;
}
```

* SymbolManager  
    I use a stack to store all the symbol tables because we need to push a symbol table when entering a new scope and pop a symbol table when exiting a scope. 

    The only disadvantage is that if we want to look up  the symbol that is not in the current symbol table (but in the parent node's symbol table.) I have to get a copy of stack and pop all the tables to search for the symbol.  

    * The member functions can do what their names suggest!

    * The private member variable `opt_sym` is 0 if there is `//D-` in the `P` language source code, which can mute the symbol table dumping. Otherwise, `opt_sym` is 1. (This information is passed from `scanner.l` to `parser.y` to `SemanticAnalyzer`).

```c++
lass SymbolManager {
  public:
    SymbolManager(bool opt_sym) : opt_sym(opt_sym){};
    std::stack<FunctionNode*>function_stack;
    void pushScope(SymbolTable *new_scope, SymbolTable* prev_table);
    void popScope();
    int getStackSize();
    int getTopTableLevel();
    SymbolTable* getTopTablePtr();
    std::stack<SymbolTable*> getAllTablesStack();
    SymbolEntry* lookup(std::string name, int cur_level);
  private:
    bool opt_sym = true;
    std::stack<SymbolTable *> tables; // store all symbol tables
}
```

### Changes in `SemanticAnalyzer.cpp`
I followed the steps provided in the spec to construct the `visit` function of each AST node:
1. Push a new symbol table if this node forms a scope.
    * Only ProgramNode, ForNode, CompoundStatementNode, FunctionNode
2. Insert the symbol into current symbol table if this node is related to
   declaration (ProgramNode, VariableNode, FunctionNode).
3. Traverse child nodes of this node.
    * Every node except ConstantValueNode
4. Perform semantic analyses of this node.
5. Pop the symbol table pushed at the 1st step. 

**The following is how I addressed some issues:**
* A FunctionNode should share the same symbol table with its body (CompoundStatementNode) .  
When visiting such CompoundStatementNode, instead of pushing a new scope into the stack, I get the parent Function's symbol table, and change its `ScopeFormingNodeType` to `TYPE_COMPOUND`.

```c++
if(this->symbol_manager.getTopTablePtr()->getScopeFormingNodeType() == ScopeFormingNodeTypeEnum::TYPE_FUNCTION){
    // function and its compound statement should share the same symbol table
    table = this->symbol_manager.getTopTablePtr();
    table->scope_forming_node = ScopeFormingNodeTypeEnum::TYPE_COMPOUND;
    table->isFunction = true;
    level = table->getLevel();
    under_a_function = true; // the symbol table of this scope will be popped in FunctionNode's visit 
}
```

* Add symbol entry & Redeclaration Error  
This is how I add entries of kind `parameter`,`loop_var`,`constant`,`variable`. I did these when visiting `VariableNode`. `addSymbol` is the function to add symbol entry to the symbol table of current scope, it will return `nullptr` if there is redeclaration.  
If there is a redeclaration, just skip the rest of semantic checks as specified in the spec.

* I add entries of kind `program` and `function` in the visit of `ProgramNode` and `FunctionNode`, respectively.

```c++
bool redeclaration_error = false;
if(table->getScopeFormingNodeType() == ScopeFormingNodeTypeEnum::TYPE_FUNCTION){
    entry = new SymbolEntry(name, "parameter", level, type, "");
}
else if(table->getScopeFormingNodeType() == ScopeFormingNodeTypeEnum::TYPE_FOR){
    entry = new SymbolEntry(name, "loop_var", level, type, "");
}
else if(p_variable.getConstantValueNodePtr()) {
    std::string attr(p_variable.getConstantValueNodePtr()->getConstantValueCString());
    entry = new SymbolEntry(name, "constant", level, type, attr);
}
else{
    entry = new SymbolEntry(name, "variable", level, type, "");
}
bool success = table->addSymbol(entry);
entry->array_dims = p_variable.getPTypeSharedPtr()->getDimensions();
entry->ptype_ptr = p_variable.getPTypeSharedPtr();

if(!success){ // redeclaration
    m_error_printer.print(SymbolRedeclarationError(p_variable.getLocation(), p_variable.getNameCString()));
    this->has_error = true;
    redeclaration_error = true;
}
if(redeclaration_error) return; 
```
### Changes in `scanner.l`
I added the `opt_sym` logic. If the scanner detects `//D-`, the `opt_sym` value passed into `parser.y` will be 0. In this case, the semantic analyzer in the parser won't dump the symbol tables.
```c++
    /* Pseudocomment */
"//&"[STD][+-].* {
    char option = yytext[3];
    switch (option) {
    case 'S':
        opt_src = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'T':
        opt_tok = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'D': // newly added
        opt_sym = (yytext[4] == '+') ? 1 : 0; 
        break;
    }
}
```
### Changes in `parser.y`
* In Declarations section:  
I added the declaration of the `opt_sym` variable, which will be passed into parser from `scanner.l`.

    ```c++
    extern uint32_t opt_sym; /* declared in scanner.l */
    ```
* In Routines section:  
I declared and initialized a `SemanticAnalyzer` instance in the `main` function, and pass the argument `opt_sym` into the semantic analyzer to determine whether to dump the symbol table in `stdout`.
    ```c++
    int main(int argc, const char *argv[]) {
        // original code

        SemanticAnalyzer sema_analyzer(opt_sym);
        root->accept(sema_analyzer);

        // original code
    } 
    ```

## What is the hardest you think in this project

> Not required, but bonus point may be given.

因為不熟悉 C++ 中 unique_pointer 和 shared_pointer 的使用，而 template 裡面用了很多這種智慧指標，我常常因為 unique_pointer 的使用錯誤而需要 debug，但是越到後面這樣的 bug 有越 de 越快。

還有一開始 construct symbol table 以及 VariableReference 的部分寫很久。

 spec 寫得很清楚了，但這次作業我有時候還是會 get 不太到 spec 的意思，例如出現哪些 semantic errors 要跳過哪裡的 semantic checks (抱歉時間來不及了找不到當初這樣想的時候的例子)。不過我有試著從 testcases 了解哪些該跳過。

## Feedback to T.A.s

> Not required, but bonus point may be given.

謝謝助教，但這次的作業 deadline 卡在期末，沒辦法再像作業三投入 40 up 個小時了 (開始後悔之前 OOP 學得太水) QwQ