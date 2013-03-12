//
//  NoPLStandardFunctions.h
//  NoPL_Runtime
//
//  Created by Brad Bambara on 8/11/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#ifndef NoPL_Runtime_NoPLStandardFunctions_h
#define NoPL_Runtime_NoPLStandardFunctions_h

#ifdef __cplusplus
extern "C" {
#endif

#include "NoPLRuntime.h"

//function that handles all standard function calls
NoPL_FunctionValue nopl_standardFunctions(const void* calledOnObject, const char* functionName, const NoPL_FunctionValue* argv, unsigned int argc);

//NoPL FUNCTIONS IMPLEMENTED IN THIS FILE:
/*

//STRINGS:
String format(String format, ...); -- use only '%' character for each variable, example: #format("My num is %", 5)
Number length(String str);
Number fastHash(String str);
String toLower(String str);
String toUpper(String str);
String substring(String str, Number startIndex);
String substring(String str, Number startIndex, Number length);
Number indexOf(String searched, String match);
Number lastIndexOf(String searched, String match);
String replaceAll(String original, String match, String replaceWith);
String replaceFirst(String original, String match, String replaceWith);
String replaceLast(String original, String match, String replaceWith);
String capitalize(String str);

//CURRENT TIME:
String timeDesc();
String timeZone();
Number year();
Number month();
Number dayOfTheYear();
Number dayOfTheMonth();
Number dayOfTheWeek();
Number hour();
Number minute();
Number second();

//MATH:
Number PI();
Number e();
Number abs();
Number degreeToRadian();
Number degreeToRadian(Number degree);
Number radianToDegree();
Number radianToDegree(Number radian);
Number sqrt(Number num);
Number sin(Number num);
Number cos(Number num);
Number tan(Number num);
Number asin(Number num);
Number acos(Number num);
Number atan(Number num);
Number atan2(Number dY, Number dX);
Number random();
Number random(Number scalar);
Number random(Number min, Number max);
Number log(Number num);
Number log(Number num, Number base);
Number round(Number num);
Number ceil(Number num);
Number floor(Number num);
*/

#ifdef __cplusplus
}
#endif

#endif
