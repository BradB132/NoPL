grammar NoPL;//NoPL is short for 'Non-Persistent Language'

options
{
	backtrack=true;
	memoize=true;
	output=AST;
/*
	language = C;
	ASTLabelType=pANTLR3_BASE_TREE;
//*/
}

tokens
{
	TYPE_CAST;
	NUMERIC_NEGATION;
	ARGUMENTS;
	FUNCTION_CALL;
	FOR_LOOP_DECL;
	FOR_LOOP_COND;
	FOR_LOOP_ITER;
}

//PARSER
program
	:	(statement|variableScope)* EOF!
	;

variableScope
	:	SCOPE_OPEN^ statement+ SCOPE_CLOSE!
	;

statement
	:	nonControlStatement STATEMENT_DELIMITER!
	|	controlFlowStatement
	|	BREAK STATEMENT_DELIMITER!
	|	CONTINUE STATEMENT_DELIMITER!
	|	EXIT STATEMENT_DELIMITER!
	|	METADATA
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
	:	numericExpression
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
	|	PAREN_OPEN DECL_OBJ PAREN_CLOSE expression -> ^(TYPE_CAST DECL_OBJ expression)
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
	:	DECL_NUMBER^ ID (ASSIGN! expression)?
	|	DECL_BOOL^ ID (ASSIGN! expression)?
	|	DECL_STRING^ ID (ASSIGN! expression)?
	|	DECL_OBJ^ ID (ASSIGN! expression)?
	;

variableAssign
	:	ID 
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
	:	ID (INCREMENT^|DECREMENT^)
	|	(INCREMENT^|DECREMENT^) ID
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
	:	LOOP_WHILE^ PAREN_OPEN! expression PAREN_CLOSE! loopBody
	;

forLoop
	:	LOOP_FOR PAREN_OPEN decl=nonControlStatement? STATEMENT_DELIMITER cond=expression? STATEMENT_DELIMITER iter=nonControlStatement? PAREN_CLOSE loopBody
		-> ^(LOOP_FOR ^(FOR_LOOP_DECL $decl)? ^(FOR_LOOP_COND $cond)? ^(FOR_LOOP_ITER $iter)? loopBody)
	;

doWhileLoop
	:	LOOP_DO loopBody LOOP_WHILE PAREN_OPEN expression PAREN_CLOSE STATEMENT_DELIMITER -> ^(LOOP_DO expression loopBody)
	;

conditional
	:	CONDITIONAL PAREN_OPEN expression PAREN_CLOSE loopBody (elseConditional)? -> ^(CONDITIONAL expression (elseConditional)? loopBody)
	;

elseConditional
	:	CONDITIONAL_ELSE^ loopBody
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
METADATA:	'<?' ( options {greedy=false;} : . )* '?>';

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
