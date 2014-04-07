grammar NoPL_API;//NoPL is short for 'Non-Persistent Language'

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

tokens
{
	FUNCTIONS;
}

//PARSER
api
	:	(statement)* EOF!
	;

statement
	:	classDecl
	|	functionDecl
	;

classDecl
	:	DECL_CLASS ID (SUBSCRIPT_OPEN SUBSCRIPT_CLOSE)? SCOPE_OPEN functionDecl* SCOPE_CLOSE -> ^(DECL_CLASS ID ^(FUNCTIONS functionDecl*) SUBSCRIPT_OPEN?)
	;

functionDecl
	:	returnType ID^ PAREN_OPEN! (functionArg (ARG_DELIMITER! functionArg)*)? PAREN_CLOSE! STATEMENT_DELIMITER!
	;

functionArg
	:	(DECL_NUMBER^ | DECL_BOOL^ | DECL_STRING^ | DECL_OBJ^) ID
	|	ARG_VARIADIC^ ARG_VARIADIC! ARG_VARIADIC!
	;

returnType
	:	(DECL_NUMBER | DECL_BOOL | DECL_STRING | DECL_OBJ | DECL_VOID)
	;

//STRUCTURES
DECL_CLASS:		(('C'|'c')'lass');

//TYPES
DECL_NUMBER:		(('n'|'N')'umber'|'int'|'float');
DECL_BOOL:		('BOOL'|('b'|'B')'ool'('ean')?);
DECL_STRING:		(('s'|'S')'tring');
DECL_OBJ:		(('o'|'O')'bject'|('p'|'P')'ointer');
DECL_VOID:		(('v'|'V')'oid');

//FUNCTIONS
ARG_DELIMITER:		',';
ARG_VARIADIC:		'.';
SUBSCRIPT_OPEN:		'[';
SUBSCRIPT_CLOSE:	']';

//OTHER OPERATORS
PAREN_OPEN:		'(';
PAREN_CLOSE:		')';
SCOPE_OPEN:		'{';
SCOPE_CLOSE:		'}';
STATEMENT_DELIMITER:	';';

ID  :	('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
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
