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
#include <string>
#ifndef _WIN32
#include <sys/stat.h>
#endif

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: %s configFile", argv[0]);
		exit(-1);
	}
	JNIProperties properties;
	loadProperties(&properties, argv[1]);
	if (properties.jarFile == "" && properties.classpath == "") {
		printf("ERROR IN CONFIGURATION! jarFile OR classpath MUST BE SET!");
	}

	CURLcode res;
	int majorVersion = 8;
	int minorVersion = 92;

	char jreFolder[512];
	sprintf(jreFolder, JRE_FOLDER_TEMPLATE, majorVersion, minorVersion);

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
		char url[512];
		sprintf(url, JRE_URL_TEMPLATE, majorVersion, minorVersion);

		char outfilename[512];
		sprintf(outfilename, JRE_FILENAME_TEMPLATE, majorVersion, minorVersion, JRE_STRING);
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

	if (properties.jarFile != "") {
		if (properties.classpath != "") properties.classpath += ";" + properties.jarFile;
		else properties.classpath = "-Djava.class.path=" + properties.jarFile;
		
		if (properties.mainClass == "") {
			char* manifestFile = extractFileFromArchive("JGL2D.jar", "META-INF/MANIFEST.MF");
			if (manifestFile == NULL) printf("ERROR! NO MANIFEST FILE!\n");
			else {
				std::string m = manifestFile;
				int pos = m.find("Main-Class:");
				if (pos == std::string::npos) {
					printf("ERROR! NO MAIN CLASS SET!\n");
					exit(-1);
				}
				pos += 11;
				int end = m.find("\n", pos);
				properties.mainClass = m.substr(pos, end-pos);
				while (properties.mainClass[0] == ' ') properties.mainClass = properties.mainClass.substr(1);
				if (properties.mainClass[properties.mainClass.length() - 1] == '\r') properties.mainClass = properties.mainClass.substr(0, properties.mainClass.length() - 1);
			}
		}
		
	}
	else if (properties.mainClass == "") {
		printf("ERROR! NO MAIN CLASS SET!\n");
		exit(-1);
	}

	for (int i = 0; i < properties.mainClass.length(); i++) {
		if (properties.mainClass[i] == '.') properties.mainClass[i] = '/';
	}
	
	addOption(jni, properties.classpath.c_str());

	jni->vm_arguments.version = JNI_VERSION_1_8;
	jni->vm_arguments.ignoreUnrecognized = JNI_TRUE;
	int jniInitRet = initializeJNI(jni);
	jint status = createJVM(jni);

	JNIEnv* env = jni->environment;

	jclass cls = env->FindClass(properties.mainClass.c_str());
	if (cls != NULL) {
		jmethodID mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
		if (mid != NULL) {
			env->CallStaticVoidMethod(cls, mid, NULL);
		}
		else {
			printf("ERROR! Unable to find the main method!\n");
		}
	}
	else {
		printf("ERROR! Unable to find the class given!\n");
	}

	destroyJNI(jni);

	return 0;
}
