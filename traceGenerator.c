#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TraceGenerator.h"

extern SymbolTable* symTable;
extern Operation* lastIssueOP;
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

        for (array_idx = 0 ; array_idx < entry->array_dim ; array_idx ++)
        {
            DeleteSymTableEntry(entry->array_entry[array_idx]);
        }
        free (entry->array_entry);

        DeleteSemanticValue(entry->value);
        entry->next = NULL;
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
    if (!table->level_head)
    {
        fprintf(stderr, "[Error] No symbol table level has been created yet in %s\n", __func__);
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

        if (curr_level->prev != NULL)
            curr_level->prev->next = NULL;
        else
            table->level_head = NULL;

        table->level_tail = curr_level->prev;
        curr_level->next = NULL;
        curr_level->prev = NULL;
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
        if (new_entry == NULL)
        {
            fprintf(stderr, "[Error] Given symbol table entry is NULL in %s\n", __func__);
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
        if (entry_list == NULL)
        {
            fprintf(stderr, "[Error] Given symbol table entry list is NULL in %s\n", __func__);
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

Operation_list* AppendOperationToList(Operation_list* origin_list, Operation* new_op)
{
    if (new_op == NULL)
        return origin_list;
    else
    {
        if (origin_list == NULL)
        {
            Operation_list* ret = (Operation_list*) malloc(sizeof(Operation_list));
            ret->operation_head = new_op;
            ret->operation_tail = new_op;
            return ret;
        }
        else
        {
            origin_list->operation_tail->next = new_op;
            origin_list->operation_tail = new_op;
            return origin_list;
        }
    }
}

Operation* CreateOperation(TypeDescriptor* type, EXPRESSION_KIND kind)
{
    if (!type)
    {
        fprintf(stderr, "[Error] Given type descriptor is NULL in %s\n", __func__);
        return NULL;
    }
    else
    {
        Operation* ret = (Operation*) malloc(sizeof(Operation));

        ret->id = g_operation_id ++;
        ret->kind = kind;

        if (type->array_desc_head != NULL)
            ret->type = NONE_TYPE;
        else if (type->kind == TYPE_WITH_PARAM)
            ret->type = NONE_TYPE;
        else
            ret->type = type->type;

        ret->issue_dep = NULL;
        ret->structural_dep = NULL;
        ret->data_dep_head = NULL;
        ret->data_dep_tail = NULL;
        return ret;
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
        // Depends on variable type (stored in symbol table entry)
        SEMANTIC_VALUE_TYPE type = TypeDescToSemanticValueType(entry->type);
        if (entry->value)
            DeleteSemanticValue(entry->value);

        entry->value = (SemanticValue*) malloc(sizeof(SemanticValue));
        entry->value->kind = value->value->kind;
        entry->value->vector = value->value->vector;
        entry->value->lastOP = value->value->lastOP;

        /* Type casting */
        entry->value->type = type;
        if (type == VALUE_SIGNED_INTEGER)
        {
            long val;
            GetValueInSemanticValue(type, value->value, &val);
            entry->value->constVal.long_val = val;
        }
        else if (type == VALUE_UNSIGNED_INTEGER)
        {
            unsigned long val;
            GetValueInSemanticValue(type, value->value, &val);
            entry->value->constVal.ulong_val = val;
        }
        else if (type == VALUE_FLOAT)
        {
            double val;
            GetValueInSemanticValue(type, value->value, &val);
            entry->value->constVal.double_val = val;
        }
        else
        {
            memset(&(entry->value->constVal), 0, sizeof(entry->value->constVal));
        }
    }
}

SymbolTableEntry_list* AppendSymTableEntryToList(SymbolTableEntry_list* origin_list, SymbolTableEntry* entry)
{
    if (!entry)
        return origin_list;

    if (origin_list == NULL)
    {
        SymbolTableEntry_list* ret = (SymbolTableEntry_list*) malloc(sizeof(SymbolTableEntry_list));
        ret->entry_head = entry;
        ret->entry_tail = entry;
        return ret;
    }
    else
    {
        origin_list->entry_tail->next = entry;
        origin_list->entry_tail = entry;
        return origin_list;
    }
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

                if (currArray->size > 0)
                {
                    ret->array_dim = currArray->size;
                    ret->array_entry = (SymbolTableEntry**) malloc(sizeof(SymbolTableEntry*) * ret->array_dim);
                    for (arrayIdx = 0 ; arrayIdx < ret->array_dim ; arrayIdx ++)
                    {
                        (ret->array_entry)[arrayIdx] = CreateSymTableEntry(prog, NULL, DuplicateTypeDesc(elem_type));
                    }
                }
                DeleteTypeDesc(elem_type);
                DeleteArrayDescNode(currArray);
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

Dependency* CreateDependency(Operation* target, unsigned long latency)
{
    Dependency* ret = (Dependency*) malloc(sizeof(Dependency));
    ret->targetOP = target;
    ret->latency = latency;
    ret->next = NULL;
    return ret;
}

// TODO dependency latency
void AddDependency(Operation* source, Operation* destination, DEPENDENCY_KIND kind)
{
    if (!source)
        return;
    else
    {
        if (kind == ISSUE_DEPENDENCY)
        {
            Dependency* new_dep = CreateDependency(destination, 1);
            source->issue_dep = new_dep;
        }
        else if (kind == STRUCTURAL_DEPENDENCY)
        {
        }
        else if (kind == DATA_DEPENDENCY)
        {
            Dependency* new_dep = CreateDependency(destination, 1);
            if (source->data_dep_head == NULL)
            {
                source->data_dep_head = new_dep;
                source->data_dep_tail = new_dep;
            }
            else
            {
                source->data_dep_tail->next = new_dep;
                source->data_dep_tail = new_dep;
            }
        }
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

SemanticValue* CreateZeroSemanticValue(SEMANTIC_VALUE_TYPE type)
{
    SemanticValue* ret = (SemanticValue*) malloc(sizeof(SemanticValue));
    ret->kind = VALUE_REGULAR;
    ret->type = type;
    ret->lastOP = NULL;
    ret->vector = CreateEmptyNDRangeVector();
    if (type == VALUE_SIGNED_INTEGER)
        ret->constVal.long_val = 0;
    else if (type == VALUE_UNSIGNED_INTEGER)
        ret->constVal.ulong_val = 0;
    else if (type == VALUE_FLOAT)
        ret->constVal.double_val = 0;
    else
        memset(&(ret->constVal), 0, sizeof(ret->constVal));
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
        else if (type->kind == TYPE_WITH_PARAM)
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
        Operation* currOP = NULL;
        ret->lvalue = NULL;
        ret->next = NULL;
        ret->value = CreateEmptySemanticValue();

        if (kind & LOGICAL_OP_MASK)
        {
            /* Logical operations would always return int */
            ret->type = CreateScalarTypeDesc(INT_TYPE, NULL);
        }
        else
        {
            /* Arithmetic operations would always return bigger type */
            ret->type = MergeTypeDesc(left->type, right->type);
        }
        ret->value->type = TypeDescToSemanticValueType(ret->type);

        if ((left && left->value->kind == VALUE_UNDEFINED) || (right && right->value->kind == VALUE_UNDEFINED))
            ret->value->kind = VALUE_UNDEFINED;
        else if ((left && left->value->kind == VALUE_IRREGULAR) || (right && right->value->kind == VALUE_IRREGULAR))
            ret->value->kind = VALUE_IRREGULAR;
        else if (ret->value->type == VALUE_POINTER)
        {
            fprintf(stderr, "[Error] Does not support pointer arithmetic for now in %s\n", __func__);
            ret->value->kind = VALUE_UNDEFINED;
        }
        else if (ret->value->type == VALUE_OTHER)
        {
            fprintf(stderr, "[Error] Invalid type in %s\n", __func__);
            ret->value->kind = VALUE_UNDEFINED;
        }

        switch (kind)
        {
            case ADDITION_OP:
            case ASSIGNMENT_ADD:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val + right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val + right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.double_val = (left_val + right_val);
                }
                currOP = CreateOperation(ret->type, ADDITION_OP);
                break;
            case SUBTRACTION_OP:
            case ASSIGNMENT_SUB:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val - right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val - right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.double_val = (left_val - right_val);
                }
                currOP = CreateOperation(ret->type, SUBTRACTION_OP);
                break;
            case MULTIPLICATION_OP:
            case ASSIGNMENT_MUL:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val * right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val * right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.double_val = (left_val * right_val);
                }
                currOP = CreateOperation(ret->type, MULTIPLICATION_OP);
                break;
            case DIVISION_OP:
            case ASSIGNMENT_DIV:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val / right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val / right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.double_val = (left_val / right_val);
                }
                currOP = CreateOperation(ret->type, DIVISION_OP);
                break;
            case MODULAR_OP:
            case ASSIGNMENT_MOD:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val % right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val % right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"%%\" in %s\n", __func__);
                }
                currOP = CreateOperation(ret->type, MODULAR_OP);
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
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val << right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val << right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"<<\" in %s\n", __func__);
                }
                currOP = CreateOperation(ret->type, SHIFT_LEFT_OP);
                break;
            case SHIFT_RIGHT_OP:
            case ASSIGNMENT_RIGHT:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val >> right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val >> right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \">>\" in %s\n", __func__);
                }
                currOP = CreateOperation(ret->type, SHIFT_RIGHT_OP);
                break;
            case BITWISE_AND_OP:
            case ASSIGNMENT_AND:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val & right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val & right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"&\" in %s\n", __func__);
                }
                currOP = CreateOperation(ret->type, BITWISE_AND_OP);
                break;
            case BITWISE_XOR_OP:
            case ASSIGNMENT_XOR:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val ^ right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val ^ right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"^\" in %s\n", __func__);
                }
                currOP = CreateOperation(ret->type, BITWISE_XOR_OP);
                break;
            case BITWISE_OR_OP:
            case ASSIGNMENT_OR:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val | right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val | right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"|\" in %s\n", __func__);
                }
                currOP = CreateOperation(ret->type, BITWISE_OR_OP);
                break;
            case LESS_OP:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val < right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val < right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val < right_val);
                }
                currOP = CreateOperation(ret->type, LESS_OP);
                break;
            case LESS_EQUAL_OP:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val <= right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val <= right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val <= right_val);
                }
                currOP = CreateOperation(ret->type, LESS_EQUAL_OP);
                break;
            case GREATER_OP:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val > right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val > right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val > right_val);
                }
                currOP = CreateOperation(ret->type, GREATER_OP);
                break;
            case GREATER_EQUAL_OP:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val >= right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val >= right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val >= right_val);
                }
                currOP = CreateOperation(ret->type, GREATER_EQUAL_OP);
                break;
            case EQUAL_OP:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val == right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val == right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val == right_val);
                }
                currOP = CreateOperation(ret->type, EQUAL_OP);
                break;
            case NOT_EQUAL_OP:
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val != right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val != right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val != right_val);
                }
                currOP = CreateOperation(ret->type, NOT_EQUAL_OP);
                break;
            case LOGICAL_AND_OP: // TODO lazy evaluation
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val && right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val && right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val && right_val);
                }
                currOP = CreateOperation(ret->type, LOGICAL_AND_OP);
                break;
            case LOGICAL_OR_OP: // TODO lazy evaluation
                if(ret->value->type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val || right_val);
                }
                else if(ret->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val || right_val);
                }
                else if(ret->value->type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val || right_val);
                }
                currOP = CreateOperation(ret->type, LOGICAL_OR_OP);
                break;
            default:
                break;
        }

        // TODO dependency

        if (currOP != NULL)
        {
            if (left && left->value)
                AddDependency(left->value->lastOP, currOP, DATA_DEPENDENCY);

            if (right && right->value)
                AddDependency(right->value->lastOP, currOP, DATA_DEPENDENCY);

            AddDependency(lastIssueOP, currOP, ISSUE_DEPENDENCY);
            lastIssueOP = currOP;

            opTrace = AppendOperationToList(opTrace, currOP);

            ret->value->lastOP = currOP;
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
            SemanticRepresentation* result = NULL;
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
                        {
                            fprintf(stderr, "[Error] Identifier \'%s\' in symbol table has a NULL semantic value in %s\n", node->direct_expr.identifier, __func__);
                            result->value = NULL;
                        }
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
                    {
                        StmtRepresentation* stmtVal = TraceExpressionStmt(node->direct_expr.expr_stmt);
                        if (stmtVal != NULL)
                        {
                            result = stmtVal->expression;
                            stmtVal->expression = NULL;
                            DeleteStmtRepresentation(stmtVal);
                        }
                    }
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
                if (result->value->type == VALUE_SIGNED_INTEGER)
                {
                    long val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, result->value, &val);
                    printf("Assign value : %ld\n", val);
                }
                else if (result->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, result->value, &val);
                    printf("Assign value : %lu\n", val);
                }
                else if (result->value->type == VALUE_FLOAT)
                {
                    double val;
                    GetValueInSemanticValue(VALUE_FLOAT, result->value, &val);
                    printf("Assign value : %lf\n", val);
                }
                AssignToSymTableEntry(left_value->lvalue, result);
            }

            DeleteSemanticRepresentation(left_value);
            DeleteSemanticRepresentation(right_value);

            return result;
        }
        else // OP_MASK
        {
            SemanticRepresentation* result = CalculateSemanticRepresentation(node->expression_kind, left_value, right_value);

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
        SymbolTableEntry_list* ret = NULL;
        Declaration_desc_node* iterDesc = node->declaration_desc_head;
        while (iterDesc != NULL)
        {
            SymbolTableEntry* entry;
            TypeDescriptor* mix_type = MixAndCreateTypeDesc(node->declaration_type, iterDesc->identifier_type);

            entry = CreateSymTableEntry(program, strdup(iterDesc->identifier_name), mix_type);
            ret = AppendSymTableEntryToList(ret, entry);

            if (iterDesc->init_expression)
            {
                SemanticRepresentation* init_value = TraceExprNode(iterDesc->init_expression);

                if (init_value->value->type == VALUE_SIGNED_INTEGER)
                {
                    long val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, init_value->value, &val);
                    printf("Initial value : %ld\n", val);
                }
                else if (init_value->value->type == VALUE_UNSIGNED_INTEGER)
                {
                    unsigned long val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, init_value->value, &val);
                    printf("Initial value : %lu\n", val);
                }
                else if (init_value->value->type == VALUE_FLOAT)
                {
                    double val;
                    GetValueInSemanticValue(VALUE_FLOAT, init_value->value, &val);
                    printf("Initial value : %lf\n", val);
                }

                AssignToSymTableEntry(entry, init_value);
                DeleteSemanticRepresentation(init_value);
            }
            else
            {
                SEMANTIC_VALUE_TYPE type = TypeDescToSemanticValueType(entry->type);
                entry->value = CreateZeroSemanticValue(type);
            }

            iterDesc = iterDesc->next;
        }
        return ret;
    }
}

StmtRepresentation* CreateStmtRepresentation(STATEMENT_KIND kind, SemanticRepresentation* value)
{
    StmtRepresentation* ret = (StmtRepresentation*) malloc(sizeof(StmtRepresentation));
    ret->kind = kind;
    ret->expression = value;
    return ret;
}

void DeleteStmtRepresentation(StmtRepresentation* rep)
{
    if (rep == NULL)
        return;
    else
    {
        DeleteSemanticRepresentation(rep->expression);
        free (rep);
    }
}

StmtRepresentation* TraceStmtNode(Statement_node* node)
{
    if (!node)
        return NULL;
    else
    {
        StmtRepresentation* result = NULL;
        switch (node->statement_kind)
        {
            case ITERATION_STMT:
                result = TraceIterationStmt(node->stmt.iteration_stmt);
                break;
            case SELECTION_STMT:
                break;
            case EXPRESSION_STMT:
                result = TraceExpressionStmt(node->stmt.expression_stmt);
                break;
            case RETURN_STMT:
                break;
            case COMPOUND_STMT:
                CreateSymTableLevel(symTable);
                result = TraceCompoundStmt(node->stmt.compound_stmt);
                DeleteSymTableLevel(symTable);
                break;
            case EMPTY_GOTO_STMT:
                fprintf(stderr, "[Error] Does not support GOTO statement for now\n");
                break;
            case EMPTY_CONTINUE_STMT:
            case EMPTY_BREAK_STMT:
            case EMPTY_RETURN_STMT:
                result = CreateStmtRepresentation(node->statement_kind, NULL);
                break;
        }
        return result;
    }
}

StmtRepresentation* TraceIterationStmt(IterationStatement* stmt)
{
    if (!stmt)
        return NULL;
    else
    {
        int loop_terminated = 0;
        StmtRepresentation* returnVal = NULL;

        /* INIT */

        if (stmt->kind == FOR_LOOP_WITH_DECL)
        {
            SymbolTableEntry_list* entry_list = NULL;
            CreateSymTableLevel(symTable);
            entry_list = TraceDeclNode(stmt->init.declaration);
            AddEntryListToSymTable(symTable, entry_list);
        }
        else
        {
            StmtRepresentation* result = TraceExpressionStmt(stmt->init.expression);
            DeleteStmtRepresentation(result);
        }

        /* CONTENT */

        if (stmt->kind != DO_WHILE_LOOP) // check the condition first
        {
            StmtRepresentation* result = TraceExpressionStmt(stmt->terminated_expression);
            if (result && result->expression)
            {
                long terminated_value;
                GetValueInSemanticValue(VALUE_SIGNED_INTEGER, result->expression->value, &terminated_value);
                if (!terminated_value)
                    loop_terminated = 1;
            }
            DeleteStmtRepresentation(result);
        }

        while (!loop_terminated)
        {
            StmtRepresentation* result;

            result = TraceStmtNode(stmt->content_statement);
            if (result && (result->kind = CONTROL_STMT_MASK))
            {
                switch (result->kind)
                {
                    case EMPTY_CONTINUE_STMT:
                        break;
                    case EMPTY_BREAK_STMT:
                        loop_terminated = 1;
                        break;
                    case EMPTY_RETURN_STMT:
                    case RETURN_STMT:
                        loop_terminated = 1;
                        returnVal = result;
                        result = NULL;
                        break;
                }
            }
            DeleteStmtRepresentation(result);

            result = TraceExpressionStmt(stmt->step_expression);
            DeleteStmtRepresentation(result);

            result = TraceExpressionStmt(stmt->terminated_expression);
            if (result && result->expression)
            {
                long terminated_value;
                GetValueInSemanticValue(VALUE_SIGNED_INTEGER, result->expression->value, &terminated_value);
                if (!terminated_value)
                    loop_terminated = 1;
            }
            DeleteStmtRepresentation(result);
        }

        /* CLEANUP */

        if (stmt->kind == FOR_LOOP_WITH_DECL)
        {
            DeleteSymTableLevel(symTable);
        }

        if (returnVal == NULL)
            returnVal = CreateStmtRepresentation(ITERATION_STMT, NULL);

        return returnVal;
    }
}

StmtRepresentation* TraceExpressionStmt(ExpressionStatement* stmt)
{
    if (!stmt)
        return NULL;
    else
    {
        Expression_node* iterNode = stmt->expression_head;
        SemanticRepresentation* resultVal = NULL;
        while (iterNode != NULL)
        {
            DeleteSemanticRepresentation(resultVal);
            resultVal = TraceExprNode(iterNode);
            iterNode = iterNode->next;
        }
        return CreateStmtRepresentation(EXPRESSION_STMT, resultVal);
    }
}

// Symbol table level should be created by caller
StmtRepresentation* TraceCompoundStmt(CompoundStatement* stmt)
{
    if (!stmt)
        return NULL;
    else
    {
        Declaration_node* iterDecl = stmt->declaration_head;
        Statement_node* iterStmt = stmt->statement_head;
        StmtRepresentation* result = NULL;
        while (iterDecl != NULL)
        {
            SymbolTableEntry_list* entry_list = TraceDeclNode(iterDecl);
            AddEntryListToSymTable(symTable, entry_list);
            iterDecl = iterDecl->next;
        }
        while (iterStmt != NULL)
        {
            result = TraceStmtNode(iterStmt);

            if (result && (result->kind & CONTROL_STMT_MASK))
                return result;
            else
                DeleteStmtRepresentation(result);

            iterStmt = iterStmt->next;
        }
        return CreateStmtRepresentation(COMPOUND_STMT, NULL);
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

void ShowOPTrace(Operation_list* list)
{
    Operation* iterOP;

    printf("\n\n========== Operation trace ==========\n\n");
    if (list != NULL)
    {
        Operation* iterOP = list->operation_head;
        while (iterOP != NULL)
        {
            printf("[#%lu] %d ", iterOP->id, iterOP->kind);
            if (iterOP->issue_dep)
                printf("issue: #%lu, ", iterOP->issue_dep->targetOP->id);
            if (iterOP->structural_dep)
                printf("struct #%lu, ", iterOP->structural_dep->targetOP->id);
            if (iterOP->data_dep_head)
            {
                Dependency* iterDep = iterOP->data_dep_head;
                printf("data ");
                while (iterDep != NULL)
                {
                    printf("#%lu ", iterDep->targetOP->id);
                    iterDep = iterDep->next;
                }
            }
            printf("\n");
            iterOP = iterOP->next;
        }
    }
}
