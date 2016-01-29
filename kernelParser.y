%{
#include <stdio.h>
#include <stdlib.h>
#include "ASTDefinition.h"

Expression_node* CreateDirectExprNode(void* ptr, Expression_node* left, Expression_node* right, EXPRESSION_KIND kind)
{
    Expression_node* ret = (Expression_node*) malloc(sizeof(Expression_node));

    ret->expression_kind = kind;
    ret->left_operand = left;
    ret->right_operand = right;
    ret->next = NULL;

    switch (kind)
    {
        case EXPRESSION_IDENTIFIER:
            ret->direct_expr.identifier = (char*)(ptr);
            break;
        case EXPRESSION_MEMBER:
            ret->direct_expr.member = (char*)(ptr);
            break;
        case EXPRESSION_CONSTANT:
            ret->direct_expr.constant = (Constant_node*)(ptr);
            break;
        case EXPRESSION_SUBSCRIPT:
            ret->direct_expr.subscript = (Expression_node*)(ptr);
            break;
        case EXPRESSION_FUNCTION:
            ret->direct_expr.function = (FunctionInvocation_node*)(ptr);
            break;
        case EXPRESSION_TYPECAST:
            ret->direct_expr.target_type = (TypeDesc_node*)(ptr);
            break;
        case EXPRESSION_EXPRSTMT:
            ret->direct_expr.expr_stmt = (ExpressionStatement*)(ptr);
            break;
        default:
            fprintf(stderr, "[Error] Unrecognized expression kind in %s\n", __func__);
            break;
    }

    return ret;
}

Expression_node* CreateNormalExprNode(EXPRESSION_KIND kind, Expression_node* left, Expression_node* right)
{
    Expression_node* ret = (Expression_node*) malloc(sizeof(Expression_node));

    ret->expression_kind = kind;
    ret->left_operand = left;
    ret->right_operand = right;
    ret->next = NULL;

    // TODO: constant folding

    /* set the content of the union space to 0 */
    ret->direct_expr.identifier = NULL;
}

FunctionInvocation_node* CreateFunctionInvocation_node(char* function_name, Expression_node_list* arguments)
{
    FunctionInvocation_node* ret = (FunctionInvocation_node*) malloc(sizeof(FunctionInvocation_node));
    ret->name = function_name;
    ret->argument_head = arguments->expression_head;
    ret->argument_tail = arguments->expression_tail;
    return ret;
}

Expression_node_list* AppendExprNodeToList(Expression_node_list* origin_list, Expression_node* new_node)
{
    if (origin_list == NULL)
    {
        Expression_node_list* ret = (Expression_node_list*) malloc(sizeof(Expression_node_list));
        ret->expression_head = new_node;
        ret->expression_tail = new_node;
        return ret;
    }
    else
    {
        origin_list->expression_tail->next = new_node;
        origin_list->expression_tail = new_node;
        return origin_list;
    }
}

ExpressionStatement* AddToExprStmt(ExpressionStatement* origin_stmt, Expression_node* new_node)
{
    if (origin_stmt == NULL)
    {
        ExpressionStatement* ret = (ExpressionStatement*) malloc(sizeof(ExpressionStatement));
        ret->expression_head = new_node;
        ret->expression_tail = new_node;
        return ret;
    }
    else
    {
        origin_stmt->expression_tail->next = new_node;
        origin_stmt->expression_tail = new_node;
        return origin_stmt;
    }
}

Statement_node* CreateStmtNode(void* ptr, STATEMENT_KIND kind)
{
    Statement_node* ret = (Statement_node*) malloc(sizeof(Statement_node));
    ret->statement_kind = kind;
    ret->next = NULL;

    switch (kind)
    {
        case ITERATION_STMT:
            ret->stmt.iteration_stmt = (IterationStatement*)(ptr);
            break;
        case SELECTION_STMT:
            ret->stmt.selection_stmt = (SelectionStatement*)(ptr);
            break;
        case EXPRESSION_STMT:
            ret->stmt.expression_stmt = (ExpressionStatement*)(ptr);
            break;
        case RETURN_STMT:
            ret->stmt.return_stmt = (ReturnStatement*)(ptr);
            break;
        case COMPOUND_STMT:
            ret->stmt.compound_stmt = (CompoundStatement*)(ptr);
            break;
    }
    return ret;
}

Declaration_node* CreateDeclNode(TypeDesc_node* type, Declaration_desc_node_list* desc_list)
{
    Declaration_node* ret = (Declaration_node*) malloc(sizeof(Declaration_node));
    ret->declaration_type = type;
    ret->next = NULL;

    if (desc_list == NULL)
    {
        ret->declaration_desc_head = NULL;
        ret->declaration_desc_tail = NULL;
    }
    else
    {
        ret->declaration_desc_head = desc_list->declaration_desc_head;
        ret->declaration_desc_tail = desc_list->declaration_desc_tail;
        free (desc_list);
    }
    return ret;
}

Declaration_desc_node_list* AppendDeclDescNodeToList(Declaration_desc_node_list* origin_list, Declaration_desc_node* new_desc)
{
    if (origin_list == NULL)
    {
        Declaration_desc_node_list* ret = (Declaration_desc_node_list*) malloc(sizeof(Declaration_desc_node_list));
        ret->declaration_desc_head = new_desc;
        ret->declaration_desc_tail = new_desc;
        return ret;
    }
    else
    {
        origin_list->declaration_desc_tail->next = new_desc;
        origin_list->declaration_desc_tail = new_desc;
        return origin_list;
    }
}

Declaration_desc_node* CreateDeclDescNode(char* identifier_name)
{
    Declaration_desc_node* ret = (Declaration_desc_node*) malloc(sizeof(Declaration_desc_node));
    ret->identifier_type = NULL;
    ret->identifier_name = identifier_name;
    ret->init_expression = NULL;
    ret->next = NULL;
    return ret;
}

ArrayDesc_node* CreateArrayDescNode(unsigned long size, ARRAY_DESC_KIND kind)
{
    ArrayDesc_node* ret = (ArrayDesc_node*) malloc(sizeof(ArrayDesc_node));
    ret->size = size;
    ret->next = NULL;
    ret->desc_kind = kind;
    return ret;
}

ArrayDesc_node_list* AppendArrayDescNodeToList(ArrayDesc_node_list* origin_list, ArrayDesc_node* new_node)
{
    if (origin_list == NULL)
    {
        ArrayDesc_node_list* ret = (ArrayDesc_node_list*) malloc(sizeof(ArrayDesc_node_list));
        ret->array_desc_head = new_node;
        ret->array_desc_tail = new_node;
        return ret;
    }
    else
    {
        origin_list->array_desc_tail->next = new_node;
        origin_list->array_desc_tail = new_node;
        return origin_list;
    }
}

Declaration_desc_node* AddParamToDeclDesc(Declaration_desc_node* decl_desc, Parameter_node_list* param_node_list)
{
    Declaration_desc_node* ret;
    TypeDesc_node* currType;
    
    if (decl_desc == NULL)
    {
        // Abstract declaration descriptor
        ret = CreateDeclDescNode(NULL);
    }
    else
    {
        ret = decl_desc;
    }
    
    currType = decl_desc->identifier_type;
    if (currType == NULL)
    {
        ret->identifier_type = (TypeDesc_node*) malloc(sizeof(TypeDesc_node));
        ret->identifier_type->type = NONE_TYPE;
        ret->identifier_type->kind = TYPE_WITH_PARAM;
        ret->identifier_type->struct_name = NULL;
        ret->identifier_type->array_desc_head = NULL;
        ret->identifier_type->array_desc_tail = NULL;
        if (param_node_list == NULL)
        {
            ret->identifier_type->parameter_head = NULL;
            ret->identifier_type->parameter_tail = NULL;
        }
        else
        {
            ret->identifier_type->parameter_head = param_node_list->parameter_head;
            ret->identifier_type->parameter_tail = param_node_list->parameter_tail;
        }
    }
    else
    {
        if (currType->parameter_head == NULL)
        {
            if (param_node_list == NULL)
            {
                currType->parameter_head = NULL;
                currType->parameter_tail = NULL;
            }
            else
            {
                currType->parameter_head = param_node_list->parameter_head;
                currType->parameter_tail = param_node_list->parameter_tail;
            }
        }
        else
        {
           fprintf(stderr, "[Error] Multiple parameter definition in %s\n", __func__);
        }
    }
    return ret;
}

Declaration_desc_node* AddArrayDescToDeclDesc(Declaration_desc_node* decl_desc, ArrayDesc_node* array_desc)
{
    Declaration_desc_node* ret;
    TypeDesc_node* currType;
    
    if (decl_desc == NULL)
    {
        // Abstract declaration descriptor
        ret = CreateDeclDescNode(NULL);
    }
    else
    {
        ret = decl_desc;
    }
        
    currType = ret->identifier_type;
    if (currType == NULL)
    {
        ret->identifier_type = (TypeDesc_node*) malloc(sizeof(TypeDesc_node));
        ret->identifier_type->type = NONE_TYPE;
        ret->identifier_type->kind = TYPE_WITHOUT_PARAM;
        ret->identifier_type->struct_name = NULL;
        ret->identifier_type->array_desc_head = array_desc;
        ret->identifier_type->array_desc_tail = array_desc;
        ret->identifier_type->parameter_head = NULL;
        ret->identifier_type->parameter_tail = NULL;
    }
    else
    {
        if (currType->array_desc_tail == NULL)
        {
            currType->array_desc_head = array_desc;
            currType->array_desc_tail = array_desc;
        }
        else
        {
            currType->array_desc_tail->next = array_desc;
            currType->array_desc_tail = array_desc;
        }
    }
    return ret;
}

Declaration_desc_node* AddArrayDescListToDeclDesc(Declaration_desc_node* decl_desc, ArrayDesc_node_list* array_desc_list)
{
    Declaration_desc_node* ret;
    TypeDesc_node* currType;
    
    if (decl_desc == NULL)
    {
        // Abstract declaration descriptor
        ret = CreateDeclDescNode(NULL);
    }
    else
    {
        ret = decl_desc;
    }
        
    currType = ret->identifier_type;
    if (currType == NULL)
    {
        ret->identifier_type = (TypeDesc_node*) malloc(sizeof(TypeDesc_node));
        ret->identifier_type->type = NONE_TYPE;
        ret->identifier_type->kind = TYPE_WITHOUT_PARAM;
        ret->identifier_type->struct_name = NULL;
        ret->identifier_type->array_desc_head = array_desc_list->array_desc_head;
        ret->identifier_type->array_desc_tail = array_desc_list->array_desc_tail;
        ret->identifier_type->parameter_head = NULL;
        ret->identifier_type->parameter_tail = NULL;
    }
    else
    {
        if (currType->array_desc_tail == NULL)
        {
            currType->array_desc_head = array_desc_list->array_desc_head;
            currType->array_desc_tail = array_desc_list->array_desc_tail;
        }
        else
        {
            currType->array_desc_tail->next = array_desc_list->array_desc_head;
            currType->array_desc_tail = array_desc_list->array_desc_tail;
        }
    }
    free (array_desc_list);
    return ret;
}

// Note that the space of returnValue should be allocated by caller
void GetValueInExprNode(Expression_node* expr_node, OPENCL_DATA_TYPE type, void* returnValue)
{
    // TODO
}

Parameter_node_list* AppendParamNodeToList(Parameter_node_list* origin_list, Parameter_node* new_node)
{
    if (origin_list == NULL)
    {
        Parameter_node_list* ret = (Parameter_node_list*) malloc(sizeof(Parameter_node_list));
        ret->parameter_head = new_node;
        ret->parameter_tail = new_node;
        return ret;
    }
    else
    {
        origin_list->parameter_tail->next = new_node;
        origin_list->parameter_tail = new_node;
        return origin_list;
    }
}

Parameter_node* CreateParamNode(TypeDesc_node* type, Declaration_desc_node* desc)
{
    Parameter_node* ret = (Parameter_node*) malloc(sizeof(Parameter_node));
    ret->parameter_type = type;
    ret->parameter_desc = desc;
    return ret;
}

TypeDesc_node* CreateScalarTypeDesc(OPENCL_DATA_TYPE type)
{
    TypeDesc_node* ret = (TypeDesc_node*) malloc(sizeof(TypeDesc_node));
    ret->type = type;
    ret->struct_name = NULL;
    ret->array_desc_head = NULL;
    ret->array_desc_tail = NULL;
    ret->kind = TYPE_WITHOUT_PARAM;
    ret->parameter_head = NULL;
    ret->parameter_tail = NULL;
    return ret;
}

Constant_node* CreateEmptyConstantNode(void)
{
    Constant_node* ret = (Constant_node*) malloc(sizeof(Constant_node));
    ret->constant_type = NULL;
    return ret;
}

%}

%union
{
    TypeDesc_node* type_desc_node;
    ArrayDesc_node_list* array_desc_node_list;
    Statement_node* stmt_node;
    Parameter_node* param_node;
    Parameter_node_list* param_node_list;
    Declaration_node* decl_node;
    Declaration_desc_node* decl_desc_node;
    Declaration_desc_node_list* decl_desc_node_list;
    ExpressionStatement* expr_stmt;
    IterationStatement* iter_stmt;
    SelectionStatement* sel_stmt;
    CompoundStatement* comp_stmt;
    ReturnStatement* ret_stmt;
    Expression_node_list* expr_node_list;
    EXPRESSION_KIND expr_kind;
    Expression_node* expr_node;
    Constant_node* const_node;
    void *ptr;
}

%token KERNEL ADDRESS_GLOBAL ADDRESS_LOCAL ADDRESS_PRIVATE ADDRESS_CONSTANT DEFINE

%token TYPE_NAME
%token <type_desc_node> OPENCL_TYPE
%token <const_node> CONSTANT
%token <ptr> IDENTIFIER
%token STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token CONST VOLATILE
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type <expr_node> primary_expression postfix_expression unary_expression cast_expression multiplicative_expression additive_expression shift_expression relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression logical_or_expression conditional_expression assignment_expression initializer

%type <expr_stmt> expression expression_statement
%type <expr_node_list> argument_expression_list
%type <type_desc_node> type_name declaration_specifiers type_specifier specifier_qualifier_list
%type <array_desc_node_list> pointer
%type <decl_node> declaration
%type <decl_desc_node> declarator init_declarator direct_declarator abstract_declarator direct_abstract_declarator
%type <decl_desc_node_list> init_declarator_list
%type <stmt_node> statement jump_statement
%type <comp_stmt> compound_statement
%type <sel_stmt> selection_statement
%type <iter_stmt> iteration_statement
%type <expr_kind> assignment_operator
%type <param_node_list> parameter_type_list parameter_list
%type <param_node> parameter_declaration

%start program_unit
%%

program_unit
    : translation_unit
    ;

primary_expression
	: IDENTIFIER {$$ = CreateDirectExprNode($1, NULL, NULL, EXPRESSION_IDENTIFIER);}
	| CONSTANT {$$ = CreateDirectExprNode($1, NULL, NULL, EXPRESSION_CONSTANT);}
	| STRING_LITERAL {$$ = NULL;}
	| '(' expression ')' {$$ = CreateDirectExprNode($2, NULL, NULL, EXPRESSION_EXPRSTMT);}
	;

/* function call here */
postfix_expression
	: primary_expression {$$ = $1;}
	| postfix_expression '[' expression ']' {$$ = CreateDirectExprNode($3, $1, NULL, EXPRESSION_SUBSCRIPT);}
	| postfix_expression '(' ')'
    {
        /* Assume that only IDENTIFIER can be used to invoke a function */
        FunctionInvocation_node* tmp = CreateFunctionInvocation_node($1->direct_expr.identifier, NULL);
        $$ = CreateDirectExprNode(tmp, $1->left_operand, $1->right_operand, EXPRESSION_FUNCTION);
        free ($1);
    }
	| postfix_expression '(' argument_expression_list ')'
    {
        /* Assume that only IDENTIFIER can be used to invoke a function */
        FunctionInvocation_node* tmp = CreateFunctionInvocation_node($1->direct_expr.identifier, $3);
        $$ = CreateDirectExprNode(tmp, $1->left_operand, $1->right_operand, EXPRESSION_FUNCTION);
        free ($1);
    }
	| postfix_expression '.' IDENTIFIER
	{
	    $$ = CreateDirectExprNode($3, $1, NULL, EXPRESSION_MEMBER);
    }
    | postfix_expression PTR_OP IDENTIFIER
	{
        $$ = CreateDirectExprNode($3, $1, NULL, EXPRESSION_MEMBER);
    }
    | postfix_expression INC_OP
    {
        $$ = CreateNormalExprNode(POST_INCREASE_OP, $1, NULL);
    }
    | postfix_expression DEC_OP
    {
        $$ = CreateNormalExprNode(POST_DECREASE_OP, $1, NULL);
    }
	| '(' type_name ')' '{' initializer_list '}' {$$ = NULL;}
	| '(' type_name ')' '{' initializer_list ',' '}' {$$ = NULL;}
    ;

argument_expression_list
	: assignment_expression {$$ = AppendExprNodeToList(NULL, $1);}
	| argument_expression_list ',' assignment_expression {$$ = AppendExprNodeToList($1, $3);}
	;

unary_expression
	: postfix_expression {$$ = $1;}
	| INC_OP unary_expression
    {
        $$ = CreateNormalExprNode(PRE_INCREASE_OP, $2, NULL);
    }
	| DEC_OP unary_expression
    {
        $$ = CreateNormalExprNode(PRE_DECREASE_OP, $2, NULL);
    }
	| unary_operator cast_expression
    {
        $$ = $2;
    }
	| SIZEOF unary_expression {$$ = NULL;}
	| SIZEOF '(' type_name ')' {$$ = NULL;}
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression
	: unary_expression {$$ = $1;}
    | '(' type_name ')' cast_expression
    {
        $$ = CreateDirectExprNode($2, $4, NULL, EXPRESSION_TYPECAST);
    }
	;

multiplicative_expression
	: cast_expression {$$ = $1;}
	| multiplicative_expression '*' cast_expression
    {
        $$ = CreateNormalExprNode(MULTIPLICATION_OP, $1, $3);
    }
	| multiplicative_expression '/' cast_expression
    {
        $$ = CreateNormalExprNode(DIVISION_OP, $1, $3);
    }
	| multiplicative_expression '%' cast_expression
    {
        $$ = CreateNormalExprNode(MODULAR_OP, $1, $3);
    }
	;

additive_expression
	: multiplicative_expression {$$ = $1;}
	| additive_expression '+' multiplicative_expression
    {
        $$ = CreateNormalExprNode(ADDITION_OP, $1, $3);
    }
	| additive_expression '-' multiplicative_expression
    {
        $$ = CreateNormalExprNode(SUBTRACTION_OP, $1, $3);
    }
	;

shift_expression
	: additive_expression {$$ = $1;}
	| shift_expression LEFT_OP additive_expression
    {
        $$ = CreateNormalExprNode(SHIFT_LEFT_OP, $1, $3);
    }
	| shift_expression RIGHT_OP additive_expression
    {
        $$ = CreateNormalExprNode(SHIFT_RIGHT_OP, $1, $3);
    }
	;

relational_expression
	: shift_expression {$$ = $1;}
	| relational_expression '<' shift_expression
    {
        $$ = CreateNormalExprNode(LESS_OP, $1, $3);
    }
	| relational_expression '>' shift_expression
    {
        $$ = CreateNormalExprNode(GREATER_OP, $1, $3);
    }
	| relational_expression LE_OP shift_expression
    {
        $$ = CreateNormalExprNode(LESS_EQUAL_OP, $1, $3);
    }
	| relational_expression GE_OP shift_expression
    {
        $$ = CreateNormalExprNode(GREATER_EQUAL_OP, $1, $3);
    }
	;

equality_expression
	: relational_expression {$$ = $1;}
    | equality_expression EQ_OP relational_expression
    {
        $$ = CreateNormalExprNode(EQUAL_OP, $1, $3);
    }
	| equality_expression NE_OP relational_expression
    {
        $$ = CreateNormalExprNode(NOT_EQUAL_OP, $1, $3);
    }
	;

and_expression
	: equality_expression {$$ = $1;}
    | and_expression '&' equality_expression
    {
        $$ = CreateNormalExprNode(BITWISE_AND_OP, $1, $3);
    }
	;

exclusive_or_expression
	: and_expression {$$ = $1;}
	| exclusive_or_expression '^' and_expression
    {
        $$ = CreateNormalExprNode(BITWISE_XOR_OP, $1, $3);
    }
	;

inclusive_or_expression
	: exclusive_or_expression {$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression
    {
        $$ = CreateNormalExprNode(BITWISE_OR_OP, $1, $3);
    }
	;

logical_and_expression
	: inclusive_or_expression {$$ = $1;}
	| logical_and_expression AND_OP inclusive_or_expression
    {
        $$ = CreateNormalExprNode(LOGICAL_AND_OP, $1, $3);
    }
	;

logical_or_expression
	: logical_and_expression {$$ = $1;}
	| logical_or_expression OR_OP logical_and_expression
    {
        $$ = CreateNormalExprNode(LOGICAL_AND_OP, $1, $3);
    }
	;

conditional_expression
	: logical_or_expression {$$ = $1;}
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression {$$ = $1;}
	| unary_expression assignment_operator assignment_expression
    {
        $$ = CreateNormalExprNode($2, $1, $3);
    }
	;

assignment_operator
	: '=' {$$ = ASSIGNMENT_NONE;}
	| MUL_ASSIGN {$$ = ASSIGNMENT_MUL;}
	| DIV_ASSIGN {$$ = ASSIGNMENT_DIV;}
	| MOD_ASSIGN {$$ = ASSIGNMENT_MOD;}
	| ADD_ASSIGN {$$ = ASSIGNMENT_ADD;}
	| SUB_ASSIGN {$$ = ASSIGNMENT_SUB;}
	| LEFT_ASSIGN {$$ = ASSIGNMENT_LEFT;}
	| RIGHT_ASSIGN {$$ = ASSIGNMENT_RIGHT;}
	| AND_ASSIGN {$$ = ASSIGNMENT_AND;}
	| XOR_ASSIGN {$$ = ASSIGNMENT_XOR;}
	| OR_ASSIGN {$$ = ASSIGNMENT_OR;}
	;

expression
	: assignment_expression {$$ = AddToExprStmt(NULL, $1);}
	| expression ',' assignment_expression {$$ = AddToExprStmt($1, $3);}
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';' {$$ = CreateDeclNode($1, NULL);}
	| declaration_specifiers init_declarator_list ';' {$$ = CreateDeclNode($1, $2);}
    | TYPEDEF declaration_specifiers ';'
    {
        $$ = NULL;
    }
    | TYPEDEF declaration_specifiers init_declarator_list ';'
    {
        $$ = NULL;
    }
	;

declaration_specifiers
	: storage_class_specifier {$$ = NULL;}
    | storage_class_specifier declaration_specifiers {$$ = $2;}
	| type_specifier {$$ = $1;}
	| type_specifier declaration_specifiers
    {
        // should not appear two type_specifier at same time
        $$ = $1;
    }
	| type_qualifier {$$ = NULL;}
	| type_qualifier declaration_specifiers {$$ = $2;}
	| function_specifier {$$ = NULL;}
	| function_specifier declaration_specifiers {$$ = $2;}
    | address_qualifier {$$ = NULL;}
    | address_qualifier declaration_specifiers {$$ = $2;}
	;

init_declarator_list
	: init_declarator {$$ = AppendDeclDescNodeToList(NULL, $1);}
	| init_declarator_list ',' init_declarator {$$ = AppendDeclDescNodeToList($1, $3);}
	;

init_declarator
	: declarator {$$ = $1;}
	| declarator '=' initializer
    {
        ($1)->init_expression = $3;
        $$ = $1;
    }
	;

storage_class_specifier
	: EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_specifier
    : struct_or_union_specifier
    {
        // TODO
    }
	| enum_specifier
    {
        // TODO
    }
    | TYPE_NAME
    {
        // TODO: SymbolTable
    }
	| OPENCL_TYPE
    {
        $$ = $1;
    }
    ;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
    {
        // should not appear two type_specifier at same time
        $$ = $1;
    }
	| type_specifier {$$ = $1;}
    | type_qualifier specifier_qualifier_list {$$ = $2;}
	| type_qualifier {$$ = NULL;}
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expression
	| declarator ':' constant_expression
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
	| IDENTIFIER '=' constant_expression
	;

type_qualifier
	: CONST
	| RESTRICT
	| VOLATILE
	;

address_qualifier
    : ADDRESS_GLOBAL
    | ADDRESS_LOCAL
    | ADDRESS_CONSTANT
    | ADDRESS_PRIVATE
    ;

function_specifier
	: INLINE
    | KERNEL
	;

declarator
	: pointer direct_declarator {$$ = AddArrayDescListToDeclDesc($2, $1);}
	| direct_declarator {$$ = $1;}
	;


direct_declarator
	: IDENTIFIER {$$ = CreateDeclDescNode($1);}
	| '(' declarator ')' {$$ = $2;}
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
    {
        unsigned long expr_value;
        GetValueInExprNode($4, ULONG_TYPE, &expr_value);
        ArrayDesc_node* tmp = CreateArrayDescNode(expr_value, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' type_qualifier_list ']'
    {
        ArrayDesc_node* tmp = CreateArrayDescNode(0, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' assignment_expression ']'
    {
        unsigned long expr_value;
        GetValueInExprNode($3, ULONG_TYPE, &expr_value);
        ArrayDesc_node* tmp = CreateArrayDescNode(expr_value, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    {
        unsigned long expr_value;
        GetValueInExprNode($5, ULONG_TYPE, &expr_value);
        ArrayDesc_node* tmp = CreateArrayDescNode(expr_value, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    {
        unsigned long expr_value;
        GetValueInExprNode($5, ULONG_TYPE, &expr_value);
        ArrayDesc_node* tmp = CreateArrayDescNode(expr_value, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' type_qualifier_list '*' ']'
    {
        ArrayDesc_node* tmp = CreateArrayDescNode(0, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' '*' ']'
    {
        // variable length of array
        ArrayDesc_node* tmp = CreateArrayDescNode(0, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' ']'
    {
        // decays to a pointer
        ArrayDesc_node* tmp = CreateArrayDescNode(0, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '(' parameter_type_list ')'
    {
        $$ = AddParamToDeclDesc($1, $3);
    }
    | direct_declarator '(' identifier_list ')'
    {
        // deprecated (old syntax)
        $$ = $1;
    }
	| direct_declarator '(' ')'
    {
        $$ = AddParamToDeclDesc($1, NULL);
    }
	;

pointer
	: '*' {$$ = AppendArrayDescNodeToList(NULL, CreateArrayDescNode(1, ARRAY_DESC_POINTER));}
	| '*' type_qualifier_list {$$ = AppendArrayDescNodeToList(NULL, CreateArrayDescNode(1, ARRAY_DESC_POINTER));}
	| '*' pointer {$$ = AppendArrayDescNodeToList($2, CreateArrayDescNode(1, ARRAY_DESC_POINTER));}
	| '*' type_qualifier_list pointer {$$ = AppendArrayDescNodeToList($3, CreateArrayDescNode(1, ARRAY_DESC_POINTER));}
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list {$$ = $1;}
	| parameter_list ',' ELLIPSIS
    {
        // ignore the case with variable length of parameters
        $$ = $1;
    }
	;

parameter_list
	: parameter_declaration {$$ = AppendParamNodeToList(NULL, $1);}
	| parameter_list ',' parameter_declaration {$$ = AppendParamNodeToList($1, $3);}
	;

parameter_declaration
	: declaration_specifiers declarator {$$ = CreateParamNode($1, $2);}
	| declaration_specifiers abstract_declarator {$$ = CreateParamNode($1, $2);}
	| declaration_specifiers {$$ = CreateParamNode($1, NULL);}
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list {$$ = $1;}
	| specifier_qualifier_list abstract_declarator
    {
        // TODO: Mix TypeDesc_node
    }
	;

abstract_declarator
	: pointer {$$ = AddArrayDescListToDeclDesc(NULL, $1);}
	| direct_abstract_declarator {$$ = $1;}
	| pointer direct_abstract_declarator {$$ = AddArrayDescListToDeclDesc($2, $1);}
	;

direct_abstract_declarator
	: '(' abstract_declarator ')' {$$ = $2;}
	| '[' ']'
    {
        ArrayDesc_node* tmp = CreateArrayDescNode(0, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc(NULL, tmp);
    }
	| '[' assignment_expression ']'
    {
        unsigned long expr_value;
        GetValueInExprNode($2, ULONG_TYPE, &expr_value);
        ArrayDesc_node* tmp = CreateArrayDescNode(expr_value, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc(NULL, tmp);
    }
	| direct_abstract_declarator '[' ']'
    {
        ArrayDesc_node* tmp = CreateArrayDescNode(0, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_abstract_declarator '[' assignment_expression ']'
    {
        unsigned long expr_value;
        GetValueInExprNode($3, ULONG_TYPE, &expr_value);
        ArrayDesc_node* tmp = CreateArrayDescNode(expr_value, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| '[' '*' ']'
    {
        ArrayDesc_node* tmp = CreateArrayDescNode(0, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc(NULL, tmp);
    }
	| direct_abstract_declarator '[' '*' ']'
    {
        ArrayDesc_node* tmp = CreateArrayDescNode(0, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| '(' ')'
    {
        $$ = AddParamToDeclDesc(NULL, NULL);
    }
	| '(' parameter_type_list ')'
    {
        $$ = AddParamToDeclDesc(NULL, $2);
    }
	| direct_abstract_declarator '(' ')'
    {
        $$ = AddParamToDeclDesc($1, NULL);
    }
	| direct_abstract_declarator '(' parameter_type_list ')'
    {
        $$ = AddParamToDeclDesc($1, $3);
    }
	;

initializer
	: assignment_expression {$$ = $1;}
	| '{' initializer_list '}' {$$ = NULL;}
	| '{' initializer_list ',' '}' {$$ = NULL;}
	;

initializer_list
	: initializer
	| designation initializer
	| initializer_list ',' initializer
	| initializer_list ',' designation initializer
	;

designation
	: designator_list '='
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: '[' constant_expression ']'
	| '.' IDENTIFIER
	;

statement
	: labeled_statement {$$ = NULL;}
	| compound_statement {$$ = CreateStmtNode($1, COMPOUND_STMT);}
	| expression_statement {$$ = CreateStmtNode($1, EXPRESSION_STMT);}
	| selection_statement {$$ = CreateStmtNode($1, SELECTION_STMT);}
	| iteration_statement {$$ = CreateStmtNode($1, ITERATION_STMT);}
	| jump_statement {$$ = $1;}
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
    | '{' block_item_list '}'
	;

block_item_list
	: block_item
	| block_item_list block_item
	;

block_item
    : declaration
	| statement
	;

expression_statement
	: ';' {$$ = NULL;}
	| expression ';' {$$ = $1;}
	;

selection_statement
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
    {

    }
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	| FOR '(' declaration expression_statement ')' statement
	| FOR '(' declaration expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
    | declaration_specifiers declarator compound_statement
	;

declaration_list
	: declaration
	| declaration_list declaration
	;


%%
#include <stdio.h>

extern char yytext[];
extern int column;

void yyerror(char const *s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}