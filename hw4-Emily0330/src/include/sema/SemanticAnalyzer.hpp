#ifndef SEMA_SEMANTIC_ANALYZER_H
#define SEMA_SEMANTIC_ANALYZER_H

#include "sema/ErrorPrinter.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <stack>
#include <string>
#include <iostream>

enum ScopeFormingNodeTypeEnum{
  TYPE_PROGRAM = 500,
  TYPE_FUNCTION,
  TYPE_COMPOUND,
  TYPE_FOR
};

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

class SymbolTable {
  public:
    std::vector<const SymbolTable*> prev_for_loop_tables;
    ScopeFormingNodeTypeEnum scope_forming_node;
    bool isFunction = false;
    SymbolTable(int level, ScopeFormingNodeTypeEnum scope_forming_node) : level(level), scope_forming_node(scope_forming_node){}; 
    bool addSymbol(SymbolEntry *entry/* attributes needed by a SymbolEntry */){ // if add successfully, return true
      
      for(auto& prev_for_loop_table_ptr: prev_for_loop_tables){
        if(prev_for_loop_table_ptr->level >= this->getLevel()) continue;
        for(auto& for_loop_table_entry_ptr: prev_for_loop_table_ptr->getEntries()){
          if(entry->name == for_loop_table_entry_ptr->name && !for_loop_table_entry_ptr->isLoopVarlErr) {
            entry->isDeclErr = true;
            entries.emplace_back(entry);
            return false;
          }
        }
      }
      // check redeclaration
      for(auto& x: entries){
        if(entry->name == x->name){ // redeclaration
          entry->isDeclErr = true;
          entries.emplace_back(entry);
          return false; // remember to print out the redeclaration error msg in the node
        }
      }
      entries.emplace_back(entry);
      return true;
    }
    int getLevel(){
      return level;
    }
    ScopeFormingNodeTypeEnum getScopeFormingNodeType(){
      return scope_forming_node;
    }
    std::vector<SymbolEntry*> getEntries() const {
      return entries;
    }
    SymbolEntry* lookup(std::string name){ // not used yet (because lookup should usually be global)
      for(auto& x: entries){
        if(x->name == name) return x;
      }
      return nullptr; // not found in symbol table
    }
    // void dumpDemarcation(const char chr) {
    //   for (size_t i = 0; i < 110; ++i) {
    //     printf("%c", chr);
    //   }
    //   puts("");
    // }
    // other methods
  private:
    // keep the order by simply using vector
    std::vector<SymbolEntry*> entries;
    int level;
    // ScopeFormingNodeTypeEnum scope_forming_node;
};

// void dumpSymbol(SymbolTable* table);// std::string name, std::string kind, int level, std::string type, std::string attr
void dumpSymbol(std::string name, std::string kind, int level, std::string type, std::string attr);
void dumpDemarcation(const char chr);

class SymbolManager {
  public:
    SymbolManager(bool opt_sym) : opt_sym(opt_sym){};
    std::stack<FunctionNode*>function_stack;
    void pushScope(SymbolTable *new_scope, SymbolTable* prev_table){
      
      if(prev_table != nullptr){
        new_scope->prev_for_loop_tables = prev_table->prev_for_loop_tables;
        if(prev_table->scope_forming_node == ScopeFormingNodeTypeEnum::TYPE_FOR) new_scope->prev_for_loop_tables.emplace_back(prev_table);
      }
      tables.push(new_scope);
      return;
    }
    void popScope(){
      if(!tables.empty()) {
        // std::cout<<"Going to pop the table\n";
        SymbolTable* table = tables.top();
        // dump the symbol table of this scope
        if(opt_sym){
          dumpDemarcation('='); 
          printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
                                                "Attribute");
          dumpDemarcation('-');
          for(auto& x: table->getEntries()){
            if(!x->isDeclErr){
              dumpSymbol(x->name,x->kind,x->level,x->type,x->attr);
            }
          }
          dumpDemarcation('-');
        }
        tables.pop();
      }
      else {
        std::cerr << "No scope to pop. (program should not come here).\n";
      }
      return;
    }
    int getStackSize(){
      return tables.size();
    }
    // other methods
    int getTopTableLevel(){
      return tables.top()->getLevel();
    }
    SymbolTable* getTopTablePtr(){
      return tables.top();
    }
    std::stack<SymbolTable*> getAllTablesStack(){
      return tables;
    }
    SymbolEntry* lookup(std::string name, int cur_level){ // find the entry of closest scope
      std::stack<SymbolTable*> tmp_tables_stack = tables;
      while(!tmp_tables_stack.empty()){
        SymbolTable* table = tmp_tables_stack.top();
          if(table->getLevel() < cur_level || table == tables.top()){
            for(auto& entry: table->getEntries()){
              if(!entry->isDeclErr && entry->name == name){
                return entry;
              }
            }
        }
        tmp_tables_stack.pop();
      }
      return nullptr; // not found
    }
  private:
    bool opt_sym = true;
    std::stack<SymbolTable *> tables;
};

class SemanticAnalyzer final : public AstNodeVisitor { // Note that SemanticAnalyzer is an AstNodeVisitor!!
  private:
    ErrorPrinter m_error_printer{stderr};
    // TODO: something like symbol manager (manage symbol tables)
    //       context manager, return type manager
    bool opt_sym = true;
    SymbolManager symbol_manager;

  public:
    ~SemanticAnalyzer() = default;
    // SemanticAnalyzer() = default; // kept
    // explicit SemanticAnalyzer(bool opt_sym) : opt_sym(opt_sym){};
    SemanticAnalyzer() : symbol_manager(opt_sym) {}
    explicit SemanticAnalyzer(bool opt_sym) 
        : opt_sym(opt_sym), symbol_manager(opt_sym) {}

    bool has_error = false;
    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;
};

inline void dumpDemarcation(const char chr) {
  for (size_t i = 0; i < 110; ++i) {
    printf("%c", chr);
  }
  puts("");
}

inline void dumpSymbol(std::string name, std::string kind, int level, std::string type, std::string attr) {
  {
      std::string level_str = (level > 0)? "(local)": "(global)";
      printf("%-33s", name.c_str());
      printf("%-11s", kind.c_str());
      printf("%d%-10s", level, level_str.c_str());
      printf("%-17s", type.c_str());
      printf("%-11s", attr.c_str());
      puts("");
  }
}

#endif
