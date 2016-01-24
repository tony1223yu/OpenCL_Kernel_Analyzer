D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P                       ([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS                      ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%{
#include <stdio.h>
#include "y.tab.h"

/* L?\"(\\.|[^\\"\n])*\"	{ printf("string : \"%s\"\n", yytext); count(); return (STRING_LITERAL); } */
void count(void);
void comment(void);
%}

%%
"/*"			{ comment(); }
"//"[^\n]*              { /* consume //-comment */ }

"__kernel"      { count(); return (KERNEL); }
"__global"      { count(); return (ADDRESS_GLOBAL); }
"__local"      { count(); return (ADDRESS_LOCAL); }
"__constant"      { count(); return (ADDRESS_CONSTANT); }
"__private"      { count(); return (ADDRESS_PRIVATE); }
"get_global_id"     { count(); return (GLOBAL_ID_FUNC); }
"get_global_size"   { count(); return (GLOBAL_SIZE_FUNC); }
"get_local_id"      { count(); return (LOCAL_ID_FUNC); }
"get_local_size"    { count(); return (LOCAL_SIZE_FUNC); }
"get_work_dim"      { count(); return (WORK_DIM_FUNC); }
"get_num_groups"      { count(); return (NUM_GROUPS_FUNC); }
"get_group_id"      { count(); return (GROUP_ID_FUNC); }

"#define"          { count(); return (DEFINE); }

"float"           { count(); return (OPENCL_TYPE); }
"float2"          { count(); return (OPENCL_TYPE); }
"float4"          { count(); return (OPENCL_TYPE); }
"float8"          { count(); return (OPENCL_TYPE); }
"float16"          { count(); return (OPENCL_TYPE); }
"double"           { count(); return (OPENCL_TYPE); }
"double2"          { count(); return (OPENCL_TYPE); }
"double4"          { count(); return (OPENCL_TYPE); }
"double8"          { count(); return (OPENCL_TYPE); }
"double16"          { count(); return (OPENCL_TYPE); }
"char"          { count(); return (OPENCL_TYPE); }
"char2"          { count(); return (OPENCL_TYPE); }
"char4"          { count(); return (OPENCL_TYPE); }
"char8"          { count(); return (OPENCL_TYPE); }
"char16"          { count(); return (OPENCL_TYPE); }
"short"           { count(); return (OPENCL_TYPE); }
"short2"          { count(); return (OPENCL_TYPE); }
"short4"          { count(); return (OPENCL_TYPE); }
"short8"          { count(); return (OPENCL_TYPE); }
"short16"          { count(); return (OPENCL_TYPE); }
"int"           { count(); return (OPENCL_TYPE); }
"int2"          { count(); return (OPENCL_TYPE); }
"int4"          { count(); return (OPENCL_TYPE); }
"int8"          { count(); return (OPENCL_TYPE); }
"int16"          { count(); return (OPENCL_TYPE); }
"long"           { count(); return (OPENCL_TYPE); }
"long2"          { count(); return (OPENCL_TYPE); }
"long4"          { count(); return (OPENCL_TYPE); }
"long8"          { count(); return (OPENCL_TYPE); }
"long16"          { count(); return (OPENCL_TYPE); }
"uchar"           { count(); return (OPENCL_TYPE); }
"unsigned char"           { count(); return (OPENCL_TYPE); }
"uchar2"          { count(); return (OPENCL_TYPE); }
"uchar4"          { count(); return (OPENCL_TYPE); }
"uchar8"          { count(); return (OPENCL_TYPE); }
"uchar16"          { count(); return (OPENCL_TYPE); }
"ushort"           { count(); return (OPENCL_TYPE); }
"unsigned short"           { count(); return (OPENCL_TYPE); }
"ushort2"          { count(); return (OPENCL_TYPE); }
"ushort4"          { count(); return (OPENCL_TYPE); }
"ushort8"          { count(); return (OPENCL_TYPE); }
"ushort16"          { count(); return (OPENCL_TYPE); }
"uint"           { count(); return (OPENCL_TYPE); }
"unsigned int"           { count(); return (OPENCL_TYPE); }
"uint2"          { count(); return (OPENCL_TYPE); }
"uint4"          { count(); return (OPENCL_TYPE); }
"uint8"          { count(); return (OPENCL_TYPE); }
"uint16"          { count(); return (OPENCL_TYPE); }
"ulong"           { count(); return (OPENCL_TYPE); }
"unsigned long"           { count(); return (OPENCL_TYPE); }
"ulong2"          { count(); return (OPENCL_TYPE); }
"ulong4"          { count(); return (OPENCL_TYPE); }
"ulong8"          { count(); return (OPENCL_TYPE); }
"ulong16"          { count(); return (OPENCL_TYPE); }
"bool"			{ count(); return (OPENCL_TYPE); }
"half"          { count(); return (OPENCL_TYPE); }
"void"			{ count(); return (OPENCL_TYPE); }

"auto"			{ count(); return (AUTO); }
"break"			{ count(); return (BREAK); }
"case"			{ count(); return (CASE); }
"const"			{ count(); return (CONST); }
"continue"		{ count(); return (CONTINUE); }
"default"		{ count(); return (DEFAULT); }
"do"			{ count(); return (DO); }
"else"			{ count(); return (ELSE); }
"enum"			{ count(); return (ENUM); }
"extern"		{ count(); return (EXTERN); }
"for"			{ count(); return (FOR); }
"goto"			{ count(); return (GOTO); }
"if"			{ count(); return (IF); }
"inline"		{ count(); return (INLINE); }
"register"		{ count(); return (REGISTER); }
"restrict"		{ count(); return (RESTRICT); }
"return"		{ count(); return (RETURN); }
"sizeof"		{ count(); return (SIZEOF); }
"static"		{ count(); return (STATIC); }
"struct"		{ count(); return (STRUCT); }
"switch"		{ count(); return (SWITCH); }
"typedef"		{ count(); return (TYPEDEF); }
"union"			{ count(); return (UNION); }
"volatile"		{ count(); return (VOLATILE); }
"while"			{ count(); return (WHILE); }

{L}({L}|{D})*		{ count(); return (check_type()); }


 /* TODO: seperate int/long and float/double */
0[xX]{H}+{IS}?		{ count(); return (CONSTANT); }
0[0-7]*{IS}?		{ count(); return (CONSTANT); }
[1-9]{D}*{IS}?		{ count(); return (CONSTANT); }
L?'(\\.|[^\\'\n])+'	{ count(); return (CONSTANT); }

{D}+{E}{FS}?		{ count(); return (CONSTANT); }
{D}*"."{D}+{E}?{FS}?	{ count(); return (CONSTANT); }
{D}+"."{D}*{E}?{FS}?	{ count(); return (CONSTANT); }
0[xX]{H}+{P}{FS}?	{ count(); return (CONSTANT); }
0[xX]{H}*"."{H}+{P}?{FS}?     { count(); return (CONSTANT); }
0[xX]{H}+"."{H}*{P}?{FS}?     { count(); return (CONSTANT); }


L?\"([^"]|\\.)*\" { count(); return (STRING_LITERAL); }

"..."			{ count(); return (ELLIPSIS); }
">>="			{ count(); return (RIGHT_ASSIGN); }
"<<="			{ count(); return (LEFT_ASSIGN); }
"+="			{ count(); return (ADD_ASSIGN); }
"-="			{ count(); return (SUB_ASSIGN); }
"*="			{ count(); return (MUL_ASSIGN); }
"/="			{ count(); return (DIV_ASSIGN); }
"%="			{ count(); return (MOD_ASSIGN); }
"&="			{ count(); return (AND_ASSIGN); }
"^="			{ count(); return (XOR_ASSIGN); }
"|="			{ count(); return (OR_ASSIGN); }
">>"			{ count(); return (RIGHT_OP); }
"<<"			{ count(); return (LEFT_OP); }
"++"			{ count(); return (INC_OP); }
"--"			{ count(); return (DEC_OP); }
"->"			{ count(); return (PTR_OP); }
"&&"			{ count(); return (AND_OP); }
"||"			{ count(); return (OR_OP); }
"<="			{ count(); return (LE_OP); }
">="			{ count(); return (GE_OP); }
"=="			{ count(); return (EQ_OP); }
"!="			{ count(); return (NE_OP); }
";"			{ count(); return (';'); }
("{"|"<%")		{ count(); return ('{'); }
("}"|"%>")		{ count(); return ('}'); }
","			{ count(); return (','); }
":"			{ count(); return (':'); }
"="			{ count(); return ('='); }
"("			{ count(); return ('('); }
")"			{ count(); return (')'); }
("["|"<:")		{ count(); return ('['); }
("]"|":>")		{ count(); return (']'); }
"."			{ count(); return ('.'); }
"&"			{ count(); return ('&'); }
"!"			{ count(); return ('!'); }
"~"			{ count(); return ('~'); }
"-"			{ count(); return ('-'); }
"+"			{ count(); return ('+'); }
"*"			{ count(); return ('*'); }
"/"			{ count(); return ('/'); }
"%"			{ count(); return ('%'); }
"<"			{ count(); return ('<'); }
">"			{ count(); return ('>'); }
"^"			{ count(); return ('^'); }
"|"			{ count(); return ('|'); }
"?"			{ count(); return ('?'); }

[ \t\v\n\f]		{ count(); }
.			{ /* Add code to complain about unmatched characters */ }

%%

int yywrap(void)
{
	return 1;
}


void comment(void)
{
	char c, prev = 0;

	while ((c = input()) != 0)      /* (EOF maps to 0) */
	{
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	error("unterminated comment");
}


int column = 0;

void count(void)
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

    //ECHO;
}


int check_type(void)
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return TYPE_NAME;
*
*	return IDENTIFIER;
*/

/*
*	it actually will only return IDENTIFIER
*/

        //return TYPE_NAME;
    {
        yylval.ptr = strdup(yytext);
        if (!yylval.ptr)
        {
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
        return IDENTIFIER;
    }
}
