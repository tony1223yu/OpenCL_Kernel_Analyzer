#ifndef __AST_Definition_H__
#define __AST_Definition_H__

typedef struct Program_node Program_node;
typedef struct TypeDescriptor TypeDescriptor;
typedef struct ArrayDesc_node ArrayDesc_node;
typedef struct ArrayDesc_node_list ArrayDesc_node_list;
typedef struct Constant_node Constant_node;
typedef struct Function_node Function_node;
typedef struct Parameter_node Parameter_node;
typedef struct Declaration_node Declaration_node;
typedef struct Statement_node Statement_node;
typedef struct ExpressionStatement ExpressionStatement;
typedef struct ReturnStatement ReturnStatement;
typedef struct IterationStatement IterationStatement;
typedef struct SelectionStatement SelectionStatement;
typedef struct Selection_node Selection_node;
typedef struct Expression_node Expression_node;
typedef struct FunctionInvocation_node FunctionInvocation_node;
typedef struct StructDeclaration_node StructDeclaration_node;
typedef struct StructMember_node StructMember_node;
typedef struct StructMember_node_list StructMember_node_list;
typedef struct Expression_node_list Expression_node_list;
typedef struct CompoundStatement CompoundStatement;
typedef struct Declaration_desc_node Declaration_desc_node;
typedef struct Declaration_desc_node_list Declaration_desc_node_list;
typedef struct Parameter_node_list Parameter_node_list;
typedef struct Declaration_node_list Declaration_node_list;
typedef struct TypeName_node TypeName_node;
typedef struct TypeNameTable TypeNameTable;

typedef enum OPENCL_DATA_TYPE OPENCL_DATA_TYPE;
typedef enum EXPRESSION_KIND EXPRESSION_KIND;
typedef enum STATEMENT_KIND STATEMENT_KIND;
typedef enum SELECTION_KIND SELECTION_KIND;
typedef enum ARRAY_DESC_KIND ARRAY_DESC_KIND;
typedef enum TYPE_DESC_KIND TYPE_DESC_KIND;
typedef enum ITERATION_STMT_KIND ITERATION_STMT_KIND;

TypeNameTable* typeTable;
Program_node* program;
int structNumber;

TypeDescriptor* CreateScalarTypeDesc(OPENCL_DATA_TYPE, char*);
TypeDescriptor* MixAndCreateTypeDesc(TypeDescriptor*, TypeDescriptor*);
Constant_node* CreateEmptyConstantNode(void);
Expression_node* CreateDirectExprNode(void*, Expression_node*, Expression_node*, EXPRESSION_KIND);
Expression_node* CreateNormalExprNode(EXPRESSION_KIND, Expression_node*, Expression_node*);
FunctionInvocation_node* CreateFunctionInvocation_node(char*, Expression_node_list*);
Expression_node_list* AppendExprNodeToList(Expression_node_list*, Expression_node*);
ExpressionStatement* AddToExprStmt(ExpressionStatement*, Expression_node*);
Statement_node* CreateStmtNode(void*, STATEMENT_KIND);
Declaration_node* CreateDeclNode(TypeDescriptor*, Declaration_desc_node_list*);
Declaration_node_list* AppenDeclNodeToList(Declaration_node_list*, Declaration_node*);
Declaration_desc_node_list* AppendDeclDescNodeToList(Declaration_desc_node_list*, Declaration_desc_node*);
Declaration_desc_node* CreateDeclDescNode(char*);
ArrayDesc_node_list* AppendArrayDescNodeToList(ArrayDesc_node_list*, ArrayDesc_node*);
ArrayDesc_node* CreateArrayDescNode(unsigned long, ARRAY_DESC_KIND);
Declaration_desc_node* AddArrayDescListToDeclDesc(Declaration_desc_node*, ArrayDesc_node_list*);
Declaration_desc_node* AddArrayDescToDeclDesc(Declaration_desc_node*, ArrayDesc_node*);
void GetUlongValueInExprNode(Expression_node*, unsigned long*);
Parameter_node_list* AppendParamNodeToList(Parameter_node_list*, Parameter_node*);
Parameter_node* CreateParamNode(TypeDescriptor*, Declaration_desc_node*);
IterationStatement* CreateIterStmt(ITERATION_STMT_KIND, void*, ExpressionStatement*, ExpressionStatement*, Statement_node*);
CompoundStatement* CreateCompoundStmt(Declaration_node*, Statement_node*);
CompoundStatement* MergeCompoundStmt(CompoundStatement*, CompoundStatement*);
Selection_node* CreateSelectionNode(SELECTION_KIND, ExpressionStatement*, Statement_node*);
SelectionStatement* CreateSelectionStmt(Selection_node*);
SelectionStatement* AddToSelectionStmt(SelectionStatement*, Selection_node*);
SelectionStatement* MergeSelectionStmt(SelectionStatement*, SelectionStatement*);
ReturnStatement* CreateReturnStmt(ExpressionStatement*);
Function_node* CreateFunctionNode(TypeDescriptor*, Declaration_desc_node*, CompoundStatement*);
void AddFuncNodeToProgram(Program_node*, Function_node*);
void AddDeclNodeToProgram(Program_node*, Declaration_node*);
Program_node* CreateProgramNode(void);
void AddStructDeclNode(Program_node*, char*, Declaration_node_list*);
void AddToTypeTable(TypeNameTable*, TypeDescriptor*, Declaration_desc_node_list*);
TypeName_node* CreateTypeNameNode(char*, TypeDescriptor*);
TypeNameTable* CreateTypeNameTable(void);
int CheckInTypeNameTable(TypeNameTable*, char*);
TypeDescriptor* GetTypeDescFromTypeNameTable(TypeNameTable*, char*);
TypeDescriptor* DuplicateTypeDesc(TypeDescriptor*);
ArrayDesc_node* DuplicateArrayDesc(ArrayDesc_node*);
Parameter_node* DuplicateParamNode(Parameter_node*);
Declaration_desc_node* DuplicateDeclDescNode(Declaration_desc_node*);

/* Garbage collection */
void DeleteProgramNode(Program_node*);
void DeleteStructDeclNode(StructDeclaration_node*);
void DeleteDeclNode(Declaration_node*);
void DeleteDeclDescNode(Declaration_desc_node*);
void DeleteFuncNode(Function_node*);
void DeleteTypeDesc(TypeDescriptor*);
void DeleteArrayDescNode(ArrayDesc_node*);
void DeleteParamNode(Parameter_node*);
void DeleteStmtNode(Statement_node*);
void DeleteCompoundStmt(CompoundStatement*);
void DeleteIterStmt(IterationStatement*);
void DeleteSelectionStmt(SelectionStatement*);
void DeleteSelectionNode(Selection_node*);
void DeleteExprStmt(ExpressionStatement*);
void DeleteReturnStmt(ReturnStatement*);
void DeleteExprNode(Expression_node*);
void DeleteFuncInvocationNode(FunctionInvocation_node*);
void DeleteConstantNode(Constant_node*);
void DeleteTypeNameTable(TypeNameTable*);
void DeleteTypeNameNode(TypeName_node*);

/* Visualizing the AST */
void DebugProgramNode(Program_node*);
void DebugFuncNode(Function_node*);
void DebugCompoundStmt(CompoundStatement*, int);
void DebugIterStmt(IterationStatement*, int);
void DebugSelectionStmt(SelectionStatement*, int);
void DebugSelectionNode(Selection_node*, int);
void DebugStmtNode(Statement_node*, int);
void DebugExprStmt(ExpressionStatement*, int);
void DebugReturnStmt(ReturnStatement*, int);
void DebugExprNode(Expression_node*, int);
void DebugTypeDesc(TypeDescriptor*, char*);
void DebugExprKind(EXPRESSION_KIND, char*);
void DebugParamNode(Parameter_node*, int);
void DebugDeclNode(Declaration_node*, int);
void DebugConstantNode(Constant_node*, int);
void DebugFuncInvocationNode(FunctionInvocation_node*, int);
void DebugStructDeclNode(StructDeclaration_node*);

enum OPENCL_DATA_TYPE
{
    CONST_TYPE_SIZE_MASK = 0xFFF,
    CONST_SIGNED_INTEGER_MASK = 0x1000,
    BOOL_TYPE,
    CHAR_TYPE,
    SHORT_TYPE,
    INT_TYPE,
    LONG_TYPE,
    CONST_UNSIGNED_INTEGER_MASK = 0x2000,
    EMPTY,      /* To compare with BOOL_TYPE */
    UCHAR_TYPE,
    USHORT_TYPE,
    UINT_TYPE,
    ULONG_TYPE,
    CONST_FLOAT_MASK = 0x4000,
    HALF_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    CONST_OTHER_MASK = 0x8000,
    NONE_TYPE,
    POINTER_TYPE,
    STRUCT_TYPE,
    UNION_TYPE,
    VOID_TYPE,
    CHAR2_TYPE,
    CHAR4_TYPE,
    CHAR8_TYPE,
    CHAR16_TYPE,
    UCHAR2_TYPE,
    UCHAR4_TYPE,
    UCHAR8_TYPE,
    UCHAR16_TYPE,
    SHORT2_TYPE,
    SHORT4_TYPE,
    SHORT8_TYPE,
    SHORT16_TYPE,
    USHORT2_TYPE,
    USHORT4_TYPE,
    USHORT8_TYPE,
    USHORT16_TYPE,
    INT2_TYPE,
    INT4_TYPE,
    INT8_TYPE,
    INT16_TYPE,
    UINT2_TYPE,
    UINT4_TYPE,
    UINT8_TYPE,
    UINT16_TYPE,
    LONG2_TYPE,
    LONG4_TYPE,
    LONG8_TYPE,
    LONG16_TYPE,
    ULONG2_TYPE,
    ULONG4_TYPE,
    ULONG8_TYPE,
    ULONG16_TYPE,
    FLOAT2_TYPE,
    FLOAT4_TYPE,
    FLOAT8_TYPE,
    FLOAT16_TYPE,
    DOUBLE2_TYPE,
    DOUBLE4_TYPE,
    DOUBLE8_TYPE,
    DOUBLE16_TYPE,
};

enum EXPRESSION_KIND
{
    MEMORY_OP = 0x0000,

    ARITHMETIC_OP_MASK = 0x1000,
    NONE_OP,
    ADDITION_OP,
    SUBTRACTION_OP,
    MULTIPLICATION_OP,
    DIVISION_OP,
    MODULAR_OP,
    SHIFT_LEFT_OP,
    SHIFT_RIGHT_OP,
    POST_INCREASE_OP,
    POST_DECREASE_OP,
    PRE_INCREASE_OP,
    PRE_DECREASE_OP,
    BITWISE_AND_OP,
    BITWISE_XOR_OP,
    BITWISE_OR_OP,
    BITWISE_COMPLEMENT_OP,
    UNARY_PLUS_OP,
    UNARY_MINUS_OP,
    MAD_OP,

    LOGICAL_OP_MASK = 0x2000,
    LESS_OP,
    LESS_EQUAL_OP,
    GREATER_OP,
    GREATER_EQUAL_OP,
    EQUAL_OP,
    NOT_EQUAL_OP,
    LOGICAL_AND_OP,
    LOGICAL_OR_OP,
    LOGICAL_COMPLEMENT_OP,

    EXPRESSION_MASK = 0x4000,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_CONSTANT,
    EXPRESSION_SUBSCRIPT,
    EXPRESSION_FUNCTION,
    EXPRESSION_MEMBER,
    EXPRESSION_TYPECAST,
    EXPRESSION_EXPRSTMT,

    ASSIGNMENT_MASK = 0x8000,
    ASSIGNMENT_NONE,
    ASSIGNMENT_ADD,
    ASSIGNMENT_SUB,
    ASSIGNMENT_MUL,
    ASSIGNMENT_DIV,
    ASSIGNMENT_MOD,
    ASSIGNMENT_LEFT,
    ASSIGNMENT_RIGHT,

    /* bitwise op */
    ASSIGNMENT_AND,
    ASSIGNMENT_XOR,
    ASSIGNMENT_OR,
};

enum ARRAY_DESC_KIND
{
    ARRAY_DESC_POINTER = 0,
    ARRAY_DESC_ARRAY,
    ARRAY_DESC_FUNC_POINTER
};

enum TYPE_DESC_KIND
{
    TYPE_WITH_PARAM = 0,
    TYPE_WITHOUT_PARAM
};

enum ITERATION_STMT_KIND
{
    FOR_LOOP_WITH_DECL = 0,
    FOR_LOOP_WITHOUT_DECL,
    WHILE_LOOP,
    DO_WHILE_LOOP
};

enum SELECTION_KIND
{
    SELECTION_WITH_COND = 0,
    SELECTION_WITHOUT_COND
};

enum STATEMENT_KIND
{
    NORMAL_STMT_MASK = 0x1000,
    ITERATION_STMT,
    SELECTION_STMT,
    EXPRESSION_STMT,
    COMPOUND_STMT,

    CONTROL_STMT_MASK = 0x2000,
    RETURN_STMT, /* with return expression */
    EMPTY_GOTO_STMT,
    EMPTY_CONTINUE_STMT,
    EMPTY_BREAK_STMT,
    EMPTY_RETURN_STMT
};

struct TypeNameTable
{
    TypeName_node* type_node_head;
    TypeName_node* type_node_tail;
};

struct TypeName_node
{
    char* identifier_name;
    TypeDescriptor* identifier_type;
    TypeName_node* next;
};

// whole program
struct Program_node
{
    StructDeclaration_node* struct_head;
    StructDeclaration_node* struct_tail;
    Declaration_node* declaration_head;
    Declaration_node* declaration_tail;
    Function_node* function_head;
    Function_node* function_tail;
};

struct StructDeclaration_node
{
    char* struct_name;
    Declaration_node* member_head;
    Declaration_node* member_tail;
    StructDeclaration_node* next;
};

struct TypeDescriptor
{
    OPENCL_DATA_TYPE type;
    char* struct_name;

    // for both pointer and array
    ArrayDesc_node* array_desc_head;
    ArrayDesc_node* array_desc_tail;

    TYPE_DESC_KIND kind;
    // parameter for function definition
    Parameter_node* parameter_head;
    Parameter_node* parameter_tail;
};

struct ArrayDesc_node_list
{
    ArrayDesc_node* array_desc_head;
    ArrayDesc_node* array_desc_tail;
};

// size of each dimension
struct ArrayDesc_node
{
    // 0 in size means that the length is unknown (in function prototype)
    unsigned long size;
    ARRAY_DESC_KIND desc_kind;
    Parameter_node* parameter_head;
    Parameter_node* parameter_tail;
    ArrayDesc_node* next;
};

struct Function_node
{
    char* function_name;
    TypeDescriptor* return_type;
    Parameter_node* parameter_head;
    Parameter_node* parameter_tail;
    CompoundStatement* content_statement;
    Function_node* next;
};

struct Parameter_node
{
    TypeDescriptor* parameter_type;
    Declaration_desc_node* parameter_desc;
    Parameter_node* next;
};

struct Parameter_node_list
{
    Parameter_node* parameter_head;
    Parameter_node* parameter_tail;
};

struct Declaration_node_list
{
    Declaration_node* declaration_head;
    Declaration_node* declaration_tail;
};

struct Declaration_node
{
    // common type for all the variable
    TypeDescriptor* declaration_type;

    Declaration_desc_node* declaration_desc_head;
    Declaration_desc_node* declaration_desc_tail;
    Declaration_node* next;
};

struct Declaration_desc_node
{
    // individual type for each variable
    TypeDescriptor* identifier_type;
    char* identifier_name;
    Expression_node* init_expression;
    Declaration_desc_node* next;
};

struct Statement_node
{
    STATEMENT_KIND statement_kind;
    union stmt
    {
        IterationStatement* iteration_stmt;
        SelectionStatement* selection_stmt;
        ExpressionStatement* expression_stmt;
        CompoundStatement* compound_stmt;
        ReturnStatement* return_stmt;
    } stmt;
    Statement_node* next;
};

struct CompoundStatement
{
    Declaration_node* declaration_head;
    Declaration_node* declaration_tail;
    Statement_node* statement_head;
    Statement_node* statement_tail;
};

struct ReturnStatement
{
    Expression_node* expression_head;
    Expression_node* expression_tail;
};

struct ExpressionStatement
{
    Expression_node* expression_head;
    Expression_node* expression_tail;
};

struct IterationStatement
{
    ITERATION_STMT_KIND kind;

    union
    {
        Declaration_node* declaration;
        ExpressionStatement* expression;
    } init;

    ExpressionStatement* terminated_expression;
    ExpressionStatement* step_expression;
    Statement_node* content_statement;
};

struct SelectionStatement
{
    Selection_node* selection_head;
    Selection_node* selection_tail;
};

struct Selection_node
{
    SELECTION_KIND condition_kind;
    ExpressionStatement* condition_expression;
    Statement_node* content_statement;
    Selection_node* next;
};

struct Expression_node
{
    EXPRESSION_KIND expression_kind;
    Expression_node* left_operand;
    Expression_node* right_operand;

    union direct_expr
    {
        char* identifier;
        ExpressionStatement* subscript;
        FunctionInvocation_node* function;
        Constant_node* constant;
        char* member;
        TypeDescriptor* target_type;
        ExpressionStatement* expr_stmt;
    } direct_expr;

    Expression_node* next;
};

struct Constant_node
{
    TypeDescriptor* constant_type;
    union
    {
        long long_val;
        unsigned long ulong_val;
        double double_val;
        /* TODO vector type constant */
    } value;
};

struct Expression_node_list
{
    Expression_node* expression_head;
    Expression_node* expression_tail;
};

struct Declaration_desc_node_list
{
    Declaration_desc_node* declaration_desc_head;
    Declaration_desc_node* declaration_desc_tail;
};

struct FunctionInvocation_node
{
    char* name;
    Expression_node* argument_head;
    Expression_node* argument_tail;
};

#endif
