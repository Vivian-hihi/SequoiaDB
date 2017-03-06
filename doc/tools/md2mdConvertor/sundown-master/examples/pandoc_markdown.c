/*
 * Copyright (c) 2011, Vicent Marti
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "markdown.h"
#include "pandoc.h"
#include "buffer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>


#if defined (_WINDOWS)
#define NEW_LINE         "\r\n"
#else
#define NEW_LINE         "\n"
#endif

#define EXCLUDE_BEGIN    "<!--manpage_exclude_begin-->"
#define EXCLUDE_END      "<!--manpage_exclude_end-->"

#define FINE_NAME_LEN    128
#define READ_UNIT        1024
#define OUTPUT_UNIT      64

static void _usage()
{
    fprintf(stderr,
        "usage: ./proc -i input_file -o output_file [-d]"NEW_LINE);
    exit(1);
}

static int _filterContent(struct buf *b)
{
    int rc    = 0;
    char *f   = NULL;
    char *e   = NULL;

retry:
    f = NULL, e = NULL;
    f = strstr((const char*)(b->data), EXCLUDE_BEGIN);
    e = strstr((const char*)(b->data), EXCLUDE_END);
    if (NULL == f && NULL == e)
    {
        goto done;
    }
    else if (NULL !=f && NULL == e)
    {
        fprintf(stderr,"Error: Lack %s in the source file"NEW_LINE, 
            EXCLUDE_END);
        rc = -1; 
        goto error;
    }
    else if (NULL == f && NULL != e)
    {
        fprintf(stderr,"Error: Lack %s in the source file"NEW_LINE, 
            EXCLUDE_BEGIN);
        rc = -1; 
        goto error;
    }
    e += strlen(EXCLUDE_END);
    /* we get both positions, let's filter the content */
    {
        int len = e - f;
        int size = b->size - (e - (const char*)(b->data));
        memmove(f, e, size);
        memset(f + size, 0, len);
        b->size -= len;
        goto retry;
    }
done:
    return rc;
error:
    goto done;
}

/* main - main function, interfacing STDIO with the parser */
int main(int argc, char **argv)
{
    struct buf *ib   = NULL;
    struct buf *ob   = NULL;
    FILE *in         = NULL;
    FILE *out        = NULL;
    int ret          = -1;
    int debug        = 0;
    int show_usage   = 0;
    int c            = -1;
    char in_file[FINE_NAME_LEN]  = { 0 };
    char out_file[FINE_NAME_LEN] = { 0 };
    char tmp_file[FINE_NAME_LEN] = { 0 };


    struct sd_callbacks callbacks;
    struct sd_markdown *markdown = NULL;

    while (-1  != (c = getopt(argc, argv, "i:o:d")))
    {
        switch(c)
        {
        case 'i':
            strncpy(in_file, optarg, FINE_NAME_LEN);
            show_usage |= 1;
            break;
        case 'o':
            strncpy(out_file, optarg, FINE_NAME_LEN);
            show_usage |= 2;
            break;
        case 'd':
            debug = 1;
            break;
        default:
            show_usage |= 4;
            break;
        }
    }

    if (0 == (show_usage & 1) ||
        0 == (show_usage & 2) ||
        1 == (show_usage & 4))
    {
        _usage();
    }

    /* opening the file if given from the command line */
    in = fopen(in_file, "r");
    if (!in) {
        fprintf(stderr,"Unable to open input file \"%s\": %s"NEW_LINE, 
                in_file, strerror(errno));
        return 1;
    }

    out = fopen(out_file, "w");
    if (!out) {
        fprintf(stderr,"Unable to open output file \"%s\": %s"NEW_LINE, 
                out_file, strerror(errno));
        return 1;
    }

    /* reading everything */
    ib = bufnew(READ_UNIT);
    bufgrow(ib, READ_UNIT);
    while ((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, in)) > 0) {
        ib->size += ret;
        bufgrow(ib, ib->size + READ_UNIT);
    }
    /* close input file handle */
    fclose(in);

    /* filter content */
    _filterContent(ib);

    /* show the filtered file contents*/
    if (debug)
    {
        FILE *tmp_file_fd = NULL;
        int len = (strlen(out_file) <= (FINE_NAME_LEN - 8)) ? 
            strlen(out_file) : (FINE_NAME_LEN - 8);
        strncpy(tmp_file, out_file, len);
        strncpy(tmp_file + len, ".source", 8);
        tmp_file_fd = fopen(tmp_file, "w");
        fwrite(ib->data, 1, ib->size, tmp_file_fd);
        fclose(tmp_file_fd);
    }

    /* performing markdown parsing */
    ob = bufnew(OUTPUT_UNIT);

//  sdhtml_renderer(&callbacks, &options, 0); // TODO:
//  markdown = sd_markdown_new(0, 16, &callbacks, &options);

    pandoc_markdown_renderer(&callbacks);
    markdown = sd_markdown_new(MKDEXT_TABLES | MKDEXT_FENCED_CODE, 
                               16, &callbacks, NULL);

    /**/
    sd_markdown_render(ob, ib->data, ib->size, markdown);
    sd_markdown_free(markdown);

    /* writing the result to stdout */
    ret = fwrite(ob->data, 1, ob->size, out);
    
    /* close output file handle */
    fclose(out);

    /* cleanup */
    bufrelease(ib);
    bufrelease(ob);

    return (ret < 0) ? -1 : 0;
}

/* vim: set filetype=c: */
