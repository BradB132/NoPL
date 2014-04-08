grammar NoPL_API;//NoPL is short for 'Non-Persistent Language'

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
	API_FUNCTIONS;
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
	:	API_DECL_CLASS API_ID (API_SUBSCRIPT_OPEN API_SUBSCRIPT_CLOSE)? API_SCOPE_OPEN functionDecl* API_SCOPE_CLOSE -> ^(API_DECL_CLASS API_ID ^(API_FUNCTIONS functionDecl*) API_SUBSCRIPT_OPEN?)
	;

functionDecl
	:	returnType API_ID^ API_PAREN_OPEN! (functionArg (API_ARG_DELIMITER! functionArg)*)? API_PAREN_CLOSE! API_STATEMENT_DELIMITER!
	;

functionArg
	:	(API_DECL_NUMBER^ | API_DECL_BOOL^ | API_DECL_STRING^ | API_DECL_OBJ^) API_ID
	|	API_ARG_VARIADIC^ API_ARG_VARIADIC! API_ARG_VARIADIC!
	;

returnType
	:	(API_DECL_NUMBER | API_DECL_BOOL | API_DECL_STRING | API_DECL_OBJ | API_DECL_VOID)
	;

//STRUCTURES
API_DECL_CLASS:		(('C'|'c')'lass');

//TYPES
API_DECL_NUMBER:	(('n'|'N')'umber'|'int'|'float');
API_DECL_BOOL:		('BOOL'|('b'|'B')'ool'('ean')?);
API_DECL_STRING:	(('s'|'S')'tring');
API_DECL_OBJ:		(('o'|'O')'bject'|('p'|'P')'ointer');
API_DECL_VOID:		(('v'|'V')'oid');

//FUNCTIONS
API_ARG_DELIMITER:		',';
API_ARG_VARIADIC:		'.';
API_SUBSCRIPT_OPEN:		'[';
API_SUBSCRIPT_CLOSE:		']';

//OTHER OPERATORS
API_PAREN_OPEN:			'(';
API_PAREN_CLOSE:		')';
API_SCOPE_OPEN:			'{';
API_SCOPE_CLOSE:		'}';
API_STATEMENT_DELIMITER:	';';

API_ID  :	('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
    ;

API_COMMENT
    :   '//' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
    |   '/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
    ;

API_WS  :   ( ' '
        | '\t'
        | '\r'
        | '\n'
        ) {$channel=HIDDEN;}
    ;
