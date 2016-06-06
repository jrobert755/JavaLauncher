/*
 * Copyright (c) 2016, Jean-Luc Roberts
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

#include "jni_util.h"

JNI* allocateJNI(const char* jrePath) {
	JNI* ret = new JNI();
	ret->vm = NULL;
	ret->environment = NULL;
	ret->vm_arguments.nOptions = 0;
	ret->options = new JavaVMOption[1];
	ret->allocatedOptions = 1;
	ret->jrePath = new char[strlen(jrePath) + 1];
	strcpy(ret->jrePath, jrePath);
	return ret;
}

#ifdef _WIN32
int initializeJNIWindows(JNI* jni) {
	if (jni == NULL) return 0;
	char dllName[256];
	strcpy(dllName, jni->jrePath);
	strcat(dllName, "\\bin\\server\\jvm.dll");
	jni->jvmDll = LoadLibrary(dllName);
	if (jni->jvmDll == NULL) return 0;
	jni->createJVMFunction = (CreateJVM)GetProcAddress(jni->jvmDll, "JNI_CreateJavaVM");
	if (jni->createJVMFunction == NULL) {
		FreeLibrary(jni->jvmDll);
		return 0;
	}

	return 1;
}
#else
#endif

int initializeJNI(JNI* jni) {
#ifdef _WIN32
	return initializeJNIWindows(jni);
#else
#endif
}

void addOption(JNI* jni, const char* option) {
	if (jni == NULL) return;
	if (jni->allocatedOptions == jni->vm_arguments.nOptions) {
		jni->allocatedOptions *= 2;
		JavaVMOption* temp = new JavaVMOption[jni->allocatedOptions];
		for (int i = 0; i < jni->vm_arguments.nOptions; i++) {
			temp[i] = jni->options[i];
		}
		delete[] jni->options;
		jni->options = temp;
	}
	//jni->options[jni->vm_arguments.nOptions++].optionString = option;
	char* newOption = new char[strlen(option) + 1];
	strcpy(newOption, option);
	jni->options[jni->vm_arguments.nOptions++].optionString = newOption;
}

jint createJVM(JNI* jni) {
	if (jni == NULL || jni->options == NULL) return -1;
	jni->vm_arguments.options = jni->options;
	return jni->createJVMFunction(&(jni->vm), (void**)&(jni->environment), &(jni->vm_arguments));
}

#ifdef _WIN32
void destroyJNIWindows(JNI* jni) {
	jni->createJVMFunction = NULL;
	if (jni->jvmDll != 0) FreeLibrary(jni->jvmDll);
}
#else
#endif

void destroyJNI(JNI* jni) {
#ifdef _WIN32
	destroyJNIWindows(jni);
#else
#endif
	for (int i = 0; i < jni->allocatedOptions; i++) {
		if (jni->options[i].optionString != NULL) delete[] jni->options[i].optionString;
	}
	delete[] jni->jrePath;
	delete[] jni->options;
}