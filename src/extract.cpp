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

#include "extract.h"

/*
* These reporting functions use low-level I/O; on some systems, this
* is a significant code reduction.  Of course, on many server and
* desktop operating systems, malloc() and even crt rely on printf(),
* which in turn pulls in most of the rest of stdio, so this is not an
* optimization at all there.  (If you're going to pay 100k or more
* for printf() anyway, you may as well use it!)
*/
static void
msg(const char *m)
{
#ifdef _WIN32
	_write(1, m, strlen(m));
#else
	write(1, m, strlen(m));
#endif
}

static void
errmsg(const char *m)
{
#ifdef _WIN32
	_write(2, m, strlen(m));
#else
	write(2, m, strlen(m));
#endif
}

static void
warn(const char *f, const char *m)
{
	errmsg(f);
	errmsg(" failed: ");
	errmsg(m);
	errmsg("\n");
}

static void
fail(const char *f, const char *m, int r)
{
	warn(f, m);
	exit(r);
}

static void
usage(void)
{
	const char *m = "Usage: untar [-tvx] [-f file] [file]\n";
	errmsg(m);
	exit(1);
}

static int
copy_data(struct archive *ar, struct archive *aw)
{
	int r;
	const void *buff;
	size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
	int64_t offset;
#else
	off_t offset;
#endif

	for (;;) {
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF)
			return (ARCHIVE_OK);
		if (r != ARCHIVE_OK)
			return (r);
		r = archive_write_data_block(aw, buff, size, offset);
		if (r != ARCHIVE_OK) {
			warn("archive_write_data_block()",
				archive_error_string(aw));
			return (r);
		}
	}
}

void extract(const char *filename, int do_extract, int flags, char verbose)
{
	struct archive *a;
	struct archive *ext;
	struct archive_entry *entry;
	int r;

	a = archive_read_new();
	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);
	/*
	* Note: archive_write_disk_set_standard_lookup() is useful
	* here, but it requires library routines that can add 500k or
	* more to a static executable.
	*/
	archive_read_support_format_tar(a);
	archive_read_support_compression_gzip(a);
	/*
	* On my system, enabling other archive formats adds 20k-30k
	* each.  Enabling gzip decompression adds about 20k.
	* Enabling bzip2 is more expensive because the libbz2 library
	* isn't very well factored.
	*/
	if (filename != NULL && strcmp(filename, "-") == 0)
		filename = NULL;
	if ((r = archive_read_open_filename(a, filename, 10240)))
		fail("archive_read_open_filename()",
			archive_error_string(a), r);
	for (;;) {
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r != ARCHIVE_OK)
			fail("archive_read_next_header()",
				archive_error_string(a), 1);
		if (verbose && do_extract)
			msg("x ");
		if (verbose || !do_extract)
			msg(archive_entry_pathname(entry));
		if (do_extract) {
			r = archive_write_header(ext, entry);
			if (r != ARCHIVE_OK)
				warn("archive_write_header()",
					archive_error_string(ext));
			else {
				copy_data(a, ext);
				r = archive_write_finish_entry(ext);
				if (r != ARCHIVE_OK)
					fail("archive_write_finish_entry()",
						archive_error_string(ext), 1);
			}

		}
		if (verbose || !do_extract)
			msg("\n");
	}
	archive_read_close(a);
	archive_read_free(a);

	archive_write_close(ext);
	archive_write_free(ext);
}