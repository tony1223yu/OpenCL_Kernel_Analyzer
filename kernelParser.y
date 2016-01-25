%{
#include <stdio.h>
#include <stdlib.h>
#include "ASTDefinition.h"

Expression_node* CreateDirectExpressionNode(void* ptr, Expression_node* left, Expression_node* right, EXPRESSION_KIND kind)
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
            ret->direct_expr.target_type = (TypeDescriptor_node*)(ptr);
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

Expression_node* CreateNormalExpressionNode(EXPRESSION_KIND kind, Expression_node* left, Expression_node* right)
{
    Expression_node* ret = (Expression_node*) malloc(sizeof(Expression_node));

    ret->expression_kind = kind;
    ret->left_operand = left;
    ret->right_operand = right;
    ret->next = NULL;

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

Expression_node_list* AppendExpressionNodeToList(Expression_node_list* origin_list, Expression_node* new_node)
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

ExpressionStatement* AddToExpressionStatement(ExpressionStatement* origin_stmt, Expression_node* new_node)
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

%}

%union
{
    TypeDescriptor_node* type_node;
    ExpressionStatement* expr_stmt;
    Expression_node_list* expr_node_list;
    EXPRESSION_KIND expr_kind;
    Expression_node* expr_node;
    Constant_node* const_node;
    void *ptr;
}

%token KERNEL ADDRESS_GLOBAL ADDRESS_LOCAL ADDRESS_PRIVATE ADDRESS_CONSTANT DEFINE

%token TYPE_NAME
%token OPENCL_TYPE
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

%type <expr_node> primary_expression postfix_expression unary_expression cast_expression multiplicative_expression additive_expression shift_expression relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression logical_or_expression conditional_expression assignment_expression

%type <expr_stmt> expression

%type <expr_node_list> argument_expression_list

%type <type_node> type_name

%type <expr_kind> assignment_operator

%start program_unit
%%

program_unit
    : translation_unit
    ;

primary_expression
	: IDENTIFIER {$$ = CreateDirectExpressionNode($1, NULL, NULL, EXPRESSION_IDENTIFIER);}
	| CONSTANT {$$ = CreateDirectExpressionNode($1, NULL, NULL, EXPRESSION_CONSTANT);}
	| STRING_LITERAL {$$ = NULL;}
	| '(' expression ')' {$$ = CreateDirectExpressionNode($2, NULL, NULL, EXPRESSION_EXPRSTMT);}
	;

/* function call here */
postfix_expression
	: primary_expression {$$ = $1;}
	| postfix_expression '[' expression ']' {$$ = CreateDirectExpressionNode($3, $1, NULL, EXPRESSION_SUBSCRIPT);}
	| postfix_expression '(' ')'
    {
        /* Assume that only IDENTIFIER can be used to invoke a function */
        FunctionInvocation_node* tmp = CreateFunctionInvocation_node($1->direct_expr.identifier, NULL);
        $$ = CreateDirectExpressionNode(tmp, $1->left_operand, $1->right_operand, EXPRESSION_FUNCTION);
        free ($1);
    }
	| postfix_expression '(' argument_expression_list ')'
    {
        /* Assume that only IDENTIFIER can be used to invoke a function */
        FunctionInvocation_node* tmp = CreateFunctionInvocation_node($1->direct_expr.identifier, $3);
        $$ = CreateDirectExpressionNode(tmp, $1->left_operand, $1->right_operand, EXPRESSION_FUNCTION);
        free ($1);
    }
	| postfix_expression '.' IDENTIFIER
	{
	    $$ = CreateDirectExpressionNode($3, $1, NULL, EXPRESSION_MEMBER);
    }
    | postfix_expression PTR_OP IDENTIFIER
	{
        $$ = CreateDirectExpressionNode($3, $1, NULL, EXPRESSION_MEMBER);
    }
    | postfix_expression INC_OP
    {
        $$ = CreateNormalExpressionNode(POST_INCREASE_OP, $1, NULL);
    }
    | postfix_expression DEC_OP
    {
        $$ = CreateNormalExpressionNode(POST_DECREASE_OP, $1, NULL);
    }
	| '(' type_name ')' '{' initializer_list '}' {$$ = NULL;}
	| '(' type_name ')' '{' initializer_list ',' '}' {$$ = NULL;}
    ;

argument_expression_list
	: assignment_expression {$$ = AppendExpressionNodeToList(NULL, $1);}
	| argument_expression_list ',' assignment_expression {$$ = AppendExpressionNodeToList($1, $3);}
	;

unary_expression
	: postfix_expression {$$ = $1;}
	| INC_OP unary_expression
    {
        $$ = CreateNormalExpressionNode(PRE_INCREASE_OP, $2, NULL);
    }
	| DEC_OP unary_expression
    {
        $$ = CreateNormalExpressionNode(PRE_DECREASE_OP, $2, NULL);
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
        $$ = CreateDirectExpressionNode($2, $4, NULL, EXPRESSION_TYPECAST);
    }
	;

multiplicative_expression
	: cast_expression {$$ = $1;}
	| multiplicative_expression '*' cast_expression
    {
        $$ = CreateNormalExpressionNode(MULTIPLICATION_OP, $1, $3);
    }
	| multiplicative_expression '/' cast_expression
    {
        $$ = CreateNormalExpressionNode(DIVISION_OP, $1, $3);
    }
	| multiplicative_expression '%' cast_expression
    {
        $$ = CreateNormalExpressionNode(MODULAR_OP, $1, $3);
    }
	;

additive_expression
	: multiplicative_expression {$$ = $1;}
	| additive_expression '+' multiplicative_expression
    {
        $$ = CreateNormalExpressionNode(ADDITION_OP, $1, $3);
    }
	| additive_expression '-' multiplicative_expression
    {
        $$ = CreateNormalExpressionNode(SUBTRACTION_OP, $1, $3);
    }
	;

shift_expression
	: additive_expression {$$ = $1;}
	| shift_expression LEFT_OP additive_expression
    {
        $$ = CreateNormalExpressionNode(SHIFT_LEFT_OP, $1, $3);
    }
	| shift_expression RIGHT_OP additive_expression
    {
        $$ = CreateNormalExpressionNode(SHIFT_RIGHT_OP, $1, $3);
    }
	;

relational_expression
	: shift_expression {$$ = $1;}
	| relational_expression '<' shift_expression
    {
        $$ = CreateNormalExpressionNode(LESS_OP, $1, $3);
    }
	| relational_expression '>' shift_expression
    {
        $$ = CreateNormalExpressionNode(GREATER_OP, $1, $3);
    }
	| relational_expression LE_OP shift_expression
    {
        $$ = CreateNormalExpressionNode(LESS_EQUAL_OP, $1, $3);
    }
	| relational_expression GE_OP shift_expression
    {
        $$ = CreateNormalExpressionNode(GREATER_EQUAL_OP, $1, $3);
    }
	;

equality_expression
	: relational_expression {$$ = $1;}
    | equality_expression EQ_OP relational_expression
    {
        $$ = CreateNormalExpressionNode(EQUAL_OP, $1, $3);
    }
	| equality_expression NE_OP relational_expression
    {
        $$ = CreateNormalExpressionNode(NOT_EQUAL_OP, $1, $3);
    }
	;

and_expression
	: equality_expression {$$ = $1;}
    | and_expression '&' equality_expression
    {
        $$ = CreateNormalExpressionNode(BITWISE_AND_OP, $1, $3);
    }
	;

exclusive_or_expression
	: and_expression {$$ = $1;}
	| exclusive_or_expression '^' and_expression
    {
        $$ = CreateNormalExpressionNode(BITWISE_XOR_OP, $1, $3);
    }
	;

inclusive_or_expression
	: exclusive_or_expression {$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression
    {
        $$ = CreateNormalExpressionNode(BITWISE_OR_OP, $1, $3);
    }
	;

logical_and_expression
	: inclusive_or_expression {$$ = $1;}
	| logical_and_expression AND_OP inclusive_or_expression
    {
        $$ = CreateNormalExpressionNode(LOGICAL_AND_OP, $1, $3);
    }
	;

logical_or_expression
	: logical_and_expression {$$ = $1;}
	| logical_or_expression OR_OP logical_and_expression
    {
        $$ = CreateNormalExpressionNode(LOGICAL_AND_OP, $1, $3);
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
        $$ = CreateNormalExpressionNode($2, $1, $3);
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
	: assignment_expression {$$ = AddToExpressionStatement(NULL, $1);}
	| expression ',' assignment_expression {$$ = AddToExpressionStatement($1, $3);}
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
    | TYPEDEF declaration_specifiers ';'
    | TYPEDEF declaration_specifiers init_declarator_list ';'
	;

declaration_specifiers
	: storage_class_specifier
    | storage_class_specifier declaration_specifiers
	| type_specifier
	| type_specifier declaration_specifiers
	| type_qualifier
	| type_qualifier declaration_specifiers
	| function_specifier
	| function_specifier declaration_specifiers
    | address_qualifier
    | address_qualifier declaration_specifiers
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' init_declarator
	;

init_declarator
	: declarator
	| declarator '=' initializer
	;

storage_class_specifier
	: EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_specifier
    : struct_or_union_specifier
	| enum_specifier
    | TYPE_NAME
	| OPENCL_TYPE
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
	| type_specifier
    | type_qualifier specifier_qualifier_list
	| type_qualifier
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
	: pointer direct_declarator
	| direct_declarator
	;


direct_declarator
	: IDENTIFIER
	| '(' declarator ')'
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list ']'
	| direct_declarator '[' assignment_expression ']'
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list '*' ']'
	| direct_declarator '[' '*' ']'
	| direct_declarator '[' ']'
	| direct_declarator '(' parameter_type_list ')'
	| direct_declarator '(' identifier_list ')'
	| direct_declarator '(' ')'
	;

pointer
	: '*'
	| '*' type_qualifier_list
	| '*' pointer
	| '*' type_qualifier_list pointer
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration
	;

parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' assignment_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' assignment_expression ']'
	| '[' '*' ']'
	| direct_abstract_declarator '[' '*' ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'
	| '{' initializer_list ',' '}'
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
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
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
	: ';'
	| expression ';'
	;

selection_statement
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
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
