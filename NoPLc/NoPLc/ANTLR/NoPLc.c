//
//  NoPLc.c
//  NoPLc
//
//  Created by Brad Bambara on 4/15/12.
//  Copyright (c) 2012. All rights reserved.
//

#include <stdio.h>
#include "NoPLc.h"
#include "NoPLLexer.h"
#include "NoPLParser.h"

typedef struct
{
	NoPL_Index arrayLength;
	pANTLR3_STACK objectStack;
	pANTLR3_STACK numberStack;
	pANTLR3_STACK booleanStack;
	pANTLR3_STACK stringStack;
	int allowsBreakStatements;
	int allowsContinueStatements;
	pANTLR3_VECTOR breakStatements;
	pANTLR3_VECTOR continueStatements;
	int errDescLength;
	int debugLine;
	NoPL_Index* objectTableSize;
	NoPL_Index* numberTableSize;
	NoPL_Index* booleanTableSize;
	NoPL_Index* stringTableSize;
	ANTLR3_MARKER tokenStart;
	unsigned int tokenArrayLengths[NoPL_TokenRangeType_count];
	void* parentContext;
}NoPL_CompileContextPrivate;

#define treeIndex(tree,index)	(pANTLR3_BASE_TREE)(tree->children->get(tree->children, index))
#define isErrorNode(tree)		(!strcmp("Tree Error Node", (char*)tree->getText(tree)->chars))
#define pContext				((NoPL_CompileContextPrivate*)context->privateAttributes)
#define private(ctx)			((NoPL_CompileContextPrivate*)ctx.privateAttributes)
#define NoPL_StackSizeHint 16
#define NoPL_VectorSizeHint 8

//error codes
const char* NoPL_ErrStr_Generic = "Syntax error";
const char* NoPL_ErrStr_ExpressionMustBeNumeric = "This expression must evaluate to a numeric value";
const char* NoPL_ErrStr_ExpressionMustBeString = "This expression must evaluate to a string value";
const char* NoPL_ErrStr_ExpressionMustBeBoolean = "This expression must evaluate to a boolean value";
const char* NoPL_ErrStr_ExpressionMustBeObject = "This expression must evaluate to a object value";
const char* NoPL_ErrStr_VariableAlreadyDeclared = "A variable with this name was already declared";
const char* NoPL_ErrStr_VariableNotDeclared = "A variable with this name was never declared";
const char* NoPL_ErrStr_CannotIncrement = "Cannot use this increment operator on a variable of this type";
const char* NoPL_ErrStr_CouldNotDetermineType = "Could not determine the type of this expression";
const char* NoPL_ErrStr_CannotCastObjectToPrimitive = "Cannot cast a pointer to any non-string primitive type";
const char* NoPL_ErrStr_EqualityExpressionsAbiguous = "The type of both expressions being compared is ambiguous, at least one expression must be explicitly cast";
const char* NoPL_ErrStr_EqualityDifferentType = "Both expressions compared by this equality operator must evaluate to the same type";
const char* NoPL_ErrStr_CannotControlFlow = "Cannot use this control flow statement in this context";
const char* NoPL_ErrStr_DuplicateSwitchCase = "This case's value matches another case in this switch statement";
const char* NoPL_ErrStr_MisplacedDefault = "The default statement must be the last case in the switch statement";

//enum for checking AST node types
typedef enum
{
	NoPL_type_Number,
	NoPL_type_String,
	NoPL_type_Boolean,
	NoPL_type_Object,
	NoPL_type_FunctionResult,
	NoPL_type_Error,
} NoPL_DataType;

#pragma mark -
#pragma mark Internal functions

void nopl_compileWithInputStream(pANTLR3_INPUT_STREAM stream, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void nopl_addBytesToContext(const void* bytes, int byteCount, NoPL_CompileContext* context);
void nopl_addOperator(const NoPL_Instruction operator, NoPL_CompileContext* context);
void nopl_appendNodeWithRequiredType(const pANTLR3_BASE_TREE tree, const NoPL_DataType type, NoPL_CompileContext* context, const NoPL_CompileOptions* options);
void nopl_appendFunctionCall(const pANTLR3_BASE_TREE objExpression, const pANTLR3_BASE_TREE funcName, const pANTLR3_BASE_TREE args, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void nopl_traverseAST(const pANTLR3_BASE_TREE tree, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
NoPL_DataType nopl_dataTypeForTree(const pANTLR3_BASE_TREE tree, const NoPL_CompileContext* context);
void nopl_pushScope(NoPL_CompileContext* context);
void nopl_popScope(NoPL_CompileContext* context);
void nopl_error(const pANTLR3_BASE_TREE tree, const char* desc, NoPL_CompileContext* context);
int nopl_variableExistsInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack);
int nopl_variableExistsInContext(const pANTLR3_STRING varName, const NoPL_CompileContext* context);
int nopl_declareVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack);
NoPL_Index nopl_declareAnonymousVariableInStack(const pANTLR3_STACK whichStack);
NoPL_Index nopl_countVariablesInStack(const pANTLR3_STACK whichStack);
NoPL_Index nopl_indexOfVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack, const pANTLR3_BASE_TREE tree, NoPL_CompileContext* context);
NoPL_CompileContext nopl_newInnerCompileContext(NoPL_CompileContext* context, int allowBreak, int allowContinue);
void nopl_freeInnerCompileContext(NoPL_CompileContext* context);
void nopl_appendContext(const NoPL_CompileContext* fromContext, NoPL_CompileContext* toContext);
void nopl_finalizeControlFlowMoves(NoPL_CompileContext* context, NoPL_Index breakIndex, NoPL_Index continueIndex);
void nopl_appendControlFlowMove(NoPL_CompileContext* context, NoPL_Index moveFromIndex, NoPL_Index moveToIndex);
void nopl_findEndLineForNode(const pANTLR3_BASE_TREE tree, int* line);
void nopl_appendErrorString(NoPL_CompileContext* context, int startLine, int endLine, const char* errString);
void nopl_findEndPositionForNode(const pANTLR3_BASE_TREE tree, int* line);
void nopl_addTokenRange(NoPL_CompileContext* context, pANTLR3_COMMON_TOKEN token, NoPL_TokenRangeType type);
static void nopl_displayRecognitionError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8* tokenNames);

#pragma mark -
#pragma mark Error Handling

void nopl_appendErrorString(NoPL_CompileContext* context, int startLine, int endLine, const char* errString)
{
	int length = (int)strlen(errString)+50;
	char formattedString[length];
	if(startLine == endLine)
		snprintf(formattedString, length, "%s (line %d)\n", errString, startLine);
	else
		snprintf(formattedString, length, "%s (lines %d-%d)\n", errString, startLine, endLine);
	
	//get the root context
	while(pContext->parentContext)
		context = pContext->parentContext;
	
	//get the error length
	int errLength = (int)strlen(formattedString);
	
	//create the string if we don't already have one
	if(!context->errDescriptions)
	{
		context->errDescriptions = malloc(errLength*2);
		memcpy(context->errDescriptions, formattedString, errLength+1);
		pContext->errDescLength = errLength*2;
		return;
	}
	
	//check if we're too long for the buffer
	int currentLength = (int)strlen(context->errDescriptions)+1;
	if(errLength+currentLength > pContext->errDescLength)
	{
		//create a bigger buffer
		pContext->errDescLength = pContext->errDescLength*2+errLength;
		context->errDescriptions = realloc(context->errDescriptions, pContext->errDescLength);
	}
	
	//append
	strcat(context->errDescriptions, formattedString);
}

static void nopl_displayRecognitionError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8* tokenNames)
{
	//report when there has been a generic parse exception
	pANTLR3_EXCEPTION ex = recognizer->state->exception;
	nopl_appendErrorString((NoPL_CompileContext*)recognizer->state->custom, ex->line, ex->line, NoPL_ErrStr_Generic);
}

void nopl_findEndPositionForNode(const pANTLR3_BASE_TREE tree, int* line)
{
	//check if this position is after
	if(tree->getLine(tree) > *line)
		*line = tree->getLine(tree);
	
	//recursively check all nodes in this AST
	NoPL_Index childCount = 0;
	if(tree->children)
		childCount = (NoPL_Index)tree->children->size(tree->children);
	ANTLR3_UINT32 i;
	pANTLR3_BASE_TREE child;
	for(i = 0; i < childCount; i++)
	{
		//get each child and traverse
		child = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
		if(!isErrorNode(child))
			nopl_findEndPositionForNode(child, line);
	}
}

void nopl_error(const pANTLR3_BASE_TREE tree, const char* desc, NoPL_CompileContext* context)
{
	//attempt to find the end line
	int endLine;
	endLine = tree->getLine(tree);
	nopl_findEndPositionForNode(tree, &endLine);
	
	//append the error
	nopl_appendErrorString(context, tree->getLine(tree), endLine, desc);
}

#pragma mark -
#pragma mark Compilation

void nopl_addTokenRange(NoPL_CompileContext* context, pANTLR3_COMMON_TOKEN token, NoPL_TokenRangeType type)
{
	//get the ranges struct
	while(pContext->parentContext)
		context = pContext->parentContext;
	NoPL_TokenRanges* ranges = context->tokenRanges;
	
	//make sure our array is big enough
	if(!ranges->ranges[type])
	{
		pContext->tokenArrayLengths[type] = 64;
		ranges->ranges[type] = malloc(sizeof(NoPL_TokenRange)*pContext->tokenArrayLengths[type]);
	}
	else if(ranges->counts[type]+1 > pContext->tokenArrayLengths[type])
	{
		//double the size of the buffer
		pContext->tokenArrayLengths[type] *= 2;
		ranges->ranges[type] = realloc(ranges->ranges[type], sizeof(NoPL_TokenRange)*pContext->tokenArrayLengths[type]);
	}
	
	//set the start and end indices for this token
	NoPL_TokenRange* range = ranges->ranges[type];
	range += ranges->counts[type];
	range->startIndex = token->getStartIndex(token)-pContext->tokenStart;
	range->endIndex = (token->getStopIndex(token)+1)-pContext->tokenStart;
	
	//increment the count for how many ranges we have for this type
	ranges->counts[type]++;
}

void nopl_addBytesToContext(const void* bytes, int byteCount, NoPL_CompileContext* context)
{
	//check if the buffer size needs to be increased
	if(!pContext->arrayLength)
	{
		pContext->arrayLength = 128;
		context->compiledData = malloc(pContext->arrayLength);
	}
	else if(byteCount+context->dataLength > pContext->arrayLength)
	{
		//double the size of the buffer
		pContext->arrayLength = (pContext->arrayLength*2+byteCount);
		context->compiledData = realloc(context->compiledData, pContext->arrayLength);
	}
	
	//add the new bytes to the buffer
	memcpy((context->compiledData+context->dataLength), bytes, byteCount);
	
	//increment the data length
	context->dataLength += byteCount;
}

void nopl_addOperator(const NoPL_Instruction operator, NoPL_CompileContext* context)
{
	nopl_addBytesToContext(&operator, sizeof(NoPL_Instruction), context);
}

void nopl_pushScope(NoPL_CompileContext* context)
{
	//create some new vectors
	pANTLR3_VECTOR stringVector = antlr3VectorNew(NoPL_VectorSizeHint);
	pANTLR3_VECTOR boolVector = antlr3VectorNew(NoPL_VectorSizeHint);
	pANTLR3_VECTOR numberVector = antlr3VectorNew(NoPL_VectorSizeHint);
	pANTLR3_VECTOR objectVector = antlr3VectorNew(NoPL_VectorSizeHint);
	
	//push new vectors onto stack
	pContext->stringStack->push(pContext->stringStack, stringVector, (void(*)(void*))(stringVector->free));
	pContext->booleanStack->push(pContext->booleanStack, boolVector, (void(*)(void*))(boolVector->free));
	pContext->numberStack->push(pContext->numberStack, numberVector, (void(*)(void*))(numberVector->free));
	pContext->objectStack->push(pContext->objectStack, objectVector, (void(*)(void*))(objectVector->free));
}

void nopl_popScope(NoPL_CompileContext* context)
{
	//remove the stop vector from each stack
	pContext->stringStack->pop(pContext->stringStack);
	pContext->booleanStack->pop(pContext->booleanStack);
	pContext->numberStack->pop(pContext->numberStack);
	pContext->objectStack->pop(pContext->objectStack);
}

int nopl_variableExistsInStack(const pANTLR3_STRING varName, const  pANTLR3_STACK whichStack)
{
	//search the stack for the given variable name
	for(int i = 0; i < whichStack->size(whichStack); i++)
	{
		pANTLR3_VECTOR vect = (pANTLR3_VECTOR)(whichStack->get(whichStack, i));
		for(int j = 0; j < vect->size(vect); j++)
		{
			pANTLR3_STRING str = (pANTLR3_STRING)(vect->get(vect,j));
			if(!strcmp((const char*)str->chars, (const char*)varName->chars))
				return 1;
		}
	}
	return 0;
}

int nopl_variableExistsInContext(const pANTLR3_STRING varName, const NoPL_CompileContext* context)
{
	return (nopl_variableExistsInStack(varName, pContext->objectStack) ||
			nopl_variableExistsInStack(varName, pContext->numberStack) ||
			nopl_variableExistsInStack(varName, pContext->booleanStack) ||
			nopl_variableExistsInStack(varName, pContext->stringStack) );
}

int nopl_declareVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack)
{
	//fail if this variable is already declared in the stack
	if(nopl_variableExistsInStack(varName, whichStack))
		return 0;
	
	pANTLR3_VECTOR topVector = whichStack->peek(whichStack);
	topVector->add(topVector, varName, NULL);
	
	return 1;
}

NoPL_Index nopl_declareAnonymousVariableInStack(const pANTLR3_STACK whichStack)
{
	//count the index of this variable
	NoPL_Index index = 0;
	for(int i = 0; i < whichStack->size(whichStack); i++)
	{
		pANTLR3_VECTOR vect = (pANTLR3_VECTOR)(whichStack->get(whichStack, i));
		index += vect->size(vect);
	}
	
	//declare a variable with a NULL name, return variable index
	pANTLR3_VECTOR topVector = whichStack->peek(whichStack);
	topVector->add(topVector, NULL, NULL);
	
	return index;
}

NoPL_Index nopl_countVariablesInStack(const pANTLR3_STACK whichStack)
{
	NoPL_Index totalSize = 0;
	for(int i = 0; i < whichStack->size(whichStack); i++)
	{
		pANTLR3_VECTOR vect = (pANTLR3_VECTOR)(whichStack->get(whichStack, i));
		totalSize += vect->size(vect);
	}
	return totalSize;
}

NoPL_Index nopl_indexOfVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack, const pANTLR3_BASE_TREE tree, NoPL_CompileContext* context)
{
	NoPL_Index currentIndex = 0;
	for(int i = 0; i < whichStack->size(whichStack); i++)
	{
		pANTLR3_VECTOR vect = (pANTLR3_VECTOR)(whichStack->get(whichStack, i));
		for(int j = 0; j < vect->size(vect); j++)
		{
			pANTLR3_STRING str = (pANTLR3_STRING)(vect->get(vect,j));
			if(!strcmp((const char*)str->chars, (const char*)varName->chars))
				return currentIndex;
			
			currentIndex++;
		}
	}
	
	//we didn't find the variable
	nopl_error(tree, NoPL_ErrStr_VariableNotDeclared, context);
	return (NoPL_Index)0;
}

NoPL_CompileContext nopl_newInnerCompileContext(NoPL_CompileContext* context, int allowBreak, int allowContinue)
{
	NoPL_CompileContext newContext;
	newContext.privateAttributes = malloc(sizeof(NoPL_CompileContextPrivate));
	newContext.compiledData = NULL;
	newContext.errDescriptions = NULL;
	newContext.tokenRanges = NULL;
	newContext.dataLength = 0;
	private(newContext)->parentContext = context;
	private(newContext)->arrayLength = 0;
	private(newContext)->debugLine = pContext->debugLine;
	private(newContext)->objectStack = pContext->objectStack;
	private(newContext)->numberStack = pContext->numberStack;
	private(newContext)->booleanStack = pContext->booleanStack;
	private(newContext)->stringStack = pContext->stringStack;
	private(newContext)->errDescLength = 0;
	private(newContext)->objectTableSize = pContext->objectTableSize;
	private(newContext)->numberTableSize = pContext->numberTableSize;
	private(newContext)->booleanTableSize = pContext->booleanTableSize;
	private(newContext)->stringTableSize = pContext->stringTableSize;
	private(newContext)->allowsBreakStatements = (allowBreak || pContext->allowsBreakStatements);
	private(newContext)->allowsContinueStatements = (allowContinue || pContext->allowsContinueStatements);
	private(newContext)->tokenStart = 0;
	
	if(private(newContext)->allowsBreakStatements)
		private(newContext)->breakStatements = antlr3VectorNew(NoPL_VectorSizeHint);
	else
		private(newContext)->breakStatements = NULL;
	
	if(private(newContext)->allowsContinueStatements)
		private(newContext)->continueStatements = antlr3VectorNew(NoPL_VectorSizeHint);
	else
		private(newContext)->continueStatements = NULL;
	
	return newContext;
}

void nopl_freeInnerCompileContext(NoPL_CompileContext* context)
{
	if(pContext->breakStatements)
		pContext->breakStatements->free(pContext->breakStatements);
	if(pContext->continueStatements)
		pContext->continueStatements->free(pContext->continueStatements);
	if(context->privateAttributes)
	{
		free(context->privateAttributes);
		context->privateAttributes = NULL;
	}
	
	//free the any other stuff
	freeNoPL_CompileContext(context);
}

NoPL_DataType nopl_dataTypeForTree(const pANTLR3_BASE_TREE tree, const NoPL_CompileContext* context)
{
	switch(tree->getType(tree))
	{
		case ADD:
		{
			//the ADD operator can be used on either numbers or strings
			NoPL_DataType type1 = nopl_dataTypeForTree(treeIndex(tree,0), context);
			NoPL_DataType type2 = nopl_dataTypeForTree(treeIndex(tree,1), context);
			if(type1 == NoPL_type_String || type2 == NoPL_type_String)
				return NoPL_type_String;
			else
				return NoPL_type_Number;
		}
		case ABS_VALUE:
		case DIVIDE:
		case EXPONENT:
		case MOD:
		case MULTIPLY:
		case NUMBER:
		case NUMERIC_NEGATION:
		case SUBTRACT:
			return NoPL_type_Number;
		case GREATER_THAN:
		case GREATER_THAN_EQUAL:
		case LESS_THAN:
		case LESS_THAN_EQUAL:
		case LITERAL_FALSE:
		case LITERAL_TRUE:
		case LOGICAL_AND:
		case LOGICAL_EQUALITY:
		case LOGICAL_INEQUALITY:
		case LOGICAL_NEGATION:
		case LOGICAL_OR:
			return NoPL_type_Boolean;
		case LITERAL_NULL:
			return NoPL_type_Object;
		case STRING:
			return NoPL_type_String;
		case TYPE_CAST:
		{
			pANTLR3_BASE_TREE firstChild = treeIndex(tree,0);
			ANTLR3_UINT32 castType = firstChild->getType(firstChild);
			if(castType == DECL_NUMBER)
				return NoPL_type_Number;
			else if(castType == DECL_BOOL)
				return NoPL_type_Boolean;
			else if(castType == DECL_OBJ)
				return NoPL_type_Object;
			else if(castType == DECL_STRING)
				return NoPL_type_String;
			else
				return NoPL_type_Error;
		}
		case SWITCH_CASE:
			return nopl_dataTypeForTree(treeIndex(tree, 0), context);
		case ID:
		{
			//check the type of this symbol in the variable stacks
			pANTLR3_STRING varName = tree->getText(tree);
			if(nopl_variableExistsInStack(varName, pContext->objectStack))
			   return NoPL_type_Object;
			else if(nopl_variableExistsInStack(varName, pContext->numberStack))
				return NoPL_type_Number;
			else if(nopl_variableExistsInStack(varName, pContext->booleanStack))
				return NoPL_type_Boolean;
			else if(nopl_variableExistsInStack(varName, pContext->stringStack))
				return NoPL_type_String;
			else
				return NoPL_type_FunctionResult;
		}
		case FUNCTION_CALL:
		case OBJECT_TO_MEMBER:
		case SUBSCRIPT_OPEN:
			return NoPL_type_FunctionResult;
		default:
			return NoPL_type_Error;
	}
}

void nopl_appendNodeWithRequiredType(const pANTLR3_BASE_TREE tree, const NoPL_DataType type, NoPL_CompileContext* context, const NoPL_CompileOptions* options)
{
	//get the type of this node
	NoPL_DataType treeType = nopl_dataTypeForTree(tree, context);
	
	//check if this node is a function call
	if(treeType == NoPL_type_FunctionResult)
	{
		//do a cast if we need to
		switch(type)
		{
			case NoPL_type_Boolean:
				nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_BOOLEAN, context);
				break;
			case NoPL_type_Number:
				nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_NUMBER, context);
				break;
			case NoPL_type_String:
				nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
				break;
			case NoPL_type_Object:
				nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_OBJECT, context);
				break;
			default:
				break;
		}
	}
	else if(treeType != type)
	{
		//we have a type mismatch for this node, show an error corresponding to the type we needed
		const char* error = NULL;
		switch(type)
		{
			case NoPL_type_Boolean:
				error = NoPL_ErrStr_ExpressionMustBeBoolean;
				break;
			case NoPL_type_Number:
				error = NoPL_ErrStr_ExpressionMustBeNumeric;
				break;
			case NoPL_type_Object:
				error = NoPL_ErrStr_ExpressionMustBeObject;
				break;
			case NoPL_type_String:
				error = NoPL_ErrStr_ExpressionMustBeString;
				break;
			default:
				break;
		}
		if(error)
			nopl_error(tree, error, context);
	}
	
	//append the node
	nopl_traverseAST(tree, options, context);
}

void nopl_appendFunctionCall(const pANTLR3_BASE_TREE objExpression, const pANTLR3_BASE_TREE funcName, const pANTLR3_BASE_TREE args, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	if(options->createTokenRanges)
		nopl_addTokenRange(context, funcName->getToken(funcName), NoPL_TokenRangeType_functions);
	
	//add the function operator
	nopl_addOperator(NoPL_BYTE_FUNCTION_CALL, context);
	
	//append the object
	if(objExpression)
		nopl_appendNodeWithRequiredType(objExpression, NoPL_type_Object, context, options);
	else
		nopl_addOperator(NoPL_BYTE_LITERAL_NULL, context);
	
	//get function name and name length
	char* functionName = (char*)funcName->getText(funcName)->chars;
	int functionNameLength = (int)strlen(functionName);
	
	//add the function name
	nopl_addBytesToContext(functionName, sizeof(char)*functionNameLength+1, context);
	
	if(args)
	{
		//get the arg count
		NoPL_Index argCount = 0;
		if(args->children)
			argCount = (NoPL_Index)args->getChildCount(args);
		
		//append bytes for count
		nopl_addBytesToContext(&argCount, sizeof(NoPL_Index), context);
		
		//loop through all children to add args
		ANTLR3_UINT32 i;
		pANTLR3_BASE_TREE childArg;
		for(i = 0; i < argCount; i++)
		{
			//get the type for each arg
			childArg = (pANTLR3_BASE_TREE)(args->children->get(args->children, i));
			NoPL_DataType argType = nopl_dataTypeForTree(childArg, context);
			
			if(argType == NoPL_type_Number)
				nopl_addOperator(NoPL_BYTE_ARG_NUMBER, context);
			else if(argType == NoPL_type_String)
				nopl_addOperator(NoPL_BYTE_ARG_STRING, context);
			else if(argType == NoPL_type_Boolean)
				nopl_addOperator(NoPL_BYTE_ARG_BOOLEAN, context);
			else if(argType == NoPL_type_Object)
				nopl_addOperator(NoPL_BYTE_ARG_OBJECT, context);
			else if(argType == NoPL_type_FunctionResult)
				nopl_error(childArg, "The type of this argument is ambiguous and requires an explicit cast", context);
			else
				nopl_error(childArg, "Cannot determine the type of this argument", context);
			
			//append the arg
			nopl_traverseAST(childArg, options, context);
		}
	}
	else
	{
		//this is a function call has specified no arguments
		NoPL_Index zero = 0;
		nopl_addBytesToContext(&zero, sizeof(NoPL_Index), context);
	}
}

void nopl_appendControlFlowMove(NoPL_CompileContext* context, NoPL_Index moveFromIndex, NoPL_Index moveToIndex)
{
	//calculate the move amount
	NoPL_Index copyToIndex = moveFromIndex-sizeof(NoPL_Index);
	NoPL_Index instructionIndex = copyToIndex-sizeof(NoPL_Instruction);
	int moveAmount = ((int)moveToIndex) - ((int)moveFromIndex);
	
	//set the operator based on if we're going forward or backwards
	NoPL_Instruction instr;
	if(moveAmount < 0)
		instr = NoPL_BYTE_BUFFER_MOVE_BACKWARD;
	else
		instr = NoPL_BYTE_BUFFER_MOVE_FORWARD;
		
	//set the instruction value in the compiled data
	memcpy((context->compiledData+instructionIndex), &instr, sizeof(NoPL_Instruction));
	
	//set the correct value in the compiled data for this new amount
	NoPL_Index moveAmt = (NoPL_Index)abs(moveAmount);
	memcpy((context->compiledData+copyToIndex), &moveAmt, sizeof(NoPL_Index));
}

void nopl_finalizeControlFlowMoves(NoPL_CompileContext* context, NoPL_Index breakIndex, NoPL_Index continueIndex)
{
	//check for breaks
	if(pContext->breakStatements)
	{
		for(int i = 0; i < pContext->breakStatements->size(pContext->breakStatements); i++)
		{
			NoPL_Index moveFromIndex = (NoPL_Index)(pContext->breakStatements->get(pContext->breakStatements,i));
			nopl_appendControlFlowMove(context, moveFromIndex, breakIndex);
		}
		
		pContext->breakStatements->free(pContext->breakStatements);
		pContext->breakStatements = NULL;
	}
	
	//check for continues
	if(pContext->continueStatements)
	{
		for(int i = 0; i < pContext->continueStatements->size(pContext->continueStatements); i++)
		{
			//calculate the move amount
			NoPL_Index moveFromIndex = (NoPL_Index)(pContext->continueStatements->get(pContext->continueStatements,i));
			nopl_appendControlFlowMove(context, moveFromIndex, continueIndex);
		}
		
		pContext->continueStatements->free(pContext->continueStatements);
		pContext->continueStatements = NULL;
	}
}

void nopl_appendContext(const NoPL_CompileContext* fromContext, NoPL_CompileContext* toContext)
{
	//check for breaks
	if(((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->breakStatements)
	{
		int size = ((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->breakStatements->size(((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->breakStatements);
		if(!((NoPL_CompileContextPrivate*)toContext->privateAttributes)->breakStatements)
			((NoPL_CompileContextPrivate*)toContext->privateAttributes)->breakStatements = antlr3VectorNew(size);
		
		for(int i = 0; i < size; i++)
		{
			//get the index and adjust by the amount of data in the new context
			NoPL_Index index = (NoPL_Index)(((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->breakStatements->get(((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->breakStatements,i));
			index += toContext->dataLength;
			
			//add the adjusted number to the new context
			((NoPL_CompileContextPrivate*)toContext->privateAttributes)->breakStatements->add(((NoPL_CompileContextPrivate*)toContext->privateAttributes)->breakStatements, (void*)index, NULL);
		}
	}
	
	//check for continues
	if(((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->continueStatements)
	{
		int size = ((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->continueStatements->size(((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->continueStatements);
		if(!((NoPL_CompileContextPrivate*)toContext->privateAttributes)->continueStatements)
			((NoPL_CompileContextPrivate*)toContext->privateAttributes)->continueStatements = antlr3VectorNew(size);
		
		for(int i = 0; i < size; i++)
		{
			//calculate the move amount
			NoPL_Index index = (NoPL_Index)(((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->continueStatements->get(((NoPL_CompileContextPrivate*)fromContext->privateAttributes)->continueStatements,i));
			index += toContext->dataLength;
			
			//add the adjusted number to the new context
			((NoPL_CompileContextPrivate*)toContext->privateAttributes)->continueStatements->add(((NoPL_CompileContextPrivate*)toContext->privateAttributes)->continueStatements, (void*)index, NULL);
		}
	}
	
	//add all bytes from the original context
	nopl_addBytesToContext(fromContext->compiledData, fromContext->dataLength, toContext);
}

void nopl_traverseAST(const pANTLR3_BASE_TREE tree, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//avoid bad acesses
	if(!tree)
		return;
	
	if(tree->isNilNode(tree))
	{
		//this is a nil node, most likely the root node, traverse all children
		NoPL_Index childCount = 0;
		if(tree->children)
			childCount = (NoPL_Index)tree->children->size(tree->children);
		ANTLR3_UINT32 i;
		pANTLR3_BASE_TREE child;
		for(i = 0; i < childCount; i++)
		{
			//get each child and append
			child = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
			nopl_traverseAST(child, options, context);
		}
	}
	else
	{
		//check if this is a new line for debug
		if(options->debugSymbols && (int)tree->getLine(tree) != pContext->debugLine)
		{
			//get the debug line
			pContext->debugLine = (int)tree->getLine(tree);
			
			//add the debug line
			nopl_addOperator(NoPL_BYTE_DEBUG_LINE, context);
			NoPL_Index lineNum = (NoPL_Index)pContext->debugLine;
			nopl_addBytesToContext(&lineNum, sizeof(NoPL_Index), context);
		}
		
		//append to the script buffer based on the type of this AST node
		switch(tree->getType(tree))
		{
			case ABS_VALUE:
			{
				//this operator should have one numeric child
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_ABS_VALUE, context);
				
				//append the expression
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
			}
				break;
			case ADD:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//the ADD operator can be used for numbers or concatenating strings
				NoPL_DataType dataType = nopl_dataTypeForTree(tree, context);
				if(dataType == NoPL_type_Number)
				{
					//add the operator
					nopl_addOperator(NoPL_BYTE_NUMERIC_ADD, context);
					
					//append the expressions
					nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
					nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
				}
				else if(dataType == NoPL_type_String)
				{
					//add the operator
					nopl_addOperator(NoPL_BYTE_STRING_CONCAT, context);
					
					//get the types for the children
					NoPL_DataType childType1 = nopl_dataTypeForTree(child1, context);
					NoPL_DataType childType2 = nopl_dataTypeForTree(child2, context);
					
					//cast the first child if we need to
					if(childType1 == NoPL_type_Number)
						nopl_addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
					else if(childType1 == NoPL_type_Boolean)
						nopl_addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
					else if(childType1 == NoPL_type_FunctionResult)
						nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
					else if(childType1 == NoPL_type_Object)
						nopl_addOperator(NoPL_BYTE_CAST_OBJECT_TO_STRING, context);
					
					//add the first child
					nopl_traverseAST(child1, options, context);
					
					//cast the second child if we need to
					if(childType2 == NoPL_type_Number)
						nopl_addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
					else if(childType2 == NoPL_type_Boolean)
						nopl_addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
					else if(childType2 == NoPL_type_FunctionResult)
						nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
					else if(childType2 == NoPL_type_Object)
						nopl_addOperator(NoPL_BYTE_CAST_OBJECT_TO_STRING, context);
					
					//add the second child
					nopl_traverseAST(child2, options, context);
				}
				else
				{
					nopl_error(tree, "something is wrong with the + operator", context);
				}
			}
				break;
			case ADD_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, assignTo->getToken(assignTo), NoPL_TokenRangeType_variables);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = nopl_dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//use the numeric increment
					nopl_addOperator(NoPL_BYTE_NUMERIC_ADD_ASSIGN, context);
					
					//add the index for the variable which will be incremented
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->numberStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					nopl_appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
				}
				else if(assignToType == NoPL_type_String)
				{
					//use the string concat
					nopl_addOperator(NoPL_BYTE_STRING_CONCAT_ASSIGN, context);
					
					//add the index for the variable which will be incremented
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->stringStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					nopl_appendNodeWithRequiredType(incrementVal, NoPL_type_String, context, options);
				}
				else
				{
					if(nopl_variableExistsInContext(assignTo->getText(assignTo), context))
						nopl_error(assignTo, NoPL_ErrStr_CannotIncrement, context);
					else
						nopl_error(assignTo, NoPL_ErrStr_VariableNotDeclared, context);
				}
			}
				break;
			case ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE expression = treeIndex(tree,1);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, assignTo->getToken(assignTo), NoPL_TokenRangeType_variables);
				
				//check the object on the left-hand side to what type should be assigned
				NoPL_DataType assignToType = nopl_dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//use the numeric assign
					nopl_addOperator(NoPL_BYTE_NUMERIC_ASSIGN, context);
					
					//add the index for the variable which will be assigned to
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->numberStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					nopl_appendNodeWithRequiredType(expression, NoPL_type_Number, context, options);
				}
				else if(assignToType == NoPL_type_String)
				{
					//use the string assign
					nopl_addOperator(NoPL_BYTE_STRING_ASSIGN, context);
					
					//add the index for the variable which will be assigned to
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->stringStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					nopl_appendNodeWithRequiredType(expression, NoPL_type_String, context, options);
				}
				else if(assignToType == NoPL_type_Boolean)
				{
					//use the boolean assign
					nopl_addOperator(NoPL_BYTE_BOOLEAN_ASSIGN, context);
					
					//add the index for the variable which will be assigned to
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->booleanStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					nopl_appendNodeWithRequiredType(expression, NoPL_type_Boolean, context, options);
				}
				else if(assignToType == NoPL_type_Object)
				{
					//use the object assign
					nopl_addOperator(NoPL_BYTE_OBJECT_ASSIGN, context);
					
					//add the index for the variable which will be assigned to
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->objectStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					nopl_appendNodeWithRequiredType(expression, NoPL_type_Object, context, options);
				}
				else
				{
					nopl_error(assignTo, NoPL_ErrStr_VariableNotDeclared, context);
				}
			}
				break;
			case BREAK:
			{
				//append the a placeholder for the operator
				nopl_addOperator(0, context);
				
				//append an empty placeholder for the move
				NoPL_Index move = 0;
				nopl_addBytesToContext(&move, sizeof(NoPL_Index), context);
				
				//check if our current context even supports this
				if(pContext->allowsBreakStatements)
				{
					//keep a list of the positions of all break statements in the buffer
					pContext->breakStatements->add(pContext->breakStatements, (void*)(context->dataLength), NULL);
				}
				else
					nopl_error(tree, NoPL_ErrStr_CannotControlFlow, context);
			}
				break;
			case CONDITIONAL:
			{
				//get the boolean for the conditional, and the first child to check if there is a corresponding 'else' statement
				pANTLR3_BASE_TREE condition = treeIndex(tree,0);
				pANTLR3_BASE_TREE firstStatement = treeIndex(tree,1);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_CONDITIONAL, context);
				
				//append the boolean expression for the conditional
				nopl_appendNodeWithRequiredType(condition, NoPL_type_Boolean, context, options);
				
				//handle the case that we have an empty if statement
				if(!firstStatement)
				{
					NoPL_Index zero = 0;
					nopl_addBytesToContext(&zero, sizeof(NoPL_Index), context);
					return;
				}
				
				//check if the first statement node is an else statement
				int hasElse = firstStatement->getType(firstStatement) == CONDITIONAL_ELSE;
				
				//get another byte buffer for everything inside this conditional
				NoPL_CompileContext innerConditional = nopl_newInnerCompileContext(context, 0, 0);
				nopl_pushScope(&innerConditional);
				
				//add all of the statements inside the conditional
				NoPL_Index childCount = 0;
				if(tree->children)
					childCount = (NoPL_Index)tree->children->size(tree->children);
				ANTLR3_UINT32 startIndex = hasElse ? 2 : 1;
				ANTLR3_UINT32 i;
				pANTLR3_BASE_TREE childArg;
				for(i = startIndex; i < childCount; i++)
				{
					childArg = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
					nopl_traverseAST(childArg, options, &innerConditional);
				}
				
				//calc how many bytes to skip if the conditional is false
				NoPL_Index move = (NoPL_Index)innerConditional.dataLength;
				if(hasElse)
				{
					//else conditionals contain an extra buffer move at the beginning of the else statement
					move += sizeof(NoPL_Instruction)+sizeof(NoPL_Index);
				}
				nopl_addBytesToContext(&move, sizeof(NoPL_Index), context);
				
				//add the inner conditional
				nopl_appendContext(&innerConditional, context);
				
				//clean up the inner conditional
				nopl_popScope(&innerConditional);
				nopl_freeInnerCompileContext(&innerConditional);
				
				//add the else statement, if we have one
				if(hasElse)
				{
					//get another byte buffer for everything inside this else statement
					NoPL_CompileContext innerElse = nopl_newInnerCompileContext(context, 0, 0);
					nopl_pushScope(&innerElse);
					
					//add all of the statements inside the else statement
					childCount = 0;
					if(firstStatement->children)
						childCount = (NoPL_Index)firstStatement->children->size(firstStatement->children);
					ANTLR3_UINT32 i;
					for(i = 0; i < childCount; i++)
					{
						childArg = (pANTLR3_BASE_TREE)(firstStatement->children->get(firstStatement->children, i));
						nopl_traverseAST(childArg, options, &innerElse);
					}
					
					//skip over all this else stuff if we're coming from the block of code for true
					nopl_addOperator(NoPL_BYTE_BUFFER_MOVE_FORWARD, context);
					move = (NoPL_Index)innerElse.dataLength;
					nopl_addBytesToContext(&move, sizeof(NoPL_Index), context);
					
					//add the else statement
					nopl_appendContext(&innerElse, context);
					
					//clean up the else statement
					nopl_popScope(&innerElse);
					nopl_freeInnerCompileContext(&innerElse);
				}
			}
				break;
			case CONTINUE:
			{
				//append the a placeholder for the operator
				nopl_addOperator(0, context);
				
				//append an empty placeholder for the move
				NoPL_Index move = 0;
				nopl_addBytesToContext(&move, sizeof(NoPL_Index), context);
				
				//check if our current context even supports this
				if(pContext->allowsContinueStatements)
				{
					//keep a list of the positions of all continue statements in the buffer
					pContext->continueStatements->add(pContext->continueStatements, (void*)(context->dataLength), NULL);
				}
				else
					nopl_error(tree, NoPL_ErrStr_CannotControlFlow, context);
			}
				break;
			case DECL_BOOL:
			{
				//get the name of the declared variable
				pANTLR3_BASE_TREE declaredVar = treeIndex(tree,0);
				pANTLR3_STRING declaredName = declaredVar->getText(declaredVar);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, declaredVar->getToken(declaredVar), NoPL_TokenRangeType_variables);
				
				//we need to evaluate the initial expression before declaring the variable
				NoPL_CompileContext initCtx = nopl_newInnerCompileContext(context, 0, 0);
				
				//get the initial value if there is one
				if(tree->getChildCount(tree) > 1)
				{
					//append the expression
					nopl_appendNodeWithRequiredType(treeIndex(tree,1), NoPL_type_Boolean, &initCtx, options);
				}
				else
				{
					//set this variable to a default value
					nopl_addOperator(NoPL_BYTE_LITERAL_BOOLEAN_FALSE, &initCtx);
				}
				
				//check if the variable was already declared elsewhere
				if(nopl_variableExistsInContext(declaredName, context))
					nopl_error(declaredVar, NoPL_ErrStr_VariableAlreadyDeclared, context);
				
				//declare the variable
				nopl_declareVariableInStack(declaredName, pContext->booleanStack);
				NoPL_Index newCount = nopl_countVariablesInStack(pContext->booleanStack);
				if(newCount > *(pContext->booleanTableSize))
					*(pContext->booleanTableSize) = newCount;
				
				//assign to the index of the newly created variable
				nopl_addOperator(NoPL_BYTE_BOOLEAN_ASSIGN, context);
				NoPL_Index index = nopl_indexOfVariableInStack(declaredName, pContext->booleanStack, declaredVar, context);
				nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//append the initialization
				nopl_appendContext(&initCtx, context);
				nopl_freeInnerCompileContext(&initCtx);
				
				//add the variable's name if we want debug symbols
				if(options->debugSymbols)
				{
					nopl_addOperator(NoPL_BYTE_DEBUG_VALUE_BOOLEAN, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					int varNameLength = (int)strlen((char*)declaredName->chars);
					nopl_addBytesToContext(declaredName->chars, varNameLength+1, context);
				}
			}
				break;
			case DECL_NUMBER:
			{
				//get the name of the declared variable
				pANTLR3_BASE_TREE declaredVar = treeIndex(tree,0);
				pANTLR3_STRING declaredName = declaredVar->getText(declaredVar);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, declaredVar->getToken(declaredVar), NoPL_TokenRangeType_variables);
				
				//we need to evaluate the initial expression before declaring the variable
				NoPL_CompileContext initCtx = nopl_newInnerCompileContext(context, 0, 0);
				
				//get the initial value if there is one
				if(tree->getChildCount(tree) > 1)
				{
					//append the expression
					nopl_appendNodeWithRequiredType(treeIndex(tree,1), NoPL_type_Number, &initCtx, options);
				}
				else
				{
					//set this variable to a default value
					nopl_addOperator(NoPL_BYTE_LITERAL_NUMBER, &initCtx);
					float defaultFloat = 0.0f;
					nopl_addBytesToContext(&defaultFloat, sizeof(float), &initCtx);
				}
				
				//check if the variable was already declared elsewhere
				if(nopl_variableExistsInContext(declaredName, context))
					nopl_error(declaredVar, NoPL_ErrStr_VariableAlreadyDeclared, context);
				
				//declare the variable
				nopl_declareVariableInStack(declaredName, pContext->numberStack);
				NoPL_Index newCount = nopl_countVariablesInStack(pContext->numberStack);
				if(newCount > *(pContext->numberTableSize))
					*(pContext->numberTableSize) = newCount;
				
				//assign to the index of the newly created variable
				nopl_addOperator(NoPL_BYTE_NUMERIC_ASSIGN, context);
				NoPL_Index index = nopl_indexOfVariableInStack(declaredName, pContext->numberStack, declaredVar, context);
				nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//append the initialization
				nopl_appendContext(&initCtx, context);
				nopl_freeInnerCompileContext(&initCtx);
				
				//add the variable's name if we want debug symbols
				if(options->debugSymbols)
				{
					nopl_addOperator(NoPL_BYTE_DEBUG_VALUE_NUMBER, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					int varNameLength = (int)strlen((char*)declaredName->chars);
					nopl_addBytesToContext(declaredName->chars, varNameLength+1, context);
				}
			}
				break;
			case DECL_OBJ:
			{
				//get the name of the declared variable
				pANTLR3_BASE_TREE declaredVar = treeIndex(tree,0);
				pANTLR3_STRING declaredName = declaredVar->getText(declaredVar);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, declaredVar->getToken(declaredVar), NoPL_TokenRangeType_variables);
				
				//we need to evaluate the initial expression before declaring the variable
				NoPL_CompileContext initCtx = nopl_newInnerCompileContext(context, 0, 0);
				
				//get the initial value if there is one
				if(tree->getChildCount(tree) > 1)
				{
					//append the expression
					nopl_appendNodeWithRequiredType(treeIndex(tree,1), NoPL_type_Object, &initCtx, options);
				}
				else
				{
					//set this variable to a default value
					nopl_addOperator(NoPL_BYTE_LITERAL_NULL, &initCtx);
				}
				
				//check if the variable was already declared elsewhere
				if(nopl_variableExistsInContext(declaredName, context))
					nopl_error(declaredVar, NoPL_ErrStr_VariableAlreadyDeclared, context);
				
				//declare the variable
				nopl_declareVariableInStack(declaredName, pContext->objectStack);
				NoPL_Index newCount = nopl_countVariablesInStack(pContext->objectStack);
				if(newCount > *(pContext->objectTableSize))
					*(pContext->objectTableSize) = newCount;
				
				//assign to the index of the newly created variable
				nopl_addOperator(NoPL_BYTE_OBJECT_ASSIGN, context);
				NoPL_Index index = nopl_indexOfVariableInStack(declaredName, pContext->objectStack, declaredVar, context);
				nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//append the initialization
				nopl_appendContext(&initCtx, context);
				nopl_freeInnerCompileContext(&initCtx);
				
				//add the variable's name if we want debug symbols
				if(options->debugSymbols)
				{
					nopl_addOperator(NoPL_BYTE_DEBUG_VALUE_OBJECT, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					int varNameLength = (int)strlen((char*)declaredName->chars);
					nopl_addBytesToContext(declaredName->chars, varNameLength+1, context);
				}
			}
				break;
			case DECL_STRING:
			{
				//get the name of the declared variable
				pANTLR3_BASE_TREE declaredVar = treeIndex(tree,0);
				pANTLR3_STRING declaredName = declaredVar->getText(declaredVar);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, declaredVar->getToken(declaredVar), NoPL_TokenRangeType_variables);
				
				//we need to evaluate the initial expression before declaring the variable
				NoPL_CompileContext initCtx = nopl_newInnerCompileContext(context, 0, 0);
				
				//get the initial value if there is one
				if(tree->getChildCount(tree) > 1)
				{
					//append the expression
					nopl_appendNodeWithRequiredType(treeIndex(tree,1), NoPL_type_String, &initCtx, options);
				}
				else
				{
					//set this variable to a default value
					nopl_addOperator(NoPL_BYTE_LITERAL_STRING, &initCtx);
					char* defaultString = "";
					nopl_addBytesToContext(&defaultString[0], sizeof(defaultString[0]), &initCtx);
				}
				
				//check if the variable was already declared elsewhere
				if(nopl_variableExistsInContext(declaredName, context))
					nopl_error(declaredVar, NoPL_ErrStr_VariableAlreadyDeclared, context);
				
				//declare the variable
				nopl_declareVariableInStack(declaredName, pContext->stringStack);
				NoPL_Index newCount = nopl_countVariablesInStack(pContext->stringStack);
				if(newCount > *(pContext->stringTableSize))
					*(pContext->stringTableSize) = newCount;
				
				//assign to the index of the newly created variable
				nopl_addOperator(NoPL_BYTE_STRING_ASSIGN, context);
				NoPL_Index index = nopl_indexOfVariableInStack(declaredName, pContext->stringStack, declaredVar, context);
				nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//append the initialization
				nopl_appendContext(&initCtx, context);
				nopl_freeInnerCompileContext(&initCtx);
				
				//add the variable's name if we want debug symbols
				if(options->debugSymbols)
				{
					nopl_addOperator(NoPL_BYTE_DEBUG_VALUE_STRING, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					int varNameLength = (int)strlen((char*)declaredName->chars);
					nopl_addBytesToContext(declaredName->chars, varNameLength+1, context);
				}
			}
				break;
			case DECREMENT:
			{
				//add the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_DECREMENT, context);
				
				//get the variable being changed
				pANTLR3_BASE_TREE var = treeIndex(tree,0);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, var->getToken(var), NoPL_TokenRangeType_variables);
				
				//check the data type of the variable
				if(nopl_dataTypeForTree(var, context) != NoPL_type_Number)
					nopl_error(var, NoPL_ErrStr_CannotIncrement, context);
				
				//append the index for the variable
				NoPL_Index index = nopl_indexOfVariableInStack(var->getText(var), pContext->numberStack, var, context);
				nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
			}
				break;
			case DIVIDE:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_DIVIDE, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case DIVIDE_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, assignTo->getToken(assignTo), NoPL_TokenRangeType_variables);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = nopl_dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					nopl_addOperator(NoPL_BYTE_NUMERIC_DIVIDE_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->numberStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					nopl_appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
				}
				else
				{
					if(nopl_variableExistsInContext(assignTo->getText(assignTo), context))
						nopl_error(assignTo, NoPL_ErrStr_CannotIncrement, context);
					else
						nopl_error(assignTo, NoPL_ErrStr_VariableNotDeclared, context);
				}
			}
				break;
			case EXIT:
				nopl_addOperator(NoPL_BYTE_PROGRAM_EXIT, context);
				break;
			case EXPONENT:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_EXPONENT, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case EXPONENT_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, assignTo->getToken(assignTo), NoPL_TokenRangeType_variables);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = nopl_dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					nopl_addOperator(NoPL_BYTE_NUMERIC_EXPONENT_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->numberStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					nopl_appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
				}
				else
				{
					if(nopl_variableExistsInContext(assignTo->getText(assignTo), context))
						nopl_error(assignTo, NoPL_ErrStr_CannotIncrement, context);
					else
						nopl_error(assignTo, NoPL_ErrStr_VariableNotDeclared, context);
				}
			}
				break;
			case FUNCTION_CALL:
			{
				//check if we have args
				pANTLR3_BASE_TREE args = NULL;
				if(tree->getChildCount(tree) > 1)
					args = treeIndex(tree, 1);
				
				//append the function call
				nopl_appendFunctionCall(NULL, treeIndex(tree, 0), args, options, context);
			}
				break;
			case GREATER_THAN:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_BOOLEAN_GREATER_THAN, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case GREATER_THAN_EQUAL:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_BOOLEAN_GREATER_THAN_EQUAL, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case ID:
			{
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges && nopl_variableExistsInContext(tree->getText(tree), context))
					nopl_addTokenRange(context, tree->getToken(tree), NoPL_TokenRangeType_variables);
				
				//check the type of the ID
				switch(nopl_dataTypeForTree(tree, context))
				{
					case NoPL_type_Boolean:
					{
						//append the operator
						nopl_addOperator(NoPL_BYTE_VARIABLE_BOOLEAN, context);
						
						//append the index of the variable
						NoPL_Index varIndex = nopl_indexOfVariableInStack(tree->getText(tree), pContext->booleanStack, tree, context);
						nopl_addBytesToContext(&varIndex, sizeof(NoPL_Index), context);
					}
						break;
					case NoPL_type_Number:
					{
						//append the operator
						nopl_addOperator(NoPL_BYTE_VARIABLE_NUMBER, context);
						
						//append the index of the variable
						NoPL_Index varIndex = nopl_indexOfVariableInStack(tree->getText(tree), pContext->numberStack, tree, context);
						nopl_addBytesToContext(&varIndex, sizeof(NoPL_Index), context);
					}
						break;
					case NoPL_type_Object:
					{
						//append the operator
						nopl_addOperator(NoPL_BYTE_VARIABLE_OBJECT, context);
						
						//append the index of the variable
						NoPL_Index varIndex = nopl_indexOfVariableInStack(tree->getText(tree), pContext->objectStack, tree, context);
						nopl_addBytesToContext(&varIndex, sizeof(NoPL_Index), context);
					}
						break;
					case NoPL_type_String:
					{
						//append the operator
						nopl_addOperator(NoPL_BYTE_VARIABLE_STRING, context);
						
						//append the index of the variable
						NoPL_Index varIndex = nopl_indexOfVariableInStack(tree->getText(tree), pContext->stringStack, tree, context);
						nopl_addBytesToContext(&varIndex, sizeof(NoPL_Index), context);
					}
						break;
					default:
					{
						//this is a global function without args
						nopl_appendFunctionCall(NULL, tree, NULL, options, context);
					}
						break;
				}
			}
				break;
			case INCREMENT:
			{
				//add the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_INCREMENT, context);
				
				//get the variable being changed
				pANTLR3_BASE_TREE var = treeIndex(tree,0);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, var->getToken(var), NoPL_TokenRangeType_variables);
				
				//check the data type of the variable
				if(nopl_dataTypeForTree(var, context) != NoPL_type_Number)
					nopl_error(var, NoPL_ErrStr_CannotIncrement, context);
				
				//append the index for the variable
				NoPL_Index index = nopl_indexOfVariableInStack(var->getText(var), pContext->numberStack, var, context);
				nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
			}
				break;
			case LESS_THAN:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_BOOLEAN_LESS_THAN, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case LESS_THAN_EQUAL:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_BOOLEAN_LESS_THAN_EQUAL, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case LITERAL_FALSE:
			{
				nopl_addOperator(NoPL_BYTE_LITERAL_BOOLEAN_FALSE, context);
			}
				break;
			case LITERAL_NULL:
			{
				nopl_addOperator(NoPL_BYTE_LITERAL_NULL, context);
			}
				break;
			case LITERAL_TRUE:
			{
				nopl_addOperator(NoPL_BYTE_LITERAL_BOOLEAN_TRUE, context);
			}
				break;
			case LOGICAL_AND:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_BOOLEAN_AND, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case LOGICAL_EQUALITY:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//get the types of expressions being compared
				NoPL_DataType child1Type = nopl_dataTypeForTree(child1, context);
				NoPL_DataType child2Type = nopl_dataTypeForTree(child2, context);
				
				//make sure we're looking at comparable expressions
				if(child1Type == NoPL_type_FunctionResult && child2Type == NoPL_type_FunctionResult)
				{
					nopl_error(tree, NoPL_ErrStr_EqualityExpressionsAbiguous, context);
				}
				else if(child1Type == child2Type || child1Type == NoPL_type_FunctionResult || child2Type == NoPL_type_FunctionResult)
				{
					//we have type agreement, find the type
					NoPL_DataType exprType = child1Type;
					if(exprType == NoPL_type_FunctionResult)
						exprType = child2Type;
					
					//append the operator appropriate for that type
					switch(exprType)
					{
						case NoPL_type_Number:
							nopl_addOperator(NoPL_BYTE_NUMERIC_LOGICAL_EQUALITY, context);
							break;
						case NoPL_type_Boolean:
							nopl_addOperator(NoPL_BYTE_BOOLEAN_LOGICAL_EQUALITY, context);
							break;
						case NoPL_type_Object:
							nopl_addOperator(NoPL_BYTE_OBJECT_LOGICAL_EQUALITY, context);
							break;
						case NoPL_type_String:
							nopl_addOperator(NoPL_BYTE_STRING_LOGICAL_EQUALITY, context);
							break;
						default:
							nopl_error(tree, NoPL_ErrStr_CouldNotDetermineType, context);
							break;
					}
					
					//append the expressions
					nopl_appendNodeWithRequiredType(child1, exprType, context, options);
					nopl_appendNodeWithRequiredType(child2, exprType, context, options);
				}
				else
					nopl_error(tree, NoPL_ErrStr_EqualityDifferentType, context);
			}
				break;
			case LOGICAL_INEQUALITY:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//get the types of expressions being compared
				NoPL_DataType child1Type = nopl_dataTypeForTree(child1, context);
				NoPL_DataType child2Type = nopl_dataTypeForTree(child2, context);
				
				//make sure we're looking at comparable expressions
				if(child1Type == NoPL_type_FunctionResult && child2Type == NoPL_type_FunctionResult)
				{
					nopl_error(tree, NoPL_ErrStr_EqualityExpressionsAbiguous, context);
				}
				else if(child1Type == child2Type || child1Type == NoPL_type_FunctionResult || child2Type == NoPL_type_FunctionResult)
				{
					//we have type agreement, find the type
					NoPL_DataType exprType = child1Type;
					if(exprType == NoPL_type_FunctionResult)
						exprType = child2Type;
					
					//append the operator appropriate for that type
					switch (exprType)
					{
						case NoPL_type_Number:
							nopl_addOperator(NoPL_BYTE_NUMERIC_LOGICAL_INEQUALITY, context);
							break;
						case NoPL_type_Boolean:
							nopl_addOperator(NoPL_BYTE_BOOLEAN_LOGICAL_INEQUALITY, context);
							break;
						case NoPL_type_Object:
							nopl_addOperator(NoPL_BYTE_OBJECT_LOGICAL_INEQUALITY, context);
							break;
						case NoPL_type_String:
							nopl_addOperator(NoPL_BYTE_STRING_LOGICAL_INEQUALITY, context);
							break;
						default:
							nopl_error(tree, NoPL_ErrStr_CouldNotDetermineType, context);
							break;
					}
					
					//append the expressions
					nopl_appendNodeWithRequiredType(child1, exprType, context, options);
					nopl_appendNodeWithRequiredType(child2, exprType, context, options);
				}
				else
					nopl_error(tree, NoPL_ErrStr_EqualityDifferentType, context);
			}
				break;
			case LOGICAL_NEGATION:
			{
				//this is a unary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_BOOLEAN_NEGATION, context);
				
				//append the expression
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Boolean, context, options);
			}
				break;
			case LOGICAL_OR:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_BOOLEAN_OR, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case LOOP_DO:
			{
				//get nodes for statements in the top of the loop (or in this case, the bottom)
				pANTLR3_BASE_TREE conditional = treeIndex(tree,0);
				
				//get another byte buffer for everything inside this loop
				NoPL_CompileContext loopCtx = nopl_newInnerCompileContext(context, 1, 1);
				
				//push a scope for the loop
				nopl_pushScope(&loopCtx);
				
				//add all of the statements inside the loop
				NoPL_Index childCount = 0;
				if(tree->children)
					childCount = (NoPL_Index)tree->children->size(tree->children);
				ANTLR3_UINT32 i;
				pANTLR3_BASE_TREE childArg;
				for(i = 1; i < childCount; i++)
				{
					childArg = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
					nopl_traverseAST(childArg, options, &loopCtx);
				}
				
				//pop a scope for the loop
				nopl_popScope(&loopCtx);
				
				//store the position that the buffer should move to when a continue statement is evaluated
				NoPL_Index continueIndex = loopCtx.dataLength;
				
				//a loop gets compiled as a repeating conditional
				nopl_addOperator(NoPL_BYTE_CONDITIONAL, &loopCtx);
				
				//append the boolean expression for the conditional
				nopl_appendNodeWithRequiredType(conditional, NoPL_type_Boolean, &loopCtx, options);
				
				//add the number of bytes to skip when the conditional fails
				NoPL_Index condMove = sizeof(NoPL_Instruction)+sizeof(NoPL_Index);
				nopl_addBytesToContext(&condMove, sizeof(NoPL_Index), &loopCtx);
				
				//the only content of the conditional is a buffer move that restarts the loop
				nopl_addOperator(NoPL_BYTE_BUFFER_MOVE_BACKWARD, &loopCtx);
				NoPL_Index loopMove = (NoPL_Index)(loopCtx.dataLength+sizeof(NoPL_Index));
				nopl_addBytesToContext(&loopMove, sizeof(NoPL_Index), &loopCtx);
				
				//append the loop
				nopl_finalizeControlFlowMoves(&loopCtx, loopCtx.dataLength, continueIndex);
				nopl_appendContext(&loopCtx, context);
				
				nopl_freeInnerCompileContext(&loopCtx);
			}
				break;
			case LOOP_FOR:
			{
				//push a scope for the statements in the top of the loop
				nopl_pushScope(context);
				
				//we need to search the children for statements in the top of the loop
				NoPL_Index childCount = 0;
				if(tree->children)
					childCount = (NoPL_Index)tree->children->size(tree->children);
				ANTLR3_UINT32 i = 0;
				pANTLR3_BASE_TREE childArg;
				
				//get nodes for statements in the top of the loop
				pANTLR3_BASE_TREE declaration = NULL;
				pANTLR3_BASE_TREE conditional = NULL;
				pANTLR3_BASE_TREE increment = NULL;
				
				//attempt to get declaration
				if(i < childCount)
				{
					childArg = treeIndex(tree, i);
					if(childArg->getType(childArg) == FOR_LOOP_DECL)
					{
						declaration = treeIndex(childArg, 0);
						i++;
					}
				}
				
				//attempt to get conditional
				if(i < childCount)
				{
					childArg = treeIndex(tree, i);
					if(childArg->getType(childArg) == FOR_LOOP_COND)
					{
						conditional = treeIndex(childArg, 0);
						i++;
					}
				}
				
				//attempt to get increment
				if(i < childCount)
				{
					childArg = treeIndex(tree, i);
					if(childArg->getType(childArg) == FOR_LOOP_ITER)
					{
						increment = treeIndex(childArg, 0);
						i++;
					}
				}
				
				//append the declaration
				if(declaration)
					nopl_traverseAST(declaration, options, context);
				
				//get a byte buffer for everything at the top of this loop
				NoPL_CompileContext outerLoopCtx = nopl_newInnerCompileContext(context, 0, 0);
				
				//a loop gets compiled as a repeating conditional
				if(conditional)
				{
					//append the boolean expression for the conditional
					nopl_addOperator(NoPL_BYTE_CONDITIONAL, &outerLoopCtx);
					nopl_appendNodeWithRequiredType(conditional, NoPL_type_Boolean, &outerLoopCtx, options);
				}
				
				//get another byte buffer for everything inside this loop
				NoPL_CompileContext innerLoopCtx = nopl_newInnerCompileContext(&outerLoopCtx, 1, 1);
				
				//push a scope for the loop
				nopl_pushScope(&innerLoopCtx);
				
				//add all of the statements inside the loop
				for(; i < childCount; i++)
				{
					childArg = treeIndex(tree, i);
					nopl_traverseAST(childArg, options, &innerLoopCtx);
				}
				
				//store the position that the buffer should move to when a continue statement is evaluated
				NoPL_Index continueIndex = innerLoopCtx.dataLength;
				
				//append the increment at the end of the loop
				if(increment)
					nopl_traverseAST(increment, options, &innerLoopCtx);
				
				//pop a scope for the loop
				nopl_popScope(&innerLoopCtx);
				
				//go back to the beginning of the loop
				nopl_addOperator(NoPL_BYTE_BUFFER_MOVE_BACKWARD, &innerLoopCtx);
				NoPL_Index repeatMove = (NoPL_Index)(outerLoopCtx.dataLength+innerLoopCtx.dataLength+(sizeof(NoPL_Index)));
				if(conditional)
					repeatMove += sizeof(NoPL_Index);
				nopl_addBytesToContext(&repeatMove, sizeof(NoPL_Index), &innerLoopCtx);
				
				//add the number of bytes to skip when the conditional fails
				if(conditional)
				{
					NoPL_Index condMove = (NoPL_Index)innerLoopCtx.dataLength;
					nopl_addBytesToContext(&condMove, sizeof(NoPL_Index), &outerLoopCtx);
				}
				
				//append the inner loop to the outer loop
				continueIndex += outerLoopCtx.dataLength;
				nopl_appendContext(&innerLoopCtx, &outerLoopCtx);
				nopl_finalizeControlFlowMoves(&outerLoopCtx, outerLoopCtx.dataLength, continueIndex);
				
				//append the loop contents
				nopl_appendContext(&outerLoopCtx, context);
				
				//pop the scope for the statements in the top of the loop
				nopl_popScope(context);
				
				//free contexts
				nopl_freeInnerCompileContext(&innerLoopCtx);
				nopl_freeInnerCompileContext(&outerLoopCtx);
			}
				break;
			case LOOP_WHILE:
			{
				//get nodes for statements in the top of the loop
				pANTLR3_BASE_TREE conditional = treeIndex(tree,0);
				
				//get a byte buffer for everything at the top of this loop
				NoPL_CompileContext outerLoopCtx = nopl_newInnerCompileContext(context, 0, 0);
				
				//a loop gets compiled as a repeating conditional
				nopl_addOperator(NoPL_BYTE_CONDITIONAL, &outerLoopCtx);
				
				//append the boolean expression for the conditional
				nopl_appendNodeWithRequiredType(conditional, NoPL_type_Boolean, &outerLoopCtx, options);
				
				//get another byte buffer for everything inside this loop
				NoPL_CompileContext innerLoopCtx = nopl_newInnerCompileContext(&outerLoopCtx, 1, 1);
				
				//push a scope for the loop
				nopl_pushScope(&innerLoopCtx);
				
				//add all of the statements inside the loop
				NoPL_Index childCount = 0;
				if(tree->children)
					childCount = (NoPL_Index)tree->children->size(tree->children);
				ANTLR3_UINT32 i;
				pANTLR3_BASE_TREE childArg;
				for(i = 1; i < childCount; i++)
				{
					childArg = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
					nopl_traverseAST(childArg, options, &innerLoopCtx);
				}
				
				//pop a scope for the loop
				nopl_popScope(&innerLoopCtx);
				
				//go back to the beginning of the loop
				nopl_addOperator(NoPL_BYTE_BUFFER_MOVE_BACKWARD, &innerLoopCtx);
				NoPL_Index repeatMove = (NoPL_Index)(outerLoopCtx.dataLength+innerLoopCtx.dataLength+(2*sizeof(NoPL_Index)));
				nopl_addBytesToContext(&repeatMove, sizeof(NoPL_Index), &innerLoopCtx);
				
				//add the number of bytes to skip when the conditional fails
				NoPL_Index condMove = (NoPL_Index)innerLoopCtx.dataLength;
				nopl_addBytesToContext(&condMove, sizeof(NoPL_Index), &outerLoopCtx);
				
				//append the inner loop to the outer loop
				nopl_appendContext(&innerLoopCtx, &outerLoopCtx);
				nopl_finalizeControlFlowMoves(&outerLoopCtx, outerLoopCtx.dataLength, 0);
				
				//append the logic for the loop
				nopl_appendContext(&outerLoopCtx, context);
				
				//free contexts
				nopl_freeInnerCompileContext(&innerLoopCtx);
				nopl_freeInnerCompileContext(&outerLoopCtx);
			}
				break;
			case METADATA:
			{
				//get the string and length
				char* string = (char*)tree->getText(tree)->chars;
				int length = (int)strlen(string);
				
				//strip the quotes
				char stringCopy[length-1];
				strcpy(stringCopy, (string+2));
				stringCopy[length-4] = 0;
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_METADATA, context);
				
				//add the string
				nopl_addBytesToContext(stringCopy, sizeof(char)*(length-3), context);
			}
				break;
			case MOD:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_MODULO, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case MOD_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, assignTo->getToken(assignTo), NoPL_TokenRangeType_variables);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = nopl_dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					nopl_addOperator(NoPL_BYTE_NUMERIC_MODULO_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->numberStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					nopl_appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
				}
				else
				{
					if(nopl_variableExistsInContext(assignTo->getText(assignTo), context))
						nopl_error(assignTo, NoPL_ErrStr_CannotIncrement, context);
					else
						nopl_error(assignTo, NoPL_ErrStr_VariableNotDeclared, context);
				}
			}
				break;
			case MULTIPLY:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_MULTIPLY, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case MULTIPLY_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, assignTo->getToken(assignTo), NoPL_TokenRangeType_variables);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = nopl_dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					nopl_addOperator(NoPL_BYTE_NUMERIC_MULTIPLY_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->numberStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					nopl_appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
				}
				else
				{
					if(nopl_variableExistsInContext(assignTo->getText(assignTo), context))
						nopl_error(assignTo, NoPL_ErrStr_CannotIncrement, context);
					else
						nopl_error(assignTo, NoPL_ErrStr_VariableNotDeclared, context);
				}
			}
				break;
			case NUMBER:
			{
				//this is a literal number, get the float value from string
				float floatVal = (float)atof((const char*)tree->getText(tree)->chars);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_LITERAL_NUMBER, context);
				
				//add the float
				nopl_addBytesToContext(&floatVal, sizeof(float), context);
			}
				break;
			case NUMERIC_NEGATION:
			{
				//this operator should have one numeric child
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				
				//append the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_NEGATION, context);
				
				//append the expression
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
			}
				break;
			case OBJECT_TO_MEMBER:
			{
				//declare some nodes that we'll need to examine to make the function call
				pANTLR3_BASE_TREE secondChild = treeIndex(tree,1);
				pANTLR3_BASE_TREE funcName = NULL;
				pANTLR3_BASE_TREE args = NULL;
				
				//check the format of this call
				ANTLR3_UINT32 callNodeType = secondChild->getType(secondChild);
				if(callNodeType == ID)
				{
					//this is a call without any args
					funcName = secondChild;
				}
				if(callNodeType == FUNCTION_CALL)
				{
					//function name will be the first child
					funcName = treeIndex(secondChild,0);
					
					//check for args
					if(secondChild->getChildCount(secondChild) > 1)
						args = treeIndex(secondChild,1);
				}
				
				//append the call
				nopl_appendFunctionCall(treeIndex(tree,0), funcName, args, options, context);
			}
				break;
			case PRINT_VALUE:
			{
				//this operator should only have one child
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_STRING_PRINT, context);
				
				//guarantee that the child is a string by casting if necessary
				switch(nopl_dataTypeForTree(child1, context))
				{
					case NoPL_type_Boolean:
						nopl_addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
						break;
					case NoPL_type_FunctionResult:
						nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
						break;
					case NoPL_type_Number:
						nopl_addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
						break;
					case NoPL_type_String:
						break;
					case NoPL_type_Object:
						nopl_addOperator(NoPL_BYTE_CAST_OBJECT_TO_STRING, context);
						break;
					default:
						nopl_error(child1, NoPL_ErrStr_CouldNotDetermineType, context);
						break;
				}
				
				//append the expression
				nopl_traverseAST(child1, options, context);
			}
				break;
			case SCOPE_OPEN:
			{
				//push a scope
				nopl_pushScope(context);
				
				//loop to append all children
				NoPL_Index childCount = 0;
				if(tree->children)
					childCount = (NoPL_Index)tree->children->size(tree->children);
				ANTLR3_UINT32 i;
				pANTLR3_BASE_TREE child;
				for(i = 0; i < childCount; i++)
				{
					//get each child and append
					child = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
					nopl_traverseAST(child, options, context);
				}
				
				//pop the scope
				nopl_popScope(context);
			}
				break;
			case STRING:
			{
				//get the string and length
				char* input = (char*)tree->getText(tree)->chars;
				
				//our string still has quotes around it, take one char off the beginning and end
				input++;
				size_t inputLength = strlen(input)-1;
				
				//make a copy of this string with substituted special characters
				char subString[inputLength+1];
				char* copyTo = subString;
				size_t i;
				for(i = 0; i < inputLength; i++)
				{
					//check for an escape character
					if(input[i] == '\\')
					{
						//skip the escape character
						i++;
						
						//check which special character we're putting in here
						switch (input[i])
						{
							case '\'':
								*copyTo = '\'';
								break;
							case '\"':
								*copyTo = '\"';
								break;
							case '?':
								*copyTo = '\?';
								break;
							case '\\':
								*copyTo = '\\';
								break;
							case '0':
								*copyTo = '\0';
								break;
							case 'f':
								*copyTo = '\f';
								break;
							case 'n':
								*copyTo = '\n';
								break;
							case 'r':
								*copyTo = '\r';
								break;
							case 't':
								*copyTo = '\t';
								break;
							case 'v':
								*copyTo = '\v';
								break;
							default:
								copyTo--;
								break;
						}
						copyTo++;
					}
					else
					{
						*copyTo = input[i];
						copyTo++;
					}
				}
				*copyTo = '\0';
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_LITERAL_STRING, context);
				
				//add the string
				nopl_addBytesToContext(subString, (int)strlen(subString)+1, context);
			}
				break;
			case SUBSCRIPT_OPEN:
			{
				//get the index and object
				pANTLR3_BASE_TREE object = treeIndex(tree,0);
				pANTLR3_BASE_TREE index = treeIndex(tree,1);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_FUNCTION_INDEX, context);
				
				//append the object expression
				nopl_appendNodeWithRequiredType(object, NoPL_type_Object, context, options);
				
				//we can only index numbers or strings
				switch(nopl_dataTypeForTree(index, context))
				{
					case NoPL_type_FunctionResult:
						nopl_error(index, "This expression must be cast to either a numeric or string value", context);
						break;
					case NoPL_type_Number:
						nopl_addOperator(NoPL_BYTE_ARG_NUMBER, context);
						break;
					case NoPL_type_String:
						nopl_addOperator(NoPL_BYTE_ARG_STRING, context);
						break;
					default:
						nopl_error(index, "Cannot use an expression of this type as an index", context);
						break;
				}
				
				//append the index expression
				nopl_traverseAST(index, options, context);
			}
				break;
			case SUBTRACT:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				nopl_addOperator(NoPL_BYTE_NUMERIC_SUBTRACT, context);
				
				//append the expressions
				nopl_appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				nopl_appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case SUBTRACT_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//variables need to be explicitly tagged, check if we wanted token ranges
				if(options->createTokenRanges)
					nopl_addTokenRange(context, assignTo->getToken(assignTo), NoPL_TokenRangeType_variables);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = nopl_dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					nopl_addOperator(NoPL_BYTE_NUMERIC_SUBTRACT_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = nopl_indexOfVariableInStack(assignTo->getText(assignTo), pContext->numberStack, assignTo, context);
					nopl_addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					nopl_appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
				}
				else
				{
					if(nopl_variableExistsInContext(assignTo->getText(assignTo), context))
						nopl_error(assignTo, NoPL_ErrStr_CannotIncrement, context);
					else
						nopl_error(assignTo, NoPL_ErrStr_VariableNotDeclared, context);
				}
			}
				break;
			case SWITCH:
			{
				//get the expression which is being evaluated
				pANTLR3_BASE_TREE expression = treeIndex(tree,0);
				NoPL_DataType expType = nopl_dataTypeForTree(expression, context);
				
				//attempt to get the result's type from the first case
				if(expType == NoPL_type_FunctionResult && tree->children)
				{
					pANTLR3_BASE_TREE firstCase = treeIndex(tree, 1);
					NoPL_DataType caseType = nopl_dataTypeForTree(firstCase, context);
					if(caseType == NoPL_type_Boolean || caseType == NoPL_type_Number || caseType == NoPL_type_String)
					{
						expType = caseType;
					}
				}
				
				//push a scope and set up an inner context
				nopl_pushScope(context);
				NoPL_CompileContext switchCtx = nopl_newInnerCompileContext(context, 1, 0);
				
				//set up some vars for appending the cases
				NoPL_Index caseCount = 0;
				if(tree->children)
					caseCount = (NoPL_Index)(tree->children->size(tree->children)-1);
				NoPL_Index caseBufferMoves[caseCount];
				NoPL_Index lastBufferMove;
				NoPL_Index bufferMoveDummyValue = 0;
				
				//declare the result of the expression as a variable so that we only have to evaluate it once
				switch(expType)
				{
					case NoPL_type_Boolean:
					{
						//check if we need an anonymous variable
						NoPL_Index index;
						if(expression->getType(expression) == ID && nopl_variableExistsInStack(expression->getText(expression), pContext->booleanStack))
						{
							//this is already a variable
							index = nopl_indexOfVariableInStack(expression->getText(expression), pContext->booleanStack, expression, &switchCtx);
						}
						else
						{
							//we're assigning an anonymous variable
							nopl_addOperator(NoPL_BYTE_BOOLEAN_ASSIGN, &switchCtx);
							index = nopl_declareAnonymousVariableInStack(pContext->booleanStack);
							nopl_addBytesToContext(&index, sizeof(NoPL_Index), &switchCtx);
							
							//append the expression
							nopl_appendNodeWithRequiredType(expression, NoPL_type_Boolean, &switchCtx, options);
						}
						
						//append checks for each of the cases
						for(int i = 0; i < caseCount; i++)
						{
							//get the case statement
							pANTLR3_BASE_TREE caseNode = treeIndex(tree, i+1);
							
							//check for default
							if(caseNode->getType(caseNode) == SWITCH_DEFAULT)
							{
								//throw an error if this is not the last case
								if(i != (caseCount-1))
									nopl_error(caseNode, NoPL_ErrStr_MisplacedDefault, &switchCtx);
								nopl_addOperator(NoPL_BYTE_BUFFER_MOVE_FORWARD, &switchCtx);
							}
							else
							{
								//this is not a default statement
								pANTLR3_BASE_TREE caseValue = treeIndex(caseNode, 0);
								
								//append a case statement for each case value
								nopl_addOperator(NoPL_BYTE_SWITCH_CASE_BOOLEAN, &switchCtx);
								nopl_addBytesToContext(&index, sizeof(NoPL_Index), &switchCtx);
								if(caseValue->getType(caseValue) == LITERAL_TRUE)
									nopl_addOperator(NoPL_BYTE_LITERAL_BOOLEAN_TRUE, &switchCtx);
								else
									nopl_addOperator(NoPL_BYTE_LITERAL_BOOLEAN_FALSE, &switchCtx);
								
								//check all previous statements to see if this one is a duplicate
								for(int j = 0; j < i; j++)
								{
									//get the value for the possible duplicate
									pANTLR3_BASE_TREE possibleDuplicate = treeIndex(tree, j+1);
									pANTLR3_BASE_TREE possibleDuplicateValue = treeIndex(possibleDuplicate, 0);
									
									//check if the value is the same
									if(caseValue->getType(caseValue) == possibleDuplicateValue->getType(possibleDuplicateValue))
									{
										nopl_error(caseValue, NoPL_ErrStr_DuplicateSwitchCase, &switchCtx);
										break;
									}
								}
							}
							
							//append a buffer move
							caseBufferMoves[i] = switchCtx.dataLength;
							nopl_addBytesToContext(&bufferMoveDummyValue, sizeof(NoPL_Index), &switchCtx);
						}
					}
						break;
					case NoPL_type_Number:
					{
						//check if we need an anonymous variable
						NoPL_Index index;
						if(expression->getType(expression) == ID && nopl_variableExistsInStack(expression->getText(expression), pContext->numberStack))
						{
							//this is already a variable
							index = nopl_indexOfVariableInStack(expression->getText(expression), pContext->numberStack, expression, &switchCtx);
						}
						else
						{
							//we're assigning an anonymous variable
							nopl_addOperator(NoPL_BYTE_NUMERIC_ASSIGN, &switchCtx);
							index = nopl_declareAnonymousVariableInStack(private(switchCtx)->numberStack);
							nopl_addBytesToContext(&index, sizeof(NoPL_Index), &switchCtx);
							
							//append the expression
							nopl_appendNodeWithRequiredType(expression, NoPL_type_Number, &switchCtx, options);
						}
						
						//append checks for each of the cases
						for(int i = 0; i < caseCount; i++)
						{
							//get the case statement
							pANTLR3_BASE_TREE caseNode = treeIndex(tree, i+1);
							
							//check for default
							if(caseNode->getType(caseNode) == SWITCH_DEFAULT)
							{
								//throw an error if this is not the last case
								if(i != (caseCount-1))
									nopl_error(caseNode, NoPL_ErrStr_MisplacedDefault, &switchCtx);
								nopl_addOperator(NoPL_BYTE_BUFFER_MOVE_FORWARD, &switchCtx);
							}
							else
							{
								//this is not a default statement
								pANTLR3_BASE_TREE caseValue = treeIndex(caseNode, 0);
								
								//append a case statement for each case value
								nopl_addOperator(NoPL_BYTE_SWITCH_CASE_NUMBER, &switchCtx);
								nopl_addBytesToContext(&index, sizeof(NoPL_Index), &switchCtx);
								float floatValue = (float)atof((const char*)caseValue->getText(caseValue)->chars);
								nopl_addBytesToContext(&floatValue, sizeof(float), &switchCtx);
								
								//check all previous statements to see if this one is a duplicate
								for(int j = 0; j < i; j++)
								{
									//get the value for the possible duplicate
									pANTLR3_BASE_TREE possibleDuplicate = treeIndex(tree, j+1);
									pANTLR3_BASE_TREE possibleDuplicateValue = treeIndex(possibleDuplicate, 0);
									
									//check if the value is the same
									float possibleDuplicateFloat = (float)atof((const char*)possibleDuplicateValue->getText(possibleDuplicateValue)->chars);
									if(floatValue == possibleDuplicateFloat)
									{
										nopl_error(caseValue, NoPL_ErrStr_DuplicateSwitchCase, &switchCtx);
										break;
									}
								}
							}
							
							//append a buffer move
							caseBufferMoves[i] = switchCtx.dataLength;
							nopl_addBytesToContext(&bufferMoveDummyValue, sizeof(NoPL_Index), &switchCtx);
						}
					}
						break;
					case NoPL_type_String:
					{
						//check if we need an anonymous variable
						NoPL_Index index;
						if(expression->getType(expression) == ID && nopl_variableExistsInStack(expression->getText(expression), pContext->stringStack))
						{
							//this is already a variable
							index = nopl_indexOfVariableInStack(expression->getText(expression), pContext->stringStack, expression, &switchCtx);
						}
						else
						{
							//we're assigning an anonymous variable
							nopl_addOperator(NoPL_BYTE_STRING_ASSIGN, &switchCtx);
							index = nopl_declareAnonymousVariableInStack(private(switchCtx)->stringStack);
							nopl_addBytesToContext(&index, sizeof(NoPL_Index), &switchCtx);
							
							//append the expression
							nopl_appendNodeWithRequiredType(expression, NoPL_type_String, &switchCtx, options);
						}
						
						//append checks for each of the cases
						for(int i = 0; i < caseCount; i++)
						{
							//get the case statement
							pANTLR3_BASE_TREE caseNode = treeIndex(tree, i+1);
							
							//check for default
							if(caseNode->getType(caseNode) == SWITCH_DEFAULT)
							{
								//throw an error if this is not the last case
								if(i != (caseCount-1))
									nopl_error(caseNode, NoPL_ErrStr_MisplacedDefault, &switchCtx);
								nopl_addOperator(NoPL_BYTE_BUFFER_MOVE_FORWARD, &switchCtx);
							}
							else
							{
								//this is not a default statement
								pANTLR3_BASE_TREE caseValue = treeIndex(caseNode, 0);
								
								//append a case statement for each case value
								nopl_addOperator(NoPL_BYTE_SWITCH_CASE_STRING, &switchCtx);
								nopl_addBytesToContext(&index, sizeof(NoPL_Index), &switchCtx);
								
								//append string value
								char* stringValue = (char*)caseValue->getText(caseValue)->chars;
								int length = (int)strlen(stringValue);
								//strip the quotes
								char stringCopy[length];
								strcpy(stringCopy, (stringValue+1));
								stringCopy[length-2] = 0;
								//add the string
								nopl_addBytesToContext(stringCopy, sizeof(char)*(length-1), &switchCtx);
								
								//check all previous statements to see if this one is a duplicate
								for(int j = 0; j < i; j++)
								{
									//get the value for the possible duplicate
									pANTLR3_BASE_TREE possibleDuplicate = treeIndex(tree, j+1);
									pANTLR3_BASE_TREE possibleDuplicateValue = treeIndex(possibleDuplicate, 0);
									
									//check if the value is the same
									if(!strcmp((char*)caseValue->getText(caseValue)->chars, (char*)possibleDuplicateValue->getText(possibleDuplicateValue)->chars))
									{
										nopl_error(caseValue, NoPL_ErrStr_DuplicateSwitchCase, &switchCtx);
										break;
									}
								}
							}
							
							//append a buffer move
							caseBufferMoves[i] = switchCtx.dataLength;
							nopl_addBytesToContext(&bufferMoveDummyValue, sizeof(NoPL_Index), &switchCtx);
						}
					}
						break;
					default:
						break;
				}
				
				//store the location of the end of the case checks
				nopl_addOperator(NoPL_BYTE_BUFFER_MOVE_FORWARD, &switchCtx);
				lastBufferMove = switchCtx.dataLength;
				nopl_addBytesToContext(&bufferMoveDummyValue, sizeof(NoPL_Index), &switchCtx);
				
				//append the contents of the case statements
				for(int i = 0; i < caseCount; i++)
				{
					//set the buffer move for this case
					NoPL_Index* bufferMovePtr = (NoPL_Index*)(switchCtx.compiledData+caseBufferMoves[i]);
					*bufferMovePtr = (switchCtx.dataLength-(caseBufferMoves[i]+sizeof(NoPL_Index)));
					
					//iterate over the statements in this case
					pANTLR3_BASE_TREE caseNode = treeIndex(tree, i+1);
					
					//recursively check all nodes in this AST
					NoPL_Index childCount = 0;
					if(tree->children)
						childCount = (NoPL_Index)caseNode->children->size(caseNode->children);
					ANTLR3_UINT32 j = (caseNode->getType(caseNode) == SWITCH_DEFAULT) ? 0 : 1;
					pANTLR3_BASE_TREE child;
					for(; j < childCount; j++)
					{
						//get each child and append
						child = (pANTLR3_BASE_TREE)(caseNode->children->get(caseNode->children, j));
						nopl_traverseAST(child, options, &switchCtx);
					}
				}
				
				//set the buffer move for this case
				NoPL_Index* bufferMovePtr = (NoPL_Index*)(switchCtx.compiledData+lastBufferMove);
				*bufferMovePtr = (switchCtx.dataLength-(lastBufferMove+sizeof(NoPL_Index)));
				
				//copy the inner context
				nopl_finalizeControlFlowMoves(&switchCtx, switchCtx.dataLength, 0);
				nopl_appendContext(&switchCtx, context);
				
				nopl_popScope(context);
			}
				break;
			case TYPE_CAST:
			{
				//get the expression which will be cast
				pANTLR3_BASE_TREE expression = treeIndex(tree,1);
				
				NoPL_DataType castType = nopl_dataTypeForTree(tree, context);
				NoPL_DataType expressionType = nopl_dataTypeForTree(expression, context);
				
				//only add this cast if the types are actually different
				if(castType != expressionType)
				{
					switch (castType)
					{
						case NoPL_type_Boolean:
							
							//cast to boolean
							if(expressionType == NoPL_type_FunctionResult)
								nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_BOOLEAN, context);
							else if(expressionType == NoPL_type_Number)
								nopl_addOperator(NoPL_BYTE_CAST_NUMBER_TO_BOOLEAN, context);
							else if(expressionType == NoPL_type_String)
								nopl_addOperator(NoPL_BYTE_CAST_STRING_TO_BOOLEAN, context);
							else if(expressionType == NoPL_type_Object)
								nopl_error(expression, NoPL_ErrStr_CannotCastObjectToPrimitive, context);
							else
								nopl_error(expression, NoPL_ErrStr_CouldNotDetermineType, context);
							
							break;
						case NoPL_type_Number:
							
							//cast to number
							if(expressionType == NoPL_type_FunctionResult)
								nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_NUMBER, context);
							else if(expressionType == NoPL_type_Boolean)
								nopl_addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_NUMBER, context);
							else if(expressionType == NoPL_type_String)
								nopl_addOperator(NoPL_BYTE_CAST_STRING_TO_NUMBER, context);
							else if(expressionType == NoPL_type_Object)
								nopl_error(expression, NoPL_ErrStr_CannotCastObjectToPrimitive, context);
							else
								nopl_error(expression, NoPL_ErrStr_CouldNotDetermineType, context);
							
							break;
						case NoPL_type_String:
							
							//cast to string
							if(expressionType == NoPL_type_FunctionResult)
								nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
							else if(expressionType == NoPL_type_Boolean)
								nopl_addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
							else if(expressionType == NoPL_type_Number)
								nopl_addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
							else if(expressionType == NoPL_type_Object)
								nopl_addOperator(NoPL_BYTE_CAST_OBJECT_TO_STRING, context);
							else
								nopl_error(expression, NoPL_ErrStr_CouldNotDetermineType, context);
							
							break;
						case NoPL_type_Object:
							
							//can only cast to an object when dealing with values returned from an external function
							if(expressionType == NoPL_type_FunctionResult)
								nopl_addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_OBJECT, context);
							else
								nopl_error(tree, "Cannot cast non-Object primitives to Object", context);
							
							break;
						default:
							break;
					}
				}
				
				//append the expression
				nopl_traverseAST(expression, options, context);
			}
				break;
		}
	}
}

void nopl_compileWithInputStream(pANTLR3_INPUT_STREAM stream, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//set up a parser
	pNoPLLexer lex = NoPLLexerNew(stream);
	pANTLR3_COMMON_TOKEN_STREAM tokenStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
	pNoPLParser parser = NoPLParserNew(tokenStream);
	
	//we want custom error hanlding
	pANTLR3_BASE_RECOGNIZER recognizer = parser->pParser->rec;
	recognizer->state->custom = context;
	parser->pParser->rec->displayRecognitionError = nopl_displayRecognitionError;
	
	//attempt to parse the program
	NoPLParser_program_return syntaxTree = parser->program(parser);
	
	//check if we want runtime performance over compile speed
	if(options->optimizeForRuntime)
	{
		//TODO: pre-process the AST for optimization here
	}
	
	//check for errors
	int errCount = recognizer->getNumberOfSyntaxErrors(recognizer);
	if(errCount == 0)
	{
		//get the offset position for the first token
		pANTLR3_VECTOR tokens = tokenStream->getTokens(tokenStream);
		if(tokens->size(tokens) > 0)
		{
			pANTLR3_COMMON_TOKEN tok = tokens->get(tokens, 0);
			pContext->tokenStart = tok->getStartIndex(tok);
		}
		
		//check if we want tokens
		if(options->createTokenRanges)
		{
			for(int i = 0; i < tokens->size(tokens); i++)
			{
				//handle all tokens except for 'ID', which is context sensitive
				pANTLR3_COMMON_TOKEN tok = tokens->get(tokens, i);
				switch(tok->getType(tok))
				{
					case NUMBER:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_numericLiterals);
						break;
					case STRING:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_stringLiterals);
						break;
					case LITERAL_TRUE:
					case LITERAL_FALSE:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_booleanLiterals);
						break;
					case LITERAL_NULL:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_pointerLiterals);
						break;
					case LOOP_WHILE:
					case LOOP_FOR:
					case LOOP_DO:
					case CONDITIONAL:
					case CONDITIONAL_ELSE:
					case SWITCH:
					case SWITCH_CASE:
					case SWITCH_DELIMITER:
					case SWITCH_DEFAULT:
					case BREAK:
					case CONTINUE:
					case EXIT:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_controlFlowKeywords);
						break;
					case DECL_NUMBER:
					case DECL_BOOL:
					case DECL_STRING:
					case DECL_OBJ:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_typeKeywords);
						break;
					case LOGICAL_EQUALITY:
					case LOGICAL_INEQUALITY:
					case LOGICAL_AND:
					case LOGICAL_OR:
					case LOGICAL_NEGATION:
					case LESS_THAN:
					case LESS_THAN_EQUAL:
					case GREATER_THAN:
					case GREATER_THAN_EQUAL:
					case ADD:
					case ADD_ASSIGN:
					case SUBTRACT:
					case SUBTRACT_ASSIGN:
					case DIVIDE:
					case DIVIDE_ASSIGN:
					case MULTIPLY:
					case MULTIPLY_ASSIGN:
					case EXPONENT:
					case EXPONENT_ASSIGN:
					case MOD:
					case MOD_ASSIGN:
					case INCREMENT:
					case DECREMENT:
					case ABS_VALUE:
					case ASSIGN:
					case PRINT_VALUE:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_operators);
						break;
					case OBJECT_TO_MEMBER:
					case ARG_DELIMITER:
					case SUBSCRIPT_OPEN:
					case SUBSCRIPT_CLOSE:
					case PAREN_OPEN:
					case PAREN_CLOSE:
					case SCOPE_OPEN:
					case SCOPE_CLOSE:
					case STATEMENT_DELIMITER:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_syntax);
						break;
					case COMMENT:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_comments);
						break;
					case METADATA:
						nopl_addTokenRange(context, tok, NoPL_TokenRangeType_metadata);
				}
			}
		}
		
		//set up counts for symbol table
		NoPL_Index objectTableSize = 0;
		NoPL_Index numberTableSize = 0;
		NoPL_Index booleanTableSize = 0;
		NoPL_Index stringTableSize = 0;
		pContext->objectTableSize = &objectTableSize;
		pContext->numberTableSize = &numberTableSize;
		pContext->booleanTableSize = &booleanTableSize;
		pContext->stringTableSize = &stringTableSize;
		
		//recurse to assemble the byte code
		nopl_traverseAST(syntaxTree.tree, options, context);
		
		//use a second context to create the symbol table
		NoPL_CompileContext symbolTableContext = newNoPL_CompileContext();
		
		//add the actual values for the symbol table sizes
		if(objectTableSize > 0)
		{
			nopl_addOperator(NoPL_BYTE_OBJECT_TABLE_SIZE, &symbolTableContext);
			nopl_addBytesToContext(&objectTableSize, sizeof(NoPL_Index), &symbolTableContext);
		}
		if(numberTableSize > 0)
		{
			nopl_addOperator(NoPL_BYTE_NUMERIC_TABLE_SIZE, &symbolTableContext);
			nopl_addBytesToContext(&numberTableSize, sizeof(NoPL_Index), &symbolTableContext);
		}
		if(booleanTableSize > 0)
		{
			nopl_addOperator(NoPL_BYTE_BOOLEAN_TABLE_SIZE, &symbolTableContext);
			nopl_addBytesToContext(&booleanTableSize, sizeof(NoPL_Index), &symbolTableContext);
		}
		if(stringTableSize > 0)
		{
			nopl_addOperator(NoPL_BYTE_STRING_TABLE_SIZE, &symbolTableContext);
			nopl_addBytesToContext(&stringTableSize, sizeof(NoPL_Index), &symbolTableContext);
		}
		
		//prepend the table
		if(symbolTableContext.dataLength > 0)
		{
			//append the script to the table buffer
			nopl_appendContext(context, &symbolTableContext);
			
			//replace the old buffer
			if(context->compiledData)
				free(context->compiledData);
			context->compiledData = symbolTableContext.compiledData;
			context->dataLength = symbolTableContext.dataLength;
			symbolTableContext.compiledData = NULL;
		}
		
		//free the symbol table context
		freeNoPL_CompileContext(&symbolTableContext);
	}
	
	//clean up
	parser->free(parser);
	tokenStream->free(tokenStream);
	lex->free(lex);
}

#pragma mark -
#pragma mark Interface

void compileContextWithFilePath(const char* path, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//run the compiler with an input stream created from URL path
	pANTLR3_INPUT_STREAM inputStream = antlr3AsciiFileStreamNew((pANTLR3_UINT8)path);
	nopl_compileWithInputStream(inputStream, options, context);
	inputStream->free(inputStream);
}

void compileContextWithString(const char* scriptString, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//run the compiler with an input stream created from a C string
	pANTLR3_INPUT_STREAM inputStream = antlr3NewAsciiStringCopyStream((pANTLR3_UINT8)scriptString, (int)strlen((const char*)scriptString), 0);
	nopl_compileWithInputStream(inputStream, options, context);
	inputStream->free(inputStream);
}

NoPL_CompileContext newNoPL_CompileContext()
{
	NoPL_CompileContext context;
	context.compiledData = NULL;
	context.dataLength = 0;
	context.errDescriptions = NULL;
	context.tokenRanges = NULL;
	context.privateAttributes = malloc(sizeof(NoPL_CompileContextPrivate));
	private(context)->parentContext = NULL;
	private(context)->arrayLength = 0;
	private(context)->objectStack = antlr3StackNew(NoPL_StackSizeHint);
	private(context)->numberStack = antlr3StackNew(NoPL_StackSizeHint);
	private(context)->booleanStack = antlr3StackNew(NoPL_StackSizeHint);
	private(context)->stringStack = antlr3StackNew(NoPL_StackSizeHint);
	private(context)->errDescLength = 0;
	private(context)->breakStatements = NULL;
	private(context)->continueStatements = NULL;
	private(context)->allowsBreakStatements = 0;
	private(context)->allowsContinueStatements = 0;
	private(context)->debugLine = -1;
	private(context)->tokenStart = 0;
	memset(private(context)->tokenArrayLengths, 0, sizeof(unsigned int)*NoPL_TokenRangeType_count);
	context.tokenRanges = malloc(sizeof(NoPL_TokenRanges));
	memset(context.tokenRanges->ranges, 0, sizeof(NoPL_TokenRange*)*NoPL_TokenRangeType_count);
	memset(context.tokenRanges->counts, 0, sizeof(unsigned int)*NoPL_TokenRangeType_count);
	nopl_pushScope(&context);
	return context;
}

void freeNoPL_CompileContext(NoPL_CompileContext* context)
{
	if(context->compiledData)
	{
		free(context->compiledData);
		context->compiledData = NULL;
	}
	if(context->errDescriptions)
	{
		free(context->errDescriptions);
		context->errDescriptions = NULL;
	}
	if(context->tokenRanges)
	{
		//free all ranges inside this struct
		for(int i = 0; i < NoPL_TokenRangeType_count; i++)
			if(context->tokenRanges->ranges[i])
				free(context->tokenRanges->ranges[i]);
		
		free(context->tokenRanges);
		context->tokenRanges = NULL;
	}
	context->dataLength = 0;
	
	if(context->privateAttributes)
	{
		pContext->arrayLength = 0;
		if(pContext->objectStack)
			pContext->objectStack->free(pContext->objectStack);
		if(pContext->numberStack)
			pContext->numberStack->free(pContext->numberStack);
		if(pContext->booleanStack)
			pContext->booleanStack->free(pContext->booleanStack);
		if(pContext->stringStack)
			pContext->stringStack->free(pContext->stringStack);
		
		free(context->privateAttributes);
		context->privateAttributes = NULL;
	}
}
