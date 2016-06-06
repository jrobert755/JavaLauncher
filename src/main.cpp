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

#include "network.h"
#include "extract.h"
#include "jni_util.h"
#include "javalauncher_config.h"
#include <cstdio>
#include <cstdlib>
#ifndef _WIN32
#include <sys/stat.h>
#endif

int main(void) {
	CURLcode res;
	int majorVersion = 8;
	int minorVersion = 92;

	char* folderTemplate = "jre1.%i.0_%i";
	char jreFolder[512];
	sprintf(jreFolder, folderTemplate, majorVersion, minorVersion);

	int statRet;
#ifdef _WIN32
	struct _stat info;
	statRet = _stat(jreFolder, &info);
#else
	struct stat info;
	statRet = lstat(jreFolder, &info);
#endif
	if (statRet == -1 && errno == ENOENT) {
		printf("Aquiring JRE!\n");
		char* urltemplate = "http://download.oracle.com/otn-pub/java/jdk/%iu%i-b14/";
		char url[512];
		sprintf(url, urltemplate, majorVersion, minorVersion);

		char outfilename[512];
		char* filenameTemplate = "jre-%iu%i-%s";
		sprintf(outfilename, filenameTemplate, majorVersion, minorVersion, JRE_STRING);
		strcat(outfilename, ".tar.gz");
		strcat(url, outfilename);

		res = downloadFile(outfilename, url, "oraclelicense=accept-securebackup-cookie");
		if (res != CURLE_OK) {
			printf("Error downloading file!\n");
			exit(-1);
		}

		printf("Extracting...\n");
		extract(outfilename, 1, ARCHIVE_EXTRACT_TIME, 1);
	} else if (statRet == -1) {
		printf("Invalid!\n");
		exit(-1);
	}

	JNI* jni = allocateJNI(jreFolder);
	addOption(jni, "-Djava.class.path=.\\Testing.jar");
	jni->vm_arguments.version = JNI_VERSION_1_8;
	jni->vm_arguments.ignoreUnrecognized = JNI_TRUE;
	int jniInitRet = initializeJNI(jni);
	jint status = createJVM(jni);

	JNIEnv* env = jni->environment;

	jclass cls = env->FindClass("Main");
	if (cls != NULL) {
		jmethodID mid = env->GetStaticMethodID(cls, "test", "(I)V");
		if (mid != NULL) {
			env->CallStaticVoidMethod(cls, mid, 100);
		}
		else {
			printf("ERROR! Unable to find the method given!\n");
		}
	}
	else {
		printf("ERROR! Unable to find the class given!\n");
	}

	destroyJNI(jni);

	getchar();

	return 0;
}
