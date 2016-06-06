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

static int old_value = 0;
/* this is how the CURLOPT_XFERINFOFUNCTION callback works */
static int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	if (dltotal == 0) return 0;
	int value = ((dlnow * 100) / dltotal);
	if (old_value != value) {
		fprintf(stderr, "\r Downloading: %i%%...", value);
		fflush(stderr);
		if (value == 100) fprintf(stderr, "\n");
		old_value = value;
	}

	return 0;
}

CURLcode downloadFile(const char* filename, const char* url, const char* cookies = NULL) {
	FILE* fp = fopen(filename, "r");
	int download = 1;
	CURLcode ret = CURLE_OK;
	if (fp != NULL) {
		download = 0;
		fclose(fp);
	}
	if (download) {
		CURL* curl = curl_easy_init();
		if (curl) {
			fp = fopen(filename, "wb");
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			curl_easy_setopt(curl, CURLOPT_COOKIE, cookies);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

			ret = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			fclose(fp);
		}
	}
	return ret;
}