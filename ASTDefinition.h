#ifndef __AST_Definition_H__
#define __AST_Definition_H__

typedef struct Program_node Program_node;
typedef struct TypeDescriptor_node TypeDescriptor_node;
typedef struct ArrayDescriptor_node ArrayDescriptor_node;
typedef struct Constant_node Constant_node;
typedef struct Function_node Function_node;
typedef struct Parameter_node Parameter_node;
typedef struct Declaration_node Declaration_node;
typedef struct Statement_node Statement_node;
typedef struct ReturnStatement ReturnStatement;
typedef struct ExpressionStatement ExpressionStatement;
typedef struct IterationStatement IterationStatement;
typedef struct SelectionStatement SelectionStatement;
typedef struct Selection_node Selection_node;
typedef struct Expression_node Expression_node;
typedef struct FunctionInvocation_node FunctionInvocation_node;
typedef struct StructDescriptor StructDescriptor;
typedef struct StructMember StructMember;
typedef struct Expression_node_list Expression_node_list;

typedef enum OPENCL_DATA_TYPE OPENCL_DATA_TYPE;
typedef enum EXPRESSION_KIND EXPRESSION_KIND;
typedef enum STATEMENT_KIND STATEMENT_KIND;

TypeDescriptor_node* CreateScalarTypeDescriptor(OPENCL_DATA_TYPE);
Constant_node* CreateEmptyConstantNode(void);
Expression_node* CreateDirectExpressionNode(void*, Expression_node*, Expression_node*, EXPRESSION_KIND);
Expression_node* CreateNormalExpressionNode(EXPRESSION_KIND, Expression_node*, Expression_node*);
FunctionInvocation_node* CreateFunctionInvocation_node(char*, Expression_node_list*);
Expression_node_list* AppendExpressionNodeToList(Expression_node_list*, Expression_node*);ExpressionStatement* AddToExpressionStatement(ExpressionStatement*, Expression_node*);

enum OPENCL_DATA_TYPE
{
    NONE_TYPE = 0x0,
    STRUCT_TYPE,
    UNION_TYPE,
    BOOL_TYPE = 0x1000,
    HALF_TYPE,
    VOID_TYPE,
    CHAR_TYPE,
    CHAR2_TYPE,
    CHAR4_TYPE,
    CHAR8_TYPE,
    CHAR16_TYPE,
    UCHAR_TYPE,
    UCHAR2_TYPE,
    UCHAR4_TYPE,
    UCHAR8_TYPE,
    UCHAR16_TYPE,
    SHORT_TYPE,
    SHORT2_TYPE,
    SHORT4_TYPE,
    SHORT8_TYPE,
    SHORT16_TYPE,
    USHORT_TYPE,
    USHORT2_TYPE,
    USHORT4_TYPE,
    USHORT8_TYPE,
    USHORT16_TYPE,
    INT_TYPE,
    INT2_TYPE,
    INT4_TYPE,
    INT8_TYPE,
    INT16_TYPE,
    UINT_TYPE,
    UINT2_TYPE,
    UINT4_TYPE,
    UINT8_TYPE,
    UINT16_TYPE,
    LONG_TYPE,
    LONG2_TYPE,
    LONG4_TYPE,
    LONG8_TYPE,
    LONG16_TYPE,
    ULONG_TYPE,
    ULONG2_TYPE,
    ULONG4_TYPE,
    ULONG8_TYPE,
    ULONG16_TYPE,
    FLOAT_TYPE = 0x2000,
    FLOAT2_TYPE,
    FLOAT4_TYPE,
    FLOAT8_TYPE,
    FLOAT16_TYPE,
    DOUBLE_TYPE,
    DOUBLE2_TYPE,
    DOUBLE4_TYPE,
    DOUBLE8_TYPE,
    DOUBLE16_TYPE,
};

enum EXPRESSION_KIND
{
    OP_BASE = 0x1000,
    NONE_OP,
    ADDITION_OP,
    SUBTRACTION_OP,
    MULTIPLICATION_OP,
    DIVISION_OP,
    MODULAR_OP,
    POST_INCREASE_OP,
    POST_DECREASE_OP,
    PRE_INCREASE_OP,
    PRE_DECREASE_OP,
    SHIFT_LEFT_OP,
    SHIFT_RIGHT_OP,
    LESS_OP,
    LESS_EQUAL_OP,
    GREATER_OP,
    GREATER_EQUAL_OP,
    EQUAL_OP,
    NOT_EQUAL_OP,
    BITWISE_AND_OP,
    BITWISE_XOR_OP,
    BITWISE_OR_OP,
    LOGICAL_AND_OP,
    LOGICAL_OR_OP,
    MEMORY_OP,

    EXPRESSION_BASE = 0x2000,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_CONSTANT,
    EXPRESSION_SUBSCRIPT,
    EXPRESSION_FUNCTION,
    EXPRESSION_MEMBER,
    EXPRESSION_TYPECAST,
    EXPRESSION_EXPRSTMT,

    ASSIGNMENT_BASE = 0x4000,
    ASSIGNMENT_NONE,
    ASSIGNMENT_MUL,
    ASSIGNMENT_DIV,
    ASSIGNMENT_MOD,
    ASSIGNMENT_ADD,
    ASSIGNMENT_SUB,
    ASSIGNMENT_LEFT,
    ASSIGNMENT_RIGHT,

    /* bitwise op */
    ASSIGNMENT_AND,
    ASSIGNMENT_XOR,
    ASSIGNMENT_OR,
};

enum STATEMENT_KIND
{
    /* w/ stmt descriptor */
    ITERATION_STMT,
    SELECTION_STMT,
    EXPRESSION_STMT,
    RETURN_STMT, /* with return expression */

    /* w/o stmt descriptor */
    EMPTY_GOTO_STMT,
    EMPTY_CONTINUE_STMT,
    EMPTY_BREAK_STMT,
    EMPTY_RETURN_STMT
};

// whole program
struct Program_node
{
    StructDescriptor* struct_head;
    StructDescriptor* struct_tail;
    Declaration_node* declaration_head;
    Declaration_node* declaration_tail;
    Function_node* function_head;
    Function_node* function_tail;
};

struct StructDescriptor
{
    char* struct_name;
    StructMember* member_head;
    StructMember* member_tail;
    StructDescriptor* next;
};

struct StructMember
{
    TypeDescriptor_node* structMember_type;
    char* structMember_name;
    StructMember* next;
};

struct TypeDescriptor_node
{
    OPENCL_DATA_TYPE type;
    char* struct_name;
    ArrayDescriptor_node* arrayDescriptor;
};

struct ArrayDescriptor_node
{
    int arrayDimension;
    long long int* sizeofDimension;
};

// single function definition
struct Function_node
{
    char* function_name;
    Parameter_node* parameter_head;
    Parameter_node* parameter_tail;
    Declaration_node* declaration_head;
    Declaration_node* declaration_tail;
    Statement_node* statement_head;
    Statement_node* statement_tail;
};

struct Parameter_node
{
    TypeDescriptor_node* parameter_type;
    char* parameter_name;
    Parameter_node* next;
};

struct Declaration_node
{
    TypeDescriptor_node* identifier_type;
    char* identifier_name;
    Expression_node* init_expression;
    Declaration_node* next;
};

struct Statement_node
{
    STATEMENT_KIND statement_kind;
    union stmt
    {
        IterationStatement* iteration_stmt;
        SelectionStatement* selection_stmt;
        ExpressionStatement* expression_stmt;
        ReturnStatement* return_stmt;
    } stmt;
    Statement_node* next;
};

struct ReturnStatement
{
    /* should be left to right, and return the value of rightmost expression */
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
    Declaration_node* init_declaration_head;
    Declaration_node* init_declaration_tail;
    ExpressionStatement* init_expression_head;
    ExpressionStatement* init_expression_tail;

    ExpressionStatement* terminated_expression;

    ExpressionStatement* step_expression;

    Statement_node* statement_head;
    Statement_node* statement_tail;
};

struct SelectionStatement
{
    Selection_node* selection_head;
    Selection_node* selection_tail;
};

struct Selection_node
{
    ExpressionStatement* condition_expression;
    Statement_node* statement_head;
    Statement_node* statement_tail;
};

struct Expression_node
{
    /* should calculate the rvalue first */
    EXPRESSION_KIND expression_kind;
    Expression_node* left_operand;
    Expression_node* right_operand;

    union direct_expr
    {
        char* identifier;
        Expression_node* subscript;
        FunctionInvocation_node* function;
        Constant_node* constant;
        char* member;
        TypeDescriptor_node* target_type;
        ExpressionStatement* expr_stmt;
    } direct_expr;

    Expression_node* next;
};

struct Constant_node
{
    TypeDescriptor_node* constant_type;
    union
    {
        int int_val;
        unsigned uint_val;
        long long_val;
        unsigned long ulong_val;
        float float_val;
        double double_val;
        /* TODO vector type constant */
    } value;
};

struct Expression_node_list
{
    Expression_node* expression_head;
    Expression_node* expression_tail;
};

struct FunctionInvocation_node
{
    char* name;
    Expression_node* argument_head;
    Expression_node* argument_tail;
};

#endif
