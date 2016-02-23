#include <stdio.h>
#include <stdlib.h>
#include "ASTDefinition.h"
#include "TraceGenerator.h"
#include "string.h"

extern int  yyparse();
extern FILE *yyin;
extern TypeNameTable* typeTable;
extern Program_node* program;
extern SymbolTable* symTable;
extern Operation_list* opTrace;

__inline__ void DebugAlignment(int align)
{
    while (align --)
        printf("\t");
}

void DebugProgramNode(Program_node* prog)
{
    if (!prog) return;
    else
    {
        StructDeclaration_node* iterStruct = prog->struct_head;
        Declaration_node* iterDecl = prog->declaration_head;
        Function_node* iterFunc = prog->function_head;
        while (iterStruct != NULL)
        {
            DebugStructDeclNode(iterStruct);
            iterStruct = iterStruct->next;
        }
        while (iterDecl != NULL)
        {
            DebugDeclNode(iterDecl, 0);
            iterDecl = iterDecl->next;
        }
        while (iterFunc != NULL)
        {
            DebugFuncNode(iterFunc);
            iterFunc = iterFunc->next;
        }
    }
}

void DebugStructDeclNode(StructDeclaration_node* struct_node)
{
    if (!struct_node) return;
    else
    {
        Declaration_node* iterNode = struct_node->member_head;
        printf("======================================================================================\n");
        printf("[Struct Name] %s\n", struct_node->struct_name);
        while (iterNode != NULL)
        {
            DebugDeclNode(iterNode, 1);
            iterNode = iterNode->next;
        }
    }
}

void DebugFuncNode(Function_node* func)
{
    if (!func) return;
    else
    {
        Parameter_node* iterParam;
        char type_name[1000];
        printf("======================================================================================\n");
        printf("[Function Name] %s\n", func->function_name);
        DebugTypeDesc(func->return_type, type_name);
        printf("\t[Retrun type] %s\n", type_name);
        if (func->parameter_head == NULL)
            printf("\t[Parameter] NULL\n");
        else
        {
            printf("\t[Parameter]\n");
            iterParam = func->parameter_head;
            while (iterParam)
            {
                DebugParamNode(iterParam, 2);
                iterParam = iterParam->next;
            }
        }

        if (func->content_statement == NULL)
            printf("\t[Content] NULL\n");
        else
        {
            printf("\t[Content]\n");
            DebugCompoundStmt(func->content_statement, 2);
        }
    }
}

void DebugParamNode(Parameter_node* param, int align)
{
    if (!param) return;
    else
    {
        char type_name[1000];
        TypeDescriptor* mix_type;
        if (param->parameter_desc != NULL)
        {
            mix_type = MixAndCreateTypeDesc(param->parameter_type, param->parameter_desc->identifier_type);
            DebugTypeDesc(mix_type, type_name);
            DeleteTypeDesc(mix_type);
        }
        else
        {
            DebugTypeDesc(param->parameter_type, type_name);
        }
        DebugAlignment(align);
        printf("[Type] %s\n", type_name);
        DebugAlignment(align);
        printf("[Name] %s\n", param->parameter_desc->identifier_name);

        if (param->parameter_desc->init_expression != NULL)
            fprintf(stderr, "[Error] Parameters should not have initial value in %s\n", __func__);
    }
}

void DebugCompoundStmt(CompoundStatement* stmt, int align)
{
    if (!stmt) return;
    else
    {
        DebugAlignment(align);
        printf("[COMPOUND]\n");
        if (stmt->declaration_head == NULL) {;}
        else
        {
            Declaration_node* iterDecl = stmt->declaration_head;
            while (iterDecl)
            {
                DebugDeclNode(iterDecl, align+1);
                iterDecl = iterDecl->next;
            }
        }

        if (stmt->statement_head == NULL) {;}
        else
        {
            Statement_node* iterStmt = stmt->statement_head;
            while (iterStmt)
            {
                DebugStmtNode(iterStmt, align+1);
                iterStmt = iterStmt->next;
            }
        }
    }
}

void DebugStmtNode(Statement_node* stmt, int align)
{
    if (!stmt) return;
    else
    {
        switch (stmt->statement_kind)
        {
            case ITERATION_STMT:
                DebugIterStmt(stmt->stmt.iteration_stmt, align);
                break;
            case SELECTION_STMT:
                DebugSelectionStmt(stmt->stmt.selection_stmt, align);
                break;
            case EXPRESSION_STMT:
                DebugExprStmt(stmt->stmt.expression_stmt, align);
                break;
            case RETURN_STMT:
                DebugReturnStmt(stmt->stmt.return_stmt, align);
                break;
            case COMPOUND_STMT:
                DebugCompoundStmt(stmt->stmt.compound_stmt, align);
                break;
            case EMPTY_GOTO_STMT:
                break;
            case EMPTY_CONTINUE_STMT:
                DebugAlignment(align);
                printf("[CONTINUE]\n");
                break;
            case EMPTY_BREAK_STMT:
                DebugAlignment(align);
                printf("[BREAK]\n");
                break;
            case EMPTY_RETURN_STMT:
                DebugAlignment(align);
                printf("[EMPTY RETURN]\n");
                break;
        }
    }
}

void DebugIterStmt(IterationStatement* stmt, int align)
{
    if (!stmt) return;
    else
    {
        switch (stmt->kind)
        {
            case FOR_LOOP_WITH_DECL:
                DebugAlignment(align);
                printf("[FOR LOOP W/ DECL]\n");
                DebugAlignment(align);
                printf("[Initial]\n");
                DebugDeclNode(stmt->init.declaration, align+1);
                DebugAlignment(align);
                printf("[Terminate condition]\n");
                DebugExprStmt(stmt->terminated_expression, align+1);
                DebugAlignment(align);
                printf("[Step]\n");
                DebugExprStmt(stmt->step_expression, align+1);
                break;
            case FOR_LOOP_WITHOUT_DECL:
                DebugAlignment(align);
                printf("[FOR LOOP W/O DECL]\n");
                DebugAlignment(align);
                printf("[Initial]\n");
                DebugExprStmt(stmt->init.expression, align+1);
                DebugAlignment(align);
                printf("[Terminate condition]\n");
                DebugExprStmt(stmt->terminated_expression, align+1);
                DebugAlignment(align);
                printf("[Step]\n");
                DebugExprStmt(stmt->step_expression, align+1);
                break;
            case WHILE_LOOP:
                DebugAlignment(align);
                printf("[WHILE LOOP]\n");
                DebugAlignment(align);
                printf("[Terminate condition]\n");
                DebugExprStmt(stmt->terminated_expression, align+1);
                break;
            case DO_WHILE_LOOP:
                DebugAlignment(align);
                printf("[DO WHILE LOOP]\n");
                DebugAlignment(align);
                printf("[Terminate condition]\n");
                DebugExprStmt(stmt->terminated_expression, align+1);
                break;
        }
        if (stmt->content_statement == NULL)
        {
            DebugAlignment(align);
            printf("[Content] NULL\n");
        }
        else
        {
            DebugAlignment(align);
            printf("[Content]\n");
            DebugStmtNode(stmt->content_statement, align+1);
        }
    }
}

void DebugSelectionStmt(SelectionStatement* stmt, int align)
{
    if (!stmt) return;
    else
    {
        Selection_node* iterNode = stmt->selection_head;
        DebugAlignment(align);
        printf("[SELECTION]\n");
        while (iterNode)
        {
            if (iterNode == stmt->selection_head)
            {
                DebugAlignment(align);
                printf("[If]\n");
                DebugSelectionNode(iterNode, align+1);
            }
            else if (iterNode == stmt->selection_tail)
            {
                DebugAlignment(align);
                printf("[Else]\n");
                DebugSelectionNode(iterNode, align+1);
            }
            else
            {
                DebugAlignment(align);
                printf("[Else if]\n");
                DebugSelectionNode(iterNode, align+1);
            }
            iterNode = iterNode->next;
        }
    }
}

void DebugSelectionNode(Selection_node* node, int align)
{
    if (!node) return;
    else
    {
        DebugAlignment(align);
        printf("[Condition]\n");
        DebugExprStmt(node->condition_expression, align+1);
        DebugAlignment(align);
        printf("[Content]\n");
        DebugStmtNode(node->content_statement, align+1);
    }
}

void DebugReturnStmt(ReturnStatement* stmt, int align)
{
    if (!stmt) return;
    else
    {
        Expression_node* iterNode = stmt->expression_head;
        DebugAlignment(align);
        printf("[RETURN]\n");
        while (iterNode)
        {
            DebugAlignment(align);
            printf("[Expression]\n");
            DebugExprNode(iterNode, align+1);
            iterNode = iterNode->next;
        }
    }
}

void DebugExprStmt(ExpressionStatement* stmt, int align)
{
    if (!stmt) return;
    else
    {
        Expression_node* iterNode = stmt->expression_head;
        DebugAlignment(align);
        printf("[EXPR_STMT]\n");
        while (iterNode)
        {
            DebugAlignment(align);
            printf("[Expression]\n");
            DebugExprNode(iterNode, align+1);
            iterNode = iterNode->next;
        }
    }
}

void DebugExprNode(Expression_node* node, int align)
{
    if (!node) return;
    else
    {
        if (node->expression_kind & EXPRESSION_MASK)
        {
            DebugExprNode(node->left_operand, align);
            DebugExprNode(node->right_operand, align);
            switch (node->expression_kind)
            {
                case EXPRESSION_IDENTIFIER:
                    DebugAlignment(align);
                    printf("[Identifier] %s\n", node->direct_expr.identifier);
                    break;
                case EXPRESSION_CONSTANT:
                    DebugConstantNode(node->direct_expr.constant, align);
                    break;
                case EXPRESSION_SUBSCRIPT:
                    DebugAlignment(align);
                    printf("[Subscript]\n");
                    DebugExprStmt(node->direct_expr.subscript, align+1);
                    break;
                case EXPRESSION_FUNCTION:
                    DebugAlignment(align);
                    printf("[FUNCTION_CALL]\n");
                    DebugFuncInvocationNode(node->direct_expr.function, align+1);
                    break;
                case EXPRESSION_MEMBER:
                    DebugAlignment(align);
                    printf("[Member] %s\n", node->direct_expr.member);
                    break;
                case EXPRESSION_TYPECAST:
                    {
                        char type_name[1000];
                        DebugAlignment(align);
                        DebugTypeDesc(node->direct_expr.target_type, type_name);
                        printf("[TypeCast] %s\n", type_name);
                        break;
                    }
                case EXPRESSION_EXPRSTMT:
                    DebugExprStmt(node->direct_expr.expr_stmt, align+1);
                    break;
            }
        }
        else if (node->expression_kind & ASSIGNMENT_MASK)
        {
            char op_name[30];
            DebugExprKind(node->expression_kind, op_name);
            DebugAlignment(align);
            printf("[ASSIGNMENT] %s\n", op_name);
            DebugAlignment(align);
            printf("[lvalue]\n");
            DebugExprNode(node->left_operand, align+1);
            DebugAlignment(align);
            printf("[rvalue]\n");
            DebugExprNode(node->right_operand, align+1);
        }
        else // OP_MASK
        {
            char op_name[30];
            DebugExprNode(node->left_operand, align);
            DebugExprNode(node->right_operand, align);
            DebugExprKind(node->expression_kind, op_name);
            DebugAlignment(align);
            printf("%s\n", op_name);
        }
    }
}

void DebugDeclNode(Declaration_node* decl, int align)
{
    if (!decl) return;
    else
    {
        Declaration_desc_node* iterDecl = decl->declaration_desc_head;
        while (iterDecl)
        {
            char type_name[1000];
            TypeDescriptor* mix_type = MixAndCreateTypeDesc(decl->declaration_type, iterDecl->identifier_type);
            DebugTypeDesc(mix_type, type_name);
            DeleteTypeDesc(mix_type);

            DebugAlignment(align);
            printf("[DECLARATION]\n");
            DebugAlignment(align);
            printf("[Type] %s\n", type_name);
            DebugAlignment(align);
            printf("[Name] %s\n", iterDecl->identifier_name);
            if (iterDecl->init_expression != NULL)
            {
                DebugAlignment(align);
                printf("[Initial]\n");
                DebugExprNode(iterDecl->init_expression, align+1);
            }
            iterDecl = iterDecl->next;
        }
    }
}

void DebugExprKind(EXPRESSION_KIND kind, char* name)
{
    switch(kind)
    {
        case NONE_OP:
			sprintf(name, "NONE_OP");
			break;
        case ADDITION_OP:
			sprintf(name, "ADDITION_OP");
			break;
        case SUBTRACTION_OP:
			sprintf(name, "SUBTRACTION_OP");
			break;
        case MULTIPLICATION_OP:
			sprintf(name, "MULTIPLICATION_OP");
			break;
        case DIVISION_OP:
			sprintf(name, "DIVISION_OP");
			break;
        case MODULAR_OP:
			sprintf(name, "MODULAR_OP");
			break;
        case POST_INCREASE_OP:
			sprintf(name, "POST_INCREASE_OP");
			break;
        case POST_DECREASE_OP:
			sprintf(name, "POST_DECREASE_OP");
			break;
        case PRE_INCREASE_OP:
			sprintf(name, "PRE_INCREASE_OP");
			break;
        case PRE_DECREASE_OP:
			sprintf(name, "PRE_DECREASE_OP");
			break;
        case SHIFT_LEFT_OP:
			sprintf(name, "SHIFT_LEFT_OP");
			break;
        case SHIFT_RIGHT_OP:
			sprintf(name, "SHIFT_RIGHT_OP");
			break;
        case LESS_OP:
			sprintf(name, "LESS_OP");
			break;
        case LESS_EQUAL_OP:
			sprintf(name, "LESS_EQUAL_OP");
			break;
        case GREATER_OP:
			sprintf(name, "GREATER_OP");
			break;
        case GREATER_EQUAL_OP:
			sprintf(name, "GREATER_EQUAL_OP");
			break;
        case EQUAL_OP:
			sprintf(name, "EQUAL_OP");
			break;
        case NOT_EQUAL_OP:
			sprintf(name, "NOT_EQUAL_OP");
			break;
        case BITWISE_AND_OP:
			sprintf(name, "BITWISE_AND_OP");
			break;
        case BITWISE_XOR_OP:
			sprintf(name, "BITWISE_XOR_OP");
			break;
        case BITWISE_OR_OP:
			sprintf(name, "BITWISE_OR_OP");
			break;
        case BITWISE_COMPLEMENT_OP:
            sprintf(name, "BITWISE_COMPLEMENT_OP");
            break;
        case UNARY_PLUS_OP:
            sprintf(name, "UNARY_PLUS_OP");
            break;
        case UNARY_MINUS_OP:
            sprintf(name, "UNARY_MINUS_OP");
            break;
        case LOGICAL_AND_OP:
			sprintf(name, "LOGICAL_AND_OP");
			break;
        case LOGICAL_OR_OP:
			sprintf(name, "LOGICAL_OR_OP");
			break;
        case LOGICAL_COMPLEMENT_OP:
            sprintf(name, "LOGICAL_COMPLEMENT_OP");
            break;
        case ASSIGNMENT_NONE:
			sprintf(name, "ASSIGNMENT_NONE");
			break;
        case ASSIGNMENT_MUL:
			sprintf(name, "ASSIGNMENT_MUL");
			break;
        case ASSIGNMENT_DIV:
			sprintf(name, "ASSIGNMENT_DIV");
			break;
        case ASSIGNMENT_MOD:
			sprintf(name, "ASSIGNMENT_MOD");
			break;
        case ASSIGNMENT_ADD:
			sprintf(name, "ASSIGNMENT_ADD");
			break;
        case ASSIGNMENT_SUB:
			sprintf(name, "ASSIGNMENT_SUB");
			break;
        case ASSIGNMENT_LEFT:
			sprintf(name, "ASSIGNMENT_LEFT");
			break;
        case ASSIGNMENT_RIGHT:
			sprintf(name, "ASSIGNMENT_RIGHT");
			break;
        case ASSIGNMENT_AND:
			sprintf(name, "ASSIGNMENT_AND");
			break;
        case ASSIGNMENT_XOR:
			sprintf(name, "ASSIGNMENT_XOR");
			break;
        case ASSIGNMENT_OR:
			sprintf(name, "ASSIGNMENT_OR");
			break;
    }
}

void DebugTypeDesc(TypeDescriptor* type, char* name)
{
    if (!type)
    {
        strcpy(name, "");
        return;
    }
    else
    {
        ArrayDesc_node* iterArray = type->array_desc_head;
        strcpy(name, "");

        while (iterArray != NULL)
        {
            if (iterArray->desc_kind == ARRAY_DESC_POINTER)
            {
                strcat(name, "Pointer to ");
            }
            else if (iterArray->desc_kind == ARRAY_DESC_ARRAY)
            {
                char tmp[50];
                if (iterArray->size != 0)
                    sprintf(tmp, "Array (%lu) of ", iterArray->size);
                else
                    sprintf(tmp, "Array (*) of ");
                strcat(name, tmp);
            }
            else
            {
                int parameterCount = 0;
                char tmp[100];
                Parameter_node* iterParam = iterArray->parameter_head;
                while (iterParam)
                {
                    parameterCount ++;
                    iterParam = iterParam->next;
                }
                sprintf(tmp, "Function pointer that takes %d arguments and returns ", parameterCount);
                strcat(name, tmp);
            }
            iterArray = iterArray->next;
        }

        if (type->kind == TYPE_WITH_PARAM)
        {
            int parameterCount = 0;
            char tmp[100];
            Parameter_node* iterParam = type->parameter_head;
            while (iterParam)
            {
                parameterCount ++;
                iterParam = iterParam->next;
            }
            sprintf(tmp, "Function that takes %d arguments and returns ", parameterCount);
            strcat(name, tmp);
        }

        switch (type->type)
        {
            char tmpName[50];
            case NONE_TYPE:
                strcat(name, "NONE_TYPE");
                break;
            case STRUCT_TYPE:
                sprintf(tmpName, "STRUCT_TYPE (%s)", type->struct_name);
                strcat(name, tmpName);
                break;
            case UNION_TYPE:
                strcat(name, "UNION_TYPE");
                break;
            case BOOL_TYPE:
                strcat(name, "BOOL_TYPE");
                break;
            case HALF_TYPE:
                strcat(name, "HALF_TYPE");
                break;
            case VOID_TYPE:
                strcat(name, "VOID_TYPE");
                break;
            case CHAR_TYPE:
                strcat(name, "CHAR_TYPE");
                break;
            case CHAR2_TYPE:
                strcat(name, "CHAR2_TYPE");
                break;
            case CHAR4_TYPE:
                strcat(name, "CHAR4_TYPE");
                break;
            case CHAR8_TYPE:
                strcat(name, "CHAR8_TYPE");
                break;
            case CHAR16_TYPE:
                strcat(name, "CHAR16_TYPE");
                break;
            case UCHAR_TYPE:
                strcat(name, "UCHAR_TYPE");
                break;
            case UCHAR2_TYPE:
                strcat(name, "UCHAR2_TYPE");
                break;
            case UCHAR4_TYPE:
                strcat(name, "UCHAR4_TYPE");
                break;
            case UCHAR8_TYPE:
                strcat(name, "UCHAR8_TYPE");
                break;
            case UCHAR16_TYPE:
                strcat(name, "UCHAR16_TYPE");
                break;
            case SHORT_TYPE:
                strcat(name, "SHORT_TYPE");
                break;
            case SHORT2_TYPE:
                strcat(name, "SHORT2_TYPE");
                break;
            case SHORT4_TYPE:
                strcat(name, "SHORT4_TYPE");
                break;
            case SHORT8_TYPE:
                strcat(name, "SHORT8_TYPE");
                break;
            case SHORT16_TYPE:
                strcat(name, "SHORT16_TYPE");
                break;
            case USHORT_TYPE:
                strcat(name, "USHORT_TYPE");
                break;
            case USHORT2_TYPE:
                strcat(name, "USHORT2_TYPE");
                break;
            case USHORT4_TYPE:
                strcat(name, "USHORT4_TYPE");
                break;
            case USHORT8_TYPE:
                strcat(name, "USHORT8_TYPE");
                break;
            case USHORT16_TYPE:
                strcat(name, "USHORT16_TYPE");
                break;
            case INT_TYPE:
                strcat(name, "INT_TYPE");
                break;
            case INT2_TYPE:
                strcat(name, "INT2_TYPE");
                break;
            case INT4_TYPE:
                strcat(name, "INT4_TYPE");
                break;
            case INT8_TYPE:
                strcat(name, "INT8_TYPE");
                break;
            case INT16_TYPE:
                strcat(name, "INT16_TYPE");
                break;
            case UINT_TYPE:
                strcat(name, "UINT_TYPE");
                break;
            case UINT2_TYPE:
                strcat(name, "UINT2_TYPE");
                break;
            case UINT4_TYPE:
                strcat(name, "UINT4_TYPE");
                break;
            case UINT8_TYPE:
                strcat(name, "UINT8_TYPE");
                break;
            case UINT16_TYPE:
                strcat(name, "UINT16_TYPE");
                break;
            case LONG_TYPE:
                strcat(name, "LONG_TYPE");
                break;
            case LONG2_TYPE:
                strcat(name, "LONG2_TYPE");
                break;
            case LONG4_TYPE:
                strcat(name, "LONG4_TYPE");
                break;
            case LONG8_TYPE:
                strcat(name, "LONG8_TYPE");
                break;
            case LONG16_TYPE:
                strcat(name, "LONG16_TYPE");
                break;
            case ULONG_TYPE:
                strcat(name, "ULONG_TYPE");
                break;
            case ULONG2_TYPE:
                strcat(name, "ULONG2_TYPE");
                break;
            case ULONG4_TYPE:
                strcat(name, "ULONG4_TYPE");
                break;
            case ULONG8_TYPE:
                strcat(name, "ULONG8_TYPE");
                break;
            case ULONG16_TYPE:
                strcat(name, "ULONG16_TYPE");
                break;
            case FLOAT_TYPE:
                strcat(name, "FLOAT_TYPE");
                break;
            case FLOAT2_TYPE:
                strcat(name, "FLOAT2_TYPE");
                break;
            case FLOAT4_TYPE:
                strcat(name, "FLOAT4_TYPE");
                break;
            case FLOAT8_TYPE:
                strcat(name, "FLOAT8_TYPE");
                break;
            case FLOAT16_TYPE:
                strcat(name, "FLOAT16_TYPE");
                break;
            case DOUBLE_TYPE:
                strcat(name, "DOUBLE_TYPE");
                break;
            case DOUBLE2_TYPE:
                strcat(name, "DOUBLE2_TYPE");
                break;
            case DOUBLE4_TYPE:
                strcat(name, "DOUBLE4_TYPE");
                break;
            case DOUBLE8_TYPE:
                strcat(name, "DOUBLE8_TYPE");
                break;
            case DOUBLE16_TYPE:
                strcat(name, "DOUBLE16_TYPE");
                break;
        }
    }
}

void DebugConstantNode(Constant_node* node, int align)
{
    if (!node) return;
    else
    {
        OPENCL_DATA_TYPE currType;

        if (node->constant_type->array_desc_head != NULL)
            fprintf(stderr, "[Error] Invalid array desc in %s\n", __func__);
        if (node->constant_type->kind != TYPE_WITHOUT_PARAM)
            fprintf(stderr, "[Error] Invalid parameter definition in %s\n", __func__);
        DebugAlignment(align);
        printf("[Constant] ");

        currType = node->constant_type->type;
        if (currType & CONST_SIGNED_INTEGER_MASK)
            printf("%ld\n", node->value.long_val);
        else if (currType & CONST_UNSIGNED_INTEGER_MASK)
            printf("%lu\n", node->value.ulong_val);
        else if (currType & CONST_FLOAT_MASK)
            printf("%lf\n", node->value.double_val);

        /* TODO vector type constant */
    }
}

void DebugFuncInvocationNode(FunctionInvocation_node* node, int align)
{
    if (!node) return;
    else
    {
        Expression_node* iterExpr = node->argument_head;
        DebugAlignment(align);
        printf("[Name] %s\n", node->name);
        DebugAlignment(align);
        printf("[Arguments]\n");
        while (iterExpr)
        {
            DebugExprNode(iterExpr, align+1);
            iterExpr = iterExpr->next;
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc<2)
    {
        printf("Please select an input file\n");
        exit(0);
    }
    FILE *fp=fopen(argv[1],"r");
    if(!fp)
    {
        printf("couldn't open file for reading\n");
        exit(0);
    }

    char funcName[100];
    StmtRepresentation* result = NULL;

    yyin=fp;

    typeTable = CreateTypeNameTable();
    program = CreateProgramNode();
    yyparse();
    DebugProgramNode(program);

    printf("Enter a function name to be traced: ");
    scanf("%s", funcName);
    opTrace = NULL;
    symTable = CreateSymTable();
    g_operation_id = 0;
    lastIssueOP = NULL;

    result = TraceFuncNode(program, funcName, NULL);
    DeleteStmtRepresentation(result);
//    ShowOPTrace(opTrace);

    /* Delete Symbol Table */

    DeleteTypeNameTable(typeTable);
    DeleteProgramNode(program);
    fclose(fp);
}
