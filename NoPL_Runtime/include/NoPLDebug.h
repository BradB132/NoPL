//
//  NoPLDebug.h
//  NoPL_Runtime
//
//  Created by Brad Bambara on 1/2/14.
//  Copyright (c) 2014 Brad Bambara. All rights reserved.
//

#ifndef NoPL_Runtime_NoPLDebug_h
#define NoPL_Runtime_NoPLDebug_h

#ifdef __cplusplus
extern "C" {
#endif

#import "NoPLRuntime.h"
	
#pragma mark - Debug API

typedef void* NoPL_DebugHandle;

NoPL_DebugHandle createNoPL_DebugHandle(const NoPL_Instruction* scriptBuffer, uint32_t bufferLength, const NoPL_Callbacks* callbacks, void* context);
void freeNoPL_DebugHandle(NoPL_DebugHandle handle);
int nopl_debugStep(NoPL_DebugHandle handle);
NoPL_FunctionValue nopl_queryValue(NoPL_DebugHandle handle, NoPL_DataType type, NoPL_Index index);

#ifdef __cplusplus
}
#endif

#endif
