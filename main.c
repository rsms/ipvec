#include "ipvec.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

// Lists and tables?
// typedef struct SList {
//   void* value;
//   struct SList* link;
// } SList;
//   L = ()           # <nil, nil>
//   L = L < A        # <"A", nil>
//   L = L < B        # <"B, <"A", nil>>
//   L = L < C        # <"C" <"B, <"A" nil>>>


// Basic operations:
//   [x] append    vector   value          -> vector
//   [x] set       vector   index   value  -> vector
//   [x] get       vector   index          -> value
//   [ ] remove    vector   index          -> vector
//   [ ] concat    vector   vector         -> vector
//   [ ] slice     vector   start   end    -> vector
//   [ ] len       vector                  -> number
//
// Compound operations (implemented w/ basic operations):
//   [ ] prepend   vector1   value         -> vector3
//       1. new vector value -> vector2
//       2. concat vector2 vector1 -> vector3

// concat:
// v = [] -> []
// v = v & ["A"] -> ["A"]
// v = v & ["B"] -> ["A" "B"]

// Figure 1:
// v = []
// v = v + "A"      # ["A"]
// v = v + "B"      # ["A" "B"]
// ...
// v = v + "E"      # ["A" "B" "C" "D" "E"]
// v = v put 7 "X"  # ["A" "B" "C" "D" "E" nil nil "X"]
// v = v + "Y"      # ["A" "B" "C" "D" "E" nil nil "X" "Y"]
// v = v rm 7       # ["A" "B" "C" "D" "E" nil nil "Y"]
// v = v rm 7       # ["A" "B" "C" "D" "E"]
// v:3              # "D"
// v:900            # nil
// v:1..4           # ["B" "C" "D"]
// v:1..            # ["B" "C" "D" "E"]
// v:..4            # ["A" "B" "C" "D"]
// v last           # "E"
// v len            # 5

//
// @ nth x default =
//   ? (((@ len) - 1) >= x) (@:x)
//   | default
//
// v nth 3 "Nothing" -> "D"
//
// nth v x =
//   ? (((v len) - 1) >= x) (v:x)
//   | default
//
// nth v 3 "Nothing" -> "D"
//

int main(int argc, const char** argv) {
  const IPVec* v = IPVecEmpty;

  //IPVec* v2 = IPVecAppend(v, (void*)"A");
  IPVec* v2 = IPVecPut(v, 0, (void*)"A");
  //printf("v2[0] = \"%s\"\n\n", (const char*)IPVecAt(v1, 0));
  IPVecDebugPrint(v2);
  printf("len: %zu\n", IPVecLen(v2));

  //IPVec* v2 = IPVecAppend(v1, (void*)"B");
  v2 = IPVecPut(v2, 1, (void*)"B");
  v2 = IPVecPut(v2, 1, (void*)"b");
  //printf("v2[1] = \"%s\"\n\n", (const char*)IPVecAt(v2, 1));
  IPVecDebugPrint(v2);
  printf("len: %zu\n", IPVecLen(v2));
  
  //v2 = IPVecAppend(v2, (void*)"C");
  v2 = IPVecPut(v2, 2, (void*)"C");
  //printf("v2[2] = \"%s\"\n\n", (const char*)IPVecAt(v2, 2));
  IPVecDebugPrint(v2);
  printf("len: %zu\n", IPVecLen(v2));

  //v2 = IPVecAppend(v2, (void*)"D");
  v2 = IPVecPut(v2, 3, (void*)"D");
  //printf("v2[3] = \"%s\"\n\n", (const char*)IPVecAt(v2, 3));
  IPVecDebugPrint(v2);
  printf("len: %zu\n", IPVecLen(v2));

  //v2 = IPVecAppend(v2, (void*)"E");
  v2 = IPVecPut(v2, 4, (void*)"E");
  IPVecDebugPrint(v2);
  printf("len: %zu\n", IPVecLen(v2));
  printf("v[4] = \"%s\"\n", (const char*)IPVecAt(v2, 4));

  v2 = IPVecPut(v2, 103, (void*)"X");
  IPVecDebugPrint(v2);
  printf("len: %zu\n", IPVecLen(v2));
  printf("v[4] = \"%s\"\n", (const char*)IPVecAt(v2, 4));
  printf("v[102] = \"%s\"\n", (const char*)IPVecAt(v2, 102));
  printf("v[103] = \"%s\"\n", (const char*)IPVecAt(v2, 103));

  return 0;
}
