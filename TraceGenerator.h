#ifndef __TRACE_GENERATOR_H__
#define __TRACE_GENERATOR_H__

#include "ASTDefinition.h"

typedef struct SymbolTable SymbolTable;
typedef struct SymbolTableLevel SymbolTableLevel;
typedef struct Dependency Dependency;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct SymbolTableEntry_list SymbolTableEntry_list;
typedef struct Operation Operation;
typedef struct Operation_list Operation_list;
typedef struct NDRangeVector NDRangeVector;
typedef struct SemanticValue SemanticValue;
typedef struct SemanticRepresentation SemanticRepresentation;
typedef struct SemanticRepresentation_list SemanticRepresentation_list;
typedef struct StmtRepresentation StmtRepresentation;

typedef enum SEMANTIC_VALUE_KIND SEMANTIC_VALUE_KIND;
typedef enum SEMANTIC_VALUE_TYPE SEMANTIC_VALUE_TYPE;
typedef enum DEPENDENCY_KIND DEPENDENCY_KIND;

SymbolTable* symTable;
Operation* lastIssueOP;
Operation_list* opTrace;
unsigned long g_operation_id;

void showOPTrace(Operation_list*);
SymbolTable* CreateSymTable();
void CreateSymbolTableLevel(SymbolTable*);
void DeleteSymTableLevel(SymbolTable*);
void AddEntryToSymTable(SymbolTable*, SymbolTableEntry*);
void AddEntryListToSymTable(SymbolTable*, SymbolTableEntry_list*);
SemanticRepresentation_list* AppendSemanticRepresentationToList(SemanticRepresentation_list*, SemanticRepresentation*);
SemanticValue* DuplicateSemanticValue(SemanticValue*);
SemanticRepresentation* DuplicateSemanticRepresentation(SemanticRepresentation*);
void AssignToSymTableEntry(SymbolTableEntry*, SemanticRepresentation*);
SymbolTableEntry_list* AppendSymTableEntryToList(SymbolTableEntry_list*, SymbolTableEntry*);
SymbolTableEntry* CreateSymbolTableEntry(Program_node*, char*, TypeDescriptor*);
void DeleteSymTableEntry(SymbolTableEntry*);
void DeleteSemanticRepresentation(SemanticRepresentation*);
void DeleteSemanticRepresentationList(SemanticRepresentation_list*);
void DeleteSemanticValue(SemanticValue*);
StmtRepresentation* TraceFuncNode(Program_node*, char*, SemanticRepresentation_list*);
StmtRepresentation* CreateStmtRepresentation(STATEMENT_KIND, SemanticRepresentation*);
void DeleteStmtRepresentation(StmtRepresentation*);
StmtRepresentation* TraceCompoundStmt(CompoundStatement*);
StmtRepresentation* TraceIterationStmt(IterationStatement*);
StmtRepresentation* TraceSelectionStmt(SelectionStatement*);
StmtRepresentation* TraceReturnStmt(ReturnStatement*);
StmtRepresentation* TraceStmtNode(Statement_node*);
StmtRepresentation* TraceExpressionStmt(ExpressionStatement*);
SemanticRepresentation* TraceExprNode(Expression_node*);
SymbolTableEntry_list* TraceDeclNode(Declaration_node*);
SemanticRepresentation* TraceExprNode(Expression_node*);
TypeDescriptor* MergeTypeDesc(TypeDescriptor*, TypeDescriptor*);
SEMANTIC_VALUE_TYPE TypeDescToSemanticValueType(TypeDescriptor*);
void GetValueInSemanticValue(SEMANTIC_VALUE_TYPE, SemanticValue*, void*);
SemanticValue* CreateEmptySemanticValue(void);
NDRangeVector CreateEmptyNDRangeVector(void);
SemanticRepresentation* CalculateSemanticRepresentation(EXPRESSION_KIND, SemanticRepresentation*, SemanticRepresentation*);
SymbolTableEntry* FindSymbolInSymTable(SymbolTable*, char*);
SymbolTableEntry* FindMemberInSymTable(SymbolTableEntry*, char*);
Operation* CreateOperation(TypeDescriptor*, EXPRESSION_KIND);
Operation_list* AppendOperationToList(Operation_list*, Operation*);
void AddDependency(Operation*, Operation*, DEPENDENCY_KIND);
Dependency* CreateDependency(Operation*, unsigned long);

enum SEMANTIC_VALUE_KIND
{
    VALUE_IRREGULAR = 0,
    VALUE_REGULAR,
    VALUE_UNDEFINED // e.g. multiply of NDRangeIndex, ...
};

enum SEMANTIC_VALUE_TYPE
{
    VALUE_SIGNED_INTEGER = 0,
    VALUE_UNSIGNED_INTEGER,
    VALUE_FLOAT,
    VALUE_POINTER,
    VALUE_OTHER
};

enum DEPENDENCY_KIND
{
    ISSUE_DEPENDENCY = 0,
    STRUCTURAL_DEPENDENCY,
    DATA_DEPENDENCY
};

struct StmtRepresentation
{
    STATEMENT_KIND kind;
    SemanticRepresentation* expression;
};

struct SemanticRepresentation_list
{
    SemanticRepresentation* value_head;
    SemanticRepresentation* value_tail;
};

struct SymbolTable
{
    SymbolTableLevel* level_head;
    SymbolTableLevel* level_tail;
};

struct SymbolTableLevel
{
    SymbolTableEntry* entry_head;
    SymbolTableEntry* entry_tail;
    SymbolTableLevel* prev;
    SymbolTableLevel* next;
};

struct SymbolTableEntry_list
{
    SymbolTableEntry* entry_head;
    SymbolTableEntry* entry_tail;
};

struct SymbolTableEntry
{
    TypeDescriptor* type;
    SemanticValue* value;
    char* name;
    SymbolTableEntry* next;

    SymbolTableEntry* member_head;
    SymbolTableEntry* member_tail;

    int array_dim;
    SymbolTableEntry** array_entry;
};

struct Dependency
{
    // Point to later operation
    Operation* targetOP;
    unsigned long latency;
    Dependency* next;
};

struct Operation_list
{
    Operation* operation_head;
    Operation* operation_tail;
};

struct Operation
{
    unsigned long id;
    EXPRESSION_KIND kind;
    OPENCL_DATA_TYPE type;

    Dependency* structural_dep;
    Dependency* data_dep_head;
    Dependency* data_dep_tail;
    Dependency* issue_dep;

    Operation* next;
};

struct NDRangeVector
{
    unsigned long globalIdx0;
    unsigned long globalIdx1;
    unsigned long globalIdx2;
    unsigned long groupIdx0;
    unsigned long groupIdx1;
    unsigned long groupIdx2;
    unsigned long localIdx0;
    unsigned long localIdx1;
    unsigned long localIdx2;
};

struct SemanticValue
{
    SEMANTIC_VALUE_KIND kind;
    SEMANTIC_VALUE_TYPE type;
    NDRangeVector vector;
    union constVal
    {
        long long_val;
        unsigned long ulong_val;
        double double_val;
    } constVal;

    /* Pointer to op in operation_list, should not free here */
    Operation* lastOP;
};

struct SemanticRepresentation
{
    TypeDescriptor* type;
    SemanticValue* value;

    /* Pointer to entry in symbol table, should not free here */
    SymbolTableEntry* lvalue;
    SemanticRepresentation* next;
};

#endif
