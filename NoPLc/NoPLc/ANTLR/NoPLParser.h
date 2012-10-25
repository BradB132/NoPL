/** \file
 *  This C header file was generated by $ANTLR version 3.4
 *
 *     -  From the grammar source file : /Users/Brad/Documents/Code/git/NoPL/ANTLR_source/NoPL.g
 *     -                            On : 2012-10-25 15:24:51
 *     -                for the parser : NoPLParserParser
 *
 * Editing it, at least manually, is not wise.
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 *
 * The parser 
NoPLParser

has the callable functions (rules) shown below,
 * which will invoke the code for the associated rule in the source grammar
 * assuming that the input stream is pointing to a token/text stream that could begin
 * this rule.
 *
 * For instance if you call the first (topmost) rule in a parser grammar, you will
 * get the results of a full parse, but calling a rule half way through the grammar will
 * allow you to pass part of a full token stream to the parser, such as for syntax checking
 * in editors and so on.
 *
 * The parser entry points are called indirectly (by function pointer to function) via
 * a parser context typedef pNoPLParser, which is returned from a call to NoPLParserNew().
 *
 * The methods in pNoPLParser are  as follows:
 *
 *  - 
 NoPLParser_program_return
      pNoPLParser->program(pNoPLParser)
 *  - 
 NoPLParser_variableScope_return
      pNoPLParser->variableScope(pNoPLParser)
 *  - 
 NoPLParser_statement_return
      pNoPLParser->statement(pNoPLParser)
 *  - 
 NoPLParser_nonControlStatement_return
      pNoPLParser->nonControlStatement(pNoPLParser)
 *  - 
 NoPLParser_controlFlowStatement_return
      pNoPLParser->controlFlowStatement(pNoPLParser)
 *  - 
 NoPLParser_expression_return
      pNoPLParser->expression(pNoPLParser)
 *  - 
 NoPLParser_booleanExpression_return
      pNoPLParser->booleanExpression(pNoPLParser)
 *  - 
 NoPLParser_booleanSubExpression_return
      pNoPLParser->booleanSubExpression(pNoPLParser)
 *  - 
 NoPLParser_expressionCast_return
      pNoPLParser->expressionCast(pNoPLParser)
 *  - 
 NoPLParser_atom_return
      pNoPLParser->atom(pNoPLParser)
 *  - 
 NoPLParser_numericExpression_return
      pNoPLParser->numericExpression(pNoPLParser)
 *  - 
 NoPLParser_mdNumericExpression_return
      pNoPLParser->mdNumericExpression(pNoPLParser)
 *  - 
 NoPLParser_eNumericExpression_return
      pNoPLParser->eNumericExpression(pNoPLParser)
 *  - 
 NoPLParser_modNumericExpression_return
      pNoPLParser->modNumericExpression(pNoPLParser)
 *  - 
 NoPLParser_variableDeclaration_return
      pNoPLParser->variableDeclaration(pNoPLParser)
 *  - 
 NoPLParser_variableAssign_return
      pNoPLParser->variableAssign(pNoPLParser)
 *  - 
 NoPLParser_unaryOperation_return
      pNoPLParser->unaryOperation(pNoPLParser)
 *  - 
 NoPLParser_objectExpression_return
      pNoPLParser->objectExpression(pNoPLParser)
 *  - 
 NoPLParser_objectAtom_return
      pNoPLParser->objectAtom(pNoPLParser)
 *  - 
 NoPLParser_functionArguments_return
      pNoPLParser->functionArguments(pNoPLParser)
 *  - 
 NoPLParser_functionCall_return
      pNoPLParser->functionCall(pNoPLParser)
 *  - 
 NoPLParser_loopBody_return
      pNoPLParser->loopBody(pNoPLParser)
 *  - 
 NoPLParser_whileLoop_return
      pNoPLParser->whileLoop(pNoPLParser)
 *  - 
 NoPLParser_forLoop_return
      pNoPLParser->forLoop(pNoPLParser)
 *  - 
 NoPLParser_doWhileLoop_return
      pNoPLParser->doWhileLoop(pNoPLParser)
 *  - 
 NoPLParser_conditional_return
      pNoPLParser->conditional(pNoPLParser)
 *  - 
 NoPLParser_elseConditional_return
      pNoPLParser->elseConditional(pNoPLParser)
 *  - 
 NoPLParser_innerSwitchStatement_return
      pNoPLParser->innerSwitchStatement(pNoPLParser)
 *  - 
 NoPLParser_switchStatement_return
      pNoPLParser->switchStatement(pNoPLParser)
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *
 * The return type for any particular rule is of course determined by the source
 * grammar file.
 */
// [The "BSD license"]
// Copyright (c) 2005-2009 Jim Idle, Temporal Wave LLC
// http://www.temporal-wave.com
// http://www.linkedin.com/in/jimidle
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef	_NoPLParser_H
#define _NoPLParser_H
/* =============================================================================
 * Standard antlr3 C runtime definitions
 */
#include    <antlr3.h>

/* End of standard antlr 3 runtime definitions
 * =============================================================================
 */

#ifdef __cplusplus
extern "C" {
#endif

// Forward declare the context typedef so that we can use it before it is
// properly defined. Delegators and delegates (from import statements) are
// interdependent and their context structures contain pointers to each other
// C only allows such things to be declared if you pre-declare the typedef.
//
typedef struct NoPLParser_Ctx_struct NoPLParser, * pNoPLParser;



#ifdef	ANTLR3_WINDOWS
// Disable: Unreferenced parameter,							- Rules with parameters that are not used
//          constant conditional,							- ANTLR realizes that a prediction is always true (synpred usually)
//          initialized but unused variable					- tree rewrite variables declared but not needed
//          Unreferenced local variable						- lexer rule declares but does not always use _type
//          potentially unitialized variable used			- retval always returned from a rule
//			unreferenced local function has been removed	- susually getTokenNames or freeScope, they can go without warnigns
//
// These are only really displayed at warning level /W4 but that is the code ideal I am aiming at
// and the codegen must generate some of these warnings by necessity, apart from 4100, which is
// usually generated when a parser rule is given a parameter that it does not use. Mostly though
// this is a matter of orthogonality hence I disable that one.
//
#pragma warning( disable : 4100 )
#pragma warning( disable : 4101 )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4189 )
#pragma warning( disable : 4505 )
#pragma warning( disable : 4701 )
#endif

/* ========================
 * BACKTRACKING IS ENABLED
 * ========================
 */

typedef struct NoPLParser_program_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_program_return;



typedef struct NoPLParser_variableScope_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_variableScope_return;



typedef struct NoPLParser_statement_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_statement_return;



typedef struct NoPLParser_nonControlStatement_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_nonControlStatement_return;



typedef struct NoPLParser_controlFlowStatement_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_controlFlowStatement_return;



typedef struct NoPLParser_expression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_expression_return;



typedef struct NoPLParser_booleanExpression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_booleanExpression_return;



typedef struct NoPLParser_booleanSubExpression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_booleanSubExpression_return;



typedef struct NoPLParser_expressionCast_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_expressionCast_return;



typedef struct NoPLParser_atom_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_atom_return;



typedef struct NoPLParser_numericExpression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_numericExpression_return;



typedef struct NoPLParser_mdNumericExpression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_mdNumericExpression_return;



typedef struct NoPLParser_eNumericExpression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_eNumericExpression_return;



typedef struct NoPLParser_modNumericExpression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_modNumericExpression_return;



typedef struct NoPLParser_variableDeclaration_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_variableDeclaration_return;



typedef struct NoPLParser_variableAssign_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_variableAssign_return;



typedef struct NoPLParser_unaryOperation_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_unaryOperation_return;



typedef struct NoPLParser_objectExpression_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_objectExpression_return;



typedef struct NoPLParser_objectAtom_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_objectAtom_return;



typedef struct NoPLParser_functionArguments_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_functionArguments_return;



typedef struct NoPLParser_functionCall_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_functionCall_return;



typedef struct NoPLParser_loopBody_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_loopBody_return;



typedef struct NoPLParser_whileLoop_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_whileLoop_return;



typedef struct NoPLParser_forLoop_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_forLoop_return;



typedef struct NoPLParser_doWhileLoop_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_doWhileLoop_return;



typedef struct NoPLParser_conditional_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_conditional_return;



typedef struct NoPLParser_elseConditional_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_elseConditional_return;



typedef struct NoPLParser_innerSwitchStatement_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_innerSwitchStatement_return;



typedef struct NoPLParser_switchStatement_return_struct
{
    /** Generic return elements for ANTLR3 rules that are not in tree parsers or returning trees
     */
    pANTLR3_COMMON_TOKEN    start;
    pANTLR3_COMMON_TOKEN    stop;
    pANTLR3_BASE_TREE	tree;

}
    NoPLParser_switchStatement_return;




/** Context tracking structure for 
NoPLParser

 */
struct NoPLParser_Ctx_struct
{
    /** Built in ANTLR3 context tracker contains all the generic elements
     *  required for context tracking.
     */
    pANTLR3_PARSER   pParser;

     NoPLParser_program_return
     (*program)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_variableScope_return
     (*variableScope)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_statement_return
     (*statement)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_nonControlStatement_return
     (*nonControlStatement)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_controlFlowStatement_return
     (*controlFlowStatement)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_expression_return
     (*expression)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_booleanExpression_return
     (*booleanExpression)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_booleanSubExpression_return
     (*booleanSubExpression)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_expressionCast_return
     (*expressionCast)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_atom_return
     (*atom)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_numericExpression_return
     (*numericExpression)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_mdNumericExpression_return
     (*mdNumericExpression)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_eNumericExpression_return
     (*eNumericExpression)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_modNumericExpression_return
     (*modNumericExpression)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_variableDeclaration_return
     (*variableDeclaration)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_variableAssign_return
     (*variableAssign)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_unaryOperation_return
     (*unaryOperation)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_objectExpression_return
     (*objectExpression)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_objectAtom_return
     (*objectAtom)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_functionArguments_return
     (*functionArguments)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_functionCall_return
     (*functionCall)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_loopBody_return
     (*loopBody)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_whileLoop_return
     (*whileLoop)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_forLoop_return
     (*forLoop)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_doWhileLoop_return
     (*doWhileLoop)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_conditional_return
     (*conditional)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_elseConditional_return
     (*elseConditional)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_innerSwitchStatement_return
     (*innerSwitchStatement)	(struct NoPLParser_Ctx_struct * ctx);

     NoPLParser_switchStatement_return
     (*switchStatement)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred16_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred17_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred18_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred20_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred40_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred42_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred43_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred44_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred65_NoPL)	(struct NoPLParser_Ctx_struct * ctx);

     ANTLR3_BOOLEAN
     (*synpred73_NoPL)	(struct NoPLParser_Ctx_struct * ctx);
    // Delegated rules

    const char * (*getGrammarFileName)();
    void            (*reset)  (struct NoPLParser_Ctx_struct * ctx);
    void	    (*free)   (struct NoPLParser_Ctx_struct * ctx);
/* @headerFile.members() */
pANTLR3_BASE_TREE_ADAPTOR	adaptor;
pANTLR3_VECTOR_FACTORY		vectors;
/* End @headerFile.members() */
};

// Function protoypes for the constructor functions that external translation units
// such as delegators and delegates may wish to call.
//
ANTLR3_API pNoPLParser NoPLParserNew         (
pANTLR3_COMMON_TOKEN_STREAM
 instream);
ANTLR3_API pNoPLParser NoPLParserNewSSD      (
pANTLR3_COMMON_TOKEN_STREAM
 instream, pANTLR3_RECOGNIZER_SHARED_STATE state);

/** Symbolic definitions of all the tokens that the 
parser
 will work with.
 * \{
 *
 * Antlr will define EOF, but we can't use that as it it is too common in
 * in C header files and that would be confusing. There is no way to filter this out at the moment
 * so we just undef it here for now. That isn't the value we get back from C recognizers
 * anyway. We are looking for ANTLR3_TOKEN_EOF.
 */
#ifdef	EOF
#undef	EOF
#endif
#ifdef	Tokens
#undef	Tokens
#endif
#define EOF      -1
#define ABS_VALUE      4
#define ADD      5
#define ADD_ASSIGN      6
#define ARGUMENTS      7
#define ARG_DELIMITER      8
#define ASSIGN      9
#define BREAK      10
#define COMMENT      11
#define CONDITIONAL      12
#define CONDITIONAL_ELSE      13
#define CONTINUE      14
#define DECL_BOOL      15
#define DECL_NUMBER      16
#define DECL_OBJ      17
#define DECL_STRING      18
#define DECREMENT      19
#define DIVIDE      20
#define DIVIDE_ASSIGN      21
#define ESC_SEQ      22
#define EXIT      23
#define EXPONENT      24
#define EXPONENT_ASSIGN      25
#define FLOAT_SPECIFIER      26
#define FOR_LOOP_COND      27
#define FOR_LOOP_DECL      28
#define FOR_LOOP_ITER      29
#define FUNCTION_CALL      30
#define GREATER_THAN      31
#define GREATER_THAN_EQUAL      32
#define HEX_DIGIT      33
#define ID      34
#define INCREMENT      35
#define LESS_THAN      36
#define LESS_THAN_EQUAL      37
#define LITERAL_FALSE      38
#define LITERAL_NULL      39
#define LITERAL_TRUE      40
#define LOGICAL_AND      41
#define LOGICAL_EQUALITY      42
#define LOGICAL_INEQUALITY      43
#define LOGICAL_NEGATION      44
#define LOGICAL_OR      45
#define LOOP_DO      46
#define LOOP_FOR      47
#define LOOP_WHILE      48
#define MOD      49
#define MOD_ASSIGN      50
#define MULTIPLY      51
#define MULTIPLY_ASSIGN      52
#define NUMBER      53
#define NUMBER_EXPONENT      54
#define NUMERIC_NEGATION      55
#define OBJECT_TO_MEMBER      56
#define OCTAL_ESC      57
#define PAREN_CLOSE      58
#define PAREN_OPEN      59
#define PRINT_VALUE      60
#define SCOPE_CLOSE      61
#define SCOPE_OPEN      62
#define STATEMENT_DELIMITER      63
#define STRING      64
#define SUBSCRIPT_CLOSE      65
#define SUBSCRIPT_OPEN      66
#define SUBTRACT      67
#define SUBTRACT_ASSIGN      68
#define SWITCH      69
#define SWITCH_CASE      70
#define SWITCH_DEFAULT      71
#define SWITCH_DELIMITER      72
#define TYPE_CAST      73
#define UNICODE_ESC      74
#define WS      75
#ifdef	EOF
#undef	EOF
#define	EOF	ANTLR3_TOKEN_EOF
#endif

#ifndef TOKENSOURCE
#define TOKENSOURCE(lxr) lxr->pLexer->rec->state->tokSource
#endif

/* End of token definitions for NoPLParser
 * =============================================================================
 */
/** } */

#ifdef __cplusplus
}
#endif

#endif

/* END - Note:Keep extra line feed to satisfy UNIX systems */
