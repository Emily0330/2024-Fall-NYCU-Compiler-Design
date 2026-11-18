#include "sema/Error.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"
#include "AST/AstDumper.hpp"

void SemanticAnalyzer::visit(ProgramNode &p_program) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 1
    SymbolTable *table = new SymbolTable(0,ScopeFormingNodeTypeEnum::TYPE_PROGRAM);
    // std::cout<<"program step 1\n";
    this->symbol_manager.pushScope(table, nullptr);
    // step 2
    SymbolEntry* entry = new SymbolEntry(p_program.getNameCString(), "program", 0, p_program.getRetTypeCString(), "");
    // table->addSymbol(entry);
    bool success = table->addSymbol(entry);
    // program is void type
    entry->ptype_ptr = std::make_shared<PType>(PType::PrimitiveTypeEnum::kVoidType);
    if(!success){ // redeclaration
        m_error_printer.print(SymbolRedeclarationError(p_program.getLocation(), p_program.getNameCString()));
        this->has_error = true;
    }
    // step 3
    // std::cout<<"program step 3\n";
    p_program.visitChildNodes(*this);
    // step 4
    // No semantic analysis of program node
    // step 5
    // std::cout<<"program step 5\n";
    this->symbol_manager.popScope();
}

void SemanticAnalyzer::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 2
    int level = this->symbol_manager.getTopTableLevel();
    SymbolTable* table = this->symbol_manager.getTopTablePtr();
    SymbolEntry* entry;
    std::string name(p_variable.getNameCString());
    std::string type(p_variable.getTypeCString());
    // std::cout<<"name: "<<name<<std::endl;
    // std::cout<<"type: "<<type<<std::endl;
    // std::cout<<"stack size: "<<this->symbol_manager.getStackSize()<<std::endl;
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
    if(redeclaration_error) return; //do i have to skip visitChildNode? if redeclaration error occurs, skip the rest of semantic checks
    // step 3
    p_variable.visitChildNodes(*this);
    // step 4: TODO
    
    for(auto& dim: p_variable.getPTypeSharedPtr()->getDimensions()){
        if(dim <= 0){
            entry->error = true;
            this->has_error = true;
            this->m_error_printer.print(NonPositiveArrayDimensionError(p_variable.getLocation(),p_variable.getNameCString()));
            break;
        }
    }
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // std::cout<<"const value: "<<p_constant_value.getConstantValueCString()<<", type: "<<p_constant_value.getTypeSharedPtr()->getPTypeCString()<<std::endl; // test
    p_constant_value.setRetType(p_constant_value.getTypeSharedPtr());
    // step 4: TODO
}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 2:
    SymbolTable* last_symbol_table = this->symbol_manager.getTopTablePtr();
    SymbolEntry* entry = new SymbolEntry(p_function.getNameCString(), "function", last_symbol_table->getLevel(), p_function.getRetTypeCString(), p_function.getParametersTypeStr());
    entry->ptype_ptr = p_function.getRetPTypeSharedPtr();
    entry->function_node = &p_function;
    bool success = last_symbol_table->addSymbol(entry);
    if(!success){ // redeclaration
        m_error_printer.print(SymbolRedeclarationError(p_function.getLocation(), p_function.getNameCString()));
        this->has_error = true;
    }
    
    // step 1
    int level = this->symbol_manager.getTopTableLevel() + 1;
    SymbolTable* prev_table = this->symbol_manager.getTopTablePtr();
    SymbolTable *table = new SymbolTable(level,ScopeFormingNodeTypeEnum::TYPE_FUNCTION);
    this->symbol_manager.pushScope(table, prev_table);
    this->symbol_manager.function_stack.push(&p_function);

    // step 3
    p_function.visitChildNodes(*this);
    // step 4: TODO
    // TODO
    // step 5
    this->symbol_manager.popScope();
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    
    // step 1
    SymbolTable *table;
    int level;
    bool under_a_function = false;
    if(this->symbol_manager.getTopTablePtr()->getScopeFormingNodeType() == ScopeFormingNodeTypeEnum::TYPE_FUNCTION){
        // function and its compound statement should share the same symbol table
        table = this->symbol_manager.getTopTablePtr();
        table->scope_forming_node = ScopeFormingNodeTypeEnum::TYPE_COMPOUND;
        table->isFunction = true;
        level = table->getLevel();
        under_a_function = true;
    }
    else{
        level = this->symbol_manager.getTopTableLevel() + 1;
        table = new SymbolTable(level, ScopeFormingNodeTypeEnum::TYPE_COMPOUND);
        SymbolTable* prev_table = this->symbol_manager.getTopTablePtr();
        this->symbol_manager.pushScope(table, prev_table);
    }
    // step 2
    // no for compound stmt
    // step 3
    p_compound_statement.visitChildNodes(*this);
    // step 4: TODO
    // TODO
    // step 5
    if(!under_a_function) {
        this->symbol_manager.popScope();
        // std::cout<<"compound stmt scope popped.\n"; // test
    } // if it is under a function, it will be popped in functionNode visit
}

void SemanticAnalyzer::visit(PrintNode &p_print) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 3
    // std::cout <<"in print...\n"; // test
    p_print.visitChildNodes(*this);
    // step 4: TODO
    auto& target = p_print.getTargetExprPtr(); // expr unique ptr
    if(target == nullptr) return;
    
    // if(target->getExprError()) std::cout <<"expr wrong\n"; // test
    // else {
    //     std::cout<<"expr correct\n"; // test
    //     std::cout<<"col: "<<p_print.getLocation().col<<", line: "<<p_print.getLocation().line<<std::endl;
    // }

    // if(target->getReturnType()) std::cout <<"has return type"<<target->getReturnType()->getPTypeCString()<<"\n"; // test
    // else std::cout<<"no return type\n"; // test

    // std::cout<<"target type: "<<target->getReturnType()->getPTypeCString()<<std::endl;
    if(target->getExprError()) return; // Skip the rest of semantic checks if there are any errors in the node of the expression (target)
    if(!target->getReturnType()->isScalar()){
        m_error_printer.print(PrintOutNonScalarTypeError(target->getLocation()));
        this->has_error = true;
        return;
    }
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // p_bin_op.setRetType(p_bin_op)
    p_bin_op.visitChildNodes(*this);
    // step 4: TODO
    Operator op = p_bin_op.getOperator();
    
    // if(!p_bin_op.getLeftOpType()) std::cout<<"name: "<<p_bin_op.getOpCString()<<std::endl; // test
    if(!p_bin_op.getLeftOpType() || !p_bin_op.getRightOpType()) {
        p_bin_op.setExprErrorTrue();
        return; // 前面就壞掉了 EX: over subscript
    }
    
    if(p_bin_op.getExprError()) {
        return;
    }
    if(p_bin_op.getLeftOpExprPtr()->getExprError() || p_bin_op.getRightOpExprPtr()->getExprError()){
        p_bin_op.setExprErrorTrue();
        return;
    }
    // check array
    if((!p_bin_op.getLeftOpType()->getDimensions().empty() || !p_bin_op.getRightOpType()->getDimensions().empty()) && p_bin_op.getLeftOpType()->getDimensions() != p_bin_op.getRightOpType()->getDimensions()){
        m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(),p_bin_op.getOperator(),p_bin_op.getLeftOpType().get(),p_bin_op.getRightOpType().get()));
        this->has_error = true;
        p_bin_op.setExprErrorTrue();
        return;
    }
    if(op == Operator::kPlusOp || op == Operator::kMinusOp || op == Operator::kMultiplyOp || op == Operator::kDivideOp){ // + - * /
        // if(!p_bin_op.getLeftOpType()) std::cout<<"name: "<<p_bin_op.getOpCString()<<std::endl; // test
        // std::cout<<"name: "<<p_bin_op.getOpCString()<<", left type: "<<p_bin_op.getLeftOpType()->getPTypeCString()<<", right type: "<<p_bin_op.getRightOpType()->getPTypeCString()<<std::endl; // test
        PTypeSharedPtr tmp;

        // string concatenation +
        if(op == Operator::kPlusOp && p_bin_op.getLeftOpType()->isString() && p_bin_op.getRightOpType()->isString()){
            tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kStringType);
            p_bin_op.setRetType(tmp);
        }
        else{ // arithmetic
            if(!((p_bin_op.getLeftOpType()->isInteger() || p_bin_op.getLeftOpType()->isReal()) && (p_bin_op.getRightOpType()->isInteger() || p_bin_op.getRightOpType()->isReal()))){ //兩個有其中一個不是real or int
                m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(),p_bin_op.getOperator(),p_bin_op.getLeftOpType().get(),p_bin_op.getRightOpType().get()));
                this->has_error = true;
                p_bin_op.setExprErrorTrue();
                return;
            }
            if(p_bin_op.getLeftOpType()->isReal() || p_bin_op.getRightOpType()->isReal()) {
                tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kRealType);
                p_bin_op.setRetType(tmp);
            }
            else{
                tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kIntegerType);
                p_bin_op.setRetType(tmp);
            }
        }
        
    }
    if(op == Operator::kAndOp || op == Operator::kOrOp){
        if(!(p_bin_op.getLeftOpType()->isBoolean() && p_bin_op.getRightOpType()->isBoolean())){ // And, Or
            m_error_printer.print((InvalidBinaryOperandError(p_bin_op.getLocation(),p_bin_op.getOperator(),p_bin_op.getLeftOpType().get(),p_bin_op.getRightOpType().get())));
            this->has_error = true;
            p_bin_op.setExprErrorTrue();
            return;
        }
        else{
            PTypeSharedPtr tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kBoolType);
            tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kBoolType);
            p_bin_op.setRetType(tmp);
        }
    }
    if(op == Operator::kModOp){
        if(!(p_bin_op.getLeftOpType()->isInteger() && p_bin_op.getRightOpType()->isInteger())){ // And, Or
            m_error_printer.print((InvalidBinaryOperandError(p_bin_op.getLocation(),p_bin_op.getOperator(),p_bin_op.getLeftOpType().get(),p_bin_op.getRightOpType().get())));
            this->has_error = true;
            p_bin_op.setExprErrorTrue();
            return;
        }
        else{
            PTypeSharedPtr tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kIntegerType);
            tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kIntegerType);
            p_bin_op.setRetType(tmp);
        }
    }
    if(op == Operator::kEqualOp || op == Operator::kGreaterOp || op == Operator::kGreaterOrEqualOp || op == Operator::kLessOp || op == Operator::kLessOrEqualOp || op == Operator::kNotEqualOp){
        PTypeSharedPtr tmp;
        if(!((p_bin_op.getLeftOpType()->isInteger() || p_bin_op.getLeftOpType()->isReal()) && (p_bin_op.getRightOpType()->isInteger() || p_bin_op.getRightOpType()->isReal()))){ //兩個有其中一個不是real or int
            m_error_printer.print(InvalidBinaryOperandError(p_bin_op.getLocation(),p_bin_op.getOperator(),p_bin_op.getLeftOpType().get(),p_bin_op.getRightOpType().get()));
            this->has_error = true;
            p_bin_op.setExprErrorTrue();
            return;
        }
        else{
            tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kBoolType);
            p_bin_op.setRetType(tmp);
        }
    }
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    
    // std::cout <<"in unary...\n"; // test
    // step 3
    p_un_op.visitChildNodes(*this);
    // step 4: TODO
    Operator op = p_un_op.getOperator();
    if(!p_un_op.getOperandType()) {
        p_un_op.setExprErrorTrue();
        return; // idk why
    }
    // if(p_un_op.getExprError()) {
    //     return;
    // }
    // std::cout<<"unary op: operand has error: "<<p_un_op.get()<<std::endl;
    if(p_un_op.getOperandExprPtr()->getExprError()){
        std::cout<<"unary op: operand has error: "<<p_un_op.getOpCString()<<std::endl;
        p_un_op.setExprErrorTrue();
        return;
    }
    // std::cout<<"hi\n";
    // std::cout<<"name: "<<p_un_op.getOpCString()<<" ,type: "<<p_un_op.getOperandType()->getPTypeCString()<<std::endl; // test
    if(op == Operator::kNegOp){
        if(!p_un_op.getOperandType()->isInteger() && !p_un_op.getOperandType()->isReal()){ // And, Or
            m_error_printer.print((InvalidUnaryOperandError(p_un_op.getLocation(),p_un_op.getOperator(),p_un_op.getOperandType().get())));
            this->has_error = true;
            p_un_op.setExprErrorTrue();
            return;
        }
        else{
            if(p_un_op.getOperandType()->isInteger()){
                PTypeSharedPtr tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kIntegerType);
                tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kIntegerType);
                p_un_op.setRetType(tmp);
            }
            else{ //p_un_op.getOperandType() is real
                PTypeSharedPtr tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kRealType);
                tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kRealType);
                p_un_op.setRetType(tmp);
            }
            
        }
    }
    if(op == Operator::kNotOp){
        if(!p_un_op.getOperandType()->isABool()){ // And, Or
            m_error_printer.print((InvalidUnaryOperandError(p_un_op.getLocation(),p_un_op.getOperator(),p_un_op.getOperandType().get())));
            this->has_error = true;
            p_un_op.setExprErrorTrue();
            return;
        }
        else{
            PTypeSharedPtr tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kBoolType);
            tmp = std::make_shared<PType>(PType::PrimitiveTypeEnum::kBoolType);
            p_un_op.setRetType(tmp);
        }
    }
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 3
    p_func_invocation.visitChildNodes(*this);
    // Start here!! (把symboltable entry 額外加一個 PTypeSharedPtr，不用改原本new symbol的code，因為dump也需要用原本的string type)Modify here!! lookup的結果可能是nullptr 
    
    // step 4: TODO
    // 1. The identifier has to be in symbol tables.
    SymbolEntry* entry = this->symbol_manager.lookup(p_func_invocation.getNameCString(),this->symbol_manager.getTopTableLevel());
    if(entry == nullptr){
        m_error_printer.print(UndeclaredSymbolError(p_func_invocation.getLocation(),p_func_invocation.getNameCString()));
        this->has_error = true;
        p_func_invocation.setExprErrorTrue();
        return; // does not exist
    }
    PTypeSharedPtr ret_type_ptr = entry->ptype_ptr;
    p_func_invocation.setRetType(ret_type_ptr);// how to know the type of function invoke=>lookup table

    // 2. The kind of symbol has to be function.
    if(entry->kind != "function"){
        m_error_printer.print(NonFunctionSymbolError(p_func_invocation.getLocation(),p_func_invocation.getNameCString()));
        this->has_error = true;
        p_func_invocation.setExprErrorTrue();
        return;
    }

    // 3. The number of arguments must be the same as one of the parameters.
    if(p_func_invocation.getNumOfArgs() != entry->function_node->getNumOfParameters()){
        m_error_printer.print(ArgumentNumberMismatchError(p_func_invocation.getLocation(),p_func_invocation.getNameCString()));
        this->has_error = true;
        p_func_invocation.setExprErrorTrue();
        return;
    }

    // 4. Traverse arguments: The type of the result of the expression (argument) must be the same type of the corresponding parameter after appropriate type coercion.
    auto &args = p_func_invocation.getArgs(); // std::vector<std::unique_ptr<ExpressionNode>>
    auto &params = entry->function_node->getParams(); // std::vector<std::unique_ptr<DeclNode>>
    if(args.empty()) return; // has no params and args
    for(int i = 0; i < params.size(); i++){
        auto arg_type = args[i]->getReturnType();
        auto param_type = params[i]->getVariables()[0]->getPTypeSharedPtr();
        if(arg_type == nullptr || param_type == nullptr) return;
        if(!arg_type->isSameAfterThisCoercion(param_type)){
            m_error_printer.print(IncompatibleArgumentTypeError(args[i]->getLocation(),param_type.get(),arg_type.get()));
            this->has_error = true;
            p_func_invocation.setExprErrorTrue();
            return;
        }
    }
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 3
    p_variable_ref.visitChildNodes(*this);
    // step 4: TODO
    // 4-1: The identifier has to be in symbol tables.
    bool in_symbol_table = false;
    std::stack<SymbolTable*> allTables = this->symbol_manager.getAllTablesStack();
    int level = this->symbol_manager.getTopTablePtr()->getLevel();
    SymbolEntry* entry;
    while(!allTables.empty()){
        SymbolTable* tmp = allTables.top();
        if(tmp->getLevel() < level || tmp == this->symbol_manager.getTopTablePtr()){
            for(auto& x: tmp->getEntries()){
                if(x->name == p_variable_ref.getNameCString()){
                    in_symbol_table = true;
                    entry = x;
                    break;
                }
            }
        }
        allTables.pop();
    }

    if(!in_symbol_table) {
        m_error_printer.print(UndeclaredSymbolError(p_variable_ref.getLocation(),p_variable_ref.getNameCString()));
        this->has_error = true;
        p_variable_ref.setExprErrorTrue();
        return;
    }

    // 4-2: The kind of symbol has to be a parameter, variable, loop_var, or constant.
    if(entry->error) {
        p_variable_ref.setExprErrorTrue();
        return; // no need to do the rest of semantic check
    }
    std::string kind = entry->kind; // if entry is null ptr, then shouldn't come here (has returned in 4-1)
    if(kind != "parameter" && kind != "loop_var" && kind != "variable" && kind != "constant"){
        m_error_printer.print(NonVariableSymbolError(p_variable_ref.getLocation(),p_variable_ref.getNameCString()));
        this->has_error = true;
        p_variable_ref.setExprErrorTrue();
        return;
    }

    // 4-3: Each index of an array reference must be of the integer type
    int idx = 0;
    for(auto& expr_ptr: p_variable_ref.getIndices()){ // const std::vector<std::unique_ptr<ExpressionNode>>&
        // if(expr_ptr->getRetTypePtr() == nullptr) break; // this is not 
        if(!expr_ptr->getReturnType()->isInteger()){
            ExpressionNode* p_expr = p_variable_ref.getIndices()[idx].get();
            m_error_printer.print(NonIntegerArrayIndexError(p_expr->getLocation(),p_variable_ref.getNameCString()));
            this->has_error = true;
            p_variable_ref.setExprErrorTrue();
            return;
        }
        idx++;
    }
    // 記得改exprNode，setRetType()
    // 4-4: An over array subscript is forbidden, that is, the number of indices of an array reference cannot be greater than the one of dimensions in the declaration.
    // for(auto& dim: entry->array_dims){}
    if(p_variable_ref.getIndices().size() > entry->array_dims.size()){ //p_variable_ref.getIndices().size() is getDimension
        // std::cout<<"iam here\n"; // test
        m_error_printer.print(OverArraySubscriptError(p_variable_ref.getLocation(),p_variable_ref.getNameCString()));
        this->has_error = true;
        p_variable_ref.setExprErrorTrue();
        // std::cout<<p_variable_ref.getExprError()<<std::endl; // test
        return;
    }
    
    p_variable_ref.setRetType(entry->ptype_ptr->getArrayReferenceType(p_variable_ref.getDimension()));
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 3
    p_assignment.visitChildNodes(*this);
    // step 4: TODO
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 3
    p_read.visitChildNodes(*this);
    // step 4: TODO
    auto& target = p_read.getTargetVarRefPtr(); // expr unique ptr
    if(target == nullptr) return;

    // std::cout<<"target type: "<<target->getReturnType()->getPTypeCString()<<std::endl;
    if(target->getExprError()) return; // Skip the rest of semantic checks if there are any errors in the node of the expression (target)
    
    // 1. The type of the variable reference must be scalar type.
    if(!target->getReturnType()->isScalar()){
        m_error_printer.print(ReadToNonScalarTypeError(target->getLocation()));
        this->has_error = true;
        return;
    }
    // 2. The kind of symbol of the variable reference cannot be constant or loop_var.
    int level = this->symbol_manager.getTopTableLevel();
    std::string var_name(target->getNameCString());
    SymbolEntry* entry = this->symbol_manager.lookup(var_name, level);
    if(entry->kind == "constant" || entry->kind == "loop_var"){
        m_error_printer.print(ReadToConstantOrLoopVarError(target->getLocation()));
        this->has_error = true;
        return;
    }

}

void SemanticAnalyzer::visit(IfNode &p_if) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 3
    p_if.visitChildNodes(*this);
    // step 4: TODO
    auto& condition_expr_ptr = p_if.getConditionExprPtr();
    if(condition_expr_ptr->getExprError()) return;
    if(!condition_expr_ptr->getReturnType()->isABool()){
        m_error_printer.print(NonBooleanConditionError(condition_expr_ptr->getLocation()));
        this->has_error = true;
        return;
    }
}

void SemanticAnalyzer::visit(WhileNode &p_while) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 3
    p_while.visitChildNodes(*this);
    // step 4: TODO
    auto& condition_expr_ptr = p_while.getConditionExprPtr();
    if(condition_expr_ptr->getExprError()) return;
    if(!condition_expr_ptr->getReturnType()->isABool()){
        m_error_printer.print(NonBooleanConditionError(condition_expr_ptr->getLocation()));
        this->has_error = true;
        return;
    }
}

void SemanticAnalyzer::visit(ForNode &p_for) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 1
    int level = this->symbol_manager.getTopTableLevel() + 1;
    SymbolTable *table = new SymbolTable(level,ScopeFormingNodeTypeEnum::TYPE_FOR);
    SymbolTable* prev_table = this->symbol_manager.getTopTablePtr();
    this->symbol_manager.pushScope(table, prev_table);
    // step 3
    p_for.visitChildNodes(*this);
    // step 4: TODO
    auto loop_var_assignment_ptr = p_for.getInitStmt();
    auto end_condition_expr_ptr = p_for.getEndConditionExprPtr();
    uint32_t initial_val = atoi(dynamic_cast<const ConstantValueNode *>(loop_var_assignment_ptr->getRExprPtr())->getConstantValueCString());
    uint32_t end_val = atoi(dynamic_cast<const ConstantValueNode *>(end_condition_expr_ptr)->getConstantValueCString());
    if(initial_val >= end_val){
        m_error_printer.print(NonIncrementalLoopVariableError(p_for.getLocation()));
        this->has_error = true;
        
        SymbolEntry* entry = this->symbol_manager.lookup(table->getEntries()[0]->name,level);
        entry->isLoopVarlErr = true;
        this->symbol_manager.popScope(); // 出錯，直接 popScope
        return;
    }
    // step 5
    this->symbol_manager.popScope();
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // step 3
    p_return.visitChildNodes(*this);
    // step 4: TODO

    // 1. The current context shouldn't be in the program or a procedure since their return type is void.
    // std::stack<SymbolTable*> tables_stack = this->symbol_manager.getAllTablesStack();
    // if(tables_stack.empty()) return;
    // SymbolTable* top = tables_stack.top();
    // if(top == nullptr) return;
    // if(tables_stack.empty()) return;
    // tables_stack.pop();
    // SymbolTable* second_top = tables_stack.top();
    // // if(this->symbol_manager.getTopTablePtr() == nullptr) return;
    
    // if(!(second_top->getScopeFormingNodeType() == ScopeFormingNodeTypeEnum::TYPE_PROGRAM && top->getScopeFormingNodeType() == ScopeFormingNodeTypeEnum::TYPE_COMPOUND)){ // 
    //     std::cout<<"top: "<<top->getScopeFormingNodeType()<<", second_top: "<<second_top->getScopeFormingNodeType()<<std::endl;
    
    //     m_error_printer.print(ReturnFromVoidError(p_return.getLocation()));//!this->symbol_manager.getTopTablePtr()->isFunction
    //     this->has_error = true;
    //     return;
    // }
    // if(this->symbol_manager.function_stack.empty()) return;
    // FunctionNode* current_function_ptr = this->symbol_manager.function_stack.top();
    // ExpressionNode* ret_expr_ptr = p_return.getRetExprPtr();
    // if(ret_expr_ptr == nullptr || current_function_ptr == nullptr) return;
    // PTypeSharedPtr ret_type_ptr = dynamic_cast<const ConstantValueNode *>(ret_expr_ptr)->getReturnType();
    // if(ret_expr_ptr == nullptr || ret_expr_ptr->getExprError()) return;
    // if(!ret_type_ptr->isSameAfterThisCoercion(current_function_ptr->getRetPTypeSharedPtr())){
    //     m_error_printer.print(IncompatibleReturnTypeError(ret_expr_ptr->getLocation(),current_function_ptr->getRetPTypeSharedPtr().get(),ret_type_ptr.get()));
    //     this->has_error = true;
    //     return;
    // }

    // this->symbol_manager.function_stack.pop(); //the function has returned
}
