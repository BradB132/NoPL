//
//  NoPLc.c
//  NoPLc
//
//  Created by Brad Bambara on 4/15/12.
//  Copyright (c) 2012 Small Planet Digital. All rights reserved.
//

#include <stdio.h>
#include "NoPLLexer.h"
#include "NoPLParser.h"
#include "NoPLc.h"

#define treeIndex(tree,index) (pANTLR3_BASE_TREE)(tree->children->get(tree->children, index))
#define NoPL_StackSizeHint 16
#define NoPL_VectorSizeHint 8

#pragma mark -
#pragma mark Internal functions

void compileWithInputStream(pANTLR3_INPUT_STREAM stream, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void addBytesToContext(const void* bytes, int byteCount, NoPL_CompileContext* context);
void addOperator(NoPL_Instruction operator, NoPL_CompileContext* context);
void traverseAST(pANTLR3_BASE_TREE tree, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
NoPL_DataType dataTypeForTree(const pANTLR3_BASE_TREE tree, NoPL_CompileContext* context);
void nopl_pushScope(NoPL_CompileContext* context);
void nopl_popScope(NoPL_CompileContext* context);
void nopl_error(const pANTLR3_BASE_TREE tree, const char* desc);
int variableExistsInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack);
int declareVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack);
NoPL_Index indexOfVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack, const pANTLR3_BASE_TREE tree);

#pragma mark -
#pragma mark Compilation

void nopl_error(const pANTLR3_BASE_TREE tree, const char* desc)
{
	//TODO: do something useful here
}

void addBytesToContext(const void* bytes, int byteCount, NoPL_CompileContext* context)
{
	//check if the buffer size needs to be increased
	if(!context->arrayLength)
	{
		context->arrayLength = 512;
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
}

void addOperator(NoPL_Instruction operator, NoPL_CompileContext* context)
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

int variableExistsInStack(const pANTLR3_STRING varName, const  pANTLR3_STACK whichStack)
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

int declareVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack)
{
	//fail if this variable is already declared in the stack
	if(variableExistsInStack(varName, whichStack))
		return 0;
	
	pANTLR3_VECTOR topVector = whichStack->peek(whichStack);
	topVector->add(topVector, varName, NULL);
	
	return 1;
}

NoPL_Index indexOfVariableInStack(const pANTLR3_STRING varName, const pANTLR3_STACK whichStack, const pANTLR3_BASE_TREE tree)
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
	nopl_error(tree, "attempted to use a variable that was not declared");
	return (NoPL_Index)0;
}

NoPL_DataType dataTypeForTree(const pANTLR3_BASE_TREE tree, NoPL_CompileContext* context)
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
			else if(type1 == NoPL_type_Number && type2 == NoPL_type_Number)
				return NoPL_type_Number;
			else
				return NoPL_type_Error;
		}
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
			return dataTypeForTree(treeIndex(tree,0), context);
		case ID:
		{
			//check the type of this symbol in the variable stacks
			pANTLR3_STRING varName = tree->getText(tree);
			if(variableExistsInStack(varName, context->objectStack))
			   return NoPL_type_Object;
			else if(variableExistsInStack(varName, context->numberStack))
				return NoPL_type_Number;
			else if(variableExistsInStack(varName, context->booleanStack))
				return NoPL_type_Boolean;
			else if(variableExistsInStack(varName, context->stringStack))
				return NoPL_type_String;
			else
				return NoPL_type_FunctionResult;
		}
		case FUNCTION_CALL:
		case OBJECT_TO_MEMBER:
			return NoPL_type_FunctionResult;
		default:
			return NoPL_type_Error;
	}
}

void traverseAST(pANTLR3_BASE_TREE tree, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	switch(tree->getType(tree))
	{
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
				
				//add the expressions summed by the add
				traverseAST(child1, options, context);
				traverseAST(child2, options, context);
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
				{
					addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
				}
				else if(childType1 == NoPL_type_Boolean)
				{
					addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
				}
				else if(childType1 == NoPL_type_Object || childType1 == NoPL_type_FunctionResult)
				{
					addOperator(NoPL_BYTE_CAST_OBJECT_TO_STRING, context);
				}
				
				//add the first child
				traverseAST(child1, options, context);
				
				//cast the second child if we need to
				if(childType2 == NoPL_type_Number)
				{
					addOperator(NoPL_BYTE_CAST_NUMBER_TO_STRING, context);
				}
				else if(childType2 == NoPL_type_Boolean)
				{
					addOperator(NoPL_BYTE_CAST_BOOLEAN_TO_STRING, context);
				}
				else if(childType2 == NoPL_type_Object || childType2 == NoPL_type_FunctionResult)
				{
					addOperator(NoPL_BYTE_CAST_OBJECT_TO_STRING, context);
				}
				
				//add the second child
				traverseAST(child2, options, context);
			}
			else
			{
				nopl_error(tree, "something is wrong with the + operator");
			}
		}
			break;
		case ADD_ASSIGN:
		{
			//this is a unary operator
			pANTLR3_BASE_TREE assignTo = treeIndex(tree,0);
			pANTLR3_BASE_TREE incrementVal = treeIndex(tree,1);
			
			//check the object on the left-hand side to make sure that it is the correct type
			NoPL_DataType assignToType = dataTypeForTree(assignTo, context);
			NoPL_DataType incrementType = dataTypeForTree(incrementVal, context);
			if(assignToType == NoPL_type_Number)
			{
				//use the numeric increment
				addOperator(NoPL_BYTE_NUMERIC_ADD_ASSIGN, context);
				
				//add the index for the variable which will be incremented
				NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->numberStack, assignTo);
				addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//cast if necessary
				if(incrementType == NoPL_type_FunctionResult)
					addOperator(NoPL_BYTE_CAST_OBJECT_TO_NUMBER, context);
				else if(incrementType != NoPL_type_Number)
					nopl_error(incrementVal, "this expression must evaluate to a numeric value");
				
				//add the increment value
				traverseAST(incrementVal, options, context);
			}
			else if(assignToType == NoPL_type_String)
			{
				//use the string concat
				addOperator(NoPL_BYTE_STRING_CONCAT_ASSIGN, context);
				
				//add the index for the variable which will be incremented
				NoPL_Index index = indexOfVariableInStack(assignTo->getText(assignTo), context->stringStack, assignTo);
				addBytesToContext(&index, sizeof(NoPL_Index), context);
				
				//cast if necessary
				if(incrementType == NoPL_type_FunctionResult)
					addOperator(NoPL_BYTE_CAST_OBJECT_TO_STRING, context);
				else if(incrementType != NoPL_type_String)
					nopl_error(incrementVal, "this expression must evaluate to a string value");
				
				//add the increment value
				traverseAST(incrementVal, options, context);
			}
			else
			{
				nopl_error(assignTo, "Cannot use this increment operator on a variable of this type");
			}
		}
			break;
	}
	
	/*//example loop
	ANTLR3_UINT32 i;
	ANTLR3_UINT32 size;
	pANTLR3_BASE_TREE childTree;
	if(tree->children != NULL)
	{
		size = tree->children->size(tree->children);
		for	(i = 0; i < size; i++)
		{
			childTree = (pANTLR3_BASE_TREE)(tree->children->get(tree->children, i));
			traverseAST(childTree, options, context);
		}
	}
	//*/
}

void compileWithInputStream(pANTLR3_INPUT_STREAM stream, const NoPL_CompileOptions* options, NoPL_CompileContext* context)
{
	//clear any previous context
	freeNoPL_CompileContext(context);
	*context = newNoPL_CompileContext();
	
	//set up a parser using the input stream
	pNoPLLexer lex = NoPLLexerNew(stream);
	pANTLR3_COMMON_TOKEN_STREAM tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
	pNoPLParser parser = NoPLParserNew(tokens);
	
	//attempt to parse the NoPL program
	NoPLParser_program_return syntaxTree = parser->program(parser);
	
	//check for errors
	pANTLR3_BASE_RECOGNIZER recognizer = parser->pParser->rec;
	int errCount = recognizer->getNumberOfSyntaxErrors(recognizer);
	
	//recurse to assemble the byte code
	traverseAST(syntaxTree.tree, options, context);
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
	pANTLR3_INPUT_STREAM inputStream = antlr3NewAsciiStringCopyStream((pANTLR3_UINT8)scriptString, strlen((const char*)scriptString), 0);
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
