#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TraceGenerator.h"

extern SymbolTable* symTable;
extern Operation_list* opTrace;
extern Program_node* program;

SymbolTable* CreateSymTable()
{
    SymbolTable* ret = (SymbolTable*) malloc(sizeof(SymbolTable));
    ret->level_head = NULL;
    ret->level_tail = NULL;

    return ret;
}

void CreateSymTableLevel(SymbolTable* table)
{
    if (!table)
    {
        fprintf(stderr, "[Error] Symbol table hasn't been created yet in %s\n", __func__);
        return;
    }
    else
    {
        SymbolTableLevel* new_level = (SymbolTableLevel*) malloc(sizeof(SymbolTableLevel));
        new_level->entry_head = NULL;
        new_level->entry_tail = NULL;
        new_level->prev = NULL;
        new_level->next = NULL;

        if (table->level_head == NULL)
        {
            table->level_head = new_level;
            table->level_tail = new_level;
        }
        else
        {
            table->level_tail->next = new_level;
            new_level->prev = table->level_tail;
            table->level_tail = new_level;
        }
    }
}

void DeleteSymTableEntry(SymbolTableEntry* entry)
{
    if (!entry)
        return;
    else
    {
        SymbolTableEntry* iterEntry = entry->member_head;
        SymbolTableEntry* nextEntry;
        int array_idx;

        while (iterEntry != NULL)
        {
            nextEntry = iterEntry->next;
            DeleteSymTableEntry(iterEntry);
            iterEntry = nextEntry;
        }
        entry->next = NULL;

        for (array_idx = 0 ; array_idx < entry->array_dim ; array_idx ++)
        {
            DeleteSymTableEntry(entry->array_entry[array_idx]);
        }
        free (entry->array_entry);

        DeleteSemanticValue(entry->value);
        if (entry->name)
            free (entry->name);

        free (entry);
    }
}

// Can only delete last level
void DeleteSymTableLevel(SymbolTable* table)
{
    if (!table)
    {
        fprintf(stderr, "[Error] Symbol table hasn't been created yet in %s\n", __func__);
        return;
    }
    else
    {
        SymbolTableLevel* curr_level = table->level_tail;
        SymbolTableEntry* iterEntry = curr_level->entry_head;
        SymbolTableEntry* nextEntry;

        while (iterEntry != NULL)
        {
            nextEntry = iterEntry->next;
            DeleteSymTableEntry(iterEntry);
            iterEntry = nextEntry;
        }

        if (table->level_head != curr_level)
            curr_level->prev->next = curr_level->next;
        curr_level->next = NULL;
        free (curr_level);
    }
}

void AddEntryToSymTable(SymbolTable* table, SymbolTableEntry* new_entry)
{
    if (!table)
    {
        fprintf(stderr, "[Error] Symbol table hasn't been created yet in %s\n", __func__);
        return;
    }
    else if (!table->level_tail)
    {
        fprintf(stderr, "[Error] Need to create symbol table level first in %s\n", __func__);
        return;
    }
    else
    {
        SymbolTableLevel* curr_level = table->level_tail;
        printf("Insert entry to table ...\n");
        if (new_entry == NULL)
        {
            fprintf(stderr, "Given symbol table entry is NULL in %s\n", __func__);
        }
        if (curr_level->entry_head == NULL)
        {
            curr_level->entry_head = new_entry;
            curr_level->entry_tail = new_entry;
        }
        else
        {
            curr_level->entry_tail->next = new_entry;
            curr_level->entry_tail = new_entry;
        }
    }
}

void AddEntryListToSymTable(SymbolTable* table, SymbolTableEntry_list* entry_list)
{
    if (!table)
    {
        fprintf(stderr, "[Error] Symbol table hasn't been created yet in %s\n", __func__);
        return;
    }
    else if (!table->level_tail)
    {
        fprintf(stderr, "[Error] Need to create symbol table level first in %s\n", __func__);
        return;
    }
    else
    {
        SymbolTableLevel* curr_level = table->level_tail;
        printf("Insert entry list to table ...\n");
        if (entry_list == NULL)
        {
            fprintf(stderr, "Given symbol table entry list is NULL in %s\n", __func__);
        }
        else if (curr_level->entry_head == NULL)
        {
            curr_level->entry_head = entry_list->entry_head;
            curr_level->entry_tail = entry_list->entry_tail;
        }
        else
        {
            curr_level->entry_tail->next = entry_list->entry_head;
            curr_level->entry_tail = entry_list->entry_tail;
        }
    }
}

SemanticRepresentation* DuplicateSemanticRepresentation(SemanticRepresentation* value)
{
    if (!value)
        return NULL;
    else
    {
        SemanticRepresentation* ret = (SemanticRepresentation*) malloc(sizeof(SemanticRepresentation));
        ret->type = DuplicateTypeDesc(value->type);
        ret->value = DuplicateSemanticValue(value->value);
        ret->lvalue = value->lvalue;
        ret->next = value->next;
        return ret;
    }
}

SemanticValue* DuplicateSemanticValue(SemanticValue* value)
{
    if (!value)
        return NULL;
    else
    {
        SemanticValue* ret = (SemanticValue*) malloc(sizeof(SemanticValue));
        ret->kind = value->kind;
        ret->type = value->type;
        ret->vector = value->vector;
        ret->lastOP = value->lastOP;
        memcpy(&(ret->constVal), &(value->constVal), sizeof(ret->constVal));
        return ret;
    }
}

void AssignToSymTableEntry(SymbolTableEntry* entry, SemanticRepresentation* value)
{
    if (!entry)
    {
        fprintf(stderr, "[Error] Given symbol table entry is NULL in %s\n", __func__);
    }
    else
    {
        /* TODO type casting */
        entry->value = DuplicateSemanticValue(value->value);
    }
}

SymbolTableEntry_list* AppendSymTableEntryToList(SymbolTableEntry_list* origin_list, SymbolTableEntry* entry)
{
    if (!entry)
        return origin_list;

    if (origin_list == NULL)
    {
        origin_list = (SymbolTableEntry_list*) malloc(sizeof(SymbolTableEntry_list));
        origin_list->entry_head = entry;
        origin_list->entry_tail = entry;
    }
    else
    {
        origin_list->entry_tail->next = entry;
        origin_list->entry_tail = entry;
    }
    return origin_list;
}

SymbolTableEntry* FindMemberInSymTable(SymbolTableEntry* entry, char* name)
{
    if (!entry)
    {
        fprintf(stderr, "[Error] Given symbol entry is NULL in %s\n", __func__);
        return NULL;
    }
    else
    {
        SymbolTableEntry* iterEntry = entry->member_head;
        while (iterEntry != NULL)
        {
            if (strcmp(iterEntry->name, name) == 0)
                return iterEntry;
            iterEntry = iterEntry->next;
        }
        fprintf(stderr, "[Error] Given member name \'%s\' is not found in %s\n", name, __func__);
        return NULL;
    }
}

SymbolTableEntry* FindSymbolInSymTable(SymbolTable* table, char* name)
{
    if (!table)
    {
        fprintf(stderr, "[Error] Symbol table hasn't been created yet in %s\n", __func__);
        return NULL;
    }
    else if (!table->level_tail)
    {
        fprintf(stderr, "[Error] Need to create symbol table level first in %s\n", __func__);
        return NULL;
    }
    else
    {
        SymbolTableLevel* iterLevel = table->level_tail;
        while (iterLevel != NULL)
        {
            SymbolTableEntry* iterEntry = iterLevel->entry_head;
            while (iterEntry != NULL)
            {
                if (strcmp(iterEntry->name, name) == 0)
                    return iterEntry;
                iterEntry = iterEntry->next;
            }
            iterLevel = iterLevel->prev;
        }
        fprintf(stderr, "[Error] Given symbol name \'%s\' is not found in %s\n", name, __func__);
        return NULL;
    }
}

SymbolTableEntry* CreateSymTableEntry(Program_node* prog, char* name, TypeDescriptor* type)
{
    if (!type)
        return NULL;
    else
    {
        SymbolTableEntry* ret = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
        ret->type = type;
        ret->value = NULL;
        ret->name = name;
        ret->next = NULL;
        ret->member_head = NULL;
        ret->member_tail = NULL;
        ret->array_entry = NULL;
        ret->array_dim = 0;

        if (type->array_desc_head != NULL)
        {
            ArrayDesc_node* currArray = type->array_desc_head;
            if (currArray->desc_kind == ARRAY_DESC_ARRAY)
            {
                int arrayIdx;
                TypeDescriptor* elem_type = DuplicateTypeDesc(type);
                ArrayDesc_node* currArray = elem_type->array_desc_head;
                elem_type->array_desc_head = elem_type->array_desc_head->next;
                DeleteArrayDescNode(currArray);

                if (currArray->size > 0)
                {
                    ret->array_dim = currArray->size;
                    ret->array_entry = (SymbolTableEntry**) malloc(sizeof(SymbolTableEntry*) * ret->array_dim);
                }
                for (arrayIdx = 0 ; arrayIdx < ret->array_dim ; arrayIdx ++)
                {
                    (ret->array_entry)[arrayIdx] = CreateSymTableEntry(prog, NULL, DuplicateTypeDesc(elem_type));
                }
                DeleteTypeDesc(elem_type);
            }
        }
        else if (type->type == STRUCT_TYPE) // array_desc is NULL
        {
            StructDeclaration_node* iterStruct = prog->struct_head;
            while (iterStruct != NULL)
            {
                if (strcmp(type->struct_name, iterStruct->struct_name) == 0)
                    break;
                iterStruct = iterStruct->next;
            }
            if (iterStruct == NULL)
            {
                fprintf(stderr, "[Error] undefined struct name in %s\n", __func__);
                return NULL;
            }
            else
            {
                Declaration_node* iterMember = iterStruct->member_head;
                while (iterMember != NULL)
                {
                    SymbolTableEntry_list* entry_list = TraceDeclNode(iterMember);

                    if (ret->member_head == NULL)
                    {
                        ret->member_head = entry_list->entry_head;
                        ret->member_tail = entry_list->entry_tail;
                    }
                    else
                    {
                        ret->member_tail->next = entry_list->entry_head;
                        ret->member_tail = entry_list->entry_tail;
                    }

                    free (entry_list);
                    iterMember = iterMember->next;
                }
            }
        }

        return ret;
    }
}

void DeleteSemanticValue(SemanticValue* value)
{
    if (!value)
        return;
    else
    {
        value->lastOP = NULL;
        free (value);
    }
}

void DeleteSemanticRepresentation(SemanticRepresentation* value)
{
    if (!value)
        return;
    else
    {
        DeleteTypeDesc(value->type);
        DeleteSemanticValue(value->value);
        value->lvalue = NULL;
        value->next = NULL;
        free (value);
    }
}

NDRangeVector CreateEmptyNDRangeVector()
{
    NDRangeVector ret;
    ret.globalIdx0 = 0;
    ret.globalIdx1 = 0;
    ret.globalIdx2 = 0;
    ret.groupIdx0 = 0;
    ret.groupIdx1 = 0;
    ret.groupIdx2 = 0;
    ret.localIdx0 = 0;
    ret.localIdx1 = 0;
    ret.localIdx2 = 0;
    return ret;
}

SemanticValue* CreateEmptySemanticValue()
{
    SemanticValue* ret = (SemanticValue*) malloc(sizeof(SemanticValue));
    ret->kind = VALUE_REGULAR;
    ret->type = VALUE_OTHER;
    ret->lastOP = NULL;
    ret->vector = CreateEmptyNDRangeVector();
    memset(&(ret->constVal), 0, sizeof(ret->constVal));
    return ret;
}

SEMANTIC_VALUE_TYPE TypeDescToSemanticValueType(TypeDescriptor* type)
{
    if (!type)
        return VALUE_OTHER;
    else
    {
        if (type->array_desc_head != NULL)
        {
            if (type->array_desc_head->desc_kind == ARRAY_DESC_ARRAY)
                return VALUE_OTHER;
            else
                return VALUE_POINTER;
        }
        else if (type->parameter_head != NULL)
        {
            return VALUE_OTHER;
        }
        else
        {
            if (type->type & CONST_SIGNED_INTEGER_MASK)
                return VALUE_SIGNED_INTEGER;
            else if (type->type & CONST_UNSIGNED_INTEGER_MASK)
                return VALUE_UNSIGNED_INTEGER;
            else if (type->type & CONST_FLOAT_MASK)
                return VALUE_FLOAT;
            else
                return VALUE_OTHER;
        }
    }
}

TypeDescriptor* MergeTypeDesc(TypeDescriptor* left, TypeDescriptor* right)
{
    if (!left && !right)
    {
        fprintf(stderr, "[Error] both type descriptor are NULL in %s\n", __func__);
        return NULL;
    }
    else if (!left)
    {
        return DuplicateTypeDesc(right);
    }
    else if (!right)
    {
        return DuplicateTypeDesc(left);
    }
    else
    {
        if ((left->array_desc_head != NULL) || (right->array_desc_head != NULL))
        {
            return CreateScalarTypeDesc(NONE_TYPE, NULL);
        }
        else if ((left->kind == TYPE_WITH_PARAM) || (right->kind == TYPE_WITH_PARAM))
        {
            return CreateScalarTypeDesc(NONE_TYPE, NULL);
        }
        else if ((left->type & CONST_OTHER_MASK) || (right->type & CONST_OTHER_MASK))
        {
            return CreateScalarTypeDesc(NONE_TYPE, NULL);
        }
        else
        {
            if ((left->type & CONST_FLOAT_MASK) && (right->type & CONST_FLOAT_MASK))
                return CreateScalarTypeDesc( ((left->type > right->type) ? left->type : right->type), NULL);
            else if (left->type & CONST_FLOAT_MASK)
                return CreateScalarTypeDesc(left->type, NULL);
            else if (right->type & CONST_FLOAT_MASK)
                return CreateScalarTypeDesc(right->type, NULL);
            else
            {
                if ((left->type | CONST_TYPE_SIZE_MASK) == (right->type | CONST_TYPE_SIZE_MASK))
                    return CreateScalarTypeDesc( ((left->type > right->type) ? left->type : right->type), NULL);
                else
                {
                    OPENCL_DATA_TYPE mix_type = ((left->type | CONST_TYPE_SIZE_MASK) > (right->type | CONST_TYPE_SIZE_MASK)) ? left->type : right->type;
                    return CreateScalarTypeDesc(mix_type, NULL);
                }
            }
        }
    }
}

void GetValueInSemanticValue(SEMANTIC_VALUE_TYPE type, SemanticValue* value, void* ret)
{
    if (type == VALUE_SIGNED_INTEGER)
    {
        if (value->type == VALUE_SIGNED_INTEGER)
            *((long *)(ret)) = value->constVal.long_val;
        else if (value->type == VALUE_UNSIGNED_INTEGER)
            *((long *)(ret)) = (long)(value->constVal.ulong_val);
        else if (value->type == VALUE_FLOAT)
            *((long *)(ret)) = (long)(value->constVal.double_val);
    }
    else if (type == VALUE_UNSIGNED_INTEGER)
    {
        if (value->type == VALUE_SIGNED_INTEGER)
            *((unsigned long *)(ret)) = (unsigned long)(value->constVal.long_val);
        else if (value->type == VALUE_UNSIGNED_INTEGER)
            *((unsigned long *)(ret)) = value->constVal.ulong_val;
        else if (value->type == VALUE_FLOAT)
            *((unsigned long *)(ret)) = (unsigned long)(value->constVal.double_val);
    }
    else if (type == VALUE_FLOAT)
    {
        if (value->type == VALUE_SIGNED_INTEGER)
            *((double *)(ret)) = (double)(value->constVal.long_val);
        else if (value->type == VALUE_UNSIGNED_INTEGER)
            *((double *)(ret)) = (double)(value->constVal.ulong_val);
        else if (value->type == VALUE_FLOAT)
            *((double *)(ret)) = value->constVal.double_val;
    }
}

SemanticRepresentation* CalculateSemanticRepresentation(EXPRESSION_KIND kind, SemanticRepresentation* left, SemanticRepresentation* right)
{
    if ((!left) && (!right))
    {
        fprintf(stderr, "[Error] Both operand are NULL in %s\n", __func__);
    }
    else
    {
        if (kind == ASSIGNMENT_NONE)
        {
            return DuplicateSemanticRepresentation(right);
        }

        SemanticRepresentation* ret = (SemanticRepresentation*) malloc(sizeof(SemanticRepresentation));
        ret->lvalue = NULL;
        ret->next = NULL;
        ret->value = CreateEmptySemanticValue();

        if (kind & LOGICAL_OP_MASK)
        {
            ret->type = CreateScalarTypeDesc(UINT_TYPE, NULL);
        }
        else
        {
            ret->type = MergeTypeDesc(left->type, right->type);
        }

        if ((left && left->value->kind == VALUE_IRREGULAR) || (right && right->value->kind == VALUE_IRREGULAR))
        {
            ret->value->kind = VALUE_IRREGULAR;
        }
        else
        {
            ret->value->type = TypeDescToSemanticValueType(ret->type);
            if (ret->value->type == VALUE_POINTER)
            {
                fprintf(stderr, "[Error] Does not support pointer arithmetic for now in %s\n", __func__);
                ret->value->kind = VALUE_IRREGULAR;
            }
            else if (ret->value->type == VALUE_OTHER)
            {
                fprintf(stderr, "[Error] Invalid type in %s\n", __func__);
                ret->value->kind = VALUE_IRREGULAR;
            }
            else
            {
                switch (kind)
                {
                    case ADDITION_OP:
                    case ASSIGNMENT_ADD:
                        if(ret->value->type == VALUE_SIGNED_INTEGER)
                        {
                            long left_val, right_val;
                            GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                            GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                            ret->value->constVal.long_val = left_val + right_val;
                        }
                        else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                        {
                            unsigned long left_val, right_val;
                            GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                            GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                            ret->value->constVal.ulong_val = left_val + right_val;
                        }
                        else if(ret->value->type == VALUE_FLOAT)
                        {
                            double left_val, right_val;
                            GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                            GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                            ret->value->constVal.double_val = left_val + right_val;
                         }
                        break;
                    case SUBTRACTION_OP:
                    case ASSIGNMENT_SUB:
                        break;
                    case MULTIPLICATION_OP:
                    case ASSIGNMENT_MUL:
                        break;
                    case DIVISION_OP:
                    case ASSIGNMENT_DIV:
                        break;
                    case MODULAR_OP:
                    case ASSIGNMENT_MOD:
                        break;
                    case POST_INCREASE_OP:
                        break;
                    case POST_DECREASE_OP:
                        break;
                    case PRE_INCREASE_OP:
                        break;
                    case PRE_DECREASE_OP:
                        break;
                    case SHIFT_LEFT_OP:
                    case ASSIGNMENT_LEFT:
                        break;
                    case SHIFT_RIGHT_OP:
                    case ASSIGNMENT_RIGHT:
                        break;
                    case BITWISE_AND_OP:
                    case ASSIGNMENT_AND:
                        break;
                    case BITWISE_XOR_OP:
                    case ASSIGNMENT_XOR:
                        break;
                    case BITWISE_OR_OP:
                    case ASSIGNMENT_OR:
                        break;
                    case MEMORY_OP:
                        break;
                    case LESS_OP:
                        break;
                    case LESS_EQUAL_OP:
                        break;
                    case GREATER_OP:
                        break;
                    case GREATER_EQUAL_OP:
                        break;
                    case EQUAL_OP:
                        break;
                    case NOT_EQUAL_OP:
                        break;
                    case LOGICAL_AND_OP:
                        break;
                    case LOGICAL_OR_OP:
                        break;
                }
            }
        }
        return ret;
    }
}

SemanticRepresentation* TraceExprNode(Expression_node* node)
{
    if (!node)
        return NULL;
    else
    {
        SemanticRepresentation* left_value = TraceExprNode(node->left_operand);
        SemanticRepresentation* right_value = TraceExprNode(node->right_operand);

        if (node->expression_kind & EXPRESSION_MASK)
        {
            SemanticRepresentation* result;
            switch (node->expression_kind)
            {
                case EXPRESSION_IDENTIFIER:
                    {
                        if (left_value != NULL)
                            fprintf(stderr, "[Error] left operand should not appear in identifier node in %s\n", __func__);
                        if (right_value != NULL)
                            fprintf(stderr, "[Error] right operand should not appear in identifier node in %s\n", __func__);

                        SymbolTableEntry* entry = FindSymbolInSymTable(symTable, node->direct_expr.identifier);
                        result = (SemanticRepresentation*) malloc(sizeof(SemanticRepresentation));
                        result->type = DuplicateTypeDesc(entry->type);
                        result->lvalue = entry;
                        result->next = NULL;

                        if (entry->value)
                            result->value = DuplicateSemanticValue(entry->value);
                        else
                            result->value = CreateEmptySemanticValue();
                    }
                    break;
                case EXPRESSION_CONSTANT:
                    {
                        if (left_value != NULL)
                            fprintf(stderr, "[Error] left operand should not appear in constant node in %s\n", __func__);
                        if (right_value != NULL)
                            fprintf(stderr, "[Error] right operand should not appear in constant node in %s\n", __func__);

                        result = (SemanticRepresentation*) malloc(sizeof(SemanticRepresentation));
                        result->type = DuplicateTypeDesc(node->direct_expr.constant->constant_type);
                        result->value = CreateEmptySemanticValue();
                        result->value->type = TypeDescToSemanticValueType(result->type);
                        memcpy(&(result->value->constVal), &(node->direct_expr.constant->value), sizeof(result->value->constVal));
                        result->lvalue = NULL;
                        result->next = NULL;
                    }
                    break;
                case EXPRESSION_SUBSCRIPT:
                    {
                        /* Memory access */
                    }
                    break;
                case EXPRESSION_FUNCTION:
                    break;
                case EXPRESSION_MEMBER:
                    {
                        if (right_value != NULL)
                            fprintf(stderr, "[Error] right operand should not appear in member node in %s\n", __func__);

                        SymbolTableEntry* entry = left_value->lvalue;
                        SymbolTableEntry* new_entry = FindMemberInSymTable(entry, node->direct_expr.member);
                        result = (SemanticRepresentation*) malloc(sizeof(SemanticRepresentation));
                        result->type = DuplicateTypeDesc(new_entry->type);
                        result->value = DuplicateSemanticValue(new_entry->value);
                        result->lvalue = new_entry;
                        result->next = NULL;
                        DeleteSemanticRepresentation(left_value);
                    }
                    break;
                case EXPRESSION_TYPECAST:
                    break;
                case EXPRESSION_EXPRSTMT:
                    break;
            }
            return result;
        }
        else if (node->expression_kind & ASSIGNMENT_MASK)
        {
            SemanticRepresentation* result = CalculateSemanticRepresentation(node->expression_kind, left_value, right_value);

            if (left_value->lvalue == NULL)
                fprintf(stderr, "[Error] Assignment to non lvalue in %s\n", __func__);
            else
            {
                AssignToSymTableEntry(left_value->lvalue, result);
            }

            DeleteSemanticRepresentation(left_value);
            DeleteSemanticRepresentation(right_value);

            return result;
        }
        else // OP_MASK
        {
            SemanticRepresentation* result = CalculateSemanticRepresentation(node->expression_kind, left_value, right_value);
            printf(".... Compute arithmetic operation ....\n");

            DeleteSemanticRepresentation(left_value);
            DeleteSemanticRepresentation(right_value);
            return result;
        }
    }
}

SymbolTableEntry_list* TraceDeclNode(Declaration_node* node)
{
    if (!node)
        return NULL;
    else
    {
        printf("Tracing declaration node\n");
        SymbolTableEntry_list* ret = NULL;
        Declaration_desc_node* iterDesc = node->declaration_desc_head;
        while (iterDesc != NULL)
        {
            SymbolTableEntry* entry;
            TypeDescriptor* mix_type = MixAndCreateTypeDesc(node->declaration_type, iterDesc->identifier_type);

            entry = CreateSymTableEntry(program, iterDesc->identifier_name, mix_type);
            ret = AppendSymTableEntryToList(ret, entry);

            if (iterDesc->init_expression)
            {
                SemanticRepresentation* init_value = TraceExprNode(iterDesc->init_expression);
                AssignToSymTableEntry(entry, init_value);
                DeleteSemanticRepresentation(init_value);
            }

            iterDesc = iterDesc->next;
        }
        printf("Tracing declaration node end\n");
        return ret;
    }
}

// Symbol table level should be created by caller
void TraceCompoundStmt(CompoundStatement* stmt)
{
    if (!stmt)
        return;
    else
    {
        Declaration_node* iterDecl = stmt->declaration_head;
        Statement_node* iterStmt = stmt->statement_head;
        while (iterDecl != NULL)
        {
            SymbolTableEntry_list* entry_list = TraceDeclNode(iterDecl);
            AddEntryListToSymTable(symTable, entry_list);
            iterDecl = iterDecl->next;
        }
        while (iterStmt != NULL)
        {
            //TraceStmtNode(iterDecl);
            iterStmt = iterStmt->next;
        }
    }
}

void TraceFuncNode(Program_node* prog, char* func_name, SemanticRepresentation_list* arguments)
{
    Function_node* func = prog->function_head;
    while (func != NULL)
    {
        if (strcmp(func_name, func->function_name) == 0)
            break;

        func = func->next;
    }

    if (!func)
        return;
    else
    {
        printf("Start tracing function %s\n", func_name);
        SemanticRepresentation* iterArg;
        SemanticRepresentation* nextArg;
        Parameter_node* iterParam = func->parameter_head;

        if (arguments == NULL)
            iterArg = NULL;
        else
            iterArg = arguments->value_head;

        CreateSymTableLevel(symTable);
        while (iterParam != NULL && iterArg != NULL)
        {
            TypeDescriptor* mix_type = MixAndCreateTypeDesc(iterParam->parameter_type, iterParam->parameter_desc->identifier_type);
            char* name = strdup(iterParam->parameter_desc->identifier_name);

            SymbolTableEntry* entry = CreateSymTableEntry(program, name, mix_type);
            AssignToSymTableEntry(entry, iterArg);
            AddEntryToSymTable(symTable, entry);

            nextArg = iterArg->next;
            DeleteSemanticRepresentation(iterArg);
            iterArg = nextArg;
            iterParam = iterParam->next;
        }

        TraceCompoundStmt(func->content_statement);
        DeleteSymTableLevel(symTable);
    }
}
