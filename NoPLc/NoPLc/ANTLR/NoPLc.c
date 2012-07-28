//
//  NoPLc.c
//  NoPLc
//
//  Created by Brad Bambara on 4/15/12.
//  Copyright (c) 2012 Small Planet Digital. All rights reserved.
//

#include <stdio.h>
#include "NoPLc.h"

#define treeIndex(tree,index) (pANTLR3_BASE_TREE)(tree->children->get(tree->children, index))
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
const char* NoPL_ErrStr_CannotImplicitCastObject = "Cannot implicitly cast an object to another type of primitive";
const char* NoPL_ErrStr_CannotCastObjectToPrimitive = "Cannot cast an object to another type of primitive";
const char* NoPL_ErrStr_EqualityExpressionsAbiguous = "The type of both expressions being compared is ambiguous, at least one expression must be explicitly cast";
const char* NoPL_ErrStr_EqualityDifferentType = "Both expressions compared by this equality operator must evaluate to the same type";
const char* NoPL_ErrStr_CannotControlFlow = "Cannot use this control flow statement in this context";

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

void compileWithInputStream(pANTLR3_INPUT_STREAM stream, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void addBytesToContext(const void* bytes, int byteCount, NoPL_CompileContext* context);
void addOperator(const NoPL_Instruction operator, NoPL_CompileContext* context);
void appendNodeWithRequiredType(const pANTLR3_BASE_TREE tree, const NoPL_DataType type, NoPL_CompileContext* context, const NoPL_CompileOptions* options);
void appendFunctionCall(const pANTLR3_BASE_TREE objExpression, const pANTLR3_BASE_TREE funcName, const pANTLR3_BASE_TREE args, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void traverseAST(const pANTLR3_BASE_TREE tree, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void traverseForErrors(const pANTLR3_BASE_TREE tree, NoPL_CompileContext* context);
NoPL_DataType dataTypeForTree(const pANTLR3_BASE_TREE tree, const NoPL_CompileContext* context);
void nopl_pushScope(NoPL_CompileContext* context);
void nopl_popScope(NoPL_CompileContext* context);
void nopl_error(const pANTLR3_BASE_TREE tree, const char* desc, NoPL_CompileContext* context);
int nopl_variableExistsInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack);
int nopl_variableExistsInContext(const pANTLR3_STRING varName, const NoPL_CompileContext* context);
int nopl_declareVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack);
NoPL_Index nopl_countVariablesInStack(const pANTLR3_STACK whichStack);
NoPL_Index indexOfVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack, const pANTLR3_BASE_TREE tree, NoPL_CompileContext* context);
NoPL_CompileContext newInnerCompileContext(NoPL_CompileContext* parentContext, int allowBreak, int allowContinue);
void freeInnerCompileContext(NoPL_CompileContext* context);

#pragma mark -
#pragma mark Compilation

void nopl_error(const pANTLR3_BASE_TREE tree, const char* desc, NoPL_CompileContext* context)
{
	//TODO: get the actual beginning and end nodes
	//find the first and last nodes
	pANTLR3_BASE_TREE firstNode = NULL;
	pANTLR3_BASE_TREE lastNode = NULL;
	pANTLR3_BASE_TREE loopNode = tree;
	while(loopNode->children && loopNode->children->size(loopNode->children))
		loopNode = loopNode->children->get(loopNode->children, 0);
	firstNode = loopNode;
	loopNode = tree;
	while(loopNode->children && loopNode->children->size(loopNode->children))
		loopNode = loopNode->children->get(loopNode->children, loopNode->children->size(loopNode->children)-1);
	lastNode = loopNode;
	
	//get the start and end line number and char position
	int startLine = firstNode->getLine(firstNode);
	int startChar = firstNode->getCharPositionInLine(firstNode);
	int endLine = lastNode->getLine(lastNode);
	int endChar = lastNode->getCharPositionInLine(lastNode)+strlen((char*)lastNode->getText(lastNode)->chars);
	
	//format the error
	char appendStr[512];
	sprintf(appendStr, "%s - %d:%d-%d:%d\n", desc, startLine, startChar, endLine, endChar);
	
	//lazy create the error string
	if(!context->errDescriptions)
	{
		pANTLR3_STRING_FACTORY fctry = context->tokenStream->tstream->tokenSource->strFactory;
		context->errDescriptions = fctry->newStr8(fctry, (pANTLR3_UINT8)"");
	}
	
	//append error string to context
	context->errDescriptions->append(context->errDescriptions, appendStr);
}

void addBytesToContext(const void* bytes, int byteCount, NoPL_CompileContext* context)
{
	//check if the buffer size needs to be increased
	if(!context->arrayLength)
	{
		context->arrayLength = 128;
		context->compiledData = malloc(sizeof(NoPL_Instruction)*context->arrayLength);
	}
	else if(byteCount+context->dataLength > context->arrayLength)
	{
		//double the size of the buffer
		NoPL_Instruction* oldBuffer = context->compiledData;
		int newByteLength = sizeof(NoPL_Instruction)*(context->arrayLength*2+byteCount);
		context->compiledData = malloc(newByteLength);
		
		//copy over and clean up the old buffer
		memcpy(context->compiledData, oldBuffer, context->arrayLength);
		context->arrayLength = newByteLength;
		free(oldBuffer);
	}
	
	//add the new bytes to the buffer
	memcpy((context->compiledData+context->dataLength), bytes, byteCount);
	
	//increment the data length
	context->dataLength += byteCount;
}

void addOperator(const NoPL_Instruction operator, NoPL_CompileContext* context)
{
	addBytesToContext(&operator, sizeof(NoPL_Instruction), context);
}

void nopl_pushScope(NoPL_CompileContext* context)
{
	//create some new vectors
	pANTLR3_VECTOR stringVector = antlr3VectorNew(NoPL_VectorSizeHint);
	pANTLR3_VECTOR boolVector = antlr3VectorNew(NoPL_VectorSizeHint);
	pANTLR3_VECTOR numberVector = antlr3VectorNew(NoPL_VectorSizeHint);
	pANTLR3_VECTOR objectVector = antlr3VectorNew(NoPL_VectorSizeHint);
	
	//push new vectors onto stack
	context->stringStack->push(context->stringStack, stringVector, (void(*)(void*))(stringVector->free));
	context->booleanStack->push(context->booleanStack, boolVector, (void(*)(void*))(boolVector->free));
	context->numberStack->push(context->numberStack, numberVector, (void(*)(void*))(numberVector->free));
	context->objectStack->push(context->objectStack, objectVector, (void(*)(void*))(objectVector->free));
}

void nopl_popScope(NoPL_CompileContext* context)
{
	//remove the stop vector from each stack
	context->stringStack->pop(context->stringStack);
	context->booleanStack->pop(context->booleanStack);
	context->numberStack->pop(context->numberStack);
	context->objectStack->pop(context->objectStack);
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
	return (nopl_variableExistsInStack(varName, context->objectStack) ||
			nopl_variableExistsInStack(varName, context->numberStack) ||
			nopl_variableExistsInStack(varName, context->booleanStack) ||
			nopl_variableExistsInStack(varName, context->stringStack) );
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

NoPL_Index indexOfVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack, const pANTLR3_BASE_TREE tree, NoPL_CompileContext* context)
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

NoPL_CompileContext newInnerCompileContext(NoPL_CompileContext* parentContext, int allowBreak, int allowContinue)
{
	NoPL_CompileContext context;
	context.compiledData = NULL;
	context.dataLength = 0;
	context.arrayLength = 0;
	context.tokenStream = parentContext->tokenStream;
	context.objectStack = parentContext->objectStack;
	context.numberStack = parentContext->numberStack;
	context.booleanStack = parentContext->booleanStack;
	context.stringStack = parentContext->stringStack;
	context.errDescriptions = parentContext->errDescriptions;
	context.objectTableSize = parentContext->objectTableSize;
	context.numberTableSize = parentContext->numberTableSize;
	context.booleanTableSize = parentContext->booleanTableSize;
	context.stringTableSize = parentContext->stringTableSize;
	
	if(allowBreak)
		context.breakStatements = antlr3VectorNew(NoPL_VectorSizeHint);
	else
		context.breakStatements = NULL;
	
	if(allowBreak)
		context.continueStatements = antlr3VectorNew(NoPL_VectorSizeHint);
	else
		context.continueStatements = NULL;
	
	return context;
}

void freeInnerCompileContext(NoPL_CompileContext* context)
{
	//null these pointers so that they aren't freed
	context->objectStack = NULL;
	context->numberStack = NULL;
	context->booleanStack = NULL;
	context->stringStack = NULL;
	context->tokenStream = NULL;
	context->errDescriptions = NULL;
	
	if(context->breakStatements)
	{
		context->breakStatements->free(context->breakStatements);
		context->breakStatements = NULL;
	}
	if(context->continueStatements)
	{
		context->continueStatements->free(context->continueStatements);
		context->continueStatements = NULL;
	}
	
	//free the byte buffer
	freeNoPL_CompileContext(context);
}

NoPL_DataType dataTypeForTree(const pANTLR3_BASE_TREE tree, const NoPL_CompileContext* context)
{
	switch(tree->getType(tree))
	{
		case ADD:
		{
			//the ADD operator can be used on either numbers or strings
			NoPL_DataType type1 = dataTypeForTree(treeIndex(tree,0), context);
			NoPL_DataType type2 = dataTypeForTree(treeIndex(tree,1), context);
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
			pANTLR3_BASE_TREE castChild = treeIndex(tree,0);
			ANTLR3_UINT32 castType = castChild->getType(castChild);
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
		case ID:
		{
			//check the type of this symbol in the variable stacks
			pANTLR3_STRING varName = tree->getText(tree);
			if(nopl_variableExistsInStack(varName, context->objectStack))
			   return NoPL_type_Object;
			else if(nopl_variableExistsInStack(varName, context->numberStack))
				return NoPL_type_Number;
			else if(nopl_variableExistsInStack(varName, context->booleanStack))
				return NoPL_type_Boolean;
			else if(nopl_variableExistsInStack(varName, context->stringStack))
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

void appendNodeWithRequiredType(const pANTLR3_BASE_TREE tree, const NoPL_DataType type, NoPL_CompileContext* context, const NoPL_CompileOptions* options)
{
	//get the type of this node
	NoPL_DataType treeType = dataTypeForTree(tree, context);
	
	//check if this node is a function call
	if(treeType == NoPL_type_FunctionResult)
	{
		//do a cast if we need to
		switch(type)
		{
			case NoPL_type_Boolean:
				addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_BOOLEAN, context);
				break;
			case NoPL_type_Number:
				addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_NUMBER, context);
				break;
			case NoPL_type_String:
				addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
				break;
			case NoPL_type_Object:
				addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_OBJECT, context);
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
	traverseAST(tree, options, context);
}

void appendFunctionCall(const pANTLR3_BASE_TREE objExpression, const pANTLR3_BASE_TREE funcName, const pANTLR3_BASE_TREE args, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//add the function operator
	addOperator(NoPL_BYTE_FUNCTION_CALL, context);
	
	//append the object
	if(objExpression)
		appendNodeWithRequiredType(objExpression, NoPL_type_Object, context, options);
	else
		addOperator(NoPL_BYTE_LITERAL_NULL, context);
	
	//get function name and name length
	char* functionName = (char*)funcName->getText(funcName)->chars;
	int functionNameLength = (int)strlen(functionName);
	
	//add the function name
	addBytesToContext(functionName, sizeof(char)*functionNameLength+1, context);
	
	if(args)
	{
		//get the arg count
		NoPL_Index argCount = 0;
		if(args->children)
			argCount = (NoPL_Index)args->getChildCount(args);
		
		//append bytes for count
		addBytesToContext(&argCount, sizeof(NoPL_Index), context);
		
		//loop through all children to add args
		ANTLR3_UINT32 i;
		pANTLR3_BASE_TREE childArg;
		for(i = 0; i < argCount; i++)
		{
			//get the type for each arg
			childArg = (pANTLR3_BASE_TREE)(args->children->get(args->children, i));
			NoPL_DataType argType = dataTypeForTree(childArg, context);
			
			if(argType == NoPL_type_Number)
				addOperator(NoPL_BYTE_ARG_NUMBER, context);
			else if(argType == NoPL_type_String)
				addOperator(NoPL_BYTE_ARG_STRING, context);
			else if(argType == NoPL_type_Boolean)
				addOperator(NoPL_BYTE_ARG_BOOLEAN, context);
			else if(argType == NoPL_type_Object)
				addOperator(NoPL_BYTE_ARG_OBJECT, context);
			else if(argType == NoPL_type_FunctionResult)
				nopl_error(childArg, "The type of this argument is ambiguous and requires an explicit cast", context);
			else
				nopl_error(childArg, "Cannot determine the type of this argument", context);
			
			//append the arg
			traverseAST(childArg, options, context);
		}
	}
	else
	{
		//this is a function call has specified no arguments
		NoPL_Index zero = 0;
		addBytesToContext(&zero, sizeof(NoPL_Index), context);
	}
}

void traverseAST(const pANTLR3_BASE_TREE tree, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
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
			traverseAST(child, options, context);
		}
	}
	else
	{
		switch(tree->getType(tree))
		{
			case ABS_VALUE:
			{
				//this operator should have one numeric child
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				
				//append the operator
				addOperator(NoPL_BYTE_NUMERIC_ABS_VALUE, context);
				
				//append the expression
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
			}
				break;
			case ADD:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//the ADD operator can be used for numbers or concatenating strings
				NoPL_DataType dataType = dataTypeForTree(tree, context);
				if(dataType == NoPL_type_Number)
				{
					//add the operator
					addOperator(NoPL_BYTE_NUMERIC_ADD, context);
					
					//append the expressions
					appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
					appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
				}
				else if(dataType == NoPL_type_String)
				{
					//add the operator
					addOperator(NoPL_BYTE_STRING_CONCAT, context);
					
					//get the types for the children
					NoPL_DataType childType1 = dataTypeForTree(child1, context);
					NoPL_DataType childType2 = dataTypeForTree(child2, context);
					
					//cast the first child if we need to
					if(childType1 == NoPL_type_Number)
						addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
					else if(childType1 == NoPL_type_Boolean)
						addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
					else if(childType1 == NoPL_type_FunctionResult)
						addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
					else if(childType1 == NoPL_type_Object)
						nopl_error(child1, NoPL_ErrStr_CannotImplicitCastObject, context);
					
					//add the first child
					traverseAST(child1, options, context);
					
					//cast the second child if we need to
					if(childType2 == NoPL_type_Number)
						addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
					else if(childType2 == NoPL_type_Boolean)
						addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
					else if(childType2 == NoPL_type_FunctionResult)
						addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
					else if(childType2 == NoPL_type_Object)
						nopl_error(child2, NoPL_ErrStr_CannotImplicitCastObject, context);
					
					//add the second child
					traverseAST(child2, options, context);
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
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//use the numeric increment
					addOperator(NoPL_BYTE_NUMERIC_ADD_ASSIGN, context);
					
					//add the index for the variable which will be incremented
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->numberStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
				}
				else if(assignToType == NoPL_type_String)
				{
					//use the string concat
					addOperator(NoPL_BYTE_STRING_CONCAT_ASSIGN, context);
					
					//add the index for the variable which will be incremented
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->stringStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					appendNodeWithRequiredType(incrementVal, NoPL_type_String, context, options);
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
				
				//check the object on the left-hand side to what type should be assigned
				NoPL_DataType assignToType = dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//use the numeric assign
					addOperator(NoPL_BYTE_NUMERIC_ASSIGN, context);
					
					//add the index for the variable which will be assigned to
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->numberStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					appendNodeWithRequiredType(expression, NoPL_type_Number, context, options);
				}
				else if(assignToType == NoPL_type_String)
				{
					//use the string assign
					addOperator(NoPL_BYTE_STRING_ASSIGN, context);
					
					//add the index for the variable which will be assigned to
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->stringStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					appendNodeWithRequiredType(expression, NoPL_type_String, context, options);
				}
				else if(assignToType == NoPL_type_Boolean)
				{
					//use the boolean assign
					addOperator(NoPL_BYTE_BOOLEAN_ASSIGN, context);
					
					//add the index for the variable which will be assigned to
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->booleanStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					appendNodeWithRequiredType(expression, NoPL_type_Boolean, context, options);
				}
				else if(assignToType == NoPL_type_Object)
				{
					//use the object assign
					addOperator(NoPL_BYTE_OBJECT_ASSIGN, context);
					
					//add the index for the variable which will be assigned to
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->objectStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					appendNodeWithRequiredType(expression, NoPL_type_Object, context, options);
				}
				else
				{
					nopl_error(assignTo, NoPL_ErrStr_VariableNotDeclared, context);
				}
			}
				break;
			case BREAK:
			{
				if(!context->breakStatements)
					nopl_error(tree, NoPL_ErrStr_CannotControlFlow, context);
				
				//append the operator
				addOperator(NoPL_BYTE_BUFFER_MOVE, context);
				
				//append an empty placeholder for the move
				NoPL_BufferMove move = 0;
				addBytesToContext(&move, sizeof(NoPL_BufferMove), context);
				
				//TODO: how to do this?
			}
				break;
			case CONDITIONAL:
			{
				//get the boolean for the conditional, and the first child to check if there is a corresponding 'else' statement
				pANTLR3_BASE_TREE condition = treeIndex(tree,0);
				pANTLR3_BASE_TREE firstStatement = treeIndex(tree,1);
				
				//add the operator
				addOperator(NoPL_BYTE_CONDITIONAL, context);
				
				//append the boolean expression for the conditional
				appendNodeWithRequiredType(condition, NoPL_type_Boolean, context, options);
				
				//check if the first statement node is an else statement
				int hasElse = firstStatement->getType(firstStatement) == CONDITIONAL_ELSE;
				
				//get another byte buffer for everything inside this conditional
				NoPL_CompileContext innerConditional = newInnerCompileContext(context, 0, 0);
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
					traverseAST(childArg, options, &innerConditional);
				}
				
				//how many bytes to skip if the conditional is false?
				NoPL_BufferMove move = (NoPL_BufferMove)innerConditional.dataLength;
				addBytesToContext(&move, sizeof(NoPL_BufferMove), context);
				
				//add the inner conditional
				addBytesToContext(innerConditional.compiledData, innerConditional.dataLength, context);
				
				//clean up the inner conditional
				nopl_popScope(&innerConditional);
				freeInnerCompileContext(&innerConditional);
				
				//add the else statement, if we have one
				if(hasElse)
				{
					//get another byte buffer for everything inside this else statement
					NoPL_CompileContext innerElse = newInnerCompileContext(context, 0, 0);
					nopl_pushScope(&innerElse);
					
					//add all of the statements inside the else statement
					childCount = 0;
					if(firstStatement->children)
						childCount = (NoPL_Index)firstStatement->children->size(firstStatement->children);
					ANTLR3_UINT32 i;
					for(i = 0; i < childCount; i++)
					{
						childArg = (pANTLR3_BASE_TREE)(firstStatement->children->get(firstStatement->children, i));
						traverseAST(childArg, options, &innerElse);
					}
					
					//skip over all this else stuff if we're coming from the block of code for true
					addOperator(NoPL_BYTE_BUFFER_MOVE, context);
					move = (NoPL_BufferMove)innerElse.dataLength;
					addBytesToContext(&move, sizeof(NoPL_BufferMove), context);
					
					//add the else statement
					addBytesToContext(innerElse.compiledData, innerElse.dataLength, context);
					
					//clean up the else statement
					nopl_popScope(&innerElse);
					freeInnerCompileContext(&innerElse);
				}
			}
				break;
			case CONTINUE:
			{
				if(!context->continueStatements)
					nopl_error(tree, NoPL_ErrStr_CannotControlFlow, context);
				
				//append the operator
				addOperator(NoPL_BYTE_BUFFER_MOVE, context);
				
				//append an empty placeholder for the move
				NoPL_BufferMove move = 0;
				addBytesToContext(&move, sizeof(NoPL_BufferMove), context);
				
				//TODO: how to do this?
			}
				break;
			case DECL_BOOL:
			{
				//get the name of the declared variable
				pANTLR3_BASE_TREE declaredVar = treeIndex(tree,0);
				pANTLR3_STRING declaredName = declaredVar->getText(declaredVar);
				
				//we need to evaluate the initial expression before declaring the variable
				NoPL_CompileContext initCtx = newInnerCompileContext(context, 0, 0);
				
				//get the initial value if there is one
				if(tree->getChildCount(tree) > 1)
				{
					//append the expression
					appendNodeWithRequiredType(treeIndex(tree,1), NoPL_type_Boolean, &initCtx, options);
				}
				else
				{
					//set this variable to a default value
					addOperator(NoPL_BYTE_LITERAL_BOOLEAN_FALSE, &initCtx);
				}
				
				//check if the variable was already declared elsewhere
				if(nopl_variableExistsInContext(declaredName, context))
					nopl_error(declaredVar, NoPL_ErrStr_VariableAlreadyDeclared, context);
				
				//declare the variable
				nopl_declareVariableInStack(declaredName, context->booleanStack);
				NoPL_Index newCount = nopl_countVariablesInStack(context->booleanStack);
				if(newCount > *(context->booleanTableSize))
					*(context->booleanTableSize) = newCount;
				
				//assign to the index of the newly created variable
				addOperator(NoPL_BYTE_BOOLEAN_ASSIGN, context);
				NoPL_Index index = indexOfVariableInStack(declaredName, context->booleanStack, declaredVar, context);
				addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//append the initialization
				addBytesToContext(initCtx.compiledData, initCtx.dataLength, context);
				freeInnerCompileContext(&initCtx);
			}
				break;
			case DECL_NUMBER:
			{
				//get the name of the declared variable
				pANTLR3_BASE_TREE declaredVar = treeIndex(tree,0);
				pANTLR3_STRING declaredName = declaredVar->getText(declaredVar);
				
				//we need to evaluate the initial expression before declaring the variable
				NoPL_CompileContext initCtx = newInnerCompileContext(context, 0, 0);
				
				//get the initial value if there is one
				if(tree->getChildCount(tree) > 1)
				{
					//append the expression
					appendNodeWithRequiredType(treeIndex(tree,1), NoPL_type_Number, &initCtx, options);
				}
				else
				{
					//set this variable to a default value
					addOperator(NoPL_BYTE_LITERAL_NUMBER, &initCtx);
					float defaultFloat = 0.0f;
					addBytesToContext(&defaultFloat, sizeof(float), &initCtx);
				}
				
				//check if the variable was already declared elsewhere
				if(nopl_variableExistsInContext(declaredName, context))
					nopl_error(declaredVar, NoPL_ErrStr_VariableAlreadyDeclared, context);
				
				//declare the variable
				nopl_declareVariableInStack(declaredName, context->numberStack);
				NoPL_Index newCount = nopl_countVariablesInStack(context->numberStack);
				if(newCount > *(context->numberTableSize))
					*(context->numberTableSize) = newCount;
				
				//assign to the index of the newly created variable
				addOperator(NoPL_BYTE_NUMERIC_ASSIGN, context);
				NoPL_Index index = indexOfVariableInStack(declaredName, context->numberStack, declaredVar, context);
				addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//append the initialization
				addBytesToContext(initCtx.compiledData, initCtx.dataLength, context);
				freeInnerCompileContext(&initCtx);
			}
				break;
			case DECL_OBJ:
			{
				//get the name of the declared variable
				pANTLR3_BASE_TREE declaredVar = treeIndex(tree,0);
				pANTLR3_STRING declaredName = declaredVar->getText(declaredVar);
				
				//we need to evaluate the initial expression before declaring the variable
				NoPL_CompileContext initCtx = newInnerCompileContext(context, 0, 0);
				
				//get the initial value if there is one
				if(tree->getChildCount(tree) > 1)
				{
					//append the expression
					appendNodeWithRequiredType(treeIndex(tree,1), NoPL_type_Object, &initCtx, options);
				}
				else
				{
					//set this variable to a default value
					addOperator(NoPL_BYTE_LITERAL_NULL, &initCtx);
				}
				
				//check if the variable was already declared elsewhere
				if(nopl_variableExistsInContext(declaredName, context))
					nopl_error(declaredVar, NoPL_ErrStr_VariableAlreadyDeclared, context);
				
				//declare the variable
				nopl_declareVariableInStack(declaredName, context->objectStack);
				NoPL_Index newCount = nopl_countVariablesInStack(context->objectStack);
				if(newCount > *(context->objectTableSize))
					*(context->objectTableSize) = newCount;
				
				//assign to the index of the newly created variable
				addOperator(NoPL_BYTE_OBJECT_ASSIGN, context);
				NoPL_Index index = indexOfVariableInStack(declaredName, context->objectStack, declaredVar, context);
				addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//append the initialization
				addBytesToContext(initCtx.compiledData, initCtx.dataLength, context);
				freeInnerCompileContext(&initCtx);
			}
				break;
			case DECL_STRING:
			{
				//get the name of the declared variable
				pANTLR3_BASE_TREE declaredVar = treeIndex(tree,0);
				pANTLR3_STRING declaredName = declaredVar->getText(declaredVar);
				
				//we need to evaluate the initial expression before declaring the variable
				NoPL_CompileContext initCtx = newInnerCompileContext(context, 0, 0);
				
				//get the initial value if there is one
				if(tree->getChildCount(tree) > 1)
				{
					//append the expression
					appendNodeWithRequiredType(treeIndex(tree,1), NoPL_type_String, &initCtx, options);
				}
				else
				{
					//set this variable to a default value
					addOperator(NoPL_BYTE_LITERAL_STRING, &initCtx);
					char* defaultString = "";
					addBytesToContext(&defaultString[0], sizeof(defaultString[0]), &initCtx);
				}
				
				//check if the variable was already declared elsewhere
				if(nopl_variableExistsInContext(declaredName, context))
					nopl_error(declaredVar, NoPL_ErrStr_VariableAlreadyDeclared, context);
				
				//declare the variable
				nopl_declareVariableInStack(declaredName, context->stringStack);
				NoPL_Index newCount = nopl_countVariablesInStack(context->stringStack);
				if(newCount > *(context->stringTableSize))
					*(context->stringTableSize) = newCount;
				
				//assign to the index of the newly created variable
				addOperator(NoPL_BYTE_STRING_ASSIGN, context);
				NoPL_Index index = indexOfVariableInStack(declaredName, context->stringStack, declaredVar, context);
				addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//append the initialization
				addBytesToContext(initCtx.compiledData, initCtx.dataLength, context);
				freeInnerCompileContext(&initCtx);
			}
				break;
			case DECREMENT:
			{
				//add the operator
				addOperator(NoPL_BYTE_NUMERIC_DECREMENT, context);
				
				//get the variable being changed
				pANTLR3_BASE_TREE var = treeIndex(tree,0);
				
				//check the data type of the variable
				if(dataTypeForTree(var, context) != NoPL_type_Number)
					nopl_error(var, NoPL_ErrStr_CannotIncrement, context);
				
				//append the index for the variable
				NoPL_Index index = indexOfVariableInStack(var->getText(var), context->numberStack, var, context);
				addBytesToContext(&index, sizeof(NoPL_Index), context);
			}
				break;
			case DIVIDE:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				addOperator(NoPL_BYTE_NUMERIC_DIVIDE, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case DIVIDE_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					addOperator(NoPL_BYTE_NUMERIC_DIVIDE_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->numberStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
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
				addOperator(NoPL_BYTE_PROGRAM_EXIT, context);
				break;
			case EXPONENT:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				addOperator(NoPL_BYTE_NUMERIC_EXPONENT, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case EXPONENT_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					addOperator(NoPL_BYTE_NUMERIC_EXPONENT_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->numberStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
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
				appendFunctionCall(NULL, treeIndex(tree, 0), args, options, context);
			}
				break;
			case GREATER_THAN:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				addOperator(NoPL_BYTE_BOOLEAN_GREATER_THAN, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case GREATER_THAN_EQUAL:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				addOperator(NoPL_BYTE_BOOLEAN_GREATER_THAN_EQUAL, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case ID:
			{
				//check the type of the ID
				switch(dataTypeForTree(tree, context))
				{
					case NoPL_type_Boolean:
					{
						//append the operator
						addOperator(NoPL_BYTE_VARIABLE_BOOLEAN, context);
						
						//append the index of the variable
						NoPL_Index varIndex = indexOfVariableInStack(tree->getText(tree), context->booleanStack, tree, context);
						addBytesToContext(&varIndex, sizeof(NoPL_Index), context);
					}
						break;
					case NoPL_type_Number:
					{
						//append the operator
						addOperator(NoPL_BYTE_VARIABLE_NUMBER, context);
						
						//append the index of the variable
						NoPL_Index varIndex = indexOfVariableInStack(tree->getText(tree), context->numberStack, tree, context);
						addBytesToContext(&varIndex, sizeof(NoPL_Index), context);
					}
						break;
					case NoPL_type_Object:
					{
						//append the operator
						addOperator(NoPL_BYTE_VARIABLE_OBJECT, context);
						
						//append the index of the variable
						NoPL_Index varIndex = indexOfVariableInStack(tree->getText(tree), context->objectStack, tree, context);
						addBytesToContext(&varIndex, sizeof(NoPL_Index), context);
					}
						break;
					case NoPL_type_String:
					{
						//append the operator
						addOperator(NoPL_BYTE_VARIABLE_STRING, context);
						
						//append the index of the variable
						NoPL_Index varIndex = indexOfVariableInStack(tree->getText(tree), context->stringStack, tree, context);
						addBytesToContext(&varIndex, sizeof(NoPL_Index), context);
					}
						break;
					default:
					{
						//this is a global function without args
						appendFunctionCall(NULL, tree, NULL, options, context);
					}
						break;
				}
			}
				break;
			case INCREMENT:
			{
				//add the operator
				addOperator(NoPL_BYTE_NUMERIC_INCREMENT, context);
				
				//get the variable being changed
				pANTLR3_BASE_TREE var = treeIndex(tree,0);
				
				//check the data type of the variable
				if(dataTypeForTree(var, context) != NoPL_type_Number)
					nopl_error(var, NoPL_ErrStr_CannotIncrement, context);
				
				//append the index for the variable
				NoPL_Index index = indexOfVariableInStack(var->getText(var), context->numberStack, var, context);
				addBytesToContext(&index, sizeof(NoPL_Index), context);
			}
				break;
			case LESS_THAN:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				addOperator(NoPL_BYTE_BOOLEAN_LESS_THAN, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case LESS_THAN_EQUAL:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				addOperator(NoPL_BYTE_BOOLEAN_LESS_THAN_EQUAL, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case LITERAL_FALSE:
			{
				addOperator(NoPL_BYTE_LITERAL_BOOLEAN_FALSE, context);
			}
				break;
			case LITERAL_NULL:
			{
				addOperator(NoPL_BYTE_LITERAL_NULL, context);
			}
				break;
			case LITERAL_TRUE:
			{
				addOperator(NoPL_BYTE_LITERAL_BOOLEAN_TRUE, context);
			}
				break;
			case LOGICAL_AND:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				addOperator(NoPL_BYTE_BOOLEAN_AND, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case LOGICAL_EQUALITY:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//get the types of expressions being compared
				NoPL_DataType child1Type = dataTypeForTree(child1, context);
				NoPL_DataType child2Type = dataTypeForTree(child2, context);
				
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
							addOperator(NoPL_BYTE_NUMERIC_LOGICAL_EQUALITY, context);
							break;
						case NoPL_type_Boolean:
							addOperator(NoPL_BYTE_BOOLEAN_LOGICAL_EQUALITY, context);
							break;
						case NoPL_type_Object:
							addOperator(NoPL_BYTE_OBJECT_LOGICAL_EQUALITY, context);
							break;
						case NoPL_type_String:
							addOperator(NoPL_BYTE_STRING_LOGICAL_EQUALITY, context);
							break;
						default:
							nopl_error(tree, NoPL_ErrStr_CouldNotDetermineType, context);
							break;
					}
					
					//append the expressions
					appendNodeWithRequiredType(child1, exprType, context, options);
					appendNodeWithRequiredType(child2, exprType, context, options);
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
				NoPL_DataType child1Type = dataTypeForTree(child1, context);
				NoPL_DataType child2Type = dataTypeForTree(child2, context);
				
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
							addOperator(NoPL_BYTE_NUMERIC_LOGICAL_INEQUALITY, context);
							break;
						case NoPL_type_Boolean:
							addOperator(NoPL_BYTE_BOOLEAN_LOGICAL_INEQUALITY, context);
							break;
						case NoPL_type_Object:
							addOperator(NoPL_BYTE_OBJECT_LOGICAL_INEQUALITY, context);
							break;
						case NoPL_type_String:
							addOperator(NoPL_BYTE_STRING_LOGICAL_INEQUALITY, context);
							break;
						default:
							nopl_error(tree, NoPL_ErrStr_CouldNotDetermineType, context);
							break;
					}
					
					//append the expressions
					appendNodeWithRequiredType(child1, exprType, context, options);
					appendNodeWithRequiredType(child2, exprType, context, options);
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
				addOperator(NoPL_BYTE_BOOLEAN_NEGATION, context);
				
				//append the expression
				appendNodeWithRequiredType(child1, NoPL_type_Boolean, context, options);
			}
				break;
			case LOGICAL_OR:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//append the operator
				addOperator(NoPL_BYTE_BOOLEAN_OR, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case LOOP_DO:
			{
				//get nodes for statements in the top of the loop (or in this case, the bottom)
				pANTLR3_BASE_TREE conditional = treeIndex(tree,0);
				
				//get another byte buffer for everything inside this loop
				NoPL_CompileContext loopCtx = newInnerCompileContext(context, 1, 1);
				
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
					traverseAST(childArg, options, &loopCtx);
				}
				
				//pop a scope for the loop
				nopl_popScope(&loopCtx);
				
				//a loop gets compiled as a repeating conditional
				addOperator(NoPL_BYTE_CONDITIONAL, &loopCtx);
				
				//append the boolean expression for the conditional
				appendNodeWithRequiredType(conditional, NoPL_type_Boolean, &loopCtx, options);
				
				//add the number of bytes to skip when the conditional fails
				NoPL_BufferMove condMove = sizeof(NoPL_Instruction)+sizeof(NoPL_BufferMove);
				addBytesToContext(&condMove, sizeof(NoPL_BufferMove), &loopCtx);
				
				//the only content of the conditional is a buffer move that restarts the loop
				NoPL_BufferMove loopMove = -((NoPL_BufferMove)(loopCtx.dataLength));
				addBytesToContext(&loopMove, sizeof(NoPL_BufferMove), &loopCtx);
				
				//append the loop
				addBytesToContext(loopCtx.compiledData, loopCtx.dataLength, context);
				
				freeInnerCompileContext(&loopCtx);
			}
				break;
			case LOOP_FOR:
			{
				//push a scope for the statements in the top of the loop
				nopl_pushScope(context);
				
				//get nodes for statements in the top of the loop
				pANTLR3_BASE_TREE declaration = treeIndex(tree,0);
				pANTLR3_BASE_TREE conditional = treeIndex(tree,1);
				pANTLR3_BASE_TREE increment = treeIndex(tree,2);
				
				//append the declaration
				traverseAST(declaration, options, context);
				
				//get a byte buffer for everything at the top of this loop
				NoPL_CompileContext outerLoopCtx = newInnerCompileContext(context, 0, 0);
				
				//a loop gets compiled as a repeating conditional
				addOperator(NoPL_BYTE_CONDITIONAL, &outerLoopCtx);
				
				//append the boolean expression for the conditional
				appendNodeWithRequiredType(conditional, NoPL_type_Boolean, &outerLoopCtx, options);
				
				//get another byte buffer for everything inside this loop
				NoPL_CompileContext innerLoopCtx = newInnerCompileContext(&outerLoopCtx, 1, 1);
				
				//push a scope for the loop
				nopl_pushScope(&innerLoopCtx);
				
				//add all of the statements inside the loop
				NoPL_Index childCount = 0;
				if(tree->children)
					childCount = (NoPL_Index)tree->children->size(tree->children);
				ANTLR3_UINT32 i;
				pANTLR3_BASE_TREE childArg;
				for(i = 3; i < childCount; i++)
				{
					childArg = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
					traverseAST(childArg, options, &innerLoopCtx);
				}
				
				//append the increment at the end of the loop
				traverseAST(increment, options, &innerLoopCtx);
				
				//pop a scope for the loop
				nopl_popScope(&innerLoopCtx);
				
				//go back to the beginning of the loop
				addOperator(NoPL_BYTE_BUFFER_MOVE, &innerLoopCtx);
				NoPL_BufferMove repeatMove = -((NoPL_BufferMove)(outerLoopCtx.dataLength+innerLoopCtx.dataLength+(2*sizeof(NoPL_BufferMove))));
				addBytesToContext(&repeatMove, sizeof(NoPL_BufferMove), &innerLoopCtx);
				
				//add the number of bytes to skip when the conditional fails
				NoPL_BufferMove condMove = (NoPL_BufferMove)innerLoopCtx.dataLength;
				addBytesToContext(&condMove, sizeof(NoPL_BufferMove), &outerLoopCtx);
				
				//append the logic for the loop
				addBytesToContext(outerLoopCtx.compiledData, outerLoopCtx.dataLength, context);
				
				//append the loop contents
				addBytesToContext(innerLoopCtx.compiledData, innerLoopCtx.dataLength, context);
				
				//pop the scope for the statements in the top of the loop
				nopl_popScope(context);
				
				//free contexts
				freeInnerCompileContext(&innerLoopCtx);
				freeInnerCompileContext(&outerLoopCtx);
			}
				break;
			case LOOP_WHILE:
			{
				//get nodes for statements in the top of the loop
				pANTLR3_BASE_TREE conditional = treeIndex(tree,0);
				
				//get a byte buffer for everything at the top of this loop
				NoPL_CompileContext outerLoopCtx = newInnerCompileContext(context, 0, 0);
				
				//a loop gets compiled as a repeating conditional
				addOperator(NoPL_BYTE_CONDITIONAL, &outerLoopCtx);
				
				//append the boolean expression for the conditional
				appendNodeWithRequiredType(conditional, NoPL_type_Boolean, &outerLoopCtx, options);
				
				//get another byte buffer for everything inside this loop
				NoPL_CompileContext innerLoopCtx = newInnerCompileContext(&outerLoopCtx, 1, 1);
				
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
					traverseAST(childArg, options, &innerLoopCtx);
				}
				
				//pop a scope for the loop
				nopl_popScope(&innerLoopCtx);
				
				//go back to the beginning of the loop
				addOperator(NoPL_BYTE_BUFFER_MOVE, &innerLoopCtx);
				NoPL_BufferMove repeatMove = -((NoPL_BufferMove)(outerLoopCtx.dataLength+innerLoopCtx.dataLength+(2*sizeof(NoPL_BufferMove))));
				addBytesToContext(&repeatMove, sizeof(NoPL_BufferMove), &innerLoopCtx);
				
				//add the number of bytes to skip when the conditional fails
				NoPL_BufferMove condMove = (NoPL_BufferMove)innerLoopCtx.dataLength;
				addBytesToContext(&condMove, sizeof(NoPL_BufferMove), &outerLoopCtx);
				
				//append the logic for the loop
				addBytesToContext(outerLoopCtx.compiledData, outerLoopCtx.dataLength, context);
				
				//append the loop contents
				addBytesToContext(innerLoopCtx.compiledData, innerLoopCtx.dataLength, context);
				
				//free contexts
				freeInnerCompileContext(&innerLoopCtx);
				freeInnerCompileContext(&outerLoopCtx);
			}
				break;
			case MOD:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				addOperator(NoPL_BYTE_NUMERIC_MODULO, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case MOD_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					addOperator(NoPL_BYTE_NUMERIC_MODULO_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->numberStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
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
				addOperator(NoPL_BYTE_NUMERIC_MULTIPLY, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case MULTIPLY_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					addOperator(NoPL_BYTE_NUMERIC_MULTIPLY_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->numberStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the increment
					appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
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
				float floatVal = atof(((const char*)tree->getText(tree)->chars));
				
				//add the operator
				addOperator(NoPL_BYTE_LITERAL_NUMBER, context);
				
				//add the float
				addBytesToContext(&floatVal, sizeof(float), context);
			}
				break;
			case NUMERIC_NEGATION:
			{
				//this operator should have one numeric child
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				
				//append the operator
				addOperator(NoPL_BYTE_NUMERIC_NEGATION, context);
				
				//append the expression
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
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
				appendFunctionCall(treeIndex(tree,0), funcName, args, options, context);
			}
				break;
			case PRINT_VALUE:
			{
				//this operator should only have one child
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				
				//add the operator
				addOperator(NoPL_BYTE_STRING_PRINT, context);
				
				//guarantee that the child is a string by casting if necessary
				switch(dataTypeForTree(child1, context))
				{
					case NoPL_type_Boolean:
						addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
						break;
					case NoPL_type_FunctionResult:
						addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
						break;
					case NoPL_type_Number:
						addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
						break;
					case NoPL_type_String:
						break;
					case NoPL_type_Object:
						nopl_error(child1, NoPL_ErrStr_CannotImplicitCastObject, context);
						break;
					default:
						nopl_error(child1, NoPL_ErrStr_CouldNotDetermineType, context);
						break;
				}
				
				//append the expression
				traverseAST(child1, options, context);
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
					traverseAST(child, options, context);
				}
				
				//pop the scope
				nopl_popScope(context);
			}
				break;
			case STRING:
			{
				//get the string and length
				char* string = (char*)tree->getText(tree)->chars;
				int length = strlen(string);
				
				//strip the quotes
				char stringCopy[length];
				strcpy(stringCopy, (string+1));
				stringCopy[length-2] = 0;
				
				//add the operator
				addOperator(NoPL_BYTE_LITERAL_STRING, context);
				
				//add the string
				addBytesToContext(stringCopy, sizeof(char)*(length-1), context);
			}
				break;
			case SUBSCRIPT_OPEN:
			{
				//get the index and object
				pANTLR3_BASE_TREE object = treeIndex(tree,0);
				pANTLR3_BASE_TREE index = treeIndex(tree,1);
				
				//add the operator
				addOperator(NoPL_BYTE_FUNCTION_INDEX, context);
				
				//append the object expression
				appendNodeWithRequiredType(object, NoPL_type_Object, context, options);
				
				//we can only index numbers or strings
				NoPL_DataType indexType = dataTypeForTree(index, context);
				if(indexType == NoPL_type_FunctionResult)
				{
					nopl_error(index, "This expression must be cast to either a numeric or string value", context);
				}
				if(indexType != NoPL_type_Number && indexType != NoPL_type_String)
				{
					nopl_error(index, "Cannot use an expression of this type as an index", context);
				}
				
				//append the index expression
				traverseAST(index, options, context);
			}
				break;
			case SUBTRACT:
			{
				//this is a binary operator
				pANTLR3_BASE_TREE child1 = treeIndex(tree,0);
				pANTLR3_BASE_TREE child2 = treeIndex(tree,1);
				
				//add the operator
				addOperator(NoPL_BYTE_NUMERIC_SUBTRACT, context);
				
				//append the expressions
				appendNodeWithRequiredType(child1, NoPL_type_Number, context, options);
				appendNodeWithRequiredType(child2, NoPL_type_Number, context, options);
			}
				break;
			case SUBTRACT_ASSIGN:
			{
				//this is an assignment operator
				pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
				pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
				
				//check the object on the left-hand side to make sure that it is the correct type
				NoPL_DataType assignToType = dataTypeForTree(assignTo, context);
				if(assignToType == NoPL_type_Number)
				{
					//append the operator
					addOperator(NoPL_BYTE_NUMERIC_SUBTRACT_ASSIGN, context);
					
					//add the index for the variable
					NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->numberStack, assignTo, context);
					addBytesToContext(&index, sizeof(NoPL_Index), context);
					
					//append the expression
					appendNodeWithRequiredType(incrementVal, NoPL_type_Number, context, options);
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
				//TODO:
			}
				break;
			case TYPE_CAST:
			{
				//get the expression which will be cast
				pANTLR3_BASE_TREE expression = treeIndex(tree,1);
				
				NoPL_DataType castType = dataTypeForTree(tree, context);
				NoPL_DataType expressionType = dataTypeForTree(tree, context);
				
				//only add this cast if the types are actually different
				if(castType != expressionType)
				{
					switch (castType)
					{
						case NoPL_type_Boolean:
							
							//cast to boolean
							if(expressionType == NoPL_type_FunctionResult)
								addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_BOOLEAN, context);
							else if(expressionType == NoPL_type_Number)
								addOperator(NoPL_BYTE_CAST_NUMBER_TO_BOOLEAN, context);
							else if(expressionType == NoPL_type_String)
								addOperator(NoPL_BYTE_CAST_STRING_TO_BOOLEAN, context);
							else if(expressionType == NoPL_type_Object)
								nopl_error(expression, NoPL_ErrStr_CannotCastObjectToPrimitive, context);
							else
								nopl_error(expression, NoPL_ErrStr_CouldNotDetermineType, context);
							
							break;
						case NoPL_type_Number:
							
							//cast to number
							if(expressionType == NoPL_type_FunctionResult)
								addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_NUMBER, context);
							else if(expressionType == NoPL_type_Boolean)
								addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_NUMBER, context);
							else if(expressionType == NoPL_type_String)
								addOperator(NoPL_BYTE_CAST_STRING_TO_NUMBER, context);
							else if(expressionType == NoPL_type_Object)
								nopl_error(expression, NoPL_ErrStr_CannotCastObjectToPrimitive, context);
							else
								nopl_error(expression, NoPL_ErrStr_CouldNotDetermineType, context);
							
							break;
						case NoPL_type_String:
							
							//cast to number
							if(expressionType == NoPL_type_FunctionResult)
								addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_STRING, context);
							else if(expressionType == NoPL_type_Boolean)
								addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
							else if(expressionType == NoPL_type_Number)
								addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
							else if(expressionType == NoPL_type_Object)
								nopl_error(expression, NoPL_ErrStr_CannotCastObjectToPrimitive, context);
							else
								nopl_error(expression, NoPL_ErrStr_CouldNotDetermineType, context);
							
							break;
						case NoPL_type_Object:
							
							//can only cast to an object when dealing with values returned from an external function
							if(expressionType == NoPL_type_FunctionResult)
								addOperator(NoPL_BYTE_RESOLVE_RESULT_TO_OBJECT, context);
							else
								nopl_error(tree, "Cannot cast non-Object primitives to Object", context);
							
							break;
						default:
							break;
					}
				}
				
				//append the expression
				traverseAST(expression, options, context);
			}
				break;
		}
	}
}

void traverseForErrors(const pANTLR3_BASE_TREE tree, NoPL_CompileContext* context)
{
	//check if this node is an error
	if(tree->isNilNode && !strcmp("Tree Error Node", (char*)tree->getText(tree)->chars))
		nopl_error(tree, NoPL_ErrStr_Generic, context);
	
	//recursively check all nodes in this AST
	NoPL_Index childCount = 0;
	if(tree->children)
		childCount = (NoPL_Index)tree->children->size(tree->children);
	ANTLR3_UINT32 i;
	pANTLR3_BASE_TREE child;
	for(i = 0; i < childCount; i++)
	{
		//get each child and append
		child = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
		traverseForErrors(child, context);
	}
}

void compileWithInputStream(pANTLR3_INPUT_STREAM stream, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//attempt to parse the NoPL program
	pNoPLLexer lex = NoPLLexerNew(stream);
	context->tokenStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
	pNoPLParser parser = NoPLParserNew(context->tokenStream);
	NoPLParser_program_return syntaxTree = parser->program(parser);
	
	//TODO: pre-process the AST for optimization here?
	
	//check for errors
	pANTLR3_BASE_RECOGNIZER recognizer = parser->pParser->rec;
	int errCount = recognizer->getNumberOfSyntaxErrors(recognizer);
	if(errCount > 0)
	{
		//this tree has errors, add them to the context
		traverseForErrors(syntaxTree.tree, context);
	}
	else
	{
		//set up counts for symbol table
		int symbolTableByteSize = 4*(sizeof(NoPL_Instruction)+sizeof(NoPL_Index));
		NoPL_Instruction symbolTable[symbolTableByteSize];
		NoPL_Index objectTableSize = 0;
		NoPL_Index numberTableSize = 0;
		NoPL_Index booleanTableSize = 0;
		NoPL_Index stringTableSize = 0;
		context->objectTableSize = &objectTableSize;
		context->numberTableSize = &numberTableSize;
		context->booleanTableSize = &booleanTableSize;
		context->stringTableSize = &stringTableSize;
		
		//make some space for the symbol table at the beginning of the buffer
		addBytesToContext(symbolTable, symbolTableByteSize, context);
		
		//recurse to assemble the byte code
		traverseAST(syntaxTree.tree, options, context);
		
		//add the actual values for the symbol table sizes
		int bytePos = 0;
		if(objectTableSize > 0)
		{
			context->compiledData[bytePos] = (NoPL_Instruction)NoPL_BYTE_OBJECT_TABLE_SIZE;
			bytePos += sizeof(NoPL_Instruction);
			context->compiledData[bytePos] = objectTableSize;
			bytePos += sizeof(NoPL_Index);
		}
		if(numberTableSize > 0)
		{
			context->compiledData[bytePos] = (NoPL_Instruction)NoPL_BYTE_NUMERIC_TABLE_SIZE;
			bytePos += sizeof(NoPL_Instruction);
			context->compiledData[bytePos] = numberTableSize;
			bytePos += sizeof(NoPL_Index);
		}
		if(booleanTableSize > 0)
		{
			context->compiledData[bytePos] = (NoPL_Instruction)NoPL_BYTE_BOOLEAN_TABLE_SIZE;
			bytePos += sizeof(NoPL_Instruction);
			context->compiledData[bytePos] = booleanTableSize;
			bytePos += sizeof(NoPL_Index);
		}
		if(stringTableSize > 0)
		{
			context->compiledData[bytePos] = (NoPL_Instruction)NoPL_BYTE_STRING_TABLE_SIZE;
			bytePos += sizeof(NoPL_Instruction);
			context->compiledData[bytePos] = stringTableSize;
		}
	}
	
	//clean up
	parser->free(parser);
	context->tokenStream->free(context->tokenStream);
	lex->free(lex);
}

#pragma mark -
#pragma mark Interface

void compileContextWithFilePath(const char* path, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//run the compiler with an input stream created from URL path
	pANTLR3_INPUT_STREAM inputStream = antlr3AsciiFileStreamNew((pANTLR3_UINT8)path);
	compileWithInputStream(inputStream, options, context);
	inputStream->free(inputStream);
}

void compileContextWithString(const char* scriptString, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//run the compiler with an input stream created from a C string
	pANTLR3_INPUT_STREAM inputStream = antlr3NewAsciiStringCopyStream((pANTLR3_UINT8)scriptString, (int)strlen((const char*)scriptString), 0);
	compileWithInputStream(inputStream, options, context);
	inputStream->free(inputStream);
}

NoPL_CompileContext newNoPL_CompileContext()
{
	NoPL_CompileContext context;
	context.compiledData = NULL;
	context.dataLength = 0;
	context.arrayLength = 0;
	context.objectStack = antlr3StackNew(NoPL_StackSizeHint);
	context.numberStack = antlr3StackNew(NoPL_StackSizeHint);
	context.booleanStack = antlr3StackNew(NoPL_StackSizeHint);
	context.stringStack = antlr3StackNew(NoPL_StackSizeHint);
	context.errDescriptions = NULL;
	context.breakStatements = NULL;
	context.continueStatements = NULL;
	nopl_pushScope(&context);
	return context;
}

void freeNoPL_CompileContext(NoPL_CompileContext* context)
{
	if(context->compiledData)
		free(context->compiledData);
	context->dataLength = 0;
	context->arrayLength = 0;
	if(context->objectStack)
	{
		context->objectStack->free(context->objectStack);
		context->objectStack = NULL;
	}
	if(context->numberStack)
	{
		context->numberStack->free(context->numberStack);
		context->numberStack = NULL;
	}
	if(context->booleanStack)
	{
		context->booleanStack->free(context->booleanStack);
		context->booleanStack = NULL;
	}
	if(context->stringStack)
	{
		context->stringStack->free(context->stringStack);
		context->stringStack = NULL;
	}
}
