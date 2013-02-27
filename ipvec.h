#ifndef _IPVEC_H_
#define _IPVEC_H_

#include <stddef.h>

typedef struct IPVec IPVec;
extern const IPVec* IPVecEmpty;

IPVec* IPVecPut(const IPVec* v, size_t index, void* value);
IPVec* IPVecAppend(const IPVec* v, void* value);
void*  IPVecAt(const IPVec* v, size_t i);
size_t IPVecLen(const IPVec* v);

void  IPVecDebugPrint(const IPVec* v);

#endif // _IPVEC_H_
// Copyright (c) 2013 Rasmus Andersson <http://rsms.me/>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
