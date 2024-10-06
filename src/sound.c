/*
 * sound.c
 *
 * Created by miles
*/

#define OGG_IMPL
#define VORBIS_IMPL
#include "minivorbis.h"

typedef struct {
    uint64_t ptr;
    uint64_t size;
    void* buf;
} __mv_file;


size_t __mv_fread(void *f, size_t sz, size_t ct, void *buf)
{
    __mv_file* a = (__mv_file*)buf;
    size_t b = sz * ct;
    if(a->ptr + b > a->size)
        b = a->size - a->ptr;
    memcpy(f, &((char*)a->buf)[a->ptr], b);
    a->ptr += b;

    return b / sz;
}


int  __mv_fseek(void *f, int64_t off, int sk)
{
    __mv_file* a = (__mv_file*)f;
    switch(sk) {
        case SEEK_SET: a->ptr = off; break;
        case SEEK_CUR: a->ptr += off; break;
        case SEEK_END: a->ptr = a->size - off; break;
    }

    if(a->ptr < 0) a->ptr = 0;
    if(a->ptr > a->size) a->ptr = a->size;

    return a->ptr;
}


int  __mv_fclose(void *f)
{
    __mv_file* a = (__mv_file*)f;
    a->ptr = 0;
    return 0;
}


long  __mv_ftell(void *f)
{
    __mv_file* a = (__mv_file*)f;
    return a->ptr;
}
