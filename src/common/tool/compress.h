#pragma once
#ifdef _WIN32
#define ZLIB_WINAPI
#endif
#include <zlib.h>

int gzcompress(Bytef *data, uLong ndata, Bytef *zdata, uLong *nzdata);
int gzuncompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata);
