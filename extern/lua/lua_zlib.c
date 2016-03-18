/*
**********************************************************************
* Author  : Brian Maher <maherb at brimworks dot com>
* Library : lua_zlib - Lua 5.1 interface to zlib
*
* The MIT License
* 
* Copyright (c) 2009 Brian Maher
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
**********************************************************************

To use this library, you need zlib, get it here:
     http://www.gzip.org/zlib/

To build this library, you can use CMake and get it here:
    http://www.cmake.org/cmake/resources/software.html

...or you can use GNU Make.
   make <platform>

Loading the library:

    If you built the library as a loadable package
        [local] zlib = require 'zlib'

    If you compiled the package statically into your application, call
    the function "luaopen_zlib(L)". It will create a table with the zlib
    functions and leave it on the stack.

-- zlib functions --

int major, int minor, int patch = zlib.version()

    returns numeric zlib version for the major, minor, and patch
    levels of the version dynamically linked in.

function stream = zlib.deflate([ int compression_level ], [ int window_size ])

    If no compression_level is provided uses Z_DEFAULT_COMPRESSION (6),
    compression level is a number from 1-9 where zlib.BEST_SPEED is 1
    and zlib.BEST_COMPRESSION is 9.

    Returns a "stream" function that compresses (or deflates) all
    strings passed in.  Specifically, use it as such:

    string deflated, bool eof, int bytes_in, int bytes_out =
            stream(string input [, 'sync' | 'full' | 'finish'])

        Takes input and deflates and returns a portion of it,
        optionally forcing a flush.

        A 'sync' flush will force all pending output to be flushed to
        the return value and the output is aligned on a byte boundary,
        so that the decompressor can get all input data available so
        far.  Flushing may degrade compression for some compression
        algorithms and so it should be used only when necessary.

        A 'full' flush will flush all output as with 'sync', and the
        compression state is reset so that decompression can restart
        from this point if previous compressed data has been damaged
        or if random access is desired. Using Z_FULL_FLUSH too often
        can seriously degrade the compression. 

        A 'finish' flush will force all pending output to be processed
        and results in the stream become unusable.  Any future
        attempts to print anything other than the empty string will
        result in an error that begins with IllegalState.

        The eof result is true if 'finish' was specified, otherwise
        it is false.

        The bytes_in is how many bytes of input have been passed to
        stream, and bytes_out is the number of bytes returned in
        deflated string chunks.

function stream = zlib.inflate([int windowBits])

    Returns a "stream" function that decompresses (or inflates) all
    strings passed in.  Optionally specify a windowBits argument
    that is passed to inflateInit2(), see zlib.h for details about
    this argument.  By default, gzip header detection is done, and
    the max window size is used.

    The "stream" function should be used as such:

    string inflated, bool eof, int bytes_in, int bytes_out =
            stream(string input)

        Takes input and inflates and returns a portion of it.  If it
        detects the end of a deflation stream, then total will be the
        total number of bytes read from input and all future calls to
        stream() with a non empty string will result in an error that
        begins with IllegalState.

        No flush options are provided since the maximal amount of
        input is always processed.

        eof will be true when the input string is determined to be at
        the "end of the file".

        The bytes_in is how many bytes of input have been passed to
        stream, and bytes_out is the number of bytes returned in
        inflated string chunks.


function compute_checksum = zlib.adler32()
function compute_checksum = zlib.crc32()

    Create a new checksum computation function using either the
    adler32 or crc32 algorithms.  This resulting function should be
    used as such:

    int checksum = compute_checksum(string input |
                                    function compute_checksum)

        The compute_checksum function takes as input either a string
        that is logically getting appended to or another
        compute_checksum function that is logically getting appended.
        The result is the updated checksum.

        For example, these uses will all result in the same checksum:

            -- All in one call:
            local csum = zlib.crc32()("one two")

            -- Multiple calls:
            local compute = zlib.crc32()
            compute("one")
            assert(csum == compute(" two"))

            -- Multiple compute_checksums joined:
            local compute1, compute2 = zlib.crc32(), zlib.crc32()
            compute1("one")
            compute2(" two")
            assert(csum == compute1(compute2))
*/

#include <ctype.h>
#include <lauxlib.h>
#include <lua.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "lua_zlib.h"

/*
 * ** compatibility with Lua 5.2
 * */
#if (LUA_VERSION_NUM >= 502)
#undef luaL_register
#define luaL_register(L,n,f) \
               { if ((n) == NULL) luaL_setfuncs(L,f,0); else luaL_newlib(L,f); }

#endif

#if (LUA_VERSION_NUM >= 503)
#undef luaL_optint
#define luaL_optint(L,n,d)  ((int)luaL_optinteger(L,(n),(d)))
#endif

#define DEF_MEM_LEVEL 8

typedef uLong (*checksum_t)        (uLong crc, const Bytef *buf, uInt len);
typedef uLong (*checksum_combine_t)(uLong crc1, uLong crc2, z_off_t len2);


static int lz_deflate(lua_State *L);
static int lz_deflate_delete(lua_State *L);
static int lz_inflate_delete(lua_State *L);
static int lz_inflate(lua_State *L);
static int lz_checksum(lua_State *L);
static int lz_checksum_new(lua_State *L, checksum_t checksum, checksum_combine_t combine);
static int lz_adler32(lua_State *L);
static int lz_crc32(lua_State *L);

static int lz_version(lua_State *L) {
    const char* version = zlibVersion();
    int         count   = strlen(version) + 1;
    char*       cur     = (char*)memcpy(lua_newuserdata(L, count),
                                        version, count);

    count = 0;
    while ( *cur ) {
        char* begin = cur;
        /* Find all digits: */
        while ( isdigit(*cur) ) cur++;
        if ( begin != cur ) {
            int is_end = *cur == '\0';
            *cur = '\0';
            lua_pushnumber(L, atoi(begin));
            count++;
            if ( is_end ) break;
            cur++;
        }
        while ( *cur && ! isdigit(*cur) ) cur++;
    }

    return count;
}

static int lz_assert(lua_State *L, int result, const z_stream* stream, const char* file, int line) {
    /* Both of these are "normal" return codes: */
    if ( result == Z_OK || result == Z_STREAM_END ) return result;
    switch ( result ) {
    case Z_NEED_DICT:
        lua_pushfstring(L, "RequiresDictionary: input stream requires a dictionary to be deflated (%s) at %s line %d",
                        stream->msg, file, line);
        break;
    case Z_STREAM_ERROR:
        lua_pushfstring(L, "InternalError: inconsistent internal zlib stream (%s) at %s line %d",
                        stream->msg, file, line);
        break;
    case Z_DATA_ERROR:
        lua_pushfstring(L, "InvalidInput: input string does not conform to zlib format or checksum failed at %s line %d",
                        file, line);
        break;
    case Z_MEM_ERROR:
        lua_pushfstring(L, "OutOfMemory: not enough memory (%s) at %s line %d",
                        stream->msg, file, line);
        break;
    case Z_BUF_ERROR:
        lua_pushfstring(L, "InternalError: no progress possible (%s) at %s line %d",
                        stream->msg, file, line);
        break;
    case Z_VERSION_ERROR:
        lua_pushfstring(L, "IncompatibleLibrary: built with version %s, but dynamically linked with version %s (%s) at %s line %d",
                        ZLIB_VERSION,  zlibVersion(), stream->msg, file, line);
        break;
    default:
        lua_pushfstring(L, "ZLibError: unknown code %d (%s) at %s line %d",
                        result, stream->msg, file, line);
    }
    lua_error(L);
    return result;
}

/**
 * @upvalue z_stream - Memory for the z_stream.
 * @upvalue remainder - Any remainder from the last deflate call.
 *
 * @param string - "print" to deflate stream.
 * @param int - flush output buffer? Z_SYNC_FLUSH, Z_FULL_FLUSH, or Z_FINISH.
 *
 * if no params, terminates the stream (as if we got empty string and Z_FINISH).
 */
static int lz_filter_impl(lua_State *L, int (*filter)(z_streamp, int), int (*end)(z_streamp), char* name) {
    int flush = Z_NO_FLUSH, result;
    z_stream* stream;
    luaL_Buffer buff;
    size_t avail_in;

    if ( filter == deflate ) {
        const char *const opts[] = { "none", "sync", "full", "finish", NULL };
        flush = luaL_checkoption(L, 2, opts[0], opts);
        if ( flush ) flush++; 
        /* Z_NO_FLUSH(0) Z_SYNC_FLUSH(2), Z_FULL_FLUSH(3), Z_FINISH (4) */

        /* No arguments or nil, we are terminating the stream: */
        if ( lua_gettop(L) == 0 || lua_isnil(L, 1) ) {
            flush = Z_FINISH;
        }
    }

    stream = (z_stream*)lua_touserdata(L, lua_upvalueindex(1));
    if ( stream == NULL ) {
        if ( lua_gettop(L) >= 1 && lua_isstring(L, 1) ) {
            lua_pushfstring(L, "IllegalState: calling %s function when stream was previously closed", name);
            lua_error(L);
        }
        lua_pushstring(L, "");
        lua_pushboolean(L, 1);
        return 2; /* Ignore duplicate calls to "close". */
    }

    luaL_buffinit(L, &buff);

    if ( lua_gettop(L) > 1 ) lua_pushvalue(L, 1);

    if ( lua_isstring(L, lua_upvalueindex(2)) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        if ( lua_gettop(L) > 1 && lua_isstring(L, -2) ) {
            lua_concat(L, 2);
        }
    }

    /*  Do the actual deflate'ing: */
    if (lua_gettop(L) > 0) {
        stream->next_in = (unsigned char*)lua_tolstring(L, -1, &avail_in);
    } else {
        stream->next_in = NULL;
        avail_in = 0;
    }
    stream->avail_in = avail_in;

    if ( ! stream->avail_in && ! flush ) {
        /*  Passed empty string, make it a noop instead of erroring out. */
        lua_pushstring(L, "");
        lua_pushboolean(L, 0);
        lua_pushinteger(L, stream->total_in);
        lua_pushinteger(L, stream->total_out);
        return 4;
    }

    do {
        stream->next_out  = (unsigned char*)luaL_prepbuffer(&buff);
        stream->avail_out = LUAL_BUFFERSIZE;
        result = filter(stream, flush);
        if ( Z_BUF_ERROR != result ) {
            /* Ignore Z_BUF_ERROR since that just indicates that we
             * need a larger buffer in order to proceed.  Thanks to
             * Tobias Markmann for finding this bug!
             */
            lz_assert(L, result, stream, __FILE__, __LINE__);
        }
        luaL_addsize(&buff, LUAL_BUFFERSIZE - stream->avail_out);
    } while ( stream->avail_out == 0 );

    /*  Need to do this before we alter the stack: */
    luaL_pushresult(&buff);

    /*  Save remainder in lua_upvalueindex(2): */
    if ( NULL != stream->next_in ) {
        lua_pushlstring(L, (char*)stream->next_in, stream->avail_in);
        lua_replace(L, lua_upvalueindex(2));
    }

    /*  "close" the stream/remove finalizer: */
    if ( result == Z_STREAM_END ) {
        /*  Clear-out the metatable so end is not called twice: */
        lua_pushnil(L);
        lua_setmetatable(L, lua_upvalueindex(1));

        /*  nil the upvalue: */
        lua_pushnil(L);
        lua_replace(L, lua_upvalueindex(1));

        /*  Close the stream: */
        lz_assert(L, end(stream), stream, __FILE__, __LINE__);

        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    lua_pushinteger(L, stream->total_in);
    lua_pushinteger(L, stream->total_out);
    return 4;
}

static void lz_create_deflate_mt(lua_State *L) {
    luaL_newmetatable(L, "lz.deflate.meta"); /*  {} */

    lua_pushcfunction(L, lz_deflate_delete);
    lua_setfield(L, -2, "__gc");

    lua_pop(L, 1); /*  <empty> */
}

static int lz_deflate_new(lua_State *L) {
    int level = luaL_optint(L, 1, Z_DEFAULT_COMPRESSION);
    int window_size = luaL_optint(L, 2, MAX_WBITS);

    /*  Allocate the stream: */
    z_stream* stream = (z_stream*)lua_newuserdata(L, sizeof(z_stream));

    stream->zalloc = Z_NULL;
    stream->zfree  = Z_NULL;

    int result = deflateInit2(stream, level, Z_DEFLATED, window_size,
                              DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);

    lz_assert(L, result, stream, __FILE__, __LINE__);

    /*  Don't allow destructor to execute unless deflateInit2 was successful: */
    luaL_getmetatable(L, "lz.deflate.meta");
    lua_setmetatable(L, -2);

    lua_pushnil(L);
    lua_pushcclosure(L, lz_deflate, 2);
    return 1;
}

static int lz_deflate(lua_State *L) {
    return lz_filter_impl(L, deflate, deflateEnd, (char*)"deflate");
}

static int lz_deflate_delete(lua_State *L) {
    z_stream* stream  = (z_stream*)lua_touserdata(L, 1);

    /*  Ignore errors. */
    deflateEnd(stream);

    return 0;
}


static void lz_create_inflate_mt(lua_State *L) {
    luaL_newmetatable(L, "lz.inflate.meta"); /*  {} */

    lua_pushcfunction(L, lz_inflate_delete);
    lua_setfield(L, -2, "__gc");

    lua_pop(L, 1); /*  <empty> */
}

static int lz_inflate_new(lua_State *L) {
    /* Allocate the stream */
    z_stream* stream = (z_stream*)lua_newuserdata(L, sizeof(z_stream));

    /*  By default, we will do gzip header detection w/ max window size */
    int window_size = lua_isnumber(L, 1) ? lua_tointeger(L, 1) : MAX_WBITS + 32;

    stream->zalloc   = Z_NULL;
    stream->zfree    = Z_NULL;
    stream->next_in  = Z_NULL;
    stream->avail_in = 0;

    lz_assert(L, inflateInit2(stream, window_size), stream, __FILE__, __LINE__);

    /*  Don't allow destructor to execute unless deflateInit was successful: */
    luaL_getmetatable(L, "lz.inflate.meta");
    lua_setmetatable(L, -2);

    lua_pushnil(L);
    lua_pushcclosure(L, lz_inflate, 2);
    return 1;
}

static int lz_inflate(lua_State *L) {
    return lz_filter_impl(L, inflate, inflateEnd, (char*)"inflate");
}

static int lz_inflate_delete(lua_State *L) {
    z_stream* stream  = (z_stream*)lua_touserdata(L, 1);

    /*  Ignore errors: */
    inflateEnd(stream);

    return 0;
}

static int lz_checksum(lua_State *L) {
    if ( lua_gettop(L) <= 0 ) {
        lua_pushvalue(L, lua_upvalueindex(3));
        lua_pushvalue(L, lua_upvalueindex(4));
    } else if ( lua_isfunction(L, 1) ) {
        checksum_combine_t combine = (checksum_combine_t)
            lua_touserdata(L, lua_upvalueindex(2));

        lua_pushvalue(L, 1);
        lua_call(L, 0, 2);
        if ( ! lua_isnumber(L, -2) || ! lua_isnumber(L, -1) ) {
            luaL_argerror(L, 1, "expected function to return two numbers");
        }

        /* Calculate and replace the checksum */
        lua_pushnumber(L,
                       combine((uLong)lua_tonumber(L, lua_upvalueindex(3)),
                               (uLong)lua_tonumber(L, -2),
                               (z_off_t)lua_tonumber(L, -1)));
        lua_pushvalue(L, -1);
        lua_replace(L, lua_upvalueindex(3));

        /* Calculate and replace the length */
        lua_pushnumber(L,
                       lua_tonumber(L, lua_upvalueindex(4)) + lua_tonumber(L, -2));
        lua_pushvalue(L, -1);
        lua_replace(L, lua_upvalueindex(4));
    } else {
        const Bytef* str;
        size_t       len;

        checksum_t checksum = (checksum_t)
            lua_touserdata(L, lua_upvalueindex(1));
        str = (const Bytef*)luaL_checklstring(L, 1, &len);
 
        /* Calculate and replace the checksum */
        lua_pushnumber(L,
                       checksum((uLong)lua_tonumber(L, lua_upvalueindex(3)),
                                str,
                                len));
        lua_pushvalue(L, -1);
        lua_replace(L, lua_upvalueindex(3));
        
        /* Calculate and replace the length */
        lua_pushnumber(L,
                       lua_tonumber(L, lua_upvalueindex(4)) + len);
        lua_pushvalue(L, -1);
        lua_replace(L, lua_upvalueindex(4));
    }
    return 2;
}

static int lz_checksum_new(lua_State *L, checksum_t checksum, checksum_combine_t combine) {
    lua_pushlightuserdata(L, &checksum);
    lua_pushlightuserdata(L, &combine);
    lua_pushnumber(L, checksum(0L, Z_NULL, 0));
    lua_pushnumber(L, 0);
    lua_pushcclosure(L, lz_checksum, 4);
    return 1;
}

static int lz_adler32(lua_State *L) {
    return lz_checksum_new(L, adler32, adler32_combine);
}

static int lz_crc32(lua_State *L) {
    return lz_checksum_new(L, crc32, crc32_combine);
}

static const luaL_Reg zlib_functions[] = {
    { "deflate", lz_deflate_new },
    { "inflate", lz_inflate_new },
    { "adler32", lz_adler32     },
    { "crc32",   lz_crc32       },
    { "version", lz_version     },
    { NULL,      NULL           }
};

#define SETLITERAL(n,v) (lua_pushliteral(L, n), lua_pushliteral(L, v), lua_settable(L, -3))
#define SETINT(n,v) (lua_pushliteral(L, n), lua_pushinteger(L, v), lua_settable(L, -3))

LUALIB_API int luaopen_zlib(lua_State * const L) {
    lz_create_deflate_mt(L);
    lz_create_inflate_mt(L);

    luaL_register(L, "zlib", zlib_functions);

    SETINT("BEST_SPEED", Z_BEST_SPEED);
    SETINT("BEST_COMPRESSION", Z_BEST_COMPRESSION);

    SETLITERAL("_COPYRIGHT", "Copyright (c) 2009-2010 Brian Maher");
    SETLITERAL("_DESCRIPTION", "Yet another binding to the zlib library");
    SETLITERAL("_VERSION", "lua-zlib $Id: 9d1f4a541bfa8de087140a835943ca0c812bf2a5 $  (HEAD -> master)");

    /* Expose this to lua so we can do a test: */
    SETINT("_TEST_BUFSIZ", LUAL_BUFFERSIZE);

    return 1;
}
