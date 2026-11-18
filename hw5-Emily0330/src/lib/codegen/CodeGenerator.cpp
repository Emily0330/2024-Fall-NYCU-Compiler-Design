#include "AST/CompoundStatement.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/program.hpp"
#include "codegen/CodeGenerator.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <iostream>

int sp = 0; // current stack pointer
int fp = 0; // current frame pointer, s0
int offset = -12; // the address fp + offset is the next place to store local variables
bool isFunction = false;
int label_cnt = 0;

typedef enum VAR_REF_ENUM{
    TYPE_LHS = 500,
    TYPE_RHS
};

VAR_REF_ENUM current_var_ref; // control whether we want value or address
const char * FuncParamRegs[11] = {"a1","a2","a3","a4","a5","a6","a7","t3","t4","t5","t6"};
CodeGenerator::CodeGenerator(const std::string &source_file_name,
                             const std::string &save_path,
                             std::unordered_map<SemanticAnalyzer::AstNodeAddr,
                                                      SymbolManager::Table>
                                 &&p_symbol_table_of_scoping_nodes)
    : m_symbol_manager(false /* no dump */),
      m_source_file_path(source_file_name),
      m_symbol_table_of_scoping_nodes(std::move(p_symbol_table_of_scoping_nodes)) {
    // FIXME: assume that the source file is always xxxx.p
    const auto &real_path =
        save_path.empty() ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind('/');
    auto dot_pos = source_file_name.rfind('.');

    if (slash_pos != std::string::npos) {
        ++slash_pos;
    } else {
        slash_pos = 0;
    }
    auto output_file_path{
        real_path + "/" +
        source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S"};
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

void CodeGenerator::visit(ProgramNode &p_program) {
    // Generate RISC-V instructions for program header
    // clang-format off
    constexpr const char *const riscv_assembly_file_prologue =
        "    .file \"%s\"\n"
        "    .option nopic\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_file_prologue,
                     m_source_file_path.c_str());

    // Reconstruct the scope for looking up the symbol entry.
    // Hint: Use m_symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_program)));

    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(*this); };
    for_each(p_program.getDeclNodes().begin(), p_program.getDeclNodes().end(),
             visit_ast_node);
    for_each(p_program.getFuncNodes().begin(), p_program.getFuncNodes().end(),
             visit_ast_node);
    // m_symbol_manager.popScope(); // test
    constexpr const char *const main_func_prologue = 
        ".section    .text\n"
        "    .align 2\n"
        "    .globl main\n"
        "    .type main, @function\n"
        "main:\n"
        "    addi sp, sp, -128\n"
        "    sw ra, 124(sp)\n"
        "    sw s0, 120(sp)\n"
        "    addi s0, sp, 128\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), main_func_prologue,
                     m_source_file_path.c_str());
    sp = 128;
    fp = 0;
    // offset = -12; // not changed
    // m_symbol_manager.pushScope(); // test
    const_cast<CompoundStatementNode &>(p_program.getBody()).accept(*this);
    dumpInstructions(m_output_file.get(),
    "    lw ra, 124(sp)\n"
    "    lw s0, 120(sp)\n"
    "    addi sp, sp, 128\n"
    "    jr ra\n"
    "    .size main, .-main\n");
     // reset sp,fp,offset
    sp = 0;
    fp = 0;
    offset = -12;
    m_symbol_manager.popScope();
}

void CodeGenerator::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

void CodeGenerator::visit(VariableNode &p_variable) {
    
    SymbolEntry* entry = const_cast<SymbolEntry *>(m_symbol_manager.lookup(p_variable.getName()));
    if(entry->getKind() == SymbolEntry::KindEnum::kParameterKind){
        // use const char * FuncParamRegs[11]
        entry->setStackPosition(fp + offset);
        dumpInstructions(m_output_file.get(),
        "    sw %s, %d(s0)\n", FuncParamRegs[(-offset - 12) / 4], offset);
        offset -= 4;
        return;
    }
    if(entry->getLevel() == 0){ // global
        if(p_variable.getConstantPtr() == nullptr){ // global variable
            dumpInstructions(m_output_file.get(), ".comm %s, 4, 4\n", entry->getNameCString());
        }
        else{ // global constant
            dumpInstructions(m_output_file.get(), 
            ".section    .rodata\n"
            "    .align 2\n"
            "    .globl %s\n"
            "    .type %s, @object\n"
            "%s:\n"
            "    .word %s\n", entry->getNameCString(),  entry->getNameCString(),  entry->getNameCString(), p_variable.getConstantPtr()->getConstantValueCString());
        }
    }
    else{ // local variable // check
        // dumpInstructions(m_output_file.get(),
        // "    addi t0, s0, %d\n"
        // "    addi sp, sp, -4\n"
        // "    sw t0, 0(sp)\n", offset);
        if(p_variable.getConstantPtr() != nullptr){ // local constant
            dumpInstructions(m_output_file.get(),
                "    addi t0, s0, %d\n"
                "    addi sp, sp, -4\n"
                "    sw t0, 0(sp)\n"
                "    li t0, %s\n"
                "    lw t1, 0(sp)\n"
                "    addi sp, sp, 4\n"
                "    sw t0, 0(t1)\n",
                offset, p_variable.getConstantPtr()->getConstantValueCString());
        }
        entry->setStackPosition(fp + offset);
        offset -= 4; //新增一個變數在stack(給他空間而已，值還沒存進去)
        // sp -= 4;
    }
    
}

void CodeGenerator::visit(ConstantValueNode &p_constant_value) {
    dumpInstructions(m_output_file.get(),
    "    li t0, %s\n"
    "    addi sp, sp, -4\n"
    "    sw t0, 0(sp)\n"
    "    mv a0, t0\n", p_constant_value.getConstantValueCString());
    sp -= 4;
}

void CodeGenerator::visit(FunctionNode &p_function) {
    // Reconstruct the scope for looking up the symbol entry.
    isFunction = true;
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_function)));
    // std::cout<<"here i am\n";
    
    dumpInstructions(m_output_file.get(),
    ".section    .text\n"
    "    .align 2\n"
    "    .globl %s\n"
    "    .type %s, @function\n", p_function.getNameCString(), p_function.getNameCString());
    
    // function prologue
    dumpInstructions(m_output_file.get(),
    "%s:\n"
    "    addi sp, sp, -128\n"
    "    sw ra, 124(sp)\n"
    "    sw s0, 120(sp)\n"
    "    addi s0, sp, 128\n", p_function.getNameCString());
    sp -= 128;
    fp = sp + 128;
    offset = -12;

    p_function.visitChildNodes(*this);

    // function epilogue
    dumpInstructions(m_output_file.get(),
    "    lw ra, 124(sp)\n"
    "    lw s0, 120(sp)\n"
    "    addi sp, sp, 128\n"
    "    jr ra\n"
    "    .size %s, .-%s\n", p_function.getNameCString(), p_function.getNameCString());
    sp += 128;
    fp = sp + 128;
    offset = -12;

    // Remove the entries in the hash table
    m_symbol_manager.popScope();
    isFunction = false;
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
    // Reconstruct the scope for looking up the symbol entry.
    if(!isFunction){
        // std::cout<<"aloha\n"; // test
        m_symbol_manager.pushScope(
            std::move(m_symbol_table_of_scoping_nodes.at(&p_compound_statement)));

        p_compound_statement.visitChildNodes(*this);

        m_symbol_manager.popScope();
    }
    else{
        // std::cout<<"aloh2a\n"; // test
        isFunction = false;
        p_compound_statement.visitChildNodes(*this);
        // isFunction = true;
    }
}

void CodeGenerator::visit(PrintNode &p_print) {
    current_var_ref = VAR_REF_ENUM::TYPE_RHS; // need value
    p_print.visitChildNodes(*this);
    // pop the value from the stack to the first argument register 'a0'
    dumpInstructions(m_output_file.get(),
    "    lw a0, 0(sp)\n"
    "    addi sp, sp, 4\n"
    "    jal ra, printInt\n");
    sp += 4;
    // 被騙了!!! dumpInstructions(m_output_file.get(),
    // "    la t0, %s\n", p_print.getTarget().)
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {
    std::cout<<"op: "<<p_bin_op.getOpCString()<<'\n'; // test
    current_var_ref = VAR_REF_ENUM::TYPE_RHS;
    p_bin_op.visitChildNodes(*this);
    dumpInstructions(m_output_file.get(),
    "    lw t1, 0(sp)\n"
    "    addi sp, sp, 4\n"
    "    lw t0, 0(sp)\n"
    "    addi sp, sp, 4\n");
    sp += 8;
    Operator op = p_bin_op.getOp();
    if(op == Operator::kPlusOp) {
        dumpInstructions(m_output_file.get(), "    add t0, t0, t1\n");
    }
    else if(op == Operator::kMinusOp){
        dumpInstructions(m_output_file.get(), "    sub t0, t0, t1\n");
    }
    else if(op == Operator::kMultiplyOp){
        dumpInstructions(m_output_file.get(), "    mul t0, t0, t1\n");
    }
    else if(op == Operator::kDivideOp){
        dumpInstructions(m_output_file.get(), "    div t0, t0, t1\n");
    }
    else if(op == Operator::kModOp){
        dumpInstructions(m_output_file.get(), "    rem t0, t0, t1\n");
    }
    else if(op == Operator::kAndOp){
        dumpInstructions(m_output_file.get(), "    and t0, t0, t1\n");
    }
    else if(op == Operator::kOrOp){
        dumpInstructions(m_output_file.get(), "    or t0, t0, t1\n");
    }
    else if(op == Operator::kLessOp){
        dumpInstructions(m_output_file.get(), "    slt t0, t0, t1\n");
    }
    else if(op == Operator::kLessOrEqualOp){
        dumpInstructions(m_output_file.get(), 
        "    sgt t2, t0, t1\n"
        "    xori t0, t2, 1\n");
    }
    else if(op == Operator::kGreaterOp){
        dumpInstructions(m_output_file.get(), "    sgt t0, t0, t1\n");
    }
    else if(op == Operator::kGreaterOrEqualOp){
        dumpInstructions(m_output_file.get(), 
        "    slt t2, t0, t1\n"
        "    xori t0, t2, 1\n");
    }
    else if(op == Operator::kEqualOp){
        dumpInstructions(m_output_file.get(), 
        "    sub t0, t0, t1\n"
        "    seqz t0, t0\n");
    }
    else if(op == Operator::kNotEqualOp){
        dumpInstructions(m_output_file.get(), 
        "    sub t0, t0, t1\n"
        "    snez t0, t0\n");
    }
    // store the result into the top of stack
    dumpInstructions(m_output_file.get(),
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)\n"
    );
    sp -= 4;
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {
    current_var_ref = VAR_REF_ENUM::TYPE_RHS;
    p_un_op.visitChildNodes(*this);
    dumpInstructions(m_output_file.get(),
    "    lw t0, 0(sp)\n"
    "    addi sp, sp, 4\n");
    sp += 4;
    Operator op = p_un_op.getOp();
    if(op == Operator::kNegOp){
        dumpInstructions(m_output_file.get(), "    neg t0, t0\n");
    }
    else if(op == Operator::kNotOp){
        dumpInstructions(m_output_file.get(), "    seqz t0, t0\n");
    }
    // store the result into the top of stack
    dumpInstructions(m_output_file.get(),
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)\n"
    );
    sp -= 4;
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation) {
    

    p_func_invocation.visitChildNodes(*this);
    const FunctionInvocationNode::ExprNodes& expr_nodes = p_func_invocation.getArguments();
    for(int i = 0; i < p_func_invocation.getArguments().size(); i++){
        // load the value to argument registers
        dumpInstructions(m_output_file.get(),
        "    lw %s, 0(sp)\n"
        "    addi sp, sp, 4\n", FuncParamRegs[i]);
        sp += 4;
    }
    // jal to call the function, and store the return value to t0 (expression node's results are stored to the register t0)
    dumpInstructions(m_output_file.get(),
                     "    jal ra, %s\n"
                     "    addi sp, sp, -4\n"
                     "    sw a0, 0(sp)\n",
                     p_func_invocation.getName().c_str());
    sp -= 4;
    return;
}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    SymbolEntry* entry = const_cast<SymbolEntry*>(m_symbol_manager.lookup(p_variable_ref.getName()));
    if(current_var_ref == VAR_REF_ENUM::TYPE_LHS){ // LHS, load variable address
        if(entry->getLevel() == 0){ // global
            dumpInstructions(m_output_file.get(),
            "    la t0, %s\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)\n", p_variable_ref.getNameCString());
        }
        else{ // local
            dumpInstructions(m_output_file.get(),
            "    addi t0, s0, %d\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)\n", entry->getStackPosition() - fp);
        }
    }
    else{ // RHS, load variable value
        if(entry->getLevel() == 0){ // global
            dumpInstructions(m_output_file.get(),
            "    la t1, %s\n"
            "    lw t0, 0(t1)\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)\n", p_variable_ref.getNameCString());
        }
        else{ // local
            dumpInstructions(m_output_file.get(),
            "    lw t0, %d(s0)\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)\n", entry->getStackPosition() - fp);
        }
    }
    sp -= 4;
}

void CodeGenerator::visit(AssignmentNode &p_assignment) {
    current_var_ref = VAR_REF_ENUM::TYPE_LHS;
    const_cast<VariableReferenceNode&>(p_assignment.getLvalue()).accept(*this); // visit left child
    current_var_ref = VAR_REF_ENUM::TYPE_RHS;
    const_cast<ExpressionNode&>(p_assignment.getExpr()).accept(*this);
    dumpInstructions(m_output_file.get(),
    "    lw t0, 0(sp)\n"
    "    addi sp, sp, 4\n"
    "    lw t1, 0(sp)\n"
    "    addi sp, sp, 4\n"
    "    sw t0, 0(t1)\n");
    sp += 8;
    return;
}

void CodeGenerator::visit(ReadNode &p_read) {
    current_var_ref = VAR_REF_ENUM::TYPE_LHS; // we need its address
    p_read.visitChildNodes(*this);
    dumpInstructions(m_output_file.get(),
    "jal ra, readInt\n"
    "lw t0, 0(sp)\n"
    "addi sp, sp, 4\n"
    "sw a0, 0(t0)\n");
    return;
}

void CodeGenerator::visit(IfNode &p_if) {
    const_cast<ExpressionNode&>(p_if.getCondition()).accept(*this);
    std::string label_true = "L" + std::to_string(++label_cnt);
    std::string label_false = "L" + std::to_string(++label_cnt);
    std::string next_section_label = "L" + std::to_string(++label_cnt);
    dumpInstructions(m_output_file.get(),
    "    lw t0, 0(sp)\n"
    "    addi sp, sp, 4\n"
    "    beqz t0, %s\n"
    , p_if.isFalseBodyPtrNull()? next_section_label.c_str(): label_false.c_str());
    sp += 4;
    dumpInstructions(m_output_file.get(), "%s:\n", label_true.c_str());
    const_cast<CompoundStatementNode&>(p_if.getTrueBody()).accept(*this);
    if(!p_if.isFalseBodyPtrNull()){
        dumpInstructions(m_output_file.get(), "    j %s\n", next_section_label.c_str());
        dumpInstructions(m_output_file.get(), "%s:\n", label_false.c_str());
        const_cast<CompoundStatementNode&>(p_if.getFalseBody()).accept(*this);
    }
    dumpInstructions(m_output_file.get(), "%s:\n", next_section_label.c_str());
    return;
}

void CodeGenerator::visit(WhileNode &p_while) {
    std::string label_true = "L" + std::to_string(++label_cnt);
    std::string label_false = "L" + std::to_string(++label_cnt);
    std::string start_label = "L" + std::to_string(++label_cnt);
    const_cast<ExpressionNode&>(p_while.getCondition()).accept(*this);
    dumpInstructions(m_output_file.get(),
    "%s:\n"
    "    lw t0, 0(sp)\n"
    "    addi sp, sp, 4\n"
    "    beqz t0, %s\n"
    , start_label.c_str(), label_false.c_str());
    sp += 4;
    dumpInstructions(m_output_file.get(), "%s:\n", label_true.c_str());
    const_cast<CompoundStatementNode&>(p_while.getBody()).accept(*this);
    const_cast<ExpressionNode&>(p_while.getCondition()).accept(*this);
    dumpInstructions(m_output_file.get(), "    j %s\n", start_label.c_str());
    dumpInstructions(m_output_file.get(), "%s:\n", label_false.c_str());
    return;
}

void CodeGenerator::visit(ForNode &p_for) {
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_for)));

    const_cast<DeclNode&>(p_for.getLoopVarDecl()).accept(*this);
    const_cast<AssignmentNode&>(p_for.getInitStmt()).accept(*this);
    SymbolEntry* entry = const_cast<SymbolEntry*>(m_symbol_manager.lookup(p_for.getInitStmt().getLvalue().getName()));
    std::string label_condition = "L" + std::to_string(++label_cnt);
    std::string label_body = "L" + std::to_string(++label_cnt);
    std::string label_end =  "L" + std::to_string(++label_cnt);
    std::string lowerbound_str(p_for.getLowerBound().getConstantValueCString());
    std::string upperbound_str(p_for.getUpperBound().getConstantValueCString());

    dumpInstructions(m_output_file.get(),
    "%s:\n"
    "    lw t0, %d(s0)\n"
    "    li t1, %d\n"
    "    bge t0, t1, %s\n"
    "%s:\n", label_condition.c_str(), entry->getStackPosition() - fp, std::stoi(upperbound_str), label_end.c_str(), label_body.c_str());
    const_cast<CompoundStatementNode&>(p_for.getBody()).accept(*this);
    dumpInstructions(m_output_file.get(),
    "    lw t0, %d(s0)\n"
    "    addi t0, t0, 1\n"
    "    sw t0, %d(s0)\n"
    "    j %s\n", entry->getStackPosition() - fp, entry->getStackPosition() - fp, label_condition.c_str());
    dumpInstructions(m_output_file.get(),
    "%s:\n", label_end.c_str());

    // Remove the entries in the hash table
    m_symbol_manager.popScope();
}

void CodeGenerator::visit(ReturnNode &p_return) {
    current_var_ref = VAR_REF_ENUM::TYPE_RHS;
    p_return.visitChildNodes(*this);
    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    mv a0, t0 \n"
                     "    lw ra, 124(sp)\n"
                     "    lw s0, 120(sp)\n"
                     "    addi sp, sp, 128\n"
                     "    jr ra\n");
    sp += 4;
    sp += 128;
    fp = sp + 128;
    offset = -12;
    return;
}
