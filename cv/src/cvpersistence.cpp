/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/* data structures I/O to XML */

#include "_cv.h"
#include <ctype.h>

#define ICV_FORMAT_TEXT 0
#define ICV_FORMAT_BINARY 1
#define ICV_IS_BINARY(storage) (((storage)->flags & 3) == CV_STORAGE_WRITE_BINARY)
#define ICV_IS_WRITE_MODE(storage) (((storage)->flags & 3) != 0)

#define ICV_DT_SHIFT(dt_buf)   (dt_buf[0] == '1' && !isdigit(dt_buf[1]))

static const char icvTypeSymbol[] = "ucsifd";
static const char* icvStandardTags[] = { "struct", "elem", 0 };

/* these are hacked versions of strtod & strtol that inserts NUL character
   at the possible end-of-number position that speeds up strtod & strtol execution in
   case of huge files ('cause they may call strlen() internally) */
static double icv_strtod( const char* ptr, char** endptr )
{
    double val;
    char* eonum = (char*)ptr;
    char char_copy;
    while( isspace(*eonum))
        eonum++;

    while( isdigit(*eonum) || *eonum == '.' || *eonum == '-' ||
           *eonum == 'e' || *eonum == 'E' || *eonum == '+' )
        eonum++;
    char_copy = *eonum;
    *eonum = (char)'\0';
    val = strtod( ptr, endptr );
    *eonum = char_copy;

    return val;
}


static long icv_strtol( const char* ptr, char** endptr, int base )
{
    long val;
    char* eonum = (char*)ptr;
    char char_copy;
    while( isspace(*eonum))
        eonum++;

    while( isdigit(*eonum) || *eonum == '-' || *eonum == '+' )
        eonum++;
    char_copy = *eonum;
    *eonum = (char)'\0';
    val = strtol( ptr, endptr, base );
    *eonum = char_copy;

    return val;
}


/* "black box" file storage */
typedef struct CvFileStorage
{
    int flags;
    int width;
    int max_width;
    FILE* file;
    CvMemStorage* dststorage;
    CvMemStorage* memstorage;
    CvSet* nodes;
    CvFileNode* root;
    CvFileNode* parent;
    int buffer_size;
    int line;
    char* filename;

    char* base64_buffer;
    char* base64_buffer_read;
    char* base64_buffer_start;
    char* base64_buffer_end;

    char* buffer_start;
    char* buffer_end;
    char* buffer;
    char* line_start;

    CvFileNode** hashtable;
    int   hashsize;
}
CvFileStorage;

/*********************************** Adding own types ***********************************/

static int
icvCheckString( const char* str, char termchar CV_DEFAULT(0))
{
    int l = 0;

    CV_FUNCNAME("icvCheckString");

    __BEGIN__;

    if( !str )
        CV_ERROR( CV_StsNullPtr, "NULL string" );

    if( (termchar & ~0x7f) != 0 || termchar == '\\' ||
        termchar == '\"' || (termchar != '\0' && !isprint(termchar)))
        CV_ERROR( CV_StsBadArg, "Invalid termination character" );

    for( ; l < CV_MAX_STRLEN && str[l] != '\0'; l++ )
    {
        int c = str[l];
        /*if( c == '\\' )
        {
            c = str[++l];
            if( c != '\\')// && c != '\"')
                CV_ERROR( CV_StsError,
                "Only \'\\\\\' esc-sequences is supported" );
            continue;
        }*/

        if( c == termchar )
            break;

        if( c > 0x7f || !isprint(c) || c == '\"' )
        {
            char msg[32];
            sprintf( msg, "Invalid character: %c (=\'\\x%02x\')", c, c );
            CV_ERROR( CV_StsBadArg, msg );
        }
    }

    if( l == CV_MAX_STRLEN )
        CV_ERROR( CV_StsOutOfRange, "Too long string" );

    __END__;

    if( cvGetErrStatus() < 0 )
        l = -1;
    return l;
}


CV_IMPL const char*
cvAttrValue( const CvAttrList* attr, const char* attr_name )
{
    while( attr && attr->attr )
    {
        int i;
        for( i = 0; attr->attr[i*2] != 0; i++ )
        {
            if( strcmp( attr_name, attr->attr[i*2] ) == 0 )
                return attr->attr[i*2+1];
        }
        attr = attr->next;
    }

    return 0;
}


static CvFileNode*
icvQueryName( CvFileStorage* storage, const char* name,
              CvFileNode* new_node CV_DEFAULT(0))
{
    CvFileNode* result_node = 0;

    CV_FUNCNAME( "icvQueryName" );

    __BEGIN__;

    unsigned hash_val = 0;
    int i;
    CvFileNode* node;

    if( !storage || !name )
        CV_ERROR( CV_StsNullPtr, "" );

    for( i = 0; name[i] != 0; i++ )
        hash_val = hash_val*33 + (uchar)name[i];

    i = hash_val % storage->hashsize;
    for( node = storage->hashtable[i]; node != 0; node = node->hash_next )
    {
        assert( node->name );
        if( node->hash_val == hash_val && strcmp( node->name, name ) == 0 )
            break;
    }

    if( node )
        result_node = node;
    else if( new_node )
    {
        new_node->hash_val = hash_val;
        assert( strcmp(new_node->name, name) == 0 );
        new_node->hash_next = storage->hashtable[i];
        storage->hashtable[i] = new_node;
        result_node = new_node;
    }

    __END__;

    return result_node;
}


#define ICV_EMPTY_TAG       1
#define ICV_INCOMPLETE_TAG  2

static void
icvPushXMLTag( CvFileStorage* storage, const char* tagname,
               CvAttrList _attr, int flags CV_DEFAULT(0))
{
    CV_FUNCNAME( "icvPushXMLTag" );

    __BEGIN__;

    int i, dl;
    CvAttrList* attr = &_attr;

    assert( storage && storage->file && tagname );

    CV_CALL( icvCheckString( tagname ));

    assert( storage->width == 0 );
    dl = 0;
    fprintf( storage->file, "<%s%n", tagname, &dl );
    storage->width += dl;

    while( attr && attr->attr )
    {
        for( i = 0; attr->attr[i] != 0; i += 2 )
        {
            CV_CALL( icvCheckString( attr->attr[i] ));
            if( attr->attr[i] == 0 )
                CV_ERROR( CV_StsNullPtr, "One of attribute values is NULL" );
            CV_CALL( icvCheckString( attr->attr[i+1] ));
            dl = 0;
            fprintf( storage->file, " %s=\"%s\"%n", attr->attr[i], attr->attr[i+1], &dl );
            storage->width += dl;
            if( storage->width >= storage->max_width )
            {
                fprintf( storage->file, "\n" );
                storage->width = 0;
            }
        }
        attr = attr->next;
    }

    if( !(flags & ICV_INCOMPLETE_TAG) )
    {
        fprintf( storage->file, (flags & ICV_EMPTY_TAG) ? "/>\n" : ">\n" );
        storage->width = 0;
    }

    for( i = 0; icvStandardTags[i] != 0; i++ )
    {
        if( strcmp( icvStandardTags[i], tagname ) == 0 )
        {
            tagname = icvStandardTags[i];
            break;
        }
    }

    if( icvStandardTags[i] == 0 )
    {
        char* ptr;
        CV_CALL( ptr = (char*)cvMemStorageAlloc( storage->memstorage, strlen(tagname) + 1 ));
        strcpy( ptr, tagname );
        tagname = ptr;
    }

    {
        CvFileNode* node = (CvFileNode*)cvSetNew( storage->nodes );
        memset( node, 0, sizeof(*node));
        node->tagname = tagname;

        if( storage->root )
        {
            if( !storage->parent )
                CV_ERROR( CV_StsError, "<opencv_storage> is already closed" );
            cvInsertNodeIntoTree( node, storage->parent, 0 );
        }
        else
            storage->root = storage->parent = node;
        if( !(flags & ICV_EMPTY_TAG) )
            storage->parent = node;
    }

    __END__;
}


static const char icvBase64Tab[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void
icvWriteBase64Block( CvFileStorage* storage, int flush_flag )
{
    /*CV_FUNCNAME( "icvWriteBase64Block" );*/

    __BEGIN__;

    uchar* buffer = (uchar*)storage->base64_buffer_start;
    int i, len = storage->base64_buffer - storage->base64_buffer_start;
    int val;

    for( i = 0; i <= len - 3; i += 3 )
    {
        int dl = 0;
        val = (buffer[i]<<16) + (buffer[i+1]<<8) + buffer[i+2];
        fprintf( storage->file, "%c%c%c%c%n",
                 icvBase64Tab[val >> 18], icvBase64Tab[(val >> 12) & 63],
                 icvBase64Tab[(val >> 6) & 63], icvBase64Tab[val & 63], &dl );
        storage->width += dl;
        if( storage->width > 72 )
        {
            fprintf( storage->file, "\n" );
            storage->width = 0;
        }
    }

    if( flush_flag )
    {
        buffer[len] = buffer[len+1] = '\0';
        val = (buffer[i]<<16) + (buffer[i+1]<<8) + buffer[i+2];

        switch( len - i )
        {
        case 0:
            if( storage->width )
                fprintf( storage->file, "\n" );
            break;
        case 1:
            fprintf( storage->file, "%c%c==\n",
                     icvBase64Tab[(val >> 18)], icvBase64Tab[(val >> 12) & 63] );
            break;
        case 2:
            fprintf( storage->file, "%c%c%c=\n",
                     icvBase64Tab[(val >> 18)], icvBase64Tab[(val >> 12) & 63],
                     icvBase64Tab[(val >> 6) & 63]);
            break;
        default:
            assert(0);
        }
        storage->width = 0;
        storage->base64_buffer = storage->base64_buffer_start;
    }
    else
    {
        len -= i;
        if( len )
            memmove( storage->base64_buffer_start,
                     storage->base64_buffer_start + i, len );
        storage->base64_buffer = storage->base64_buffer_start + len;
    }

    __END__;
}


static const uchar icvInvBase64Tab[] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 62, 255, 255, 255, 63, 52, 53,
54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
255, 64, 255, 255, 255, 0, 1, 2, 3, 4,
5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
25, 255, 255, 255, 255, 255, 255, 26, 27, 28,
29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
49, 50, 51, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255 };

static int
icvReadBase64Block( CvFileStorage* storage )
{
    int ok = 0;

    /*CV_FUNCNAME( "icvReadBase64Block" );*/

    __BEGIN__;

    uchar* dst = (uchar*)storage->base64_buffer_start;
    uchar* limit = (uchar*)storage->base64_buffer_end;
    uchar* src = (uchar*)storage->buffer;
    int c[4], k = 0;
    int len = storage->base64_buffer_read - storage->base64_buffer;

    if( len > 0  )
    {
        // copy remaining bytes to the beginning of the buffer
        memmove( storage->base64_buffer_start, storage->base64_buffer, len );
        dst += len;
    }

    while( dst < limit )
    {
        for( k = 0; k < 4; k++ )
        {
            c[k] = icvInvBase64Tab[src[0]];
            if( c[k] > 63 )
            {
                while( isspace((char)src[0]))
                    src++;
                c[k] = icvInvBase64Tab[src[0]];
                if( c[k] > 63 )
                    goto loop_end;
            }
            src++;
        }
        dst[0] = (uchar)((c[0] << 2)|(c[1] >> 4));
        dst[1] = (uchar)((c[1] << 4)|(c[2] >> 2));
        dst[2] = (uchar)((c[2] << 6)|c[3]);
        dst += 3;
    }

loop_end:

    switch( k & 3 )
    {
    case 0:
        break;
    case 1:
        EXIT; // error
    case 2:
        dst[0] = (uchar)((c[0] << 2)|(c[1] >> 4));
        dst++;
        break;
    case 3:
        dst[0] = (uchar)((c[0] << 2)|(c[1] >> 4));
        dst[1] = (uchar)((c[1] << 4)|(c[2] >> 2));
        dst += 2;
    }

    storage->base64_buffer = storage->base64_buffer_start;
    storage->base64_buffer_read = (char*)dst;
    storage->buffer = (char*)src;
    ok = 1;

    __END__;

    return ok;
}


static void
icvPopXMLTag( CvFileStorage* storage )
{
    /*CV_FUNCNAME( "icvPopXMLTag" );*/

    __BEGIN__;

    assert( storage && storage->file && storage->root && storage->parent &&
            storage->parent->tagname );

    icvWriteBase64Block( storage, 1 );

    fprintf( storage->file, "%s</%s>\n",
             storage->width == 0 ? "" : "\n",
             storage->parent->tagname );
    storage->width = 0;
    storage->parent = storage->parent->v_prev;

    __END__;
}


static void
icvWriteRawData( CvFileStorage* storage, const char* dt, int format,
                 const void* _data, int count )
{
    const char* data = (const char*)_data;

    CV_FUNCNAME( "icvWriteRawData" );

    __BEGIN__;

    int i = 0, k = 0;
    int pl = strlen( dt );

    if( pl == 0 )
        CV_ERROR( CV_StsBadArg, "Empty format specification" );

    if( format == ICV_FORMAT_TEXT )
    {
        if( pl == 1 || (pl == 2 && dt[0] == '1') )
        {
            switch( dt[pl-1] )
            {
            case 'u':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file, " %3d%n", ((uchar*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;
            case 'c':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file, " %4d%n", ((char*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;
            case 's':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file, " %6d%n", ((short*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;

            case 'i':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file, " %6d%n", ((int*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;

            case 'f':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file," %14.6e%n", ((float*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;

            case 'd':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file," %23.15e%n", ((double*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;
            case 'a':
                if( count != 1 )
                    CV_ERROR( CV_StsBadArg, "Only a single string can be written at once" );
                {
                    int dl = 0;
                    fprintf( storage->file,"%s%n", (char*)data, &dl );

                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;
            default:
                CV_ERROR( CV_StsBadArg, "Unknown format specifier" );
            }
        }
        else
        {
            for( i = 0; i < count; i++ )
            {
                int n = 0;
                char c = '\0';
                const char* dtp = dt;

                for( ;; )
                {
                    int dl = 0;

                    if( --n <= 0 )
                    {
                        if( !dtp[0] )
                            break;
                        n = 1;
                        if( isdigit(dtp[0]))
                        {
                            if( !isdigit(dtp[1]))
                            {
                                n = dtp[0] - '0';
                                dtp++;
                            }
                            else
                            {
                                if( sscanf( dtp, "%d%n", &n, &dl ) <= 0 )
                                    CV_ERROR(CV_StsBadArg,
                                            "Invalid data type specification");
                                dtp += dl;
                                dl = 0;
                            }
                            if( n == 0 )
                                CV_ERROR(CV_StsBadArg,
                                        "Invalid data type specification");
                        }
                        c = *dtp++;
                    }

                    switch( c )
                    {
                    case 'u':
                        fprintf( storage->file, " %3d%n", *(uchar*)(data+k), &dl );
                        k++;
                        break;
                    case 'c':
                        fprintf( storage->file, " %4d%n", *(char*)(data+k), &dl );
                        k++;
                        break;
                    case 's':
                        k = (k + sizeof(short) - 1) & -(int)sizeof(short);
                        fprintf( storage->file, " %6d%n", *(short*)(data+k), &dl );
                        k += sizeof(short);
                        break;
                    case 'i':
                        k = (k + sizeof(int) - 1) & -(int)sizeof(int);
                        fprintf( storage->file, " %6d%n", *(int*)(data+k), &dl );
                        k += sizeof(int);
                        break;
                    case 'f':
                        k = (k + sizeof(float) - 1) & -(int)sizeof(float);
                        fprintf( storage->file, " %14.6e%n", *(float*)(data+k), &dl );
                        k += sizeof(float);
                        break;
                    case 'd':
                        k = (k + sizeof(double) - 1) & -(int)sizeof(double);
                        fprintf( storage->file, " %23.15e%n", *(double*)(data+k), &dl );
                        k += sizeof(double);
                        break;
                    case 'p':
                        k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                        k += sizeof(void*);
                        break;
                    case 'r':
                        k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                        fprintf( storage->file, " %6u%n",
                                 (unsigned)(unsigned long)*(void**)(data+k), &dl );
                        k += sizeof(void*);
                        break;
                    default:
                        CV_ERROR( CV_StsBadArg, "Unknown format specifier" );
                    }

                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
            }
        }
    }
    else if( format == ICV_FORMAT_BINARY )
    {
        char* buffer = storage->base64_buffer;
        char* buffer_end = storage->base64_buffer_end;

        assert( sizeof(float) == sizeof(int) &&
                sizeof(float) == 4 &&
                sizeof(double) == 8 );

        if( pl == 1 )
        {
            i = 0;

            for( ;; )
            {
                switch( dt[0] )
                {
                case 'u':
                case 'c':
                    while( i < count )
                    {
                        *buffer++ = ((char*)data)[i++];
                        if( buffer >= buffer_end )
                            break;
                    }
                    break;
                case 's':
                    while( i < count )
                    {
                        int val = ((short*)data)[i++];
                        buffer[0] = (char)(val >> 8);
                        buffer[1] = (char)val;
                        if( (buffer += 2) >= buffer_end )
                            break;
                    }
                    break;
                case 'i':
                case 'f':
                    while( i < count )
                    {
                        int val = ((int*)data)[i++];
                        buffer[0] = (char)(val >> 24);
                        buffer[1] = (char)(val >> 16);
                        buffer[2] = (char)(val >> 8);
                        buffer[3] = (char)val;
                        if( (buffer += 4) >= buffer_end )
                            break;
                    }
                    break;
                case 'd':
                    while( i < count )
                    {
                        int hi = (int)(((uint64*)data)[i] >> 32);
                        int lo = (int)(((uint64*)data)[i++]);
                        buffer[0] = (char)(hi >> 24);
                        buffer[1] = (char)(hi >> 16);
                        buffer[2] = (char)(hi >> 8);
                        buffer[3] = (char)hi;
                        buffer[4] = (char)(lo >> 24);
                        buffer[5] = (char)(lo >> 16);
                        buffer[6] = (char)(lo >> 8);
                        buffer[7] = (char)lo;
                        if( (buffer += 8) >= buffer_end )
                            break;
                    }
                    break;
                default:
                    CV_ERROR( CV_StsBadArg, "Unknown format specifier" );
                }

                if( buffer >= buffer_end )
                {
                    storage->base64_buffer = buffer;
                    icvWriteBase64Block( storage, 0 );
                    buffer = storage->base64_buffer;
                }

                if( i == count )
                    break;
            }
        }
        else
        {
            for( i = 0; i < count; i++ )
            {
                int n = 0;
                char c = '\0';
                const char* dtp = dt;

                for( ;; )
                {
                    if( --n <= 0 )
                    {
                        if( !dtp[0] )
                            break;
                        n = 1;
                        if( isdigit(dtp[0]))
                        {
                            if( !isdigit(dtp[1]))
                            {
                                n = dtp[0] - '0';
                                dtp++;
                            }
                            else
                            {
                                int dl = 0;
                                if( sscanf( dtp, "%d%n", &n, &dl ) <= 0 )
                                    CV_ERROR(CV_StsBadArg,
                                            "Invalid data type specification");
                                dtp += dl;
                            }
                            if( n == 0 )
                                CV_ERROR(CV_StsBadArg,
                                        "Invalid data type specification");
                        }
                        c = *dtp++;
                    }

                    switch( c )
                    {
                    case 'u':
                    case 'c':
                        *buffer++ = *(char*)(data + k);
                        k++;
                        break;
                    case 's':
                        {
                            int val;
                            k = (k + sizeof(short) - 1) & -(int)sizeof(short);
                            val = *(short*)(data + k);
                            k += sizeof(short);
                            buffer[0] = (char)(val >> 8);
                            buffer[1] = (char)val;
                            buffer += 2;
                        }
                        break;
                    case 'i':
                    case 'f':
                        {
                            int val;
                            k = (k + sizeof(int) - 1) & -(int)sizeof(int);
                            val = *(int*)(data + k);
                            k += sizeof(int);
                            buffer[0] = (char)(val >> 24);
                            buffer[1] = (char)(val >> 16);
                            buffer[2] = (char)(val >> 8);
                            buffer[3] = (char)val;
                            buffer += 4;
                        }
                        break;
                    case 'd':
                        {
                            int hi, lo;
                            k = (k + sizeof(double) - 1) & -(int)sizeof(double);
                            hi = (int)(*(uint64*)(data+k) >> 32);
                            lo = (int)(*(uint64*)(data+k));
                            k += sizeof(double);
                            buffer[0] = (char)(hi >> 24);
                            buffer[1] = (char)(hi >> 16);
                            buffer[2] = (char)(hi >> 8);
                            buffer[3] = (char)hi;
                            buffer[4] = (char)(lo >> 24);
                            buffer[5] = (char)(lo >> 16);
                            buffer[6] = (char)(lo >> 8);
                            buffer[7] = (char)lo;
                            buffer += 8;
                        }
                        break;
                    case 'p':
                        {
                            /* just skip it */
                            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                            k += sizeof(void*);
                        }
                        break;
                    case 'r':
                        {
                            unsigned val;
                            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                            val = (unsigned)(unsigned long)(*(void**)(data+k));
                            buffer[0] = (char)(val >> 24);
                            buffer[1] = (char)(val >> 16);
                            buffer[2] = (char)(val >> 8);
                            buffer[3] = (char)val;
                            k += sizeof(void*);
                            buffer += 4;
                        }
                        break;
                    default:
                        CV_ERROR( CV_StsBadArg, "Unknown format specifier" );
                    }

                    if( buffer >= buffer_end )
                    {
                        storage->base64_buffer = buffer;
                        icvWriteBase64Block( storage, 0 );
                        buffer = storage->base64_buffer;
                    }
                }
            }
        }

        storage->base64_buffer = buffer;
    }
    else
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Unsupported encoding format" );
    }

    __END__;
}


static int
icvReadRawData( CvFileStorage* storage, const char* dt,
                int format, void* _data, int count )
{
    int result = 0;
    const char* data = (const char*)_data;
    const char* buffer = storage->buffer;

    CV_FUNCNAME( "icvReadRawData" );

    __BEGIN__;

    int i = 0, k = 0;
    int pl;
    char* endptr = 0;

    if( !dt || (pl = strlen(dt)) == 0 )
        EXIT; // invalid format specification

    if( format == ICV_FORMAT_TEXT )
    {
        if( pl == 1 || (pl == 2 && isdigit(dt[0])) )
        {
            if( pl == 2 )
                count *= dt[0] - '0';

            switch( dt[pl-1] )
            {
            case 'u':
                for( i = 0; i < count; i++ )
                {
                    ((uchar*)data)[i] = (uchar)icv_strtol( buffer, &endptr, 10 );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'c':
                for( i = 0; i < count; i++ )
                {
                    ((char*)data)[i] = (char)icv_strtol( buffer, &endptr, 10 );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 's':
                for( i = 0; i < count; i++ )
                {
                    ((short*)data)[i] = (short)icv_strtol( buffer, &endptr, 10 );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'i':
                for( i = 0; i < count; i++ )
                {
                    ((int*)data)[i] = (int)icv_strtol( buffer, &endptr, 10 );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'f':
                for( i = 0; i < count; i++ )
                {
                    ((float*)data)[i] = (float)icv_strtod( buffer, &endptr );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'd':
                for( i = 0; i < count; i++ )
                {
                    ((double*)data)[i] = (double)icv_strtod( buffer, &endptr );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            default:
                EXIT;
            }
        }
        else
        {
            for( i = 0; i < count; i++ )
            {
                int n = 0;
                char c = '\0';
                const char* dtp = dt;

                for( ;; )
                {
                    int dl = 0;

                    if( --n <= 0 )
                    {
                        if( !dtp[0] )
                            break;
                        n = 1;
                        if( isdigit(dtp[0]))
                        {
                            if( !isdigit(dtp[1]))
                            {
                                n = dtp[0] - '0';
                                dtp++;
                            }
                            else
                            {
                                if( sscanf( dtp, "%d%n", &n, &dl ) <= 0 )
                                    CV_ERROR(CV_StsBadArg,
                                            "Invalid data type specification");
                                dtp += dl;
                                dl = 0;
                            }
                            if( n == 0 )
                                CV_ERROR(CV_StsBadArg,
                                        "Invalid data type specification");
                        }
                        c = *dtp++;
                    }

                    switch( c )
                    {
                    case 'u':
                        *(uchar*)(data+k) = (uchar)icv_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k++;
                        break;
                    case 'c':
                        *(char*)(data+k) = (char)icv_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k++;
                        break;
                    case 's':
                        k = (k + sizeof(short) - 1) & -(int)sizeof(short);
                        *(short*)(data+k) = (short)icv_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(short);
                        break;
                    case 'i':
                        k = (k + sizeof(int) - 1) & -(int)sizeof(int);
                        *(int*)(data+k) = (int)icv_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(int);
                        break;
                    case 'f':
                        k = (k + sizeof(float) - 1) & -(int)sizeof(float);
                        *(float*)(data+k) = (float)icv_strtod( buffer, &endptr );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(float);
                        break;
                    case 'd':
                        k = (k + sizeof(double) - 1) & -(int)sizeof(double);
                        *(double*)(data+k) = (double)icv_strtod( buffer, &endptr );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(double);
                        break;
                    case 'p':
                        k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                        *(void**)(data+k) = 0;
                        k += sizeof(void*);
                        break;
                    case 'r':
                        k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                        *(unsigned*)(data+k) = (unsigned)icv_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(void*);
                        break;
                    default:
                        EXIT;
                    }
                }
            }
        }
    }
    else if( format == ICV_FORMAT_BINARY )
    {
        uchar* buffer = (uchar*)storage->base64_buffer;
        uchar* buffer_end = (uchar*)storage->base64_buffer_read;

        assert( sizeof(float) == sizeof(int) &&
                sizeof(float) == 4 &&
                sizeof(double) == 8 );

        if( pl == 1 )
        {
            i = 0;

            for( ;; )
            {
                switch( dt[0] )
                {
                case 'u':
                case 'c':
                    while( i < count )
                    {
                        if( buffer >= buffer_end )
                            break;
                        ((uchar*)data)[i++] = *buffer++;
                    }
                    break;
                case 's':
                    while( i < count )
                    {
                        if( buffer + 1 >= buffer_end )
                            break;
                        ((short*)data)[i++] = (short)((buffer[0] << 8) + buffer[1]);
                        buffer += 2;
                    }
                    break;
                case 'i':
                case 'f':
                    while( i < count )
                    {
                        if( buffer + 3 >= buffer_end )
                            break;
                        ((int*)data)[i++] = (buffer[0] << 24) + (buffer[1] << 16) +
                                            (buffer[2] << 8) + buffer[3];
                        buffer += 4;
                    }
                    break;
                case 'd':
                    while( i < count )
                    {
                        unsigned lo, hi;
                        if( buffer + 7 >= buffer_end )
                            break;
                        hi = (buffer[0] << 24) + (buffer[1] << 16) +
                             (buffer[2] << 8) + buffer[3];
                        lo = (buffer[4] << 24) + (buffer[5] << 16) +
                             (buffer[6] << 8) + buffer[7];
                        ((uint64*)data)[i++] = ((uint64)hi << 32) + lo;
                        buffer += 8;
                    }
                    break;
                default:
                    EXIT;
                }

                if( i < count )
                {
                    int len = buffer_end - buffer;
                    storage->base64_buffer = (char*)buffer;
                    if( !icvReadBase64Block( storage ))
                        EXIT;
                    buffer = (uchar*)storage->base64_buffer;
                    buffer_end = (uchar*)storage->base64_buffer_read;
                    if( len >= buffer_end - buffer )
                        EXIT;
                }
                else
                    break;
            }
        }
        else
        {
            for( i = 0; i < count; i++ )
            {
                int n = 0;
                char c = '\0';
                const char* dtp = dt;
                int k0 = k;

                for( ;; )
                {
                    if( (n -= (k != k0)) <= 0 )
                    {
                        if( !dtp[0] )
                            break;
                        n = 1;
                        if( isdigit(dtp[0]))
                        {
                            if( !isdigit(dtp[1]))
                            {
                                n = dtp[0] - '0';
                                dtp++;
                            }
                            else
                            {
                                int dl;
                                if( sscanf( dtp, "%d%n", &n, &dl ) <= 0 )
                                    CV_ERROR(CV_StsBadArg,
                                            "Invalid data type specification");
                                dtp += dl;
                            }
                            if( n == 0 )
                                CV_ERROR(CV_StsBadArg,
                                        "Invalid data type specification");
                        }
                        c = *dtp++;
                    }

                    k0 = k;
                    switch( c )
                    {
                    case 'u':
                    case 'c':
                        if( buffer < buffer_end )
                        {
                            *(uchar*)(data + k) = *buffer++;
                            k++;
                        }
                        break;
                    case 's':
                        if( buffer + 1 < buffer_end )
                        {
                            k = (k + sizeof(short) - 1) & -(int)sizeof(short);
                            *(short*)(data + k) = (short)((buffer[0] << 8) + buffer[1]);
                            k += sizeof(short);
                            buffer += 2;
                        }
                        break;
                    case 'i':
                    case 'f':
                        if( buffer + 3 < buffer_end )
                        {
                            k = (k + sizeof(int) - 1) & -(int)sizeof(int);
                            *(int*)(data + k) = (buffer[0] << 24) + (buffer[1] << 16) +
                                                (buffer[2] << 8) + buffer[3];
                            k += sizeof(int);
                            buffer += 4;
                        }
                        break;
                    case 'd':
                        if( buffer + 7 < buffer_end )
                        {
                            unsigned lo, hi;
                            hi = (buffer[0] << 24) + (buffer[1] << 16) +
                                 (buffer[2] << 8) + buffer[3];
                            lo = (buffer[4] << 24) + (buffer[5] << 16) +
                                 (buffer[6] << 8) + buffer[7];
                            k = (k + sizeof(uint64) - 1) & -(int)sizeof(uint64);
                            *(uint64*)(data + k) = ((uint64)hi << 32) + lo;
                            k += sizeof(uint64);
                            buffer += 8;
                        }
                        break;
                    case 'p':
                        {
                            /* just skip it */
                            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                            *(void**)(data+k) = 0;
                            k += sizeof(void*);
                        }
                        break;
                    case 'r':
                        if( buffer + 3 < buffer_end )
                        {
                            unsigned val;
                            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                            val = (buffer[0] << 24) + (buffer[1] << 16) +
                                 (buffer[2] << 8) + buffer[3];
                            *(void**)(data+k) = (void*)(size_t)val;
                            k += sizeof(void*);
                            buffer += 4;
                        }
                        break;
                    default:
                        EXIT;
                    }

                    if( k == k0 )
                    {
                        int len = buffer_end - buffer;
                        storage->base64_buffer = (char*)buffer;
                        if( !icvReadBase64Block( storage ))
                            EXIT;
                        buffer = (uchar*)storage->base64_buffer;
                        buffer_end = (uchar*)storage->base64_buffer_read;
                        if( len >= buffer_end - buffer )
                            EXIT;
                    }
                }
            }
        }

        storage->base64_buffer = (char*)buffer;
    }
    else
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Unsupported encoding format" );
    }

    result = 1;

    __END__;

    if( format == ICV_FORMAT_TEXT )
        storage->buffer = (char*)buffer;

    return result;
}


static int
icvCalcSize( const char* dt )
{
    int size = 0;

    CV_FUNCNAME( "icvCalcSize" );

    __BEGIN__;

    int k = 0;

    if( !dt )
        EXIT;

    for( ;; )
    {
        char c = '\0';
        int n = 1;

        if( !*dt )
            break;
        if( isdigit(*dt))
        {
            int dl = 0;
            sscanf( dt, "%d%n", &n, &dl );
            if( n == 0 )
                CV_ERROR( CV_StsBadArg, "Invalid data type specification" );
            dt += dl;
        }
        c = *dt++;

        switch( c )
        {
        case 'u':
        case 'c':
            k += n;
            break;
        case 's':
            k = (k + sizeof(short) - 1) & -(int)sizeof(short);
            k += sizeof(short)*n;
            break;
        case 'i':
        case 'f':
            k = (k + sizeof(int) - 1) & -(int)sizeof(int);
            k += sizeof(int)*n;
            break;
        case 'd':
            k = (k + sizeof(double) - 1) & -(int)sizeof(double);
            k += sizeof(double)*n;
            break;
        case 'p':
        case 'r':
            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
            k += sizeof(void*)*n;
            break;
        default:
            CV_ERROR( CV_StsBadArg, "Unknown format specifier" );
        }
    }

    size = k;

    __END__;

    return size;
}


#define ICV_OPENING_TAG 0
#define ICV_CLOSING_TAG 1

static int
icvFindTag( CvFileStorage* storage )
{
    char* buffer = storage->buffer;
    int is_comment = 0, is_control = 0, is_quot = 0, is_directive = 0;
    int result = -1;

    for( ; buffer < storage->buffer_end; buffer++ )
    {
        int c = buffer[0];
        if( c & ~0x7f )
            goto exit_func;
        if( isspace(c))
        {
            if( c == '\n' )
            {
                storage->line++;
                storage->line_start = buffer;
            }
            continue;
        }

        switch( buffer[0] )
        {
        case '-':
            if( !is_quot )
            {
                if( is_comment )
                {
                    if( buffer[1] == '-' && buffer[2] == '>' )
                        buffer += 2, is_comment = 0;
                }
            }
            break;
        case '<':
            if( !is_quot && !is_comment )
            {
                if( buffer[1] == '!' )
                {
                    if( buffer[2] == '-' )
                    {
                        if( buffer[3] == '-' )
                            buffer += 2, is_comment = 1;
                        else
                            goto exit_func;
                    }
                    else
                        is_control++;
                }
                else if( buffer[1] == '?' )
                {
                    buffer++;
                    is_directive = 1;
                }
                else if( buffer[1] == '/' )
                {
                    result = ICV_CLOSING_TAG;
                    buffer += 2;
                    goto exit_func;
                }
                else
                {
                    result = ICV_OPENING_TAG;
                    buffer++;
                    goto exit_func;
                }
            }
            break;
        case '\"':
        case '\'':
            if( is_quot )
            {
                if( c == is_quot )
                    is_quot = 0;
            }
            else if( !is_comment )
            {
                if( is_control == 0 && is_directive == 0 )
                    goto exit_func;
                is_quot = c;
            }
            break;
        case '>':
            if( !is_quot && !is_comment )
            {
                if( --is_control < 0 )
                    goto exit_func;
            }
            break;
        case '?':
            if( !is_quot && !is_comment )
            {
                if( buffer[1] == '>' )
                    if( is_directive )
                        buffer++, is_directive = 0;
                    else
                        goto exit_func;
            }
            break;
        /*default:
            if( !is_quot && !is_comment && !is_control && !is_directive )
                goto exit_func;*/
        }
    }

exit_func:

    storage->buffer = buffer;
    return result;
}


#define ICV_NEED_TAGNAME 0
#define ICV_NEED_ATTRNAME 1
#define ICV_NEED_EQSIGN 2
#define ICV_NEED_ATTRVAL 3

static int
icvParseXML( CvFileStorage* storage )
{
    const int chunk_size = 10;
    int result = 0;

    CV_FUNCNAME( "icvParseXML" );

    __BEGIN__;

    char errbuf[100];
    char* buffer;

    buffer = storage->line_start = storage->buffer = storage->buffer_start;
    storage->line = 1;

    for(;;)
    {
        int need = ICV_NEED_TAGNAME;
        CvFileNode* node = 0;
        CvFileNode stub_node;
        int free_attr = 0;
        char** attr_arr = 0;
        CvAttrList* last_attr = 0;
        int is_id = 0, is_empty = 0;
        int c, tag;

        storage->buffer = buffer;
        tag = icvFindTag( storage );

        if( tag < 0 )
        {
            sprintf( errbuf, "Syntax error at line %d, column %d",
                     storage->line, (int)(buffer - storage->line_start) );
            CV_ERROR( CV_StsError, errbuf );
        }

        if( tag == ICV_OPENING_TAG )
        {
            CV_CALL( node = (CvFileNode*)cvSetNew( storage->nodes ));
        }
        else
            node = &stub_node;

        memset( node, 0, sizeof(*node));

        buffer = storage->buffer;

        #define process_space()                                 \
            if( c != '\n' )                                     \
                ;                                               \
            else                                                \
                storage->line++, storage->line_start = buffer

        for( ; buffer < storage->buffer_end; buffer++ )
        {
            c = buffer[0];
skip_read:
            if( isspace(c) )
            {
                process_space();
                continue;
            }

            if( isalpha(c) || c == '_' || c == ':' )
            {
                char* name_start = buffer;

                if( need != ICV_NEED_TAGNAME && need != ICV_NEED_ATTRNAME )
                    EXIT;
                do
                {
                    c = *++buffer;
                }
                while( isalnum(c) || c == '_' || c == ':' || c == '.' || c == '-' );

                if( need == ICV_NEED_TAGNAME )
                {
                    buffer[0] = '\0';
                    node->tagname = name_start;

                    if( !isspace(c) && c != '/' && c != '>' )
                        EXIT; // a space or closing angle bracket
                              // should go after tag name
                    need = ICV_NEED_ATTRNAME;
                    goto skip_read;
                }
                else if( need == ICV_NEED_ATTRNAME )
                {
                    buffer[0] = '\0';
                    is_id = strcmp( name_start, "id" ) == 0;

                    if( free_attr == 0 )
                    {
                        // allocate another chunk of attributes
                        CvAttrList* new_attr;
                        CV_CALL( new_attr = (CvAttrList*)cvMemStorageAlloc(
                                storage->memstorage,
                                sizeof(*new_attr) + (chunk_size+1)*2*sizeof(char*)));
                        new_attr->attr = (char**)(new_attr+1);
                        new_attr->next = 0;
                        if( !last_attr )
                            node->attr = new_attr;
                        else
                        {
                            *attr_arr = 0;
                            last_attr->next = new_attr;
                        }
                        last_attr = new_attr;
                        attr_arr = new_attr->attr;
                        free_attr = chunk_size;
                    }

                    *attr_arr++ = name_start;

                    if( c == '=' )
                        need = ICV_NEED_ATTRVAL;
                    else
                    {
                        if( !isspace(c))
                            EXIT;
                        need = ICV_NEED_EQSIGN;
                        goto skip_read;
                    }
                }
            }
            else if( c == '=' )
            {
                if( need != ICV_NEED_EQSIGN )
                    EXIT; // unexpected character
                else
                    need = ICV_NEED_ATTRVAL;
            }
            else if( c == '\"' || c == '\'' )
            {
                if( need != ICV_NEED_ATTRVAL )
                    EXIT; // unexpected character
                else
                {
                    int quot_char = c;
                    char* val_start = ++buffer;
                    char* val_end = 0;

                    for( ; buffer < storage->buffer_end; buffer++ )
                    {
                        c = buffer[0];
                        if( isspace(c))
                        {
                            process_space();
                            continue;
                        }

                        if( c == quot_char )
                        {
                            val_end = buffer;
                            *val_end = '\0';
                            break;
                        }
                    }

                    if( !val_end )
                        EXIT; // the attribute value is not closed

                    *attr_arr++ = val_start;
                    if( is_id )
                    {
                        if( node->name )
                            EXIT; // repeated 'id = "..."'
                        else
                            node->name = val_start;
                    }
                    need = ICV_NEED_ATTRNAME;
                }
            }
            else if( c == '/' )
            {
                if( need != ICV_NEED_ATTRNAME )
                    EXIT; // unexpected character
                else if( buffer[1] != '>' )
                    EXIT; // '>' is expected after '/'
                else
                {
                    is_empty = 1;
                    buffer += 2;
                    break;
                }
            }
            else if( c == '>' )
            {
                if( need != ICV_NEED_ATTRNAME )
                    EXIT; // unexpected character
                else
                {
                    buffer++;
                    break;
                }
            }
            else
                EXIT; // unexpected character;
        }

        if( attr_arr )
            *attr_arr++ = 0;

        if( !node->tagname )
            EXIT; // dummy empty tag (e.g. <>)

        if( tag == ICV_CLOSING_TAG )
        {
            if( attr_arr )
                EXIT; // closing tag has attributes
            if( !storage->parent || !storage->parent->tagname ||
                strcmp( storage->parent->tagname, node->tagname ) != 0 )
                EXIT; // unmatched names in the opening and closing tags
            storage->parent = storage->parent->v_prev;
            if( storage->parent == 0 )
            {
                result = 1; // XML has been read completely and
                            // its structure seems to be correct
                break;
            }
        }
        else
        {
            char* type_name;
            if( node->name )
                if( icvQueryName( storage, node->name, node ) != node )
                    EXIT; // duplicated id
            type_name = (char*)cvAttrValue( node->attr, "type" );
            if( type_name )
                CV_CALL( node->typeinfo = cvFindType( type_name ));
            if( !storage->root )
            {
                storage->root = node;
                if( is_empty )
                    EXIT; // the first tag (<opencv_storage>) must be non-empty
            }
            else
                cvInsertNodeIntoTree( node, storage->parent, 0 );
            if( !is_empty )
                storage->parent = node;
            node->body = buffer;
        }
    }

    __END__;

    return result;
}



/************************ standard types *************************/

/*********************** CvMat or CvMatND ************************/

static int
icvIsMat( const void* ptr )
{
    return CV_IS_MAT(ptr);
}


static int
icvIsMatND( const void* ptr )
{
    return CV_IS_MATND(ptr);
}


static void
icvWriteArray( CvFileStorage* storage, const char* name,
               const void* struct_ptr, CvAttrList attr, int /*flags*/ )
{
    CV_FUNCNAME( "icvWriteArray" );

    __BEGIN__;

    char dt_buf[16];
    char size_buf[10*CV_MAX_DIM];
    int format;

    char* new_attr[10];
    int idx = 0;
    CvMat* mat = (CvMat*)struct_ptr;
    CvMatND header;
    CvMatNDIterator iterator;

    if( !icvIsMat(mat) && !icvIsMatND(mat))
        CV_ERROR( CV_StsBadArg, "The input structure is not an array" );

    new_attr[idx++] = "dt";

    sprintf( dt_buf, "%d%c", CV_MAT_CN(mat->type),
             icvTypeSymbol[CV_MAT_DEPTH(mat->type)] );
    new_attr[idx++] = dt_buf + ICV_DT_SHIFT(dt_buf);

    new_attr[idx++] = "size";
    if( CV_IS_MAT(mat))
        sprintf( size_buf, "%d %d", mat->rows, mat->cols );
    else
    {
        CvMatND* matnd = (CvMatND*)struct_ptr;
        int i, size_buf_len = 0;
        for( i = 0; i < matnd->dims; i++ )
        {
            int dl = 0;
            sprintf( size_buf + size_buf_len, "%s%d%n", i == 0 ? "" : " ",
                     matnd->dim[i].size, &dl );
            size_buf_len += dl;
        }
    }
    new_attr[idx++] = size_buf;

    new_attr[idx++] = "format";
    if( ICV_IS_BINARY(storage) )
    {
        format = ICV_FORMAT_BINARY;
        new_attr[idx++] = "xdr.base64";
    }
    else
    {
        format = ICV_FORMAT_TEXT;
        new_attr[idx++] = "text";
    }
    new_attr[idx++] = 0;

    CV_CALL( cvStartWriteStruct( storage, name, CV_IS_MAT(mat) ? "CvMat" : "CvMatND",
                                 struct_ptr, cvAttrList(new_attr, &attr)));
    CV_CALL( icvPrepareArrayOp( 1, (void**)&struct_ptr, 0, &header, &iterator ));

    iterator.size.width *= CV_MAT_CN(mat->type);

    do
    {
        CV_CALL( icvWriteRawData( storage, dt_buf + 1, format,
                                  iterator.ptr[0], iterator.size.width ));
    }
    while( icvNextMatNDSlice( &iterator ));

    CV_CALL( cvEndWriteStruct( storage ));

    __END__;
}


static void*
icvReadArray( CvFileStorage* storage, CvFileNode* node )
{
    void* ptr = 0;

    CV_FUNCNAME( "icvReadArray" );

    __BEGIN__;

    int size[CV_MAX_DIM+1], dims;
    int pl, type = 0, format;
    CvMatND header;
    CvMatNDIterator iterator;
    const char* size_buf;
    const char* dt;
    const char* format_buf;
    const char* pos;
    int is_matnd;

    assert( node && node->tagname && strcmp( node->tagname, "struct" ) == 0 && node->typeinfo &&
            (strcmp( node->typeinfo->type_name, "CvMat") == 0 ||
            strcmp( node->typeinfo->type_name, "CvMatND") == 0));

    is_matnd = strcmp( node->typeinfo->type_name, "CvMatND") == 0;

    size_buf = cvAttrValue( node->attr, "size" );
    format_buf = cvAttrValue( node->attr, "format" );
    dt = cvAttrValue( node->attr, "dt" );

    if( !size_buf || !format_buf || !dt )
        EXIT;

    pl = strlen(dt);
    if( pl != 1 && (pl != 2 || !isdigit(dt[0])))
        EXIT;

    pos = strchr( icvTypeSymbol, dt[pl-1] );
    if( !pos )
        EXIT;

    type = pos - icvTypeSymbol;
    if( pl == 2 )
    {
        if( dt[0] - '0' < 1 || dt[0] - '0' > 4 )
            EXIT;
        type += (dt[0] - '0')*8;
    }

    memset( size, 0, sizeof(size));
    storage->buffer = (char*)size_buf;
    icvReadRawData( storage, "i", ICV_FORMAT_TEXT, size, CV_MAX_DIM );
    for( dims = 0; size[dims] != 0; dims++ )
        if( size[dims] < 0 )
            EXIT;

    if( dims < 1 )
        EXIT;

    if( strcmp( format_buf, "text" ) == 0 )
        format = ICV_FORMAT_TEXT;
    else if( strcmp( format_buf, "xdr.base64" ) == 0 )
        format = ICV_FORMAT_BINARY;
    else
        EXIT;

    if( is_matnd )
    {
        CV_CALL( node->content = cvCreateMatND( dims, size, type ));
    }
    else
    {
        CV_CALL( node->content = cvCreateMat( size[0], dims > 1 ? size[1] : 1, type ));
    }

    CV_CALL( icvPrepareArrayOp( 1, (void**)&node->content, 0, &header, &iterator ));

    iterator.size.width *= CV_MAT_CN(type);
    storage->buffer = (char*)node->body;
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    do
    {
        if( !icvReadRawData( storage, dt + pl - 1, format,
                             iterator.ptr[0], iterator.size.width ))
            EXIT;
    }
    while( icvNextMatNDSlice( &iterator ));

    ptr = (void*)node->content;

    __END__;

    if( !ptr && node )
        cvRelease( (void**)&node->content );

    return ptr;
}


/******************************* IplImage ******************************/

static int
icvIsImage( const void* ptr )
{
    return CV_IS_IMAGE(ptr);
}


static void
icvWriteImage( CvFileStorage* storage, const char* name,
               const void* struct_ptr, CvAttrList attr, int /*flags*/ )
{
    CV_FUNCNAME( "icvWriteImage" );

    __BEGIN__;

    char dt_buf[16];
    char width_buf[16];
    char height_buf[16];
    char roi_buf[32];
    int format, depth;
    int y, width_n;

    const char* new_attr[20];
    int idx = 0;
    IplImage* img = (IplImage*)struct_ptr;

    if( !icvIsImage(img) )
        CV_ERROR( CV_StsBadArg, "The input structure is not an IplImage" );

    if( img->nChannels > 1 && img->dataOrder == IPL_DATA_ORDER_PLANE )
        CV_ERROR( CV_StsBadArg, "Planar IplImage data layout is not supported" );

    if( img->nChannels > 4 )
        CV_ERROR( CV_BadDepth, "Unsupported number of channels" );

    depth = icvIplToCvDepth(img->depth);
    if( depth < 0 )
        CV_ERROR( CV_BadDepth, "Unsupported image depth" );

    new_attr[idx++] = "dt";
    sprintf( dt_buf, "%d%c", img->nChannels, icvTypeSymbol[depth] );

    new_attr[idx++] = dt_buf + ICV_DT_SHIFT(dt_buf);

    new_attr[idx++] = "width";
    sprintf( width_buf, "%d", img->width );
    new_attr[idx++] = width_buf;

    new_attr[idx++] = "height";
    sprintf( height_buf, "%d", img->height );
    new_attr[idx++] = height_buf;

    if( img->roi )
    {
        new_attr[idx++] = "roi";
        sprintf( roi_buf, "%d %d %d %d %d", img->roi->coi,
                 img->roi->xOffset, img->roi->yOffset,
                 img->roi->width, img->roi->height );
        new_attr[idx++] = roi_buf;
    }

    new_attr[idx++] = "origin";
    new_attr[idx++] = img->origin == IPL_ORIGIN_TL ? "tl" : "bl";

    new_attr[idx++] = "format";
    if( ICV_IS_BINARY(storage) )
    {
        format = ICV_FORMAT_BINARY;
        new_attr[idx++] = "xdr.base64";
    }
    else
    {
        format = ICV_FORMAT_TEXT;
        new_attr[idx++] = "text";
    }
    new_attr[idx++] = 0;

    CV_CALL( cvStartWriteStruct( storage, name, "IplImage",
                                 struct_ptr, cvAttrList((char**)new_attr, &attr)));

    width_n = img->width * img->nChannels;

    for( y = 0; y < img->height; y++ )
    {
        CV_CALL( icvWriteRawData( storage, dt_buf + 1, format,
                                  img->imageData + y*img->widthStep, width_n ));
    }

    CV_CALL( cvEndWriteStruct( storage ));

    __END__;
}


static void*
icvReadImage( CvFileStorage* storage, CvFileNode* node )
{
    void* ptr = 0;

    CV_FUNCNAME( "icvReadImage" );

    __BEGIN__;

    CvSize size;
    int pl, depth = 0, channels = 1, format, origin;
    CvMatND header;
    CvMatNDIterator iterator;
    const char* width_buf;
    const char* height_buf;
    const char* dt;
    const char* format_buf;
    const char* roi_buf;
    const char* origin_buf;
    const char* pos;

    assert( node && node->tagname && strcmp( node->tagname, "struct" ) == 0 &&
            node->typeinfo && strcmp( node->typeinfo->type_name, "IplImage") == 0 );

    width_buf = cvAttrValue( node->attr, "width" );
    height_buf = cvAttrValue( node->attr, "height" );
    format_buf = cvAttrValue( node->attr, "format" );
    origin_buf = cvAttrValue( node->attr, "origin" );
    dt = cvAttrValue( node->attr, "dt" );
    roi_buf = cvAttrValue( node->attr, "roi" );

    if( !width_buf || !height_buf || !format_buf || !dt || !origin_buf )
        EXIT;

    pl = strlen(dt);
    if( pl != 1 && (pl != 2 || !isdigit(dt[0])))
        EXIT;

    pos = strchr( icvTypeSymbol, dt[pl-1] );
    if( !pos )
        EXIT;

    depth = icvCvToIplDepth( pos - icvTypeSymbol );

    if( pl == 2 )
    {
        channels = dt[0] - '0';
        if( channels < 1 || channels > 4 )
            EXIT;
    }

    size.width = size.height = 0;
    storage->buffer = (char*)width_buf;
    icvReadRawData( storage, "i", ICV_FORMAT_TEXT, &size.width, 1 );
    storage->buffer = (char*)height_buf;
    icvReadRawData( storage, "i", ICV_FORMAT_TEXT, &size.height, 1 );
    if( size.width <= 0 || size.height <= 0 )
        EXIT;

    if( strcmp( format_buf, "text" ) == 0 )
        format = ICV_FORMAT_TEXT;
    else if( strcmp( format_buf, "xdr.base64" ) == 0 )
        format = ICV_FORMAT_BINARY;
    else
        EXIT;

    if( strcmp( origin_buf, "tl" ) == 0 )
        origin = IPL_ORIGIN_TL;
    else if( strcmp( origin_buf, "bl" ) == 0 )
        origin = IPL_ORIGIN_BL;
    else
        EXIT;

    CV_CALL( node->content = cvCreateImage( size, depth, channels ));
    ((IplImage*)node->content)->origin = origin;

    CV_CALL( icvPrepareArrayOp( 1, (void**)&node->content, 0, &header, &iterator ));

    iterator.size.width *= channels;
    storage->buffer = (char*)node->body;
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    do
    {
        if( !icvReadRawData( storage, dt + pl - 1, format,
                             iterator.ptr[0], iterator.size.width ))
            EXIT;
    }
    while( icvNextMatNDSlice( &iterator ));

    if( roi_buf )
    {
        IplROI roi;
        storage->buffer = (char*)roi_buf;
        if( !icvReadRawData( storage, "5i", ICV_FORMAT_TEXT, &roi, 1 ))
            EXIT;
        CV_CALL( cvSetImageROI( (IplImage*)node->content,
                 cvRect( roi.xOffset, roi.yOffset, roi.width, roi.height )));
        CV_CALL( cvSetImageCOI( (IplImage*)node->content, roi.coi ));
    }

    ptr = (void*)node->content;

    __END__;

    if( !ptr && node )
        cvRelease( (void**)&node->content );

    return ptr;
}


/******************************* CvSeq ******************************/

static int
icvIsSeq( const void* ptr )
{
    return CV_IS_SEQ(ptr) || CV_IS_SET(ptr);
}


static void
icvReleaseSeq( void** ptr )
{
    CV_FUNCNAME( "icvReleaseSeq" );

    __BEGIN__;

    if( !ptr )
        CV_ERROR( CV_StsNullPtr, "NULL double pointer" );

    *ptr = 0; // it's impossible now to release seq, so just clear the pointer

    __END__;
}


static void*
icvCloneSeq( const void* ptr )
{
    return cvSeqSlice( (CvSeq*)ptr, CV_WHOLE_SEQ,
                       0 /* use the same storage as for the original sequence */, 1 );
}


static void
icvStartWriteDynStruct( CvFileStorage* storage, const char* name,
                        const char* type_name, const void* struct_ptr,
                        const char* dt, const char* header_dt0,
                        CvAttrList attr, int header_offset )
{
    CV_FUNCNAME( "icvStartWriteDynSeq" );

    __BEGIN__;

    char header_dt_buf[64];
    char flags_buf[16];
    char name_buf[16];
    char ptr_buf[4][16];
    const char* header_dt = 0;
    int dl = 0;

    const char* new_attr[30];
    int idx = 0;
    CvSeq* seq = (CvSeq*)struct_ptr;

    if( name && name[0] )
    {
        new_attr[idx++] = "id";
        if( strcmp(name,"<auto>") == 0 )
        {
            sprintf( name_buf, "%p", struct_ptr );
            name = name_buf;
        }
        new_attr[idx++] = (char*)name;
    }

    new_attr[idx++] = "type";
    new_attr[idx++] = (char*)type_name;

    new_attr[idx++] = "flags";
    sprintf( flags_buf, "%08x", seq->flags );
    new_attr[idx++] = flags_buf;

    if( seq->h_prev )
    {
        new_attr[idx++] = "h_prev";
        sprintf( ptr_buf[0], "%p", seq->h_prev );
        new_attr[idx++] = ptr_buf[0];
    }

    if( seq->h_next )
    {
        new_attr[idx++] = "h_next";
        sprintf( ptr_buf[1], "%p", seq->h_next );
        new_attr[idx++] = ptr_buf[1];
    }

    if( seq->v_prev )
    {
        new_attr[idx++] = "v_prev";
        sprintf( ptr_buf[2], "%p", seq->v_prev );
        new_attr[idx++] = ptr_buf[2];
    }

    if( seq->v_next )
    {
        new_attr[idx++] = "v_next";
        sprintf( ptr_buf[3], "%p", seq->v_next );
        new_attr[idx++] = ptr_buf[3];
    }

    if( !cvAttrValue( &attr, "dt" ))
    {
        new_attr[idx++] = "dt";
        new_attr[idx++] = (char*)dt;
    }

    new_attr[idx++] = "format";
    new_attr[idx++] = ICV_IS_BINARY(storage) ? "xdr.base64" : "text";

    if( header_offset == 0 )
        header_offset = CV_IS_GRAPH(seq) ? sizeof(CvGraph) :
                        CV_IS_SET(seq) ? sizeof(CvSet) : sizeof(CvSeq);

    if( seq->header_size > header_offset )
    {
        header_dt = cvAttrValue( &attr, "header_dt" );
        if( !header_dt )
        {
            if( header_dt0 )
                header_dt = header_dt0;
            else
            {
                sprintf( header_dt_buf, "%du", seq->header_size - header_offset );
                header_dt = header_dt_buf;
            }
            new_attr[idx++] = "header_dt";
            new_attr[idx++] = (char*)header_dt;
        }
        else
        {
            int size;
            CV_CALL( size = icvCalcSize( header_dt ));
            if( size != seq->header_size - header_offset )
                CV_ERROR( CV_StsBadArg,
                "Sequence header data spec doesn't match "
                "to the total size of user-defined header fields" );
        }
    }

    new_attr[idx++] = 0;
    CV_CALL( icvPushXMLTag( storage, "struct", cvAttrList( (char**)new_attr, &attr ),
                            ICV_INCOMPLETE_TAG ));

    if( header_dt )
    {
        fprintf( storage->file, " header=\"%n", &dl );
        storage->width += dl;

        CV_CALL( icvWriteRawData( storage, header_dt, ICV_FORMAT_TEXT,
                                  (char*)seq + header_offset, 1 ));
        fprintf( storage->file, "\"" );
    }
    fprintf( storage->file, ">\n" );
    storage->width = 0;

    __END__;
}


static void
icvWriteSeq( CvFileStorage* storage, const char* name,
             const void* struct_ptr, CvAttrList attr, int flags )
{
    CV_FUNCNAME( "icvWriteSeq" );

    __BEGIN__;

    CvSeq* seq = (CvSeq*)struct_ptr;
    CvTreeNodeIterator iterator;

    char dt_buf[64];
    const char* dt = cvAttrValue( &attr, "dt" );
    const char* header_dt = cvAttrValue( &attr, "header_dt" );
    char header_dt_buf[64];
    int format = ICV_IS_BINARY(storage);

    if( CV_IS_SEQ(seq) && CV_IS_SEQ_POINT_SET(seq) && !header_dt )
    {
        if( seq->header_size == sizeof(CvContour) )
            header_dt = "8i";
        else
        {
            sprintf( header_dt_buf, "8i%du", (int)(seq->header_size - sizeof(CvContour)) );
            header_dt = header_dt_buf;
        }
    }

    if( !dt )
    {
        if( CV_IS_SEQ( seq ) && icvPixSize[CV_SEQ_ELTYPE(seq)] == seq->elem_size )
            sprintf( dt_buf, "%d%c", CV_MAT_CN(seq->flags),
                     icvTypeSymbol[CV_MAT_DEPTH(seq->flags)] );
        else
            sprintf( dt_buf, "%du", seq->elem_size );
        dt = dt_buf + ICV_DT_SHIFT(dt_buf);
    }
    else
    {
        int size;
        CV_CALL( size = icvCalcSize( dt ));
        if( size != seq->elem_size )
            CV_ERROR( CV_StsBadArg,
            "Sequence element data spec doesn't match to the element size" );
    }

    if( flags & CV_WRITE_TREE )
    {
        CV_CALL( cvStartWriteStruct( storage, name, "CvSeq.tree", 0, attr ));
        name = "<auto>";
        attr = cvAttrList();
        cvInitTreeNodeIterator( &iterator, seq, INT_MAX );
    }
    else
    {
        iterator.node = struct_ptr;
    }

    while( iterator.node )
    {
        CvSeq* seq1 = (CvSeq*)iterator.node;
        CV_CALL( icvStartWriteDynStruct( storage, name, "CvSeq", seq1,
                                         dt, header_dt, attr, 0 ));
        if( CV_IS_SEQ( seq1 ))
        {
            CvSeqBlock* block = seq1->first;
            do
            {
                CV_CALL( icvWriteRawData( storage, dt, format,
                                          block->data, block->count ));
                block = block->next;
            }
            while( block != seq1->first );
        }
        else
        {
            int i;
            CvSeqReader reader;
            cvStartReadSeq( seq, &reader );
            for( i = 0; i < seq->total; i++ )
            {
                if( CV_IS_SET_ELEM( reader.ptr ))
                {
                    CV_CALL( icvWriteRawData( storage, dt, format,
                                              reader.ptr, 1 ));
                }
                CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
            }
        }
        CV_CALL( icvPopXMLTag( storage ));
        if( !(flags & CV_WRITE_TREE) )
            EXIT;
        cvNextTreeNode( &iterator );
    }

    CV_CALL( cvEndWriteStruct( storage ));

    __END__;
}


static void*
icvReadDynStruct( CvFileStorage* storage, CvFileNode* node,
                  int header_size0 )
{
    void* ptr = 0;

    CV_FUNCNAME( "icvReadDynStruct" );

    __BEGIN__;

    int format, flags, elem_size;
    int header_size;
    const char* dt;
    const char* format_buf;
    const char* flags_buf;
    const char* header_dt;
    const char* header_buf = 0;
    void* elem_buf = 0;

    assert( node && node->tagname && strcmp( node->tagname, "struct" ) == 0 &&
            node->typeinfo && strcmp( node->typeinfo->type_name, "CvSeq") == 0 );

    format_buf = cvAttrValue( node->attr, "format" );
    flags_buf = cvAttrValue( node->attr, "flags" );
    dt = cvAttrValue( node->attr, "dt" );
    header_dt = cvAttrValue( node->attr, "header_dt" );

    if( !format_buf || !flags_buf || !dt )
        EXIT;

    elem_size = icvCalcSize( dt );
    if( !elem_size )
        EXIT;

    if( strcmp( format_buf, "text" ) == 0 )
        format = ICV_FORMAT_TEXT;
    else if( strcmp( format_buf, "xdr.base64" ) == 0 )
        format = ICV_FORMAT_BINARY;
    else
        EXIT;

    if( sscanf( flags_buf, "%x", &flags ) <= 0 )
        EXIT;

    if( header_size0 == 0 )
        header_size0 = (flags & CV_MAGIC_MASK) == CV_SEQ_MAGIC_VAL ? sizeof(CvSeq) :
                       (flags & CV_MAGIC_MASK) == CV_SET_MAGIC_VAL ? sizeof(CvSet) : 0;

    header_size = header_size0;

    if( header_dt )
    {
        header_buf = cvAttrValue( node->attr, "header" );
        int size = icvCalcSize( header_dt );
        if( !header_buf || !size )
            EXIT;
        header_size += size;
    }

    header_size = (header_size + sizeof(void*) - 1) & -(int)sizeof(void*);

    elem_buf = alloca( elem_size );
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    switch( flags & CV_MAGIC_MASK )
    {
    case CV_SEQ_MAGIC_VAL:
        {
            CvSeqWriter writer;
            CvSeq* seq;

            if( header_size0 < (int)sizeof(CvSeq))
                EXIT;

            CV_CALL( node->content = seq = cvCreateSeq( flags, header_size, elem_size,
                                                        storage->dststorage ));
            if( header_dt )
            {
                storage->buffer = (char*)header_buf;
                if( !icvReadRawData( storage, header_dt, ICV_FORMAT_TEXT,
                                     (char*)seq + header_size0, 1 ))
                    EXIT;
            }

            storage->buffer = (char*)node->body;

            cvStartAppendToSeq( seq, &writer );

            for(;;)
            {
                if( !icvReadRawData( storage, dt, format, elem_buf, 1 ))
                    break;
                CV_WRITE_SEQ_ELEM_VAR( elem_buf, writer );
            }
            cvEndWriteSeq( &writer );
        }
        break;
    case CV_SET_MAGIC_VAL:
        {
            CvSet* set;

            if( header_size0 < (int)sizeof(CvSet))
                EXIT;

            CV_CALL( node->content = set = cvCreateSet( flags, header_size, elem_size,
                                                        storage->dststorage ));

            if( header_dt )
            {
                storage->buffer = (char*)header_buf;
                if( !icvReadRawData( storage, header_dt, ICV_FORMAT_TEXT,
                                     (char*)set + header_size0, 1 ))
                    EXIT;
            }

            storage->buffer = (char*)node->body;

            for(;;)
            {
                CvSetElem* set_elem = cvSetNew( set );
                if( !icvReadRawData( storage, dt, format, set_elem, 1 ))
                {
                    cvSetRemoveByPtr( set, set_elem );
                    break;
                }

                set_elem->flags &= INT_MAX;
            }
        }
        break;
    default:
        EXIT;
    }

    ptr = (void*)node->content;

    __END__;

    if( !ptr && node )
        cvRelease( (void**)&node->content );

    return ptr;
}


static void*
icvReadSeq( CvFileStorage* storage, CvFileNode* node )
{
    void* ptr = 0;

    CV_FUNCNAME( "icvReadSeq" );

    __BEGIN__;

    if( strcmp( node->typeinfo->type_name, "CvSeq" ) == 0 )
    {
        CV_CALL( ptr = icvReadDynStruct( storage, node, 0 ));
    }
    else if( strcmp( node->typeinfo->type_name, "CvSeq.tree" ) == 0 )
    {
        CvFileNode* root = node->v_next;
        CvTreeNodeIterator iterator;

        // pass 1. make sure all the underlying contours have been read
        cvInitTreeNodeIterator( &iterator, root, INT_MAX );
        for( ; iterator.node; cvNextTreeNode( &iterator ))
        {
            CvFileNode* contour_node = (CvFileNode*)iterator.node;
            if( !contour_node->content )
            {
                assert( contour_node->typeinfo && contour_node->typeinfo->read );
                contour_node->typeinfo->read( storage, contour_node );
            }
            assert( CV_IS_SEQ( contour_node->content ));
        }

        // pass 2. restore the links
        cvInitTreeNodeIterator( &iterator, root, INT_MAX );
        for( ; iterator.node; cvNextTreeNode( &iterator ))
        {
            CvFileNode* contour_node = (CvFileNode*)iterator.node;
            CvSeq* contour = (CvSeq*)contour_node->content;
            const char* v_next = cvAttrValue( contour_node->attr, "v_next" );
            const char* v_prev = cvAttrValue( contour_node->attr, "v_prev" );
            const char* h_next = cvAttrValue( contour_node->attr, "h_next" );
            const char* h_prev = cvAttrValue( contour_node->attr, "h_prev" );

            if( h_next && !contour->h_next )
            {
                CvFileNode* t = (CvFileNode*)cvGetFileNode( storage, h_next );
                CvSeq* t_content;
                if( !t || !CV_IS_SEQ(t->content) || t->v_prev != node )
                    EXIT;
                t_content = (CvSeq*)t->content;
                contour->h_next = t_content;
                t_content->h_prev = contour;
                if( contour->v_prev )
                    t_content->v_prev = contour->v_prev;
                else if( t_content->v_prev )
                    contour->v_prev = t_content->v_prev;
            }

            if( h_prev && !contour->h_prev )
            {
                CvFileNode* t = (CvFileNode*)cvGetFileNode( storage, h_prev );
                CvSeq* t_content;
                if( !t || !CV_IS_SEQ(t->content) || t->v_prev != node )
                    EXIT;
                t_content = (CvSeq*)t->content;
                contour->h_prev = t_content;
                t_content->h_next = contour;
                if( contour->v_prev )
                    t_content->v_prev = contour->v_prev;
                else if( t_content->v_prev )
                    contour->v_prev = t_content->v_prev;
            }

            if( v_next && !contour->v_next )
            {
                CvFileNode* t = (CvFileNode*)cvGetFileNode( storage, v_next );
                if( !t || !CV_IS_SEQ(t->content) || t->v_prev != node )
                    EXIT;
                contour->v_next = (CvSeq*)t->content;
                ((CvSeq*)t->content)->v_prev = contour;
            }

            if( v_prev && !contour->v_prev )
            {
                CvFileNode* t = (CvFileNode*)cvGetFileNode( storage, v_prev );
                if( !t || !CV_IS_SEQ(t->content) || t->v_prev != node )
                    EXIT;
                contour->v_prev = (CvSeq*)t->content;
            }
        }

        // find the tree root
        {
        CvSeq* root_contour = (CvSeq*)root->content;
        while( root_contour->v_prev )
            root_contour = root_contour->v_prev;
        while( root_contour->h_prev )
            root_contour = root_contour->h_prev;

        node->content = ptr = root_contour;
        }
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "Unknown type name" );
    }

    __END__;

    return ptr;
}



/******************************* CvGraph ******************************/

static int
icvIsGraph( const void* ptr )
{
    return CV_IS_GRAPH(ptr);
}


static void
icvReleaseGraph( void** ptr )
{
    CV_FUNCNAME( "icvReleaseGraph" );

    __BEGIN__;

    if( !ptr )
        CV_ERROR( CV_StsNullPtr, "NULL double pointer" );

    *ptr = 0; // it's impossible now to release graph, so just clear the pointer

    __END__;
}


static void*
icvCloneGraph( const void* ptr )
{
    return cvCloneGraph( (const CvGraph*)ptr, 0 );
}


static void
icvWriteGraph( CvFileStorage* storage, const char* name,
               const void* struct_ptr, CvAttrList attr, int /*flags*/ )
{
    int* flag_buffer = 0;

    CV_FUNCNAME( "icvWriteGraph" );

    __BEGIN__;

    const char* header_dt = cvAttrValue( &attr, "header_dt" );
    const char* vtx_dt = cvAttrValue( &attr, "vtx_dt" );
    const char* edge_dt = cvAttrValue( &attr, "edge_dt" );
    char vtx_dt_buf[64];
    char edge_dt_buf[64];
    int i, k;
    int format = ICV_IS_BINARY(storage);
    CvGraph* graph = (CvGraph*)struct_ptr;
    CvSeqReader reader;

    assert( CV_IS_GRAPH(graph) );

    CV_CALL( flag_buffer = (int*)cvAlloc( graph->total*sizeof(flag_buffer[0])));

    if( !vtx_dt )
    {
        if( graph->elem_size > (int)sizeof(CvGraphVtx))
        {
            sprintf( vtx_dt_buf, "ip%du", (int)(graph->elem_size - sizeof(CvGraphVtx)));
            vtx_dt = vtx_dt_buf;
        }
        else
            vtx_dt = "ip";
    }
    else if( strncmp( vtx_dt, "ip", 2 ) != 0 )
        CV_ERROR( CV_StsBadArg, "Graph vertex format spec should "
                                "start with CvGraphVtx (ip)" );

    if( !edge_dt )
    {
        if( graph->edges->elem_size > (int)sizeof(CvGraphEdge))
        {
            sprintf( edge_dt_buf, "if2p2r%du", (int)(graph->elem_size - sizeof(CvGraphVtx)));
            edge_dt = edge_dt_buf;
        }
        else
            edge_dt = "if2p2r";
    }
    else if( strncmp( edge_dt, "if2p2r", 6 ) != 0 )
        CV_ERROR( CV_StsBadArg, "Graph vertex format spec should "
                                "start with CvGraphEdge (\"if2p2r\")" );

    CV_CALL( cvStartWriteStruct( storage, name, "CvGraph", graph, attr ));

    // pass 1. Save flags and write vertices
    CV_CALL( icvStartWriteDynStruct( storage, 0, "CvSeq", graph, vtx_dt,
                                     header_dt, cvAttrList(), 0 ));

    cvStartReadSeq( (CvSeq*)graph, &reader );
    for( i = 0, k = 0; i < graph->total; i++ )
    {
        if( CV_IS_SET_ELEM( reader.ptr ))
        {
            CvGraphVtx* vtx = (CvGraphVtx*)reader.ptr;
            CV_CALL( icvWriteRawData( storage, vtx_dt, format, vtx, 1 ));
            flag_buffer[k] = vtx->flags;
            vtx->flags = k++;
        }
        CV_NEXT_SEQ_ELEM( graph->elem_size, reader );
    }

    CV_CALL( icvPopXMLTag( storage ));

    // pass 2. Write edges
    CV_CALL( icvStartWriteDynStruct( storage, 0, "CvSeq", graph->edges, edge_dt,
                                     0, cvAttrList(), 0 ));
    cvStartReadSeq( (CvSeq*)graph->edges, &reader );
    for( i = 0, k = 0; i < graph->edges->total; i++ )
    {
        if( CV_IS_SET_ELEM( reader.ptr ))
        {
            CvGraphEdge* edge = (CvGraphEdge*)reader.ptr;
            CvGraphVtx* vtx0 = edge->vtx[0];
            CvGraphVtx* vtx1 = edge->vtx[1];
            edge->vtx[0] = (CvGraphVtx*)(size_t)(vtx0->flags);
            edge->vtx[1] = (CvGraphVtx*)(size_t)(vtx1->flags);
            CV_CALL( icvWriteRawData( storage, edge_dt, format, edge, 1 ));
            edge->vtx[0] = vtx0;
            edge->vtx[1] = vtx1;
        }
        CV_NEXT_SEQ_ELEM( graph->edges->elem_size, reader );
    }

    CV_CALL( icvPopXMLTag( storage ));

    // pass 3. Restore flags
    cvStartReadSeq( (CvSeq*)graph, &reader );
    for( i = 0, k = 0; i < graph->total; i++ )
    {
        if( CV_IS_SET_ELEM( reader.ptr ))
        {
            CvGraphVtx* vtx = (CvGraphVtx*)reader.ptr;
            vtx->flags = flag_buffer[k++];
        }
        CV_NEXT_SEQ_ELEM( graph->elem_size, reader );
    }

    CV_CALL( cvEndWriteStruct( storage ));

    __END__;

    cvFree( (void**)&flag_buffer );
}


static void*
icvReadGraph( CvFileStorage* storage, CvFileNode* node )
{
    void* ptr = 0;
    CvGraphVtx** ptr_buffer = 0;

    CV_FUNCNAME( "icvReadGraph" );

    __BEGIN__;

    CvGraph* graph;
    CvSeqReader reader;
    CvFileNode *vtx_node, *edge_node;
    int i, k;
    int flags = 0;
    const int graph_mask = CV_MAGIC_MASK|CV_SEQ_KIND_MASK;
    const int graph_sign = CV_SET_MAGIC_VAL|CV_SEQ_KIND_GRAPH;

    int format, vtx_size, edge_size;
    int header_size = sizeof(CvGraph);
    const char* vtx_dt;
    const char* edge_dt;
    const char* format_buf;
    const char* flags_buf;
    const char* header_dt;
    const char* header_buf = 0;
    int count = 0;
    CvGraphVtx* vtx_buffer;
    CvGraphEdge* edge_buffer;

    vtx_node = node->v_next;
    edge_node = node->v_next->h_next;

    flags_buf = cvAttrValue( vtx_node->attr, "flags" );
    if( !flags_buf || sscanf( flags_buf, "%x", &flags ) <= 0 )
        EXIT;

    if( (flags & graph_mask) != graph_sign )
    {
        CvFileNode* t;
        CV_SWAP( vtx_node, edge_node, t );
        flags_buf = cvAttrValue( vtx_node->attr, "flags" );
        if( !flags_buf || sscanf( flags_buf, "%x", &flags ) <= 0 )
            EXIT;
        if( (flags & graph_mask) != graph_sign )
            EXIT;
    }

    vtx_dt = cvAttrValue( vtx_node->attr, "dt" );
    edge_dt = cvAttrValue( edge_node->attr, "dt" );
    header_dt = cvAttrValue( vtx_node->attr, "header_dt" );

    if( !vtx_dt || !edge_dt )
        EXIT;

    vtx_size = icvCalcSize( vtx_dt );
    edge_size = icvCalcSize( edge_dt );
    vtx_buffer = (CvGraphVtx*)alloca( vtx_size );
    edge_buffer = (CvGraphEdge*)alloca( edge_size );

    if( vtx_size < (int)sizeof(CvGraphVtx) || edge_size < (int)sizeof(CvGraphEdge))
        EXIT;

    if( header_dt )
    {
        header_buf = cvAttrValue( node->attr, "header" );
        int size = icvCalcSize( header_dt );
        if( !header_buf || !size )
            EXIT;
        header_size += size;
    }

    header_size = (header_size + sizeof(void*) - 1) & -(int)sizeof(void*);

    CV_CALL( graph = cvCreateGraph( flags, header_size, vtx_size,
                                    edge_size, storage->dststorage ));

    if( header_dt )
    {
        storage->buffer = (char*)header_buf;
        if( !icvReadRawData( storage, header_dt, ICV_FORMAT_TEXT,
                             (char*)(graph + 1), 1 ))
            EXIT;
    }

    format_buf = cvAttrValue( vtx_node->attr, "format" );
    if( strcmp( format_buf, "text" ) == 0 )
        format = ICV_FORMAT_TEXT;
    else if( strcmp( format_buf, "xdr.base64" ) == 0 )
        format = ICV_FORMAT_BINARY;
    else
        EXIT;

    storage->buffer = (char*)vtx_node->body;
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    // pass 1. read vertices
    for(;;)
    {
        if( !icvReadRawData( storage, vtx_dt, format, vtx_buffer, 1 ))
            break;
        cvGraphAddVtx( graph, vtx_buffer, 0 );
        count++;
    }

    CV_CALL( ptr_buffer = (CvGraphVtx**)cvAlloc( count*sizeof(ptr_buffer[0])));

    // pass 2: init vertices pointers
    cvStartReadSeq( (CvSeq*)graph, &reader );
    for( i = 0, k = 0; i < graph->total; i++ )
    {
        if( CV_IS_SET_ELEM( reader.ptr ))
        {
            ptr_buffer[k++] = (CvGraphVtx*)reader.ptr;
        }
        CV_NEXT_SEQ_ELEM( vtx_size, reader );
    }

    storage->buffer = (char*)edge_node->body;
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    // pass 3. read edges
    for(;;)
    {
        CvGraphVtx *org, *dst;
        if( !icvReadRawData( storage, edge_dt, format, edge_buffer, 1 ))
            break;
        org = ptr_buffer[(unsigned)(size_t)edge_buffer->vtx[0]];
        dst = ptr_buffer[(unsigned)(size_t)edge_buffer->vtx[1]];
        cvGraphAddEdgeByPtr( graph, org, dst, edge_buffer, 0 );
    }

    node->content = ptr = graph;

    __END__;

    cvFree( (void**)&ptr_buffer );

    if( !ptr && node )
        cvRelease( (void**)&node->content );

    return ptr;
}

/**************************** standard type initialization *****************************/

static CvTypeInfo*
icvInitTypeInfo( CvTypeInfo* info, const char* type_name,
                 CvIsInstanceFunc is_instance, CvReleaseFunc release,
                 CvReadFunc read, CvWriteFunc write,
                 CvCloneFunc clone )
{
    info->flags = 0;
    info->header_size = sizeof(CvTypeInfo);
    info->type_name = type_name;
    info->prev = info->next = 0;
    info->is_instance = is_instance;
    info->release = release;
    info->clone = clone;
    info->read = read;
    info->write = write;

    return info;
}


static CvTypeInfo *icvFirstType = 0, *icvLastType = 0;

static void
icvCreateStandardTypes(void)
{
    CvTypeInfo info;
    CvTypeInfo dummy_first;

    icvFirstType = &dummy_first;

    cvRegisterType( icvInitTypeInfo( &info, "CvSeq", icvIsSeq, icvReleaseSeq,
                                    icvReadSeq, icvWriteSeq, icvCloneSeq ));
    cvRegisterType( icvInitTypeInfo( &info, "CvSeq.tree", icvIsSeq, icvReleaseSeq,
                                    icvReadSeq, icvWriteSeq, icvCloneSeq ));
    cvRegisterType( icvInitTypeInfo( &info, "CvGraph", icvIsGraph, icvReleaseGraph,
                                    icvReadGraph, icvWriteGraph, icvCloneGraph ));
    cvRegisterType( icvInitTypeInfo( &info, "IplImage", icvIsImage,
                                    (CvReleaseFunc)cvReleaseImage,
                                    icvReadImage, icvWriteImage,
                                    (CvCloneFunc)cvCloneImage ));
    cvRegisterType( icvInitTypeInfo( &info, "CvMat", icvIsMat, (CvReleaseFunc)cvReleaseMat,
                                     icvReadArray, icvWriteArray, (CvCloneFunc)cvCloneMat ));
    cvRegisterType( icvInitTypeInfo( &info, "CvMatND", icvIsMatND, (CvReleaseFunc)cvReleaseMatND,
                                     icvReadArray, icvWriteArray, (CvCloneFunc)cvCloneMatND ));

    /*icvLastType = icvLastType->prev;
    icvLastType->next = 0;*/
}


CV_IMPL  CvTypeInfo*
cvRegisterType( CvTypeInfo* _info )
{
    CvTypeInfo* info = 0;

    CV_FUNCNAME("cvRegisterType" );

    __BEGIN__;

    int len;

    if( !icvFirstType )
        icvCreateStandardTypes();

    if( !_info || _info->header_size != sizeof(CvTypeInfo) )
        CV_ERROR( CV_StsBadSize, "Invalid type info" );

    if( !_info->is_instance || !_info->release ||
        !_info->read || !_info->write )
        CV_ERROR( CV_StsNullPtr,
        "Some of required function pointers "
        "(is_instance, release, read or write) are NULL");

    CV_CALL( len = icvCheckString(_info->type_name) );
    CV_CALL( info = (CvTypeInfo*)cvAlloc( sizeof(*info) + len + 1 ));

    *info = *_info;
    info->type_name = (char*)(info + 1);

    strcpy( (char*)info->type_name, _info->type_name );

    info->flags = 0;
    info->next = 0;

    if( !icvLastType )
        icvFirstType = icvLastType = info;
    else
    {
        info->next = icvFirstType;
        icvFirstType = icvFirstType->prev = info;
    }

    __END__;

    return info;
}


CV_IMPL void
cvUnregisterType( const char* type_name )
{
    CV_FUNCNAME("cvUnregisterType" );

    __BEGIN__;

    CvTypeInfo* info;

    CV_CALL( info = cvFindType( type_name ));
    if( info )
    {
        if( info->prev )
            info->prev->next = info->next;
        else
            icvFirstType = info->next;

        if( info->next )
            info->next->prev = info->prev;
        else
            icvLastType = info->prev;

        if( !icvFirstType || !icvLastType )
            icvFirstType = icvLastType = 0;

        cvFree( (void**)&info );
    }

    __END__;
}


CV_IMPL CvTypeInfo*
cvFirstType( void )
{
    CvTypeInfo* info = 0;

    /*CV_FUNCNAME("cvFirstType" );*/

    __BEGIN__;

    if( !icvFirstType )
        icvCreateStandardTypes();

    info = icvFirstType;

    __END__;

    return info;
}


CV_IMPL CvTypeInfo*
cvFindType( const char* type_name )
{
    CvTypeInfo* info = 0;

    CV_FUNCNAME("cvFindType" );

    __BEGIN__;

    CV_CALL( icvCheckString( type_name ));

    if( !icvFirstType )
        icvCreateStandardTypes();

    for( info = icvFirstType; info != 0; info = info->next )
        if( strcmp( info->type_name, type_name ) == 0 )
            break;

    __END__;

    return info;
}


CV_IMPL CvTypeInfo*
cvTypeOf( const void* struct_ptr )
{
    CvTypeInfo* info = 0;

    /*CV_FUNCNAME("cvFindType" );*/

    __BEGIN__;

    if( !icvFirstType )
        icvCreateStandardTypes();

    for( info = icvFirstType; info != 0; info = info->next )
        if( info->is_instance( struct_ptr ))
            break;

    __END__;

    return info;
}


/* universal functions */
CV_IMPL void
cvRelease( void** struct_ptr )
{
    CV_FUNCNAME("cvRelease" );

    __BEGIN__;

    CvTypeInfo* info;

    if( !struct_ptr )
        CV_ERROR( CV_StsNullPtr, "NULL double pointer" );

    if( *struct_ptr )
    {
        CV_CALL( info = cvTypeOf( *struct_ptr ));
        if( !info->release )
            CV_ERROR( CV_StsError, "release function pointer is NULL" );

        CV_CALL( info->release( struct_ptr ));
        *struct_ptr = 0;
    }

    __END__;
}


void* cvClone( const void* struct_ptr )
{
    void* struct_copy = 0;

    CV_FUNCNAME("cvClone" );

    __BEGIN__;

    CvTypeInfo* info;

    if( !struct_ptr )
        CV_ERROR( CV_StsNullPtr, "NULL structure pointer" );

    CV_CALL( info = cvTypeOf( struct_ptr ));
    if( !info->clone )
        CV_ERROR( CV_StsError, "clone function pointer is NULL" );

    CV_CALL( struct_copy = info->clone( struct_ptr ));

    __END__;

    return struct_copy;
}


/********************************** High-level functions ********************************/


/* "black box" file storage */
typedef struct CvFileStorage CvFileStorage;

#define ICV_FILE_STORAGE_INPUT_FLAG_MASK (0xffff)

/* opens existing or creates new file storage */
CV_IMPL  CvFileStorage*
cvOpenFileStorage( const char* filename, CvMemStorage* memstorage, int flags )
{
    CvFileStorage* storage = 0;
    int ok = 0;

    CV_FUNCNAME("cvOpenFileStorage" );

    __BEGIN__;

    const char* mode = "rb";
    int base64_buffer_size = 1 << 10;

    CV_CALL( storage = (CvFileStorage*)cvAlloc( sizeof(*storage) ));
    memset( storage, 0, sizeof(*storage));
    storage->flags = flags & ICV_FILE_STORAGE_INPUT_FLAG_MASK;

    if( memstorage )
    {
        if( memstorage->block_size < base64_buffer_size )
            CV_ERROR( CV_StsBadSize, "destination storage has too small size" );
        storage->dststorage = memstorage;
        CV_CALL( storage->memstorage = cvCreateChildMemStorage( storage->dststorage ));
    }
    else
        CV_CALL( storage->dststorage = storage->memstorage =
                 cvCreateMemStorage( base64_buffer_size*16 ));

    CV_CALL( storage->filename = (char*)cvMemStorageAlloc( storage->memstorage,
                                                           strlen(filename) + 1 ));

    if( (flags & 3) != CV_STORAGE_READ )
        mode = "wt";

    strcpy( storage->filename, filename );
    storage->file = fopen( storage->filename, mode );
    if( !storage->file )
        EXIT;

    CV_CALL( storage->nodes = cvCreateSet( 0, sizeof(CvSet),
                   sizeof(CvFileNode), storage->memstorage));
    storage->root = storage->parent = 0;

    storage->hashsize = base64_buffer_size / sizeof(storage->hashtable[0]);
    assert( (storage->hashsize & (storage->hashsize - 1)) == 0 );
    CV_CALL( storage->hashtable = (CvFileNode**)cvMemStorageAlloc( storage->memstorage,
                                storage->hashsize*sizeof(storage->hashtable[0])));
    memset( storage->hashtable, 0, storage->hashsize*sizeof(storage->hashtable[0]));

    storage->max_width = 71;

    CV_CALL( storage->base64_buffer = storage->base64_buffer_start =
                 (char*)cvAlloc( base64_buffer_size+256 ));
    storage->base64_buffer_end = storage->base64_buffer_start + base64_buffer_size;

    if( ICV_IS_WRITE_MODE(storage) )
    {
        fprintf( storage->file, "<?xml version=\"1.0\"?>\n" );
        CV_CALL( icvPushXMLTag( storage, "opencv_storage", cvAttrList() ));
        ok = 1;
    }
    else
    {
        fseek( storage->file, 0, SEEK_END );
        int buffer_size = ftell( storage->file );
        fseek( storage->file, 0, SEEK_SET );

        CV_CALL( storage->buffer = storage->buffer_start =
                 (char*)cvAlloc( buffer_size+256 ));

        buffer_size = fread( storage->buffer_start, 1, buffer_size, storage->file );
        fclose( storage->file );
        storage->file = 0;
        storage->buffer_start[buffer_size] = '\0';
        storage->buffer_end = storage->buffer_start + buffer_size;

        CV_CALL( ok = icvParseXML( storage ));
    }

    __END__;

    if( (cvGetErrStatus() < 0 || !ok) && storage )
        cvReleaseFileStorage( &storage );

    return storage;
}


/* closes file storage and deallocates buffers */
CV_IMPL  void
cvReleaseFileStorage( CvFileStorage** pstorage )
{
    CV_FUNCNAME("cvReleaseFileStorage" );

    __BEGIN__;

    if( !pstorage )
        CV_ERROR( CV_StsNullPtr, "NULL double pointer to file storage" );

    if( *pstorage )
    {
        CvFileStorage* storage = *pstorage;
        CvTreeNodeIterator iterator;

        *pstorage = 0;

        if( storage->file )
        {
            if( ICV_IS_WRITE_MODE(storage) )
                icvPopXMLTag( storage );
            fclose( storage->file );
            storage->file = 0;
        }

        if( storage->root )
        {
            cvInitTreeNodeIterator( &iterator, storage->root, INT_MAX );
            while( iterator.node )
            {
                CvFileNode* node = (CvFileNode*)iterator.node;
                if( node->content && node->typeinfo && node->typeinfo->release )
                    node->typeinfo->release( (void**)&node->content );

                cvNextTreeNode( &iterator );
            }
        }

        cvFree( (void**)&storage->buffer_start );
        cvFree( (void**)&storage->base64_buffer_start );

        cvReleaseMemStorage( &storage->memstorage );

        memset( storage, 0, sizeof(*storage) );
        cvFree( (void**)&storage );
    }

    __END__;
}


/* writes matrix, image, sequence, graph etc. */
CV_IMPL  void
cvWrite( CvFileStorage* storage, const char* name,
         const void* struct_ptr, CvAttrList attr, int flags )
{
    CV_FUNCNAME( "cvWrite" );

    __BEGIN__;

    CvTypeInfo* info;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "NULL filestorage pointer" );

    if( !struct_ptr )
        CV_ERROR( CV_StsNullPtr, "NULL structure pointer" );

    CV_CALL( info = cvTypeOf( struct_ptr ));
    if( !info || !info->write )
        CV_ERROR( CV_StsUnsupportedFormat, "The structure type is unknown "
                  "or writing operation is not implemented" );

    CV_CALL( info->write( storage, name, struct_ptr, attr, flags ));

    __END__;
}

/* writes opening tag of a compound structure */
CV_IMPL  void
cvStartWriteStruct( CvFileStorage* storage, const char* name,
              const char* type_name, const void* struct_ptr,
              CvAttrList attr )
{
    CV_FUNCNAME( "cvStartWriteStruct" );

    __BEGIN__;

    char name_buf[16];
    char* new_attr[10];
    int idx = 0;

    if( name && name[0] )
    {
        new_attr[idx++] = "id";
        if( strcmp(name,"<auto>") == 0 )
        {
            sprintf( name_buf, "%p", struct_ptr );
            name = name_buf;
        }
        new_attr[idx++] = (char*)name;
    }

    if( type_name && type_name[0] )
    {
        new_attr[idx++] = "type";
        new_attr[idx++] = (char*)type_name;
    }

    new_attr[idx++] = 0;
    CV_CALL( icvPushXMLTag( storage, "struct", cvAttrList( new_attr, &attr ) ));

    __END__;
}


/* writes closing tag of a compound structure */
CV_IMPL  void
cvEndWriteStruct( CvFileStorage* storage )
{
    CV_FUNCNAME( "cvEndWriteStruct" );

    __BEGIN__;

    CV_CALL( icvPopXMLTag( storage ));

    __END__;
}

/* writes a scalar */
CV_IMPL  void
cvWriteElem( CvFileStorage* storage, const char* name,
             const char* elem_dt, const void* data_ptr )
{
    CV_FUNCNAME( "cvWriteElem" );

    __BEGIN__;

    char name_buf[16];
    char* new_attr[10];
    int idx = 0, dl;

    if( !data_ptr )
        CV_ERROR( CV_StsNullPtr, "NULL data pointer" );

    if( !elem_dt || !elem_dt[0] )
        CV_ERROR( CV_StsBadArg, "NULL or empty data type specification" );

    if( name && name[0] )
    {
        new_attr[idx++] = "id";
        if( strcmp(name,"<auto>") == 0 )
        {
            sprintf( name_buf, "%p", data_ptr );
            name = name_buf;
        }
        new_attr[idx++] = (char*)name;
    }

    new_attr[idx++] = "dt";
    new_attr[idx++] = (char*)elem_dt;
    /*new_attr[idx++] = "format";
    new_attr[idx++] = "text";*/
    new_attr[idx++] = 0;

    CV_CALL( icvPushXMLTag( storage, "elem", cvAttrList( new_attr, 0 ),
                            ICV_INCOMPLETE_TAG + ICV_EMPTY_TAG ));

    dl = 0;
    fprintf( storage->file, " value=\"%n", &dl );
    storage->width += dl;

    CV_CALL( icvWriteRawData( storage, elem_dt, ICV_FORMAT_TEXT, data_ptr, 1 ));
    fprintf( storage->file, "\"/>\n" );
    storage->width = 0;

    __END__;
}


/* returns index of storage item or negative number if element is not found */
CV_IMPL  CvFileNode*
cvGetFileNode( CvFileStorage* storage, const char* name )
{
    CvFileNode* node = 0;

    CV_FUNCNAME( "cvGetFileNode" );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "" );

    if( !name )
        node = storage->root;
    else
    {
        CV_CALL( node = icvQueryName( storage, name ));
    }

    __END__;

    return node;
}


/* reads matrix, image, sequence, graph etc. */
CV_IMPL  const void*
cvReadFileNode( CvFileStorage* storage, CvFileNode* node, CvAttrList** list )
{
    const void* ptr = 0;
    CvAttrList* attr_list = 0;

    CV_FUNCNAME( "cvReadFileNode" );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "NULL file storage pointer" );

    if( !node )
        EXIT;

    if( !node->content )
    {
        if( strcmp( node->tagname, "elem" ) == 0 )
        {
            int size, ok;
            const char* dt = cvAttrValue( node->attr, "dt" );
            CV_CALL( size = icvCalcSize(dt));

            if( size == 0 )
                EXIT;

            node->elem_size = size;

            CV_CALL( node->content = (void*)cvMemStorageAlloc(storage->memstorage, size));

            storage->buffer = (char*)cvAttrValue( node->attr, "value" );
            CV_CALL( ok = icvReadRawData( storage, dt, ICV_FORMAT_TEXT,
                                          (void*)node->content, 1 ));
            if( !ok )
                node->content = 0;
        }
        else if( strcmp( node->tagname, "struct" ) == 0 )
        {
            if( node->typeinfo && node->typeinfo->read )
            {
                CV_CALL( node->typeinfo->read( storage, node ));
            }
            else
            {
                CvFileNode* child = node->v_next;
                // decode recursively the child nodes
                for( ; child != 0; child = child->h_next )
                    CV_CALL( cvReadFileNode( storage, child ));
            }
        }
        else
            CV_ERROR( CV_StsBadArg, "The node is neither an element nor structure" );
    }

    ptr = node->content;
    attr_list = node->attr;

    __END__;

    if( list )
        *list = attr_list;

    return ptr;
}


/* reads a scalar */
CV_IMPL  int
cvReadElem( CvFileStorage* storage, const char* name, void* dst )
{
    int result = 0;

    CV_FUNCNAME( "cvReadElem" );

    __BEGIN__;

    CvFileNode* node;

    CV_CALL( node = cvGetFileNode( storage, name ));
    if( node )
    {
        const void* ptr = node->content;

        if( strcmp( node->tagname, "elem" ) != 0 )
            EXIT;

        if( !ptr )
            CV_CALL( ptr = cvReadFileNode( storage, node ));

        if( ptr )
        {
            if( dst )
                memcpy( dst, ptr, node->elem_size );
            result = 1;
        }
    }

    __END__;

    return result;
}

/* End of file. */
