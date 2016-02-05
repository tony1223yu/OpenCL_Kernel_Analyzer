%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ASTDefinition.h"

extern Program_node* program;
extern int structNumber;

void AddFuncNodeToProgram(Program_node* prog, Function_node* func_node)
{
    if (func_node == NULL)
        return;
    else
    {
        if (prog == NULL)
        {
            fprintf(stderr, "[Error] program_node is NULL in %s\n", __func__);
            return;
        }

        if (prog->function_head == NULL)
        {
            prog->function_head = func_node;
            prog->function_tail = func_node;
        }
        else
        {
            prog->function_tail->next = func_node;
            prog->function_tail = func_node;
        }
    }
}

void AddDeclNodeToProgram(Program_node* prog, Declaration_node* decl_node)
{
    if (decl_node == NULL)
        return;
    else
    {
        if (prog == NULL)
        {
            fprintf(stderr, "[Error] program_node is NULL in %s\n", __func__);
            return;
        }

        if (prog->declaration_head == NULL)
        {
            prog->declaration_head = decl_node;
            prog->declaration_tail = decl_node;
        }
        else
        {
            prog->declaration_tail->next = decl_node;
            prog->declaration_tail = decl_node;
        }
    }
}

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
            ret->direct_expr.subscript = (ExpressionStatement*)(ptr);
            break;
        case EXPRESSION_FUNCTION:
            ret->direct_expr.function = (FunctionInvocation_node*)(ptr);
            break;
        case EXPRESSION_TYPECAST:
            ret->direct_expr.target_type = (TypeDescriptor*)(ptr);
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

    /* set the content of the union space to 0 */
    ret->direct_expr.identifier = NULL;
    return ret;
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
    if (new_node == NULL)
        return origin_list;

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

Declaration_node* CreateDeclNode(TypeDescriptor* type, Declaration_desc_node_list* desc_list)
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
    if (new_desc == NULL)
        return origin_list;

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
    ret->parameter_head = NULL;
    ret->parameter_tail = NULL;
    return ret;
}

ArrayDesc_node_list* AppendArrayDescNodeToList(ArrayDesc_node_list* origin_list, ArrayDesc_node* new_node)
{
    if (new_node == NULL)
        return origin_list;

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
    TypeDescriptor* currType;

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
        ret->identifier_type = CreateScalarTypeDesc(NONE_TYPE, NULL);
        ret->identifier_type->kind = TYPE_WITH_PARAM;
        if (param_node_list != NULL)
        {
            ret->identifier_type->parameter_head = param_node_list->parameter_head;
            ret->identifier_type->parameter_tail = param_node_list->parameter_tail;
            free (param_node_list);
        }
    }
    else
    {
        if (currType->array_desc_head == NULL)
        {
            if (currType->kind != TYPE_WITHOUT_PARAM)
                fprintf(stderr, "[Error] Redefined parameter in %s\n", __func__);

            currType->kind = TYPE_WITH_PARAM;
            if (param_node_list != NULL)
            {
                currType->parameter_head = param_node_list->parameter_head;
                currType->parameter_tail = param_node_list->parameter_tail;
                free (param_node_list);
            }
        }
        else
        {
            ArrayDesc_node* currArray = currType->array_desc_tail;
            if (currArray->desc_kind == ARRAY_DESC_ARRAY)
                fprintf(stderr, "[Error] Define array of functions in %s\n", __func__);
            else if (currArray->desc_kind == ARRAY_DESC_FUNC_POINTER)
                fprintf(stderr, "[Error] Redefined parameter of function pointers in %s\n", __func__);
            else // ARRAY_DESC_POINTER
            {
                currArray->desc_kind = ARRAY_DESC_FUNC_POINTER;
                if (param_node_list != NULL)
                {
                    currArray->parameter_head = param_node_list->parameter_head;
                    currArray->parameter_tail = param_node_list->parameter_tail;
                }
                free (param_node_list);
            }
        }
    }
    return ret;
}

Declaration_desc_node* AddArrayDescToDeclDesc(Declaration_desc_node* decl_desc, ArrayDesc_node* array_desc)
{
    if (array_desc == NULL)
        return decl_desc;
    else
    {
        Declaration_desc_node* ret;
        TypeDescriptor* currType;

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
            ret->identifier_type = CreateScalarTypeDesc(NONE_TYPE, NULL);
            ret->identifier_type->array_desc_head = array_desc;
            ret->identifier_type->array_desc_tail = array_desc;
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
}

Declaration_desc_node* AddArrayDescListToDeclDesc(Declaration_desc_node* decl_desc, ArrayDesc_node_list* array_desc_list)
{
    if (array_desc_list == NULL)
        return decl_desc;
    else
    {
        Declaration_desc_node* ret;
        TypeDescriptor* currType;

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
            ret->identifier_type = CreateScalarTypeDesc(NONE_TYPE, NULL);
            ret->identifier_type->array_desc_head = array_desc_list->array_desc_head;
            ret->identifier_type->array_desc_tail = array_desc_list->array_desc_tail;
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
}

// Note that the space of returnValue should be allocated by caller
void GetUlongValueInExprNode(Expression_node* expr_node, unsigned long* returnValue)
{
    if (!expr_node) return;
    if (expr_node->expression_kind != EXPRESSION_CONSTANT)
        fprintf(stderr, "[Error] Expression is not a constant in %s\n", __func__);
    else
    {
        Constant_node* node = expr_node->direct_expr.constant;
        OPENCL_DATA_TYPE currType;
        if (node->constant_type->kind != TYPE_WITHOUT_PARAM)
            fprintf(stderr, "[Error] Invalid type of constant in %s\n", __func__);
        if (node->constant_type->array_desc_head != NULL)
            fprintf(stderr, "[Error] Invalid type of constant in %s\n", __func__);

        currType = node->constant_type->type;
        if (currType & CONST_UNSIGNED_INTEGER_MASK)
            *returnValue = (unsigned long)(node->value.ulong_val);
        else if (currType & CONST_SIGNED_INTEGER_MASK)
            *returnValue = (unsigned long)(node->value.long_val);
        else
            fprintf(stderr, "[Error] Invalid type of constant in %s\n", __func__);
    }
}

Parameter_node_list* AppendParamNodeToList(Parameter_node_list* origin_list, Parameter_node* new_node)
{
    if (new_node == NULL)
        return origin_list;

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

Parameter_node* CreateParamNode(TypeDescriptor* type, Declaration_desc_node* desc)
{
    Parameter_node* ret = (Parameter_node*) malloc(sizeof(Parameter_node));
    ret->parameter_type = type;
    ret->parameter_desc = desc;
    return ret;
}

TypeDescriptor* CreateScalarTypeDesc(OPENCL_DATA_TYPE type, char* struct_name)
{
    TypeDescriptor* ret = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
    ret->type = type;
    ret->struct_name = struct_name;
    ret->array_desc_head = NULL;
    ret->array_desc_tail = NULL;
    ret->kind = TYPE_WITHOUT_PARAM;
    ret->parameter_head = NULL;
    ret->parameter_tail = NULL;
    return ret;
}

TypeDescriptor* MixAndCreateTypeDesc(TypeDescriptor* left, TypeDescriptor* right)
{
    TypeDescriptor* ret = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));

    if ((left == NULL) && (right == NULL))
    {
        fprintf(stderr, "[Error] Both type descriptor are NULL\n");
    }
    else if (left == NULL)
    {
        ret->type = right->type;
        ret->struct_name = right->struct_name;
        ret->array_desc_head = right->array_desc_head;
        ret->array_desc_tail = right->array_desc_tail;
        ret->parameter_head = right->parameter_head;
        ret->parameter_tail = right->parameter_tail;
        ret->kind = right->kind;
    }
    else if (right == NULL)
    {
        ret->type = left->type;
        ret->struct_name = left->struct_name;
        ret->array_desc_head = left->array_desc_head;
        ret->array_desc_tail = left->array_desc_tail;
        ret->parameter_head = left->parameter_head;
        ret->parameter_tail = left->parameter_tail;
        ret->kind = left->kind;
    }
    else
    {
        /* should be left */
        ret->type = left->type;
        ret->struct_name = left->struct_name;
        ret->array_desc_head = right->array_desc_head;
        ret->array_desc_tail = right->array_desc_tail;

        if (left->array_desc_head != NULL)
        {
            if (ret->array_desc_head != NULL)
            {
                ret->array_desc_tail->next = left->array_desc_head;
                ret->array_desc_tail = left->array_desc_tail;
            }
            else
            {
                ret->array_desc_head = left->array_desc_head;
                ret->array_desc_tail = left->array_desc_tail;
            }
        }

        if ((left->kind == TYPE_WITH_PARAM) && (right->kind == TYPE_WITH_PARAM))
        {
            fprintf(stderr, "[Error] Redefined parameters in %s", __func__);
        }
        else if (left->kind == TYPE_WITH_PARAM)
        {
            ret->kind = TYPE_WITH_PARAM;
            ret->parameter_head = left->parameter_head;
            ret->parameter_tail = left->parameter_tail;
        }
        else if (right->kind == TYPE_WITH_PARAM)
        {
            ret->kind = TYPE_WITH_PARAM;
            ret->parameter_head = right->parameter_head;
            ret->parameter_tail = right->parameter_tail;
        }
        else
        {
            ret->kind = TYPE_WITHOUT_PARAM;
            ret->parameter_head = NULL;
            ret->parameter_tail = NULL;
        }
    }
    return ret;
}

Constant_node* CreateEmptyConstantNode(void)
{
    Constant_node* ret = (Constant_node*) malloc(sizeof(Constant_node));
    ret->constant_type = NULL;
    return ret;
}


IterationStatement* CreateIterStmt(ITERATION_STMT_KIND kind, void* init, ExpressionStatement* terminated, ExpressionStatement* step, Statement_node* content)
{
    IterationStatement* ret = (IterationStatement*) malloc(sizeof(IterationStatement));

    if (kind == FOR_LOOP_WITH_DECL)
    {
        ret->init.declaration = (Declaration_node*) init;
    }
    else
    {
        ret->init.expression = (ExpressionStatement*) init;
    }

    ret->terminated_expression = terminated;
    ret->step_expression = step;
    ret->content_statement = content;

    return ret;
}

CompoundStatement* CreateCompoundStmt(Declaration_node* decl_node, Statement_node* stmt_node)
{
    CompoundStatement* ret = (CompoundStatement*) malloc(sizeof(CompoundStatement));
    ret->declaration_head = decl_node;
    ret->declaration_tail = decl_node;
    ret->statement_head = stmt_node;
    ret->statement_tail = stmt_node;
    return ret;
}

CompoundStatement* MergeCompoundStmt(CompoundStatement* left, CompoundStatement* right)
{
    if ((left == NULL) && (right == NULL))
        return NULL;
    else if (left == NULL)
        return right;
    else if (right == NULL)
        return left;
    else
    {
        if (right->declaration_head != NULL)
        {
            if (left->declaration_head != NULL)
            {
                left->declaration_tail->next = right->declaration_head;
                left->declaration_tail = right->declaration_tail;
            }
            else
            {
                left->declaration_head = right->declaration_head;
                left->declaration_tail = right->declaration_tail;
            }
        }

        if (right->statement_head != NULL)
        {
            if (left->statement_head != NULL)
            {
                left->statement_tail->next = right->statement_head;
                left->statement_tail = right->statement_tail;
            }
            else
            {
                left->statement_head = right->statement_head;
                left->statement_tail = right->statement_tail;
            }
        }

        free (right);
        return left;
    }
}

Selection_node* CreateSelectionNode(SELECTION_KIND kind, ExpressionStatement* expr, Statement_node* stmt)
{
    Selection_node* ret = (Selection_node*) malloc(sizeof(Selection_node));
    ret->condition_kind = kind;
    ret->condition_expression = expr;
    ret->content_statement = stmt;
    ret->next = NULL;
    return ret;
}

SelectionStatement* CreateSelectionStmt(Selection_node* node)
{
    if (node == NULL)
        return NULL;
    else
    {
        SelectionStatement* ret = (SelectionStatement*) malloc(sizeof(SelectionStatement));
        ret->selection_head = node;
        ret->selection_tail = node;
        return ret;
    }
}

SelectionStatement* AddToSelectionStmt(SelectionStatement* stmt, Selection_node* node)
{
    if (stmt == NULL)
    {
        return CreateSelectionStmt(node);
    }
    else
    {
        if (node)
        {
           stmt->selection_tail->next = node;
           stmt->selection_tail = node;
        }
        return stmt;
    }
}

SelectionStatement* MergeSelectionStmt(SelectionStatement* left, SelectionStatement* right)
{
    if ((left == NULL) && (right == NULL))
        return NULL;
    else if (left == NULL)
        return right;
    else if (right == NULL)
        return left;
    else
    {
        left->selection_tail->next = right->selection_head;
        left->selection_tail = right->selection_head;
        free (right);
        return left;
    }
}

Function_node* CreateFunctionNode(TypeDescriptor* type, Declaration_desc_node* decl_desc, CompoundStatement* compound_stmt)
{
    Function_node* ret = (Function_node*) malloc(sizeof(Function_node));
    ret->function_name = decl_desc->identifier_name;
    if (decl_desc->identifier_type == NULL)
        fprintf(stderr, "[Error] No parameter declaration in %s\n", __func__);
    else
    {
        ret->parameter_head = decl_desc->identifier_type->parameter_head;
        ret->parameter_tail = decl_desc->identifier_type->parameter_tail;
        decl_desc->identifier_type->parameter_head = NULL;
        decl_desc->identifier_type->parameter_tail = NULL;
        decl_desc->identifier_type->kind = TYPE_WITHOUT_PARAM;

        ret->return_type = MixAndCreateTypeDesc(type, decl_desc->identifier_type);
        free (type);
        free (decl_desc);
    }
    ret->content_statement = compound_stmt;
    ret->next = NULL;
    return ret;
}

Program_node* CreateProgramNode(void)
{
    Program_node* ret = (Program_node*) malloc(sizeof(Program_node));
    ret->struct_head = NULL;
    ret->struct_tail = NULL;
    ret->declaration_head = NULL;
    ret->declaration_tail = NULL;
    ret->function_head = NULL;
    ret->function_tail = NULL;
}

Declaration_node_list* AppendDeclNodeToList(Declaration_node_list* origin_list, Declaration_node* new_node)
{
    if (new_node == NULL)
        return origin_list;

    if (origin_list == NULL)
    {
        Declaration_node_list* ret = (Declaration_node_list*) malloc(sizeof(Declaration_node_list));
        ret->declaration_head = new_node;
        ret->declaration_tail = new_node;
        return ret;
    }
    else
    {
        origin_list->declaration_tail->next = new_node;
        origin_list->declaration_tail = new_node;
        return origin_list;
    }
}

void AddStructDeclNode(Program_node* prog, char* name, Declaration_node_list* member_list)
{
    if (prog == NULL)
    {
        fprintf(stderr, "[Error] Given program node is NULL\n");
        return;
    }
    else
    {
        StructDeclaration_node* struct_decl = (StructDeclaration_node*) malloc(sizeof(StructDeclaration_node));
        struct_decl->struct_name = name;
        struct_decl->member_head = member_list->declaration_head;
        struct_decl->member_tail = member_list->declaration_tail;
        struct_decl->next = NULL;

        if (prog->struct_head == NULL)
        {
            prog->struct_head = struct_decl;
            prog->struct_tail = struct_decl;
        }
        else
        {
            prog->struct_tail->next = struct_decl;
            prog->struct_tail = struct_decl;
        }
    }
}

TypeName_node* CreateTypeNameNode(char* name, TypeDescriptor* type)
{
    if (CheckInTypeNameTable(typeTable, name))
    {
        // TODO
        fprintf(stderr, "[Error] Given identifier is already a type in %s\n", __func__);
        return NULL;
    }
    else
    {
        TypeName_node* ret = (TypeName_node*) malloc(sizeof(TypeName_node));
        ret->identifier_name = name;
        ret->identifier_type = type;
        return ret;
    }
}

void AddToTypeTable(TypeNameTable* table, TypeDescriptor* type, Declaration_desc_node_list* node_list)
{
    if (node_list == NULL)
        return;
    else
    {
        Declaration_desc_node* iterNode = node_list->declaration_desc_head;
        while (iterNode != NULL)
        {
            TypeDescriptor* mix_type = MixAndCreateTypeDesc(type, iterNode->identifier_type);
            TypeName_node* new_node = CreateTypeNameNode(iterNode->identifier_name, mix_type);
            if (table == NULL)
            {
                fprintf(stderr, "[Error] Type name table is NULL in %s\n", __func__);
            }
            else
            {
                if (table->type_node_head == NULL)
                {
                    table->type_node_head = new_node;
                    table->type_node_tail = new_node;
                }
                else
                {
                    table->type_node_tail->next = new_node;
                    table->type_node_tail = new_node;
                }
            }
            iterNode = iterNode->next;
        }
    }
}

TypeDescriptor* DuplicateTypeDesc(TypeDescriptor* type)
{
    if (type == NULL)
        return NULL;
    else
    {
        TypeDescriptor* ret = CreateScalarTypeDesc(type->type, NULL);
        ret->kind = type->kind;

        if (type->struct_name != NULL)
            ret->struct_name = strdup(type->struct_name);

        if (type->array_desc_head != NULL)
        {
            ArrayDesc_node* iterArray = type->array_desc_head;
            while (iterArray != NULL)
            {
                ArrayDesc_node* tmp = DuplicateArrayDesc(iterArray);
                if (ret->array_desc_head == NULL)
                {
                    ret->array_desc_head = tmp;
                    ret->array_desc_tail = tmp;
                }
                else
                {
                    ret->array_desc_tail->next = tmp;
                    ret->array_desc_tail = tmp;
                }
                iterArray = iterArray->next;
            }
        }

        if (type->parameter_head != NULL)
        {
            Parameter_node* iterParam = type->parameter_head;
            while (iterParam != NULL)
            {
                Parameter_node* tmp = DuplicateParamNode(iterParam);
                if (ret->parameter_head == NULL)
                {
                    ret->parameter_head = tmp;
                    ret->parameter_tail = tmp;
                }
                else
                {
                    ret->parameter_tail->next = tmp;
                    ret->parameter_tail = tmp;
                }
                iterParam = iterParam->next;
            }
        }

        return ret;
    }
}

ArrayDesc_node* DuplicateArrayDesc(ArrayDesc_node* array)
{
    if (array == NULL)
        return NULL;
    else
    {
        ArrayDesc_node* ret = (ArrayDesc_node*) malloc(sizeof(ArrayDesc_node));
        ret->size = array->size;
        ret->desc_kind = array->desc_kind;

        if (array->parameter_head != NULL)
        {
            Parameter_node* iterParam = array->parameter_head;
            while (iterParam != NULL)
            {
                Parameter_node* tmp = DuplicateParamNode(iterParam);
                if (ret->parameter_head == NULL)
                {
                    ret->parameter_head = tmp;
                    ret->parameter_tail = tmp;
                }
                else
                {
                    ret->parameter_tail->next = tmp;
                    ret->parameter_tail = tmp;
                }
                iterParam = iterParam->next;
            }
        }

        return ret;
    }
}

Parameter_node* DuplicateParamNode(Parameter_node* param)
{
    if (param == NULL)
        return NULL;
    else
    {
        Parameter_node* ret = (Parameter_node*) malloc(sizeof(Parameter_node));
        ret->parameter_type = DuplicateTypeDesc(param->parameter_type);
        ret->parameter_desc = DuplicateDeclDescNode(param->parameter_desc);
        return ret;
    }
}

Declaration_desc_node* DuplicateDeclDescNode(Declaration_desc_node* desc)
{
    if (desc == NULL)
        return NULL;
    else
    {
        Declaration_desc_node* ret = (Declaration_desc_node*) malloc(sizeof(Declaration_desc_node));

        if (desc->identifier_name != NULL)
            ret->identifier_name = strdup(desc->identifier_name);

        ret->identifier_type = DuplicateTypeDesc(desc->identifier_type);

        // TODO
        if (desc->init_expression != NULL)
            fprintf(stderr, "[Error] Cannot duplicate declaration node with initial expression\n");
        else
            ret->init_expression = NULL;

        return ret;
    }
}

int CheckInTypeNameTable(TypeNameTable* table, char* name)
{
    TypeName_node* iter_node = table->type_node_head;
    while (iter_node != NULL)
    {
        if (strcmp(iter_node->identifier_name, name) == 0)
        {
            return 1;
        }
        else
            iter_node = iter_node -> next;
    }
    return 0;
}

TypeDescriptor* GetTypeDescFromTypeNameTable(TypeNameTable* table, char* name)
{
    TypeName_node* iter_node = table->type_node_head;
    while (iter_node != NULL)
    {
        if (strcmp(iter_node->identifier_name, name) == 0)
        {
            return DuplicateTypeDesc(iter_node->identifier_type);
        }
        else
            iter_node = iter_node -> next;
    }
    return NULL;
}

TypeNameTable* CreateTypeNameTable(void)
{
    TypeNameTable* ret = (TypeNameTable*) malloc(sizeof(TypeNameTable));
    ret->type_node_head = NULL;
    ret->type_node_tail = NULL;
    return ret;
}

#if 0
void DeleteTypeDesc(TypeDescriptor* type)
{
    if (type == NULL)
        return;

    if (type->array_desc_head != NULL)
    {
        ArrayDesc_node* iterNode = type->array_desc_head;
        ArrayDesc_node* nextNode;
        while (iterNode != NULL)
        {
            /* TODO */
            nextNode = iterNode->next;
            free (iterNode);
            iterNode = nextNode;
        }
    }

    if (type->parameter_head != NULL)
    {
        /* TODO */
    }
}
#endif

%}

%union
{
    OPENCL_DATA_TYPE opencl_type;
    TypeDescriptor* type_desc_node;
    ArrayDesc_node_list* array_desc_node_list;
    Statement_node* stmt_node;
    Parameter_node* param_node;
    Parameter_node_list* param_node_list;
    Declaration_node* decl_node;
    Declaration_desc_node* decl_desc_node;
    Declaration_desc_node_list* decl_desc_node_list;
    Declaration_node_list* decl_node_list;
    Function_node* func_node;
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

%token <type_desc_node> OPENCL_TYPE
%token <const_node> CONSTANT
%token <ptr> IDENTIFIER TYPE_NAME
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
%type <opencl_type> struct_or_union
%type <expr_stmt> expression expression_statement
%type <expr_node_list> argument_expression_list
%type <type_desc_node> type_name declaration_specifiers type_specifier specifier_qualifier_list struct_or_union_specifier
%type <array_desc_node_list> pointer
%type <decl_node_list> struct_declaration_list
%type <decl_node> declaration struct_declaration
%type <decl_desc_node> declarator init_declarator direct_declarator abstract_declarator direct_abstract_declarator struct_declarator
%type <decl_desc_node_list> init_declarator_list struct_declarator_list
%type <stmt_node> statement jump_statement
%type <comp_stmt> compound_statement block_item_list block_item
%type <sel_stmt> selection_statement
%type <iter_stmt> iteration_statement
%type <expr_kind> assignment_operator
%type <param_node_list> parameter_type_list parameter_list
%type <param_node> parameter_declaration
%type <func_node> function_definition

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
	: declaration_specifiers ';'
    {
        //DeleteTypeDesc($1);
        $$ = NULL;
    }
	| declaration_specifiers init_declarator_list ';' {$$ = CreateDeclNode($1, $2);}
    | TYPEDEF declaration_specifiers ';'
    {
        $$ = NULL;
    }
    | TYPEDEF declaration_specifiers init_declarator_list ';'
    {
        AddToTypeTable(typeTable, $2, $3);
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
        fprintf(stderr, "[Error] Two type specifier\n");
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
        $$ = $1;
    }
	| enum_specifier
    {
        // TODO
    }
    | TYPE_NAME
    {
        $$ = GetTypeDescFromTypeNameTable(typeTable, $1);
    }
	| OPENCL_TYPE
    {
        $$ = $1;
    }
    ;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
    {
        AddStructDeclNode(program, strdup($2), $4);
        $$ = CreateScalarTypeDesc($1, $2);
    }
	| struct_or_union TYPE_NAME '{' struct_declaration_list '}'
    {
        // struct name is different from typedef name
        AddStructDeclNode(program, strdup($2), $4);
        $$ = CreateScalarTypeDesc($1, $2);
    }
	| struct_or_union '{' struct_declaration_list '}'
    {
        char tmp[50];
        sprintf(tmp, "_UNNAME_STRUCT_%d", structNumber ++);
        AddStructDeclNode(program, strdup(tmp), $3);
        $$ = CreateScalarTypeDesc($1, strdup(tmp));
    }
	| struct_or_union IDENTIFIER
    {
        $$ = CreateScalarTypeDesc($1, $2);
    }
    | struct_or_union TYPE_NAME
    {
        $$ = CreateScalarTypeDesc($1, $2);
    }
	;

struct_or_union
	: STRUCT {$$ = STRUCT_TYPE;}
	| UNION
    {
        /* Behavior is roughly the same as struct */
        $$ = STRUCT_TYPE;
    }
	;

struct_declaration_list
	: struct_declaration {$$ = AppendDeclNodeToList(NULL, $1);}
	| struct_declaration_list struct_declaration {$$ = AppendDeclNodeToList($1, $2);}
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';' {$$ = CreateDeclNode($1, $2);}
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
	: struct_declarator {$$ = AppendDeclDescNodeToList(NULL, $1);}
	| struct_declarator_list ',' struct_declarator {$$ = AppendDeclDescNodeToList($1, $3);}
	;

struct_declarator
	: declarator {$$ = $1;}
	| ':' constant_expression {$$ = NULL;}
	| declarator ':' constant_expression {$$ = $1;}
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
        GetUlongValueInExprNode($4, &expr_value);
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
        GetUlongValueInExprNode($3, &expr_value);
        ArrayDesc_node* tmp = CreateArrayDescNode(expr_value, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    {
        unsigned long expr_value;
        GetUlongValueInExprNode($5, &expr_value);
        ArrayDesc_node* tmp = CreateArrayDescNode(expr_value, ARRAY_DESC_ARRAY);
        $$ = AddArrayDescToDeclDesc($1, tmp);
    }
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    {
        unsigned long expr_value;
        GetUlongValueInExprNode($5, &expr_value);
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
        // TODO: Mix TypeDescriptor
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
        GetUlongValueInExprNode($2, &expr_value);
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
        GetUlongValueInExprNode($3, &expr_value);
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
	: '{' '}' {$$ = NULL;}
    | '{' block_item_list '}' {$$ = $2;}
	;

block_item_list
	: block_item {$$ = $1;}
	| block_item_list block_item {$$ = MergeCompoundStmt($1, $2);}
	;

block_item
    : declaration {$$ = CreateCompoundStmt($1, NULL);}
	| statement {$$ = CreateCompoundStmt(NULL, $1);}
	;

expression_statement
	: ';' {$$ = NULL;}
	| expression ';' {$$ = $1;}
	;

selection_statement
	: IF '(' expression ')' statement
    {
       Selection_node* tmp = CreateSelectionNode(SELECTION_WITH_COND, $3, $5);
       $$ = CreateSelectionStmt(tmp);
    }
	| IF '(' expression ')' statement ELSE statement
    {
        Selection_node* tmp = CreateSelectionNode(SELECTION_WITH_COND, $3, $5);
        SelectionStatement* stmt = CreateSelectionStmt(tmp);
        if ($7->statement_kind == SELECTION_STMT)
        {
            $$ = MergeSelectionStmt(stmt, $7->stmt.selection_stmt);
        }
        else
        {
            $$ = AddToSelectionStmt(stmt, CreateSelectionNode(SELECTION_WITHOUT_COND, NULL, $7));
        }
    }
	| SWITCH '(' expression ')' statement {$$ = NULL;}
	;

iteration_statement
	: WHILE '(' expression ')' statement
    {
        $$ = CreateIterStmt(WHILE_LOOP, NULL, $3, NULL, $5);
    }
	| DO statement WHILE '(' expression ')' ';'
    {
        $$ = CreateIterStmt(DO_WHILE_LOOP, NULL, $5, NULL, $2);
    }
	| FOR '(' expression_statement expression_statement ')' statement
	{
        $$ = CreateIterStmt(FOR_LOOP_WITHOUT_DECL, $3, $4, NULL, $6);
    }
    | FOR '(' expression_statement expression_statement expression ')' statement
    {
        $$ = CreateIterStmt(FOR_LOOP_WITHOUT_DECL, $3, $4, $5, $7);
    }
	| FOR '(' declaration expression_statement ')' statement
    {
        $$ = CreateIterStmt(FOR_LOOP_WITH_DECL, $3, $4, NULL, $6);
    }
	| FOR '(' declaration expression_statement expression ')' statement
    {
        $$ = CreateIterStmt(FOR_LOOP_WITH_DECL, $3, $4, $5, $7);
    }
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
	: function_definition {AddFuncNodeToProgram(program, $1);}
	| declaration {AddDeclNodeToProgram(program, $1);}
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
    {
        // deprecated (old syntax): e.g. int func(A, B, C) int A; float B; char C; {...}
        $$ = NULL;
    }
    | declaration_specifiers declarator compound_statement
    {
        $$ = CreateFunctionNode($1, $2, $3);
    }
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
