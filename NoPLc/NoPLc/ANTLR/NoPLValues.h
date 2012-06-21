//
//  NoPLByteValues.h
//  NoPLc
//
//  Created by Brad Bambara on 4/26/12.
//  Copyright (c) 2012 Small Planet Digital. All rights reserved.
//

#ifndef NoPLc_NoPLByteValues_h
#define NoPLc_NoPLByteValues_h

//typedefs for NoPL-specific types
typedef unsigned short NoPL_Index;
typedef unsigned char NoPL_Instruction;

typedef enum
{
	NoPL_type_Number,
	NoPL_type_String,
	NoPL_type_Boolean,
	NoPL_type_Object,
	NoPL_type_FunctionResult,
	NoPL_type_Error,
} NoPL_DataType;

//SYMBOL TABLES

//symbol table total sizes
#define NoPL_BYTE_BOOLEAN_TABLE_SIZE 1
#define NoPL_BYTE_NUMERIC_TABLE_SIZE 2
#define NoPL_BYTE_STRING_TABLE_SIZE 3

//listing of constant values
#define NoPL_BYTE_NUMERIC_CONSTANTS 4
#define NoPL_BYTE_STRING_CONSTANTS 5

//LITERALS

//literal primitives
#define NoPL_BYTE_LITERAL_NUMBER 6
#define NoPL_BYTE_LITERAL_BOOLEAN_TRUE 7
#define NoPL_BYTE_LITERAL_BOOLEAN_FALSE 8
#define NoPL_BYTE_LITERAL_STRING 9
#define NoPL_BYTE_LITERAL_NULL 10

//DECLARATIONS

#define NoPL_BYTE_DECLARE_NUMBER 11
#define NoPL_BYTE_DECLARE_BOOLEAN 12
#define NoPL_BYTE_DECLARE_STRING 13
#define NoPL_BYTE_DECLARE_OBJECT 14

//NUMERIC OPERATORS

//binary numeric operators
#define NoPL_BYTE_NUMERIC_ADD 15
#define NoPL_BYTE_NUMERIC_SUBTRACT 16
#define NoPL_BYTE_NUMERIC_DIVIDE 17
#define NoPL_BYTE_NUMERIC_MULTIPLY 18
#define NoPL_BYTE_NUMERIC_EXPONENT 19
#define NoPL_BYTE_NUMERIC_MODULO 20

//unary numeric operators
#define NoPL_BYTE_NUMERIC_INCREMENT 21
#define NoPL_BYTE_NUMERIC_DECREMENT 22

//numeric assignment operators
#define NoPL_BYTE_NUMERIC_ASSIGN 23
#define NoPL_BYTE_NUMERIC_ADD_ASSIGN 24
#define NoPL_BYTE_NUMERIC_SUBTRACT_ASSIGN 25
#define NoPL_BYTE_NUMERIC_DIVIDE_ASSIGN 26
#define NoPL_BYTE_NUMERIC_MULTIPLY_ASSIGN 27
#define NoPL_BYTE_NUMERIC_EXPONENT_ASSIGN 28
#define NoPL_BYTE_NUMERIC_MODULO_ASSIGN 29

//BOOLEAN OPERATORS

//binary boolean operators
#define NoPL_BYTE_BOOLEAN_EQUALITY 30
#define NoPL_BYTE_BOOLEAN_INEQUALITY 31
#define NoPL_BYTE_BOOLEAN_AND 32
#define NoPL_BYTE_BOOLEAN_OR 33
#define NoPL_BYTE_BOOLEAN_LESS_THAN 34
#define NoPL_BYTE_BOOLEAN_LESS_THAN_EQUAL 35
#define NoPL_BYTE_BOOLEAN_GREATER_THAN 36
#define NoPL_BYTE_BOOLEAN_GREATER_THAN_EQUAL 37

//unary boolean operators
#define NoPL_BYTE_BOOLEAN_NEGATION 38

//boolean assignment operators
#define NoPL_BYTE_BOOLEAN_ASSIGN 39

//STRING OPERATORS

//binary string operators
#define NoPL_BYTE_STRING_CONCAT 40

//string assignment operators
#define NoPL_BYTE_STRING_ASSIGN 41
#define NoPL_BYTE_STRING_CONCAT_ASSIGN 42

//OBJECT OPERATORS

#define NoPL_BYTE_OBJECT_TO_MEMBER 43
#define NoPL_BYTE_FUNCTION_ARG_COUNT 44
#define NoPL_BYTE_SUBSCRIPT 45

//CONTROL FLOW

#define NoPL_BYTE_CONDITIONAL 46
#define NoPL_BYTE_BUFFER_MOVE 47
#define NoPL_BYTE_PROGRAM_EXIT 48

//CASTING

#define NoPL_BYTE_CAST_OBJECT_TO_NUMBER 49
#define NoPL_BYTE_CAST_OBJECT_TO_BOOLEAN 50
#define NoPL_BYTE_CAST_OBJECT_TO_STRING 51

#define NoPL_BYTE_CAST_NUMBER_TO_BOOLEAN 52
#define NoPL_BYTE_CAST_NUMBER_TO_STRING 53

#define NoPL_BYTE_CAST_BOOLEAN_TO_NUMBER 54
#define NoPL_BYTE_CAST_BOOLEAN_TO_STRING 55

#define NoPL_BYTE_CAST_STRING_TO_NUMBER 56
#define NoPL_BYTE_CAST_STRING_TO_BOOLEAN 57

//DEBUGGING

#define NoPL_BYTE_DEBUG_LINE 58

#endif