grammar NoPL;//NoPL is short for 'Non-Persistent Language'

options
{
	backtrack=true;
	memoize=true;
	output=AST;
//*
	language = C;
	ASTLabelType=pANTLR3_BASE_TREE;
//*/
}

//TO DO:
//	disallow global function calls with no parens
//	Pre-compile:
//		includes
//		value defines
//

tokens
{
	TYPE_CAST;
	NUMERIC_NEGATION;
	ARGUMENTS;
	FUNCTION_CALL;
}

//////////////////////////////////
//JAVA
//////////////////////////////////
/*
@members
{

//declare variable stack
public Stack variableStack = null;

//dummy object to make API's match with C target
Object ctx = null;

public void pushScope()
{
	variableStack.push(new ArrayList());
}

public void popScope()
{
	variableStack.pop();
}

public void parsingWillStart()
{
	//create stack
	variableStack = new Stack();
	
	//push the first scope
	pushScope();
}

public void parsingDidFinish()
{
	//pop the last scope
	popScope();
}

public boolean variableExists(String varName)
{
	for(int i = 0; i < variableStack.size(); i++)
	{
		ArrayList arr = (ArrayList)(variableStack.get(i));
		for(int j = 0; j < arr.size(); j++)
		{
			String str = (String)(arr.get(j));
			if(str.equals(varName))
				return true;
		}
	}
	return false;
}

public void declareVariable(String name)
{
	if(variableStack.size() == 0)
		return;//just in case
	
	ArrayList arr = (ArrayList)(variableStack.peek());
	arr.add(name);
}

public String nextString(Object obj)
{
	return input.LT(1).getText();
}

}
//*/

//////////////////////////////////
//C
//////////////////////////////////
//*

@parser::includes
{
//pANTLR3_VECTOR errorStrings = NULL;
//pANTLR3_VECTOR errorTokens = NULL;
}

@members
{

#define NoPL_StackSizeHint 16
#define NoPL_VectorSizeHint 8

//declare various typed stacks
pANTLR3_STACK variableStack = NULL;

void pushScope()
{
	//create a new vector
	pANTLR3_VECTOR variableVector = antlr3VectorNew(NoPL_VectorSizeHint);
	
	//push new vectors onto stack
	variableStack->push(variableStack, variableVector, (void(*)(void*))(variableVector->free));
}

void popScope()
{
	variableStack->pop(variableStack);
}

//void reportError(pANTLR3_STRING errDesc, pANTLR3_COMMON_TOKEN errToken)
//{
//	//lazy create error vectors
//	if(!errorStrings)
//		errorStrings = antlr3VectorNew(NoPL_VectorSizeHint);
//	if(!errorTokens)
//		errorTokens = antlr3VectorNew(NoPL_VectorSizeHint);
//	
//	//add the error
//	errorStrings->add(errorStrings, errDesc, NULL);
//	errorTokens->add(errorTokens, errToken, NULL);
//}

void parsingWillStart()
{
	//create the stacks if this is the first time we've needed them
	if(!variableStack)
		variableStack = antlr3StackNew(NoPL_StackSizeHint);
	
	//push the first scope
	pushScope();
}

void parsingDidFinish()
{
	//pop the last scope
	popScope();
}

int variableExists(pANTLR3_STRING varName)
{
	//search the stack for the given variable name
	for(int i = 0; i < variableStack->size(variableStack); i++)
	{
		pANTLR3_VECTOR vect = (pANTLR3_VECTOR)(variableStack->get(variableStack, i));
		for(int j = 0; j < vect->size(vect); j++)
		{
			pANTLR3_STRING str = (pANTLR3_STRING)(vect->get(vect,j));
			if(!strcmp((const char*)str->chars, (const char*)varName->chars))
				return 1;
		}
	}
	return 0;
}

void declareVariable(pANTLR3_STRING name)
{
	pANTLR3_VECTOR vect = (pANTLR3_VECTOR)(variableStack->peek(variableStack));
	vect->add(vect, name, NULL);
}

pANTLR3_STRING nextString(pNoPLParser ctx)
{
	pANTLR3_COMMON_TOKEN tok = LT(1);
	return tok->getText(tok);
}

}
//*/

//PARSER
program
	:	{ parsingWillStart(); } (statement|variableScope)+ EOF! { parsingDidFinish(); }
	;

variableScope
	:	SCOPE_OPEN^ { pushScope(); } statement+ { popScope(); } SCOPE_CLOSE!
	;

statement
	:	nonControlStatement STATEMENT_DELIMITER!
	|	controlFlowStatement
	|	BREAK STATEMENT_DELIMITER!
	|	CONTINUE STATEMENT_DELIMITER!
	|	EXIT STATEMENT_DELIMITER!
	;

nonControlStatement
	:	PRINT_VALUE^ expression
	|	(
			variableDeclaration |
			variableAssign |
			unaryOperation |
			objectExpression
		)
	;

controlFlowStatement
	:	whileLoop
	|	forLoop
	|	doWhileLoop
	|	conditional
	|	switchStatement
	;

expression
	:	booleanExpression
	|	numericExpression
	|	objectExpression
	|	atom
	;

booleanExpression
	:	booleanSubExpression ((LOGICAL_AND^ | LOGICAL_OR^) booleanSubExpression)?
	;

booleanSubExpression
	:	atom
		(
			LOGICAL_EQUALITY^ |
			LOGICAL_INEQUALITY^ |
			LESS_THAN^ |
			LESS_THAN_EQUAL^ |
			GREATER_THAN^ |
			GREATER_THAN_EQUAL^
		) expression
	;

expressionCast
	:	PAREN_OPEN DECL_BOOL PAREN_CLOSE expression -> ^(TYPE_CAST DECL_BOOL expression)
	|	PAREN_OPEN DECL_STRING PAREN_CLOSE expression -> ^(TYPE_CAST DECL_STRING expression)
	|	PAREN_OPEN DECL_NUMBER PAREN_CLOSE expression -> ^(TYPE_CAST DECL_NUMBER expression)
	;

atom
	:	LITERAL_TRUE
	|	LITERAL_FALSE
	|	LITERAL_NULL
	|	NUMBER
	|	STRING
	|	expressionCast
	|	SUBTRACT atom -> ^(NUMERIC_NEGATION atom)
	|	LOGICAL_NEGATION^ booleanExpression
	|	PAREN_OPEN! expression PAREN_CLOSE!
	|	ABS_VALUE^ expression ABS_VALUE!
	|	objectExpression
	;

//NUMBERS
numericExpression
	:	mdNumericExpression ((ADD^ | SUBTRACT^) numericExpression)?
	;

mdNumericExpression
	:	eNumericExpression ((MULTIPLY^ | DIVIDE^) mdNumericExpression)?
	;

eNumericExpression
	:	modNumericExpression (EXPONENT^ eNumericExpression)?
	;

modNumericExpression
	:	atom (MOD^ modNumericExpression)?
	;

//VARIABLES
variableDeclaration
	:	DECL_NUMBER^ { !variableExists(nextString(ctx)) }? => ID (ASSIGN! expression)? { declareVariable($ID.text); }
	|	DECL_BOOL^ { !variableExists(nextString(ctx)) }? => ID (ASSIGN! expression)? { declareVariable($ID.text); }
	|	DECL_STRING^ { !variableExists(nextString(ctx)) }? => ID (ASSIGN! expression)? { declareVariable($ID.text); }
	|	DECL_OBJ^ { !variableExists(nextString(ctx)) }? => ID (ASSIGN! expression)? { declareVariable($ID.text); }
	;

variableAssign
	:	{}{ variableExists(nextString(ctx)) }? => ID 
		(
			ASSIGN^ |
			ADD_ASSIGN^ |
			SUBTRACT_ASSIGN^ |
			MULTIPLY_ASSIGN^ |
			DIVIDE_ASSIGN^ |
			EXPONENT_ASSIGN^ |
			MOD_ASSIGN^
		) expression
	;

unaryOperation
	:	{ variableExists(nextString(ctx)) }? => ID (INCREMENT^|DECREMENT^)
	|	(INCREMENT^|DECREMENT^) { variableExists(nextString(ctx)) }? => ID
	;

//OBJECTS AND FUNCTIONS
objectExpression
	:	(objectAtom|functionCall)
		(
			OBJECT_TO_MEMBER^ (ID|functionCall) |
			SUBSCRIPT_OPEN^ expression SUBSCRIPT_CLOSE!
		)*
	;

objectAtom
	:	ID (SUBSCRIPT_OPEN^ expression SUBSCRIPT_CLOSE!)?
	;

functionArguments
	:	expression (ARG_DELIMITER expression)* -> ^(ARGUMENTS expression+)
	;

functionCall
	:	ID PAREN_OPEN functionArguments? PAREN_CLOSE -> ^(FUNCTION_CALL ID functionArguments?)
	;

//CONTROL FLOW
loopBody
	:	SCOPE_OPEN! statement* SCOPE_CLOSE!
	|	statement
	;

whileLoop
	:	LOOP_WHILE^ { pushScope(); } PAREN_OPEN! expression PAREN_CLOSE! loopBody { popScope(); }
	;

forLoop
	:	LOOP_FOR^ { pushScope(); } PAREN_OPEN! nonControlStatement STATEMENT_DELIMITER! expression STATEMENT_DELIMITER! nonControlStatement PAREN_CLOSE! loopBody { popScope(); }
	;

doWhileLoop
	:	LOOP_DO { pushScope(); } loopBody LOOP_WHILE PAREN_OPEN expression PAREN_CLOSE STATEMENT_DELIMITER { popScope(); } -> ^(LOOP_DO expression loopBody)
	;

conditional
	:	CONDITIONAL { pushScope(); } PAREN_OPEN expression PAREN_CLOSE loopBody { popScope(); } (elseConditional)? -> ^(CONDITIONAL expression (elseConditional)? loopBody)
	;

elseConditional
	:	CONDITIONAL_ELSE^ { pushScope(); } loopBody { popScope(); }
	;

innerSwitchStatement
	:	SWITCH_CASE^ (NUMBER|STRING|LITERAL_TRUE|LITERAL_FALSE) SWITCH_DELIMITER! statement*
	|	SWITCH_DEFAULT^ SWITCH_DELIMITER! statement*
	;

switchStatement
	:	SWITCH^ PAREN_OPEN! expression PAREN_CLOSE! SCOPE_OPEN! innerSwitchStatement+ SCOPE_CLOSE!
	;

//BOOLEAN OERATORS
LOGICAL_EQUALITY:	'==';
LOGICAL_INEQUALITY:	'!=';
LOGICAL_AND:		('&&'|'and');
LOGICAL_OR:		('||'|'or');
LOGICAL_NEGATION:	'!';
LESS_THAN:		'<';
LESS_THAN_EQUAL:	'<=';
GREATER_THAN:		'>';
GREATER_THAN_EQUAL:	'>=';
LITERAL_TRUE:		('true'|'YES');
LITERAL_FALSE:		('false'|'NO');

//MATHEMATIC OPERATORS
ADD:			'+';
ADD_ASSIGN:		'+=';
SUBTRACT:		'-';
SUBTRACT_ASSIGN:	'-=';
DIVIDE:			'/';
DIVIDE_ASSIGN:		'/=';
MULTIPLY:		'*';
MULTIPLY_ASSIGN:	'*=';
EXPONENT:		'^';
EXPONENT_ASSIGN:	'^=';
MOD:			'%';
MOD_ASSIGN:		'%=';
INCREMENT:		'++';
DECREMENT:		'--';
ABS_VALUE:		'|';

//TYPES
DECL_NUMBER:		(('n'|'N')'umber'|'int'|'float');
DECL_BOOL:		('BOOL'|('b'|'B')'ool'('ean')?);
DECL_STRING:		(('s'|'S')'tring');
DECL_OBJ:		(('o'|'O')'bject'|('p'|'P')'ointer');

//NULL
LITERAL_NULL:		('NULL'|'null'|'nil');

//FUNCTIONS
OBJECT_TO_MEMBER:	('.'|'->');
ARG_DELIMITER:		',';
SUBSCRIPT_OPEN:		'[';
SUBSCRIPT_CLOSE:	']';

//CONTROL FLOW
LOOP_WHILE:		'while';
LOOP_FOR:		'for';
LOOP_DO:		'do';
CONDITIONAL:		'if';
CONDITIONAL_ELSE:	'else';
SWITCH:			'switch';
SWITCH_CASE:		'case';
SWITCH_DELIMITER:	':';
SWITCH_DEFAULT:		'default';
BREAK:			'break';
CONTINUE:		'continue';
EXIT:			('exit'|'return');

//OTHER OPERATORS
PAREN_OPEN:		'(';
PAREN_CLOSE:		')';
SCOPE_OPEN:		'{';
SCOPE_CLOSE:		'}';
ASSIGN:			'=';
STATEMENT_DELIMITER:	';';
PRINT_VALUE:		'#';

//OTHER MISC LEXER
ID  :	('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
    ;
    
NUMBER
    :   ('0'..'9')+ '.' ('0'..'9')* NUMBER_EXPONENT? FLOAT_SPECIFIER?
    |   '.' ('0'..'9')+ NUMBER_EXPONENT? FLOAT_SPECIFIER?
    |   ('0'..'9')+ (NUMBER_EXPONENT FLOAT_SPECIFIER)?
    ;

COMMENT
    :   '//' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
    |   '/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
    ;

WS  :   ( ' '
        | '\t'
        | '\r'
        | '\n'
        ) {$channel=HIDDEN;}
    ;

STRING
    :  '"' ( ESC_SEQ | ~('\\'|'"') )* '"'
    |  '\'' ( ESC_SEQ | ~('\\'|'\'') )* '\''
    ;

fragment
NUMBER_EXPONENT : ('e'|'E') ('+'|'-')? ('0'..'9')+ ;

fragment
FLOAT_SPECIFIER	: 'f';

fragment
HEX_DIGIT : ('0'..'9'|'a'..'f'|'A'..'F') ;

fragment
ESC_SEQ
    :   '\\' ('b'|'t'|'n'|'f'|'r'|'\"'|'\''|'\\')
    |   UNICODE_ESC
    |   OCTAL_ESC
    ;

fragment
OCTAL_ESC
    :   '\\' ('0'..'3') ('0'..'7') ('0'..'7')
    |   '\\' ('0'..'7') ('0'..'7')
    |   '\\' ('0'..'7')
    ;

fragment
UNICODE_ESC
    :   '\\' 'u' HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT
    ;
