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

Operation* CreateOperation(TypeDescriptor* type, EXPRESSION_KIND kind, SemanticValue* value)
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
        ret->value = value;

        if (type->array_desc_head != NULL)
            ret->type = POINTER_TYPE;
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
        else if ((type == VALUE_UNSIGNED_INTEGER) || (type == VALUE_POINTER))
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

void DeleteSemanticRepresentationList(SemanticRepresentation_list* list)
{
    if (!list)
        return;
    else
    {
        SemanticRepresentation* iterValue = list->value_head;
        SemanticRepresentation* nextValue;

        while (iterValue != NULL)
        {
            nextValue = iterValue->next;
            DeleteSemanticRepresentation(iterValue);
            iterValue = nextValue;
        }

        free (list);
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

/* return 0 if zero, 1 otherwise */
int CheckNDRangeVector(NDRangeVector vector)
{
    unsigned long result;
    result = vector.globalIdx[0] | vector.globalIdx[1] | vector.globalIdx[2]
        | vector.groupIdx[0] | vector.groupIdx[1] | vector.groupIdx[2]
        | vector.localIdx[0] | vector.localIdx[1] | vector.localIdx[2];

    return (result == 0) ? 0 : 1;
}

NDRangeVector CreateZeroNDRangeVector()
{
    NDRangeVector ret;
    ret.globalIdx[0] = 0;
    ret.globalIdx[1] = 0;
    ret.globalIdx[2] = 0;
    ret.groupIdx[0] = 0;
    ret.groupIdx[1] = 0;
    ret.groupIdx[2] = 0;
    ret.localIdx[0] = 0;
    ret.localIdx[1] = 0;
    ret.localIdx[2] = 0;
    return ret;
}

SemanticValue* CreateZeroSemanticValue(SEMANTIC_VALUE_TYPE type)
{
    SemanticValue* ret = (SemanticValue*) malloc(sizeof(SemanticValue));
    ret->kind = VALUE_REGULAR;
    ret->type = type;
    ret->lastOP = NULL;
    ret->vector = CreateZeroNDRangeVector();
    if (type == VALUE_SIGNED_INTEGER)
        ret->constVal.long_val = 0;
    else if ((type == VALUE_UNSIGNED_INTEGER) || (type == VALUE_POINTER))
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
    ret->vector = CreateZeroNDRangeVector();
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
            if (type->array_desc_head->desc_kind == ARRAY_DESC_ARRAY) // name of array, regard as pointer
                return VALUE_POINTER;
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

TypeDescriptor* DereferenceAndCreateTypeDesc(TypeDescriptor* type)
{
    if (!type)
        return NULL;
    else
    {
        TypeDescriptor* ret = DuplicateTypeDesc(type);
        if (ret->array_desc_head == NULL)
        {
            fprintf(stderr, "[Error] given type cannot be dereferenced in %s\n", __func__);
        }
        else
        {
            if (ret->array_desc_head == ret->array_desc_tail)
            {
                DeleteArrayDescNode(ret->array_desc_head);
                ret->array_desc_head = NULL;
                ret->array_desc_tail = NULL;
            }
            else
            {
                ArrayDesc_node* tmp = ret->array_desc_head->next;
                DeleteArrayDescNode(ret->array_desc_head);
                ret->array_desc_head = tmp;
            }
        }
        return ret;
    }
}

TypeDescriptor* ComputeAndCreateTypeDesc(TypeDescriptor* left, TypeDescriptor* right)
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
        if ((left->array_desc_head != NULL) && (right->array_desc_head != NULL))
        {
            fprintf(stderr, "[Error] Both operand have Array/Pointer type in %s\n", __func__);
            return CreateScalarTypeDesc(NONE_TYPE, NULL);
        }
        else if (left->array_desc_head != NULL)
        {
            return DuplicateTypeDesc(left);
        }
        else if (right->array_desc_head != NULL)
        {
            return DuplicateTypeDesc(right);
        }
        else if ((left->kind == TYPE_WITH_PARAM) || (right->kind == TYPE_WITH_PARAM))
        {
            fprintf(stderr, "[Error] Function type in %s\n", __func__);
            return CreateScalarTypeDesc(NONE_TYPE, NULL);
        }
        else if ((left->type & CONST_OTHER_MASK) || (right->type & CONST_OTHER_MASK))
        {
            fprintf(stderr, "[Error] Unsupported type in %s\n", __func__);
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

SemanticRepresentation_list* AppendSemanticRepresentationToList(SemanticRepresentation_list* origin_list, SemanticRepresentation* new_value)
{
    if (!new_value)
        return origin_list;

    if (origin_list == NULL)
    {
        SemanticRepresentation_list* ret = (SemanticRepresentation_list*) malloc(sizeof(SemanticRepresentation_list));
        ret->value_head = new_value;
        ret->value_tail = new_value;
        return ret;
    }
    else
    {
        origin_list->value_tail->next = new_value;
        origin_list->value_tail = new_value;
        return origin_list;
    }
}

// Only return constVal part in SemanticValue
void GetValueInSemanticValue(SEMANTIC_VALUE_TYPE type, SemanticValue* value, void* ret)
{
    if (type == VALUE_SIGNED_INTEGER)
    {
        if (value->type == VALUE_SIGNED_INTEGER)
            *((long *)(ret)) = value->constVal.long_val;
        else if ((value->type == VALUE_UNSIGNED_INTEGER) || (value->type == VALUE_POINTER))
            *((long *)(ret)) = (long)(value->constVal.ulong_val);
        else if (value->type == VALUE_FLOAT)
            *((long *)(ret)) = (long)(value->constVal.double_val);
    }
    else if ((type == VALUE_UNSIGNED_INTEGER) || (type == VALUE_POINTER))
    {
        if (value->type == VALUE_SIGNED_INTEGER)
            *((unsigned long *)(ret)) = (unsigned long)(value->constVal.long_val);
        else if ((value->type == VALUE_UNSIGNED_INTEGER) || (value->type == VALUE_POINTER))
            *((unsigned long *)(ret)) = value->constVal.ulong_val;
        else if (value->type == VALUE_FLOAT)
            *((unsigned long *)(ret)) = (unsigned long)(value->constVal.double_val);
    }
    else if (type == VALUE_FLOAT)
    {
        if (value->type == VALUE_SIGNED_INTEGER)
            *((double *)(ret)) = (double)(value->constVal.long_val);
        else if ((value->type == VALUE_UNSIGNED_INTEGER) || (value->type == VALUE_POINTER))
            *((double *)(ret)) = (double)(value->constVal.ulong_val);
        else if (value->type == VALUE_FLOAT)
            *((double *)(ret)) = value->constVal.double_val;
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
                        if (right_value != NULL)
                            fprintf(stderr, "[Error] right operand should not appear in memory access node in %s\n", __func__);

                        Operation* madOP;
                        Operation* memOP;
                        StmtRepresentation* index = TraceExpressionStmt(node->direct_expr.subscript);
                        result = (SemanticRepresentation*) malloc(sizeof(SemanticRepresentation));
                        result->type = DereferenceAndCreateTypeDesc(left_value->type);
                        result->value = CreateEmptySemanticValue();
                        result->value->kind = VALUE_IRREGULAR;

                        madOP = CreateOperation(DuplicateTypeDesc(left_value->type), MAD_OP, NULL); // for index calculation
                        memOP = CreateOperation(result->type, MEMORY_OP, DuplicateSemanticValue(index->expression->value)); // TODO: calculate the actual address instead of index
                        if ((madOP != NULL) && (memOP != NULL))
                        {
                            if (left_value && left_value->value)
                                AddDependency(left_value->value->lastOP, madOP, DATA_DEPENDENCY);

                            if (index && index->expression && index->expression->value)
                                AddDependency(index->expression->value->lastOP, madOP, DATA_DEPENDENCY);

                            AddDependency(lastIssueOP, madOP, ISSUE_DEPENDENCY);
                            opTrace = AppendOperationToList(opTrace, madOP);

                            AddDependency(madOP, memOP, DATA_DEPENDENCY);
                            AddDependency(madOP, memOP, ISSUE_DEPENDENCY);
                            opTrace = AppendOperationToList(opTrace, memOP);
                            lastIssueOP = memOP;
                            result->value->lastOP = memOP;
                        }
                        DeleteSemanticRepresentation(left_value);
                        DeleteStmtRepresentation(index);
                    }
                    break;
                case EXPRESSION_FUNCTION:
                    {
                        if (left_value != NULL)
                            fprintf(stderr, "[Error] left operand should not appear in function invocation node in %s\n", __func__);
                        if (right_value != NULL)
                            fprintf(stderr, "[Error] right operand should not appear in function invocation node in %s\n", __func__);

                        FunctionInvocation_node* currFunc = node->direct_expr.function;
                        Expression_node* iterArg = currFunc->argument_head;
                        SemanticRepresentation_list* args = NULL;
                        SemanticRepresentation* tmp;
                        StmtRepresentation* returnVal = NULL;

                        while (iterArg != NULL)
                        {
                            tmp = TraceExprNode(iterArg);
                            args = AppendSemanticRepresentationToList(args, tmp);
                            iterArg = iterArg->next;
                        }
                        returnVal = TraceFuncNode(program, currFunc->name, args);

                        if (returnVal != NULL)
                            result = returnVal->expression;
                        else
                            result = NULL;

                        DeleteSemanticRepresentationList(args);
                        DeleteSemanticRepresentation(left_value);
                    }
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
                        result->next = left_value->next;
                        DeleteSemanticRepresentation(left_value);
                    }
                    break;
                case EXPRESSION_TYPECAST:
                    // TODO type casting expression
                    break;
                case EXPRESSION_EXPRSTMT:
                    {
                        if (left_value != NULL)
                            fprintf(stderr, "[Error] left operand should not appear in expression statement node in %s\n", __func__);
                        if (right_value != NULL)
                            fprintf(stderr, "[Error] right operand should not appear in expression statement node in %s\n", __func__);

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
                    printf("Assign value : %ld", val);
                    ShowNDRangeVector(result->value->vector);
                    printf("\n");
                }
                else if ((result->value->type == VALUE_UNSIGNED_INTEGER) || (result->value->type == VALUE_POINTER))
                {
                    unsigned long val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, result->value, &val);
                    printf("Assign value : %lu", val);
                    ShowNDRangeVector(result->value->vector);
                    printf("\n");
                }
                else if (result->value->type == VALUE_FLOAT)
                {
                    double val;
                    GetValueInSemanticValue(VALUE_FLOAT, result->value, &val);
                    printf("Assign value : %lf", val);
                    ShowNDRangeVector(result->value->vector);
                    printf("\n");
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
                    printf("Initial value : %ld", val);
                    ShowNDRangeVector(init_value->value->vector);
                    printf("\n");
                }
                else if ((init_value->value->type == VALUE_UNSIGNED_INTEGER) || (init_value->value->type == VALUE_POINTER))
                {
                    unsigned long val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, init_value->value, &val);
                    printf("Initial value : %lu", val);
                    ShowNDRangeVector(init_value->value->vector);
                    printf("\n");
                }
                else if (init_value->value->type == VALUE_FLOAT)
                {
                    double val;
                    GetValueInSemanticValue(VALUE_FLOAT, init_value->value, &val);
                    printf("Initial value : %lf", val);
                    ShowNDRangeVector(init_value->value->vector);
                    printf("\n");
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
                result = TraceSelectionStmt(node->stmt.selection_stmt);
                break;
            case EXPRESSION_STMT:
                result = TraceExpressionStmt(node->stmt.expression_stmt);
                break;
            case RETURN_STMT:
                result = TraceReturnStmt(node->stmt.return_stmt);
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

StmtRepresentation* TraceReturnStmt(ReturnStatement* stmt)
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
        return CreateStmtRepresentation(RETURN_STMT, resultVal);
    }
}

StmtRepresentation* TraceSelectionStmt(SelectionStatement* stmt)
{
    if (!stmt)
        return NULL;
    else
    {
        StmtRepresentation* result = NULL;
        Selection_node* iterSelect = stmt->selection_head;
        while (iterSelect != NULL)
        {
            if (iterSelect->condition_kind == SELECTION_WITH_COND)
            {
                result = TraceExpressionStmt(iterSelect->condition_expression);
                if (result && result->expression)
                {
                    long condition_value;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, result->expression->value, &condition_value);
                    DeleteStmtRepresentation(result);
                    if (condition_value)
                    {
                        result = TraceStmtNode(iterSelect->content_statement);
                        if (result && (result->kind & CONTROL_STMT_MASK))
                            return result;
                        else
                            DeleteStmtRepresentation(result);

                        break;
                    }
                }
                else
                    DeleteStmtRepresentation(result);

            }
            else if (iterSelect->condition_kind == SELECTION_WITHOUT_COND)
            {
                result = TraceStmtNode(iterSelect->content_statement);
                if (result && (result->kind & CONTROL_STMT_MASK))
                    return result;
                else
                    DeleteStmtRepresentation(result);

                break;
            }

            iterSelect = iterSelect->next;
        }
        return CreateStmtRepresentation(SELECTION_STMT, NULL);
    }
}

StmtRepresentation* TraceIterationStmt(IterationStatement* stmt)
{
    if (!stmt)
        return NULL;
    else
    {
        int loop_terminated = 0;

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
            StmtRepresentation* result = TraceStmtNode(stmt->content_statement);
            if (result && (result->kind & CONTROL_STMT_MASK))
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
                        return result;
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
                {
                    loop_terminated = 1;
                }
            }
            DeleteStmtRepresentation(result);
        }

        /* CLEANUP */

        if (stmt->kind == FOR_LOOP_WITH_DECL)
        {
            DeleteSymTableLevel(symTable);
        }

        return CreateStmtRepresentation(ITERATION_STMT, NULL);
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

StmtRepresentation* TraceFuncNode(Program_node* prog, char* func_name, SemanticRepresentation_list* arguments)
{
    if (CheckPrimitiveFunc(func_name))
    {
        return CreateStmtRepresentation(EXPRESSION_STMT, ProcessPrimitiveFunc(func_name, arguments));
    }
    else
    {
        Function_node* func = prog->function_head;
        while (func != NULL)
        {
            if (strcmp(func_name, func->function_name) == 0)
                break;

            func = func->next;
        }

        if (!func)
            return NULL;
        else
        {
            printf("Start tracing function %s\n", func_name);
            SemanticRepresentation* iterArg;
            StmtRepresentation* result = NULL;
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

                iterArg = iterArg->next;
                iterParam = iterParam->next;
            }

            result = TraceCompoundStmt(func->content_statement);
            DeleteSymTableLevel(symTable);

            if (result && ((result->kind == RETURN_STMT) || (result->kind == EMPTY_RETURN_STMT)))
                return result;
            else
            {
                DeleteStmtRepresentation(result);
                return NULL;
            }
        }
    }
}

int CheckPrimitiveFunc(char* name)
{
    if (strcmp(name, "get_global_id") == 0) return 1;
    else if (strcmp(name, "get_local_id") == 0) return 1;
    else if (strcmp(name, "get_group_id") == 0) return 1;
    else if (strcmp(name, "get_num_groups") == 0) return 1;
    else if (strcmp(name, "get_work_dim") == 0) return 1;
    else if (strcmp(name, "get_global_size") == 0) return 1;
    else if (strcmp(name, "get_local_size") == 0) return 1;
    return 0;
}

// should only have one args
SemanticRepresentation* ProcessPrimitiveFunc(char* name, SemanticRepresentation_list* arg)
{
    // TODO: Different kind of primitive function
    unsigned int idx;
    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, arg->value_head->value, &idx);
    SemanticRepresentation* ret = (SemanticRepresentation*) malloc(sizeof(SemanticRepresentation));
    ret->lvalue = NULL;
    ret->next = NULL;
    ret->type = CreateScalarTypeDesc(ULONG_TYPE, NULL);
    ret->value = CreateZeroSemanticValue(VALUE_UNSIGNED_INTEGER);
    if (strcmp(name, "get_global_id") == 0) ret->value->vector.globalIdx[idx] = 1;
    else if (strcmp(name, "get_local_id") == 0) ret->value->vector.localIdx[idx] = 1;
    else if (strcmp(name, "get_group_id") == 0) ret->value->vector.groupIdx[idx] = 1;
    else if (strcmp(name, "get_num_groups") == 0) ;
    else if (strcmp(name, "get_work_dim") == 0) ;
    else if (strcmp(name, "get_global_size") == 0) ;
    else if (strcmp(name, "get_local_size") == 0) ;

    return ret;
}

void GetOperationName(Operation* op, char* name)
{
    strcpy(name, "");
    if (!op)
        return;
    else
    {
        switch (op->type)
        {
            case NONE_TYPE:
                strcat(name, "NONE_");
                break;
            case POINTER_TYPE:
                strcat(name, "POINTER_");
                break;
            case STRUCT_TYPE:
                strcat(name, "STRUCT_");
                break;
            case UNION_TYPE:
                strcat(name, "UNION_");
                break;
            case BOOL_TYPE:
                strcat(name, "BOOL_");
                break;
            case HALF_TYPE:
                strcat(name, "HALF_");
                break;
            case VOID_TYPE:
                strcat(name, "VOID_");
                break;
            case CHAR_TYPE:
                strcat(name, "CHAR_");
                break;
            case CHAR2_TYPE:
                strcat(name, "CHAR2_");
                break;
            case CHAR4_TYPE:
                strcat(name, "CHAR4_");
                break;
            case CHAR8_TYPE:
                strcat(name, "CHAR8_");
                break;
            case CHAR16_TYPE:
                strcat(name, "CHAR16_");
                break;
            case UCHAR_TYPE:
                strcat(name, "UCHAR_");
                break;
            case UCHAR2_TYPE:
                strcat(name, "UCHAR2_");
                break;
            case UCHAR4_TYPE:
                strcat(name, "UCHAR4_");
                break;
            case UCHAR8_TYPE:
                strcat(name, "UCHAR8_");
                break;
            case UCHAR16_TYPE:
                strcat(name, "UCHAR16_");
                break;
            case SHORT_TYPE:
                strcat(name, "SHORT_");
                break;
            case SHORT2_TYPE:
                strcat(name, "SHORT2_");
                break;
            case SHORT4_TYPE:
                strcat(name, "SHORT4_");
                break;
            case SHORT8_TYPE:
                strcat(name, "SHORT8_");
                break;
            case SHORT16_TYPE:
                strcat(name, "SHORT16_");
                break;
            case USHORT_TYPE:
                strcat(name, "USHORT_");
                break;
            case USHORT2_TYPE:
                strcat(name, "USHORT2_");
                break;
            case USHORT4_TYPE:
                strcat(name, "USHORT4_");
                break;
            case USHORT8_TYPE:
                strcat(name, "USHORT8_");
                break;
            case USHORT16_TYPE:
                strcat(name, "USHORT16_");
                break;
            case INT_TYPE:
                strcat(name, "INT_");
                break;
            case INT2_TYPE:
                strcat(name, "INT2_");
                break;
            case INT4_TYPE:
                strcat(name, "INT4_");
                break;
            case INT8_TYPE:
                strcat(name, "INT8_");
                break;
            case INT16_TYPE:
                strcat(name, "INT16_");
                break;
            case UINT_TYPE:
                strcat(name, "UINT_");
                break;
            case UINT2_TYPE:
                strcat(name, "UINT2_");
                break;
            case UINT4_TYPE:
                strcat(name, "UINT4_");
                break;
            case UINT8_TYPE:
                strcat(name, "UINT8_");
                break;
            case UINT16_TYPE:
                strcat(name, "UINT16_");
                break;
            case LONG_TYPE:
                strcat(name, "LONG_");
                break;
            case LONG2_TYPE:
                strcat(name, "LONG2_");
                break;
            case LONG4_TYPE:
                strcat(name, "LONG4_");
                break;
            case LONG8_TYPE:
                strcat(name, "LONG8_");
                break;
            case LONG16_TYPE:
                strcat(name, "LONG16_");
                break;
            case ULONG_TYPE:
                strcat(name, "ULONG_");
                break;
            case ULONG2_TYPE:
                strcat(name, "ULONG2_");
                break;
            case ULONG4_TYPE:
                strcat(name, "ULONG4_");
                break;
            case ULONG8_TYPE:
                strcat(name, "ULONG8_");
                break;
            case ULONG16_TYPE:
                strcat(name, "ULONG16_");
                break;
            case FLOAT_TYPE:
                strcat(name, "FLOAT_");
                break;
            case FLOAT2_TYPE:
                strcat(name, "FLOAT2_");
                break;
            case FLOAT4_TYPE:
                strcat(name, "FLOAT4_");
                break;
            case FLOAT8_TYPE:
                strcat(name, "FLOAT8_");
                break;
            case FLOAT16_TYPE:
                strcat(name, "FLOAT16_");
                break;
            case DOUBLE_TYPE:
                strcat(name, "DOUBLE_");
                break;
            case DOUBLE2_TYPE:
                strcat(name, "DOUBLE2_");
                break;
            case DOUBLE4_TYPE:
                strcat(name, "DOUBLE4_");
                break;
            case DOUBLE8_TYPE:
                strcat(name, "DOUBLE8_");
                break;
            case DOUBLE16_TYPE:
                strcat(name, "DOUBLE16_");
                break;

        }

        switch (op->kind)
        {
            case NONE_OP:
                strcat(name, "NONE_OP");
                break;
            case MEMORY_OP:
                strcat(name, "MEMORY_OP");
                break;
            case ADDITION_OP:
                strcat(name, "ADDITION_OP");
                break;
            case SUBTRACTION_OP:
                strcat(name, "SUBTRACTION_OP");
                break;
            case MULTIPLICATION_OP:
                strcat(name, "MULTIPLICATION_OP");
                break;
            case DIVISION_OP:
                strcat(name, "DIVISION_OP");
                break;
            case MODULAR_OP:
                strcat(name, "MODULAR_OP");
                break;
            case POST_INCREASE_OP:
                strcat(name, "POST_INCREASE_OP");
                break;
            case POST_DECREASE_OP:
                strcat(name, "POST_DECREASE_OP");
                break;
            case PRE_INCREASE_OP:
                strcat(name, "PRE_INCREASE_OP");
                break;
            case PRE_DECREASE_OP:
                strcat(name, "PRE_DECREASE_OP");
                break;
            case SHIFT_LEFT_OP:
                strcat(name, "SHIFT_LEFT_OP");
                break;
            case SHIFT_RIGHT_OP:
                strcat(name, "SHIFT_RIGHT_OP");
                break;
            case LESS_OP:
                strcat(name, "LESS_OP");
                break;
            case LESS_EQUAL_OP:
                strcat(name, "LESS_EQUAL_OP");
                break;
            case GREATER_OP:
                strcat(name, "GREATER_OP");
                break;
            case GREATER_EQUAL_OP:
                strcat(name, "GREATER_EQUAL_OP");
                break;
            case EQUAL_OP:
                strcat(name, "EQUAL_OP");
                break;
            case NOT_EQUAL_OP:
                strcat(name, "NOT_EQUAL_OP");
                break;
            case BITWISE_AND_OP:
                strcat(name, "BITWISE_AND_OP");
                break;
            case BITWISE_XOR_OP:
                strcat(name, "BITWISE_XOR_OP");
                break;
            case BITWISE_OR_OP:
                strcat(name, "BITWISE_OR_OP");
                break;
            case BITWISE_COMPLEMENT_OP:
                strcat(name, "BITWISE_COMPLEMENT_OP");
                break;
            case UNARY_PLUS_OP:
                strcat(name, "UNARY_PLUS_OP");
                break;
            case UNARY_MINUS_OP:
                strcat(name, "UNARY_MINUS_OP");
                break;
            case LOGICAL_AND_OP:
                strcat(name, "LOGICAL_AND_OP");
                break;
            case LOGICAL_OR_OP:
                strcat(name, "LOGICAL_OR_OP");
                break;
            case LOGICAL_COMPLEMENT_OP:
                strcat(name, "LOGICAL_COMPLEMENT_OP");
                break;
            case MAD_OP:
                strcat(name, "MAD_OP");
                break;
        }
    }

}

void DeleteSymbolTable(SymbolTable* table)
{
    if (!table)
        return;
    else
    {
        SymbolTableLevel* iterLevel = table->level_tail;

        while (iterLevel != NULL)
        {
            DeleteSymTableLevel(table);
            iterLevel = table->level_tail;
        }
        free (table);
    }
}

void DeleteOPTrace(Operation_list* list)
{
    if (!list)
        return;
    else
    {
        Operation* iterOP = list->operation_head;
        Operation* nextOP;

        while (iterOP != NULL)
        {
            nextOP = iterOP->next;
            DeleteOperation(iterOP);
            iterOP = nextOP;
        }
        free (list);
    }
}

void DeleteOperation(Operation* op)
{
    if (!op)
        return;
    else
    {
        if (op->value)
            DeleteSemanticValue(op->value);
        if (op->structural_dep)
            free (op->structural_dep);
        if (op->issue_dep)
            free (op->issue_dep);
        if (op->data_dep_head)
        {
            Dependency* iterDep = op->data_dep_head;
            Dependency* nextDep;
            while (iterDep != NULL)
            {
                nextDep = iterDep->next;
                free (iterDep);
                iterDep = nextDep;
            }
        }

        free (op);
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
            char name[100];
            GetOperationName(iterOP, name);

            printf("[#%lu] %-30s ", iterOP->id, name);
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
            if (iterOP->value)
            {
                // Treat as unsigned value
                unsigned long constVal;
                GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, iterOP->value, &constVal);
                printf("[Index]: %lu", constVal);
                ShowNDRangeVector(iterOP->value->vector);
            }
            printf("\n");
            iterOP = iterOP->next;
        }
    }
}

void ShowNDRangeVector(NDRangeVector vector)
{
    if (!CheckNDRangeVector(vector)) // zero value
        return;
    else
    {
        if (vector.globalIdx[0] != 0) printf(" + %ld x globalID(0)", vector.globalIdx[0]);
        if (vector.globalIdx[1] != 0) printf(" + %ld x globalID(1)", vector.globalIdx[1]);
        if (vector.globalIdx[2] != 0) printf(" + %ld x globalID(2)", vector.globalIdx[2]);
        if (vector.localIdx[0] != 0) printf(" + %ld x localID(0)", vector.localIdx[0]);
        if (vector.localIdx[1] != 0) printf(" + %ld x localID(1)", vector.localIdx[1]);
        if (vector.localIdx[2] != 0) printf(" + %ld x localID(2)", vector.localIdx[2]);
        if (vector.groupIdx[0] != 0) printf(" + %ld x groupID(0)", vector.groupIdx[0]);
        if (vector.groupIdx[1] != 0) printf(" + %ld x groupID(1)", vector.groupIdx[1]);
        if (vector.groupIdx[2] != 0) printf(" + %ld x groupID(2)", vector.groupIdx[2]);
    }
}

/* Would set result_value->kind if undefined NDRange calculation is assigned */
NDRangeVector CalculateNDRangeVector(NDRangeVector left_vector, NDRangeVector right_vector, unsigned long left_const, unsigned long right_const, EXPRESSION_KIND kind, SemanticValue* result_value)
{
    NDRangeVector ret = CreateZeroNDRangeVector();
    switch (kind)
    {
        case ADDITION_OP:
            ret.globalIdx[0] = left_vector.globalIdx[0] + right_vector.globalIdx[0];
            ret.globalIdx[1] = left_vector.globalIdx[1] + right_vector.globalIdx[1];
            ret.globalIdx[2] = left_vector.globalIdx[2] + right_vector.globalIdx[2];
            ret.localIdx[0] = left_vector.localIdx[0] + right_vector.localIdx[0];
            ret.localIdx[1] = left_vector.localIdx[1] + right_vector.localIdx[1];
            ret.localIdx[2] = left_vector.localIdx[2] + right_vector.localIdx[2];
            ret.groupIdx[0] = left_vector.groupIdx[0] + right_vector.groupIdx[0];
            ret.groupIdx[1] = left_vector.groupIdx[1] + right_vector.groupIdx[1];
            ret.groupIdx[2] = left_vector.groupIdx[2] + right_vector.groupIdx[2];
            break;
        case SUBTRACTION_OP:
            ret.globalIdx[0] = left_vector.globalIdx[0] - right_vector.globalIdx[0];
            ret.globalIdx[1] = left_vector.globalIdx[1] - right_vector.globalIdx[1];
            ret.globalIdx[2] = left_vector.globalIdx[2] - right_vector.globalIdx[2];
            ret.localIdx[0] = left_vector.localIdx[0] - right_vector.localIdx[0];
            ret.localIdx[1] = left_vector.localIdx[1] - right_vector.localIdx[1];
            ret.localIdx[2] = left_vector.localIdx[2] - right_vector.localIdx[2];
            ret.groupIdx[0] = left_vector.groupIdx[0] - right_vector.groupIdx[0];
            ret.groupIdx[1] = left_vector.groupIdx[1] - right_vector.groupIdx[1];
            ret.groupIdx[2] = left_vector.groupIdx[2] - right_vector.groupIdx[2];
            break;
        case MULTIPLICATION_OP:
            if (CheckNDRangeVector(left_vector) && CheckNDRangeVector(right_vector))
            {
                fprintf(stderr, "[Error] Unsupported multiplication of two non-zero NDRange vector in %s\n", __func__);
                result_value->kind = VALUE_UNDEFINED;
            }
            else
            {
                ret.globalIdx[0] = left_vector.globalIdx[0] * right_const + left_const * right_vector.globalIdx[0];
                ret.globalIdx[1] = left_vector.globalIdx[1] * right_const + left_const * right_vector.globalIdx[1];
                ret.globalIdx[2] = left_vector.globalIdx[2] * right_const + left_const * right_vector.globalIdx[2];
                ret.localIdx[0] = left_vector.localIdx[0] * right_const + left_const * right_vector.localIdx[0];
                ret.localIdx[1] = left_vector.localIdx[1] * right_const + left_const * right_vector.localIdx[1];
                ret.localIdx[2] = left_vector.localIdx[2] * right_const + left_const * right_vector.localIdx[2];
                ret.groupIdx[0] = left_vector.groupIdx[0] * right_const + left_const * right_vector.groupIdx[0];
                ret.groupIdx[1] = left_vector.groupIdx[1] * right_const + left_const * right_vector.groupIdx[1];
                ret.groupIdx[2] = left_vector.groupIdx[2] * right_const + left_const * right_vector.groupIdx[2];
            }
            break;
        default:
            {
                char name[100];
                DebugExprKind(kind, name);
                fprintf(stderr, "[Error] Unsupported expression kind \'%s\' in %s\n", name, __func__);
            }
            break;
    }
    return ret;
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

        TypeDescriptor* large_type = NULL;
        SEMANTIC_VALUE_TYPE large_value_type;
        SemanticRepresentation* ret = (SemanticRepresentation*) malloc(sizeof(SemanticRepresentation));
        Operation* currOP = NULL;
        ret->lvalue = NULL;
        ret->next = NULL;
        ret->value = CreateEmptySemanticValue();

        if (left && right)
            large_type = ComputeAndCreateTypeDesc(left->type, right->type);
        else if (left)
            large_type = DuplicateTypeDesc(left->type);
        else if (right)
            large_type = DuplicateTypeDesc(right->type);

        /* Determine the type of the result */
        if (kind & LOGICAL_OP_MASK)
        {
            /* Logical operations would always return int */
            ret->type = CreateScalarTypeDesc(INT_TYPE, NULL);
        }
        else
        {
            /* Arithmetic operations would always return bigger type */
            ret->type = DuplicateTypeDesc(large_type);
        }
        ret->value->type = TypeDescToSemanticValueType(ret->type);
        large_value_type = TypeDescToSemanticValueType(large_type);

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
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->vector = CalculateNDRangeVector(left->value->vector, right->value->vector, left_val, right_val, ADDITION_OP, ret->value);
                    ret->value->constVal.long_val = (left_val + right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->vector = CalculateNDRangeVector(left->value->vector, right->value->vector, left_val, right_val, ADDITION_OP, ret->value);
                    ret->value->constVal.ulong_val = (left_val + right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.double_val = (left_val + right_val);
                }
                currOP = CreateOperation(large_type, ADDITION_OP, NULL);
                break;
            case SUBTRACTION_OP:
            case ASSIGNMENT_SUB:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->vector = CalculateNDRangeVector(left->value->vector, right->value->vector, left_val, right_val, SUBTRACTION_OP, ret->value);
                    ret->value->constVal.long_val = (left_val - right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->vector = CalculateNDRangeVector(left->value->vector, right->value->vector, left_val, right_val, SUBTRACTION_OP, ret->value);
                    ret->value->constVal.ulong_val = (left_val - right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.double_val = (left_val - right_val);
                }
                currOP = CreateOperation(large_type, SUBTRACTION_OP, NULL);
                break;
            case MULTIPLICATION_OP:
            case ASSIGNMENT_MUL:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->vector = CalculateNDRangeVector(left->value->vector, right->value->vector, left_val, right_val, MULTIPLICATION_OP, ret->value);
                    ret->value->constVal.long_val = (left_val * right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->vector = CalculateNDRangeVector(left->value->vector, right->value->vector, left_val, right_val, MULTIPLICATION_OP, ret->value);
                    ret->value->constVal.ulong_val = (left_val * right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.double_val = (left_val * right_val);
                }
                currOP = CreateOperation(large_type, MULTIPLICATION_OP, NULL);
                break;
            case DIVISION_OP:
            case ASSIGNMENT_DIV:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val / right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val / right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.double_val = (left_val / right_val);
                }
                currOP = CreateOperation(large_type, DIVISION_OP, NULL);
                break;
            case MODULAR_OP:
            case ASSIGNMENT_MOD:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val % right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val % right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"%%\" in %s\n", __func__);
                }
                currOP = CreateOperation(large_type, MODULAR_OP, NULL);
                break;
            case POST_INCREASE_OP:
                {
                    SymbolTableEntry* entry = left->lvalue;
                    if (entry == NULL)
                    {
                        fprintf(stderr, "[Error] Invalid lvalue in post increase operator in %s\n", __func__);
                    }
                    else
                    {
                        if (large_value_type == VALUE_SIGNED_INTEGER)
                        {
                            long left_val;
                            GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                            entry->value->constVal.long_val = (left_val + 1);
                            ret->value->constVal.long_val = left_val;
                        }
                        else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                        {
                            unsigned long left_val;
                            GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                            entry->value->constVal.ulong_val = (left_val + 1);
                            ret->value->constVal.ulong_val = left_val;
                        }
                        else if (large_value_type == VALUE_FLOAT)
                        {
                            double left_val;
                            GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                            entry->value->constVal.double_val = (left_val + 1);
                            ret->value->constVal.double_val = left_val;
                        }
                        currOP = CreateOperation(large_type, ADDITION_OP, NULL);
                        entry->value->lastOP = currOP;
                    }
                }
                break;
            case POST_DECREASE_OP:
                {
                    SymbolTableEntry* entry = left->lvalue;
                    if (entry == NULL)
                    {
                        fprintf(stderr, "[Error] Invalid lvalue in post increase operator in %s\n", __func__);
                    }
                    else
                    {
                        if (large_value_type == VALUE_SIGNED_INTEGER)
                        {
                            long left_val;
                            GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                            entry->value->constVal.long_val = (left_val - 1);
                            ret->value->constVal.long_val = left_val;
                        }
                        else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                        {
                            unsigned long left_val;
                            GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                            entry->value->constVal.ulong_val = (left_val - 1);
                            ret->value->constVal.ulong_val = left_val;
                        }
                        else if (large_value_type == VALUE_FLOAT)
                        {
                            double left_val;
                            GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                            entry->value->constVal.double_val = (left_val - 1);
                            ret->value->constVal.double_val = left_val;
                        }
                        currOP = CreateOperation(large_type, SUBTRACTION_OP, NULL);
                        entry->value->lastOP = currOP;
                    }
                }
                break;
            case PRE_INCREASE_OP:
                {
                    SymbolTableEntry* entry = left->lvalue;
                    if (entry == NULL)
                    {
                        fprintf(stderr, "[Error] Invalid lvalue in post increase operator in %s\n", __func__);
                    }
                    else
                    {
                        if (large_value_type == VALUE_SIGNED_INTEGER)
                        {
                            long left_val;
                            GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                            entry->value->constVal.long_val = (left_val + 1);
                            ret->value->constVal.long_val = (left_val + 1);
                        }
                        else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                        {
                            unsigned long left_val;
                            GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                            entry->value->constVal.ulong_val = (left_val + 1);
                            ret->value->constVal.ulong_val = (left_val + 1);
                        }
                        else if (large_value_type == VALUE_FLOAT)
                        {
                            double left_val;
                            GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                            entry->value->constVal.double_val = (left_val + 1);
                            ret->value->constVal.double_val = (left_val + 1);
                        }
                        currOP = CreateOperation(large_type, ADDITION_OP, NULL);
                        entry->value->lastOP = currOP;
                    }
                }
                break;
            case PRE_DECREASE_OP:
                {
                    SymbolTableEntry* entry = left->lvalue;
                    if (entry == NULL)
                    {
                        fprintf(stderr, "[Error] Invalid lvalue in post increase operator in %s\n", __func__);
                    }
                    else
                    {
                        if (large_value_type == VALUE_SIGNED_INTEGER)
                        {
                            long left_val;
                            GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                            entry->value->constVal.long_val = (left_val - 1);
                            ret->value->constVal.long_val = (left_val - 1);
                        }
                        else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                        {
                            unsigned long left_val;
                            GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                            entry->value->constVal.ulong_val = (left_val - 1);
                            ret->value->constVal.ulong_val = (left_val - 1);
                        }
                        else if (large_value_type == VALUE_FLOAT)
                        {
                            double left_val;
                            GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                            entry->value->constVal.double_val = (left_val - 1);
                            ret->value->constVal.double_val = (left_val - 1);
                        }
                        currOP = CreateOperation(large_type, SUBTRACTION_OP, NULL);
                        entry->value->lastOP = currOP;
                    }
                }
                break;
            case SHIFT_LEFT_OP:
            case ASSIGNMENT_LEFT:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val << right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val << right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"<<\" in %s\n", __func__);
                }
                currOP = CreateOperation(large_type, SHIFT_LEFT_OP, NULL);
                break;
            case SHIFT_RIGHT_OP:
            case ASSIGNMENT_RIGHT:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val >> right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val >> right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \">>\" in %s\n", __func__);
                }
                currOP = CreateOperation(large_type, SHIFT_RIGHT_OP, NULL);
                break;
            case BITWISE_AND_OP:
            case ASSIGNMENT_AND:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val & right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val & right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"&\" in %s\n", __func__);
                }
                currOP = CreateOperation(large_type, BITWISE_AND_OP, NULL);
                break;
            case BITWISE_XOR_OP:
            case ASSIGNMENT_XOR:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val ^ right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val ^ right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"^\" in %s\n", __func__);
                }
                currOP = CreateOperation(large_type, BITWISE_XOR_OP, NULL);
                break;
            case BITWISE_OR_OP:
            case ASSIGNMENT_OR:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val | right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.ulong_val = (left_val | right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"|\" in %s\n", __func__);
                }
                currOP = CreateOperation(large_type, BITWISE_OR_OP, NULL);
                break;
            case BITWISE_COMPLEMENT_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    ret->value->constVal.long_val = (~left_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    ret->value->constVal.ulong_val = (~left_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    fprintf(stderr, "[Error] Invalid operand type for operator \"~\" in %s\n", __func__);
                }
                currOP = CreateOperation(large_type, BITWISE_COMPLEMENT_OP, NULL);
                break;
             case UNARY_PLUS_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    ret->value->constVal.long_val = (+left_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    ret->value->constVal.ulong_val = (+left_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    ret->value->constVal.double_val = (+left_val);
                }
                currOP = CreateOperation(large_type, UNARY_PLUS_OP, NULL);
                break;
             case UNARY_MINUS_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    ret->value->constVal.long_val = (-left_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    ret->value->constVal.ulong_val = (-left_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    ret->value->constVal.double_val = (-left_val);
                }
                currOP = CreateOperation(large_type, UNARY_MINUS_OP, NULL);
                break;
            case LESS_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val < right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val < right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val < right_val);
                }
                currOP = CreateOperation(large_type, LESS_OP, NULL);
                break;
            case LESS_EQUAL_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val <= right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val <= right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val <= right_val);
                }
                currOP = CreateOperation(large_type, LESS_EQUAL_OP, NULL);
                break;
            case GREATER_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val > right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val > right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val > right_val);
                }
                currOP = CreateOperation(large_type, GREATER_OP, NULL);
                break;
            case GREATER_EQUAL_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val >= right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val >= right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val >= right_val);
                }
                currOP = CreateOperation(large_type, GREATER_EQUAL_OP, NULL);
                break;
            case EQUAL_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val == right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val == right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val == right_val);
                }
                currOP = CreateOperation(large_type, EQUAL_OP, NULL);
                break;
            case NOT_EQUAL_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val != right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val != right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val != right_val);
                }
                currOP = CreateOperation(large_type, NOT_EQUAL_OP, NULL);
                break;
            case LOGICAL_AND_OP: // TODO lazy evaluation
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val && right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val && right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val && right_val);
                }
                currOP = CreateOperation(large_type, LOGICAL_AND_OP, NULL);
                break;
            case LOGICAL_OR_OP: // TODO lazy evaluation
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val, right_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val || right_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val, right_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val || right_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val, right_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    GetValueInSemanticValue(VALUE_FLOAT, right->value, &right_val);
                    ret->value->constVal.long_val = (left_val || right_val);
                }
                currOP = CreateOperation(large_type, LOGICAL_OR_OP, NULL);
                break;
            case LOGICAL_COMPLEMENT_OP:
                if (large_value_type == VALUE_SIGNED_INTEGER)
                {
                    long left_val;
                    GetValueInSemanticValue(VALUE_SIGNED_INTEGER, left->value, &left_val);
                    ret->value->constVal.long_val = (!left_val);
                }
                else if ((large_value_type == VALUE_UNSIGNED_INTEGER) || (large_value_type == VALUE_POINTER))
                {
                    unsigned long left_val;
                    GetValueInSemanticValue(VALUE_UNSIGNED_INTEGER, left->value, &left_val);
                    ret->value->constVal.long_val = (!left_val);
                }
                else if (large_value_type == VALUE_FLOAT)
                {
                    double left_val;
                    GetValueInSemanticValue(VALUE_FLOAT, left->value, &left_val);
                    ret->value->constVal.long_val = (!left_val);
                }
                currOP = CreateOperation(large_type, LOGICAL_COMPLEMENT_OP, NULL);
                break;

            default:
                break;
        }

        DeleteTypeDesc(large_type);
        large_type = NULL;

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

