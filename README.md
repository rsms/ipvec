# ipvec

Educational immutable, persistent vector implementation in C, inspired by the [Clojure vector](http://clojure.org/data_structures#Data%20Structures-Vectors%20%28IPersistentVector%29) implementation.

Have a look at [`main.c`](https://github.com/rsms/ipvec/blob/master/main.c) to get started. Run `make && ./ipvec` to build run the demo program.

Note that this is educational and there's really no memory management. I leave this detail as an exercise for the reader (hint: count refernces or implement GC and `free()` Node structs when dead.)

## API

```c
typedef struct IPVec IPVec;
// An opaque structure representing a vector.

extern const IPVec* IPVecEmpty;
// The empty vector. This is usually what you will start with when building
// a new vector.

IPVec* IPVecPut(const IPVec* v, size_t index, void* value);
// Derive a vector from v which has value at index

IPVec* IPVecAppend(const IPVec* v, void* value);
// Derive a vector from v which has value at the end of v

void*  IPVecAt(const IPVec* v, size_t index);
// Return the value at index, or NULL if not set

size_t IPVecLen(const IPVec* v);
// Return the virtual length of v. Note that IPVec is sparese, and thus a
// vector with only two items can have a length greater than 2.
```

# MIT License

Copyright (c) 2013 Rasmus Andersson <http://rsms.me/>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
