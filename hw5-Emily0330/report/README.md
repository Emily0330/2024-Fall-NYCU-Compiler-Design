# hw5 report

|||
|-:|:-|
|Name|your full name |
|ID|your student id|

## How much time did you spend on this project

20 hours for coding & 5 hours for report.

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way.
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here.

In this assignment, I mainly modified `CodeGenerator.cpp`. I implemented the code generation using **visitor pattern**, like we did in the semantic analysis in Assignment 4. I dumped the corresponding RISC-V assembly language of each node when traversing them. Every node in code generation stage is traversed in **post order**. Also, the code generation is implemented using a **stack machine**.

### Table of Contents
- [Stack machine variables](#stack-machine-variables)
- [Declarations of Variables and Constants](#declarations-of-variables-and-constants)
- [Assignment](#assignment)
- [Function](#function)
    - [Functions share same symbol table with their body](#functions-share-same-symbol-table-with-their-compound-statement-body)
    - [Early return](#early-return)
- [Loop and Condition](#loop-and-condition)
    - [Visit child nodes seperately](#visit-child-nodes-seperately)
    - [Unique label for each basic block](#unique-label-for-each-basic-block)

### Stack machine variables
To implement the stack machine, I defined the following three variables. The usage and definitions of these variables are shown in the comments below.  
I just increment/decrement these values according to the assembly language of each node.
```c++
int sp = 0; // current stack pointer
int fp = 0; // current frame pointer, s0
int offset = -12; // the address fp + offset is the next place to store local variables
```

### Declarations of Variables and Constants
I dump the instructions of variable/constant declarations when traversing to the corresponding variable nodes. There are four different categories of declarations:  
1. global constants
2. global variables
3. local constants
4. local variables
5. function parameters  

    The way of identifying which category a variable belongs is shown below:  

    ```c++
    void CodeGenerator::visit(VariableNode &p_variable) {
        // ...
        // determine the kind of variables
        if(entry->getKind() == SymbolEntry::KindEnum::kParameterKind) // function parameters
        else if(entry->getLevel() == 0){ // global
            if (p_variable.getConstantPtr() == nullptr) // global variable
            else // global constant
        }
        else{ // local
            if(p_variable.getConstantPtr() != nullptr) // local constant
            // both local constants and variables
        }
    }
    ```

### Assignment
To deal with assignment, we have to push address of variable at LHS and value of  expression at RHS into stack. To achieve this, I use a global Enum variable `current_var_ref` to indicate whether the current node is on the left hand side or right hand side of the assignment.

```c++
typedef enum VAR_REF_ENUM{
    TYPE_LHS = 500,
    TYPE_RHS
};

void CodeGenerator::visit(AssignmentNode &p_assignment) {
    current_var_ref = VAR_REF_ENUM::TYPE_LHS; // change current_var_ref to TYPE_LFS, indicating we will push the address of the variable reference child node into stack
    const_cast<VariableReferenceNode&>(p_assignment.getLvalue()).accept(*this); // visit left child (a variable reference node)
    current_var_ref = VAR_REF_ENUM::TYPE_RHS;
    const_cast<ExpressionNode&>(p_assignment.getExpr()).accept(*this);
    // dump instructions of the assignment itself
    return;
}
```

### Function
To support function definitions and function calls, I modified the visit member functions of `FunctionNode`, `FunctionInvocationNode`, `ReturnNode`, and `CompoundStatementNode`. The following are the points I want to discuss about my implementation.

#### Functions share same symbol table with their compound statement body
I use a global boolean variable `isFunction` to determine whether the compound statement node is the body of a function. (`isFunction` is set to `true` in the visit of function node, before the visit of function body)
    ```c++
    void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
        // Reconstruct the scope for looking up the symbol entry.
        if(!isFunction){ // not the body of a function (has its own symbol table)
            m_symbol_manager.pushScope(
                std::move(m_symbol_table_of_scoping_nodes.at(&p_compound_statement)));

            p_compound_statement.visitChildNodes(*this);

            m_symbol_manager.popScope();
        }
        else{ // the compound statement is the body of a function
            // turn isFunction off because there may be other compound statement 
            // nodes inside the function that we need to traverse.
            isFunction = false; 
            p_compound_statement.visitChildNodes(*this);
        }
    }
    ```
#### Early return  
Instead of moving the stack pointer back to the top of the last stack and jumping back to the return address at the end of the function node traversal, we should do this in the traversal of the first return statement.  
    ```c++
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
    ```

### Loop and Condition
#### Visit child nodes seperately
In order to dump the assembly language in the correct order, instead of using the `visitChildNodes` function of these classes, I visit the different child nodes of them seperately. Take `while` for example:

    ```c++
    void CodeGenerator::visit(WhileNode &p_while) {
        // ...
        const_cast<ExpressionNode&>(p_while.getCondition()).accept(*this); // traverse the condition expression node
        dumpInstructions(m_output_file.get(), // dump instruction of the condition 
        "%s:\n"
        "    lw t0, 0(sp)\n"
        "    addi sp, sp, 4\n"
        "    beqz t0, %s\n"
        , start_label.c_str(), label_false.c_str());
        sp += 4;
        dumpInstructions(m_output_file.get(), "%s:\n", label_true.c_str()); // dump the true label
        const_cast<CompoundStatementNode&>(p_while.getBody()).accept(*this); // traverse the body compound statement node
        const_cast<ExpressionNode&>(p_while.getCondition()).accept(*this); // traverse the condition expression node again to dump the child node's assembly code 
        dumpInstructions(m_output_file.get(), "    j %s\n", start_label.c_str()); // jump back to condition
        dumpInstructions(m_output_file.get(), "%s:\n", label_false.c_str()); // end label
        return;
    }
    ```

#### Unique label for each basic block  
Each for loop contains three labels (condition, body, and end label); each while loop contains three labels (condition, true, false label); and each if condition contains three labels (true, false, next_section label).  
I use a global variable `label_cnt` to keep track of the count of the current label and increment it whenever a new label is requested. This guarantees that every label is unique.  
    ```c++
    std::string LABEL_NAME = "L" + std::to_string(++label_cnt); // define a label name
    ```

## What is the hardest you think in this project

> Not required, but bonus point may be given.
1. Debug: Functions share same symbol table with their compound statement body.  
這個bug de了快6個小時，測資 03_function.p 的 compiler output 一直跳出以下錯誤:
    ```bash
    terminate called after throwing an instance of 'std::out_of_range'
    what():  _Map_base::at
    ```
    本來想嘗試 print 點東西出來但是沒辦法，output 就只有這個錯誤訊息，再加上 CompoundStatementNode 的 visit function 是原本 template 就寫好的，就沒有懷疑那裡需要改。最後上 GitHub issue 看到有人問了這個問題，有好心人解答原因 (因為 function 和其 body 的 compound statement body 共用一個 symbol table)，才用一個  global boolean variable `isFunction` 來決定在 compound statement node 要不要 push & pop scope。

2. 寫組語的過程  
尤其是哪些組語要在哪個 node 寫，因為一個 statement 可能涉及多個 nodes 的 traverse，而 GitHub README 上的組語或是自己寫C語言編譯出來的組語都是該 statement 涉及的所有組語寫在一起，不知道哪個 node 要寫哪一段組語。  
另外，自己寫 C 語言編譯出來的組語跟 GitHub README 上的組語沒有很像，有時候會思考要參考誰的寫才對 QwQ
## Feedback to T.A.s

> Not required, but bonus point may be given.  
謝謝 TA，作業三四五好難，我會好好重學 OOP 的 >< 但是第一次寫組語，好酷！
