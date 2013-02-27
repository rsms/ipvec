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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdint.h> // Note: Usually not available on Win32

#define BR_FACTOR 3  // Branching factor
#define DEPTH_MAX 10 // Max branch depth. TODO: calculate from BR_FACTOR

#define TODO assert(!"TODO")
//#define SDB(fmt, ...) printf("br %s " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define SDB(fmt, ...)

#if defined(__has_attribute) && __has_attribute(always_inline)
#define AINLINE __attribute__((always_inline))
#else
#define AINLINE inline static
#endif

#define NodeSlotEnableBr(n, bitno)  ((n)->flags |=  (((uint32_t)1) << bitno))
#define NodeSlotDisableBr(n, bitno) ((n)->flags &= ~(((uint32_t)1) << bitno))
#define NodeSlotIsBr(n, bitno)      ((n)->flags &   (((uint32_t)1) << bitno))

// Produces the path bottom-up, eg 120 is produced as 0,2,1. Modifies all three
// arguments.
#define NodePath(i, path, pathlen) do { \
  (path)[(pathlen)++] = (i) % BR_FACTOR; \
  (i) /= BR_FACTOR; \
} while ((i) != 0)

// Order matters b/c of constant initializers
typedef struct {
  void*    value;    // Node top value
  uint32_t flags;    // When bitN is 1, slotv[bitN] is a value (void*)
                     // When bitN is 0, slotv[bitN] is a branch (const Node*)
  uint8_t  _unused;
  uint8_t  slotc;    // Number of slots allocated
  void*    slotv[0]; // Branches to nodes or values
} Node;
// 24 bytes when 64-bit aligned

typedef struct {
  const Node* root;   // tree
  size_t      length; // next index (might be greater than actual items)
} IPVec;

// The empty vector
static const Node NodeEmpty = {0,0,0,0};
const IPVec _IPVecEmpty = {&NodeEmpty, 0};
const IPVec* IPVecEmpty = &_IPVecEmpty;

AINLINE uint8_t SMaxU8(uint8_t a, uint8_t b) {
  return b > a ? b : a;
}

AINLINE size_t NodeSize(uint8_t slotc) {
  return sizeof(Node) + (sizeof(void*) * slotc);
}

AINLINE Node* NodeAlloc(uint8_t slotc) {
  return (Node*)malloc(NodeSize(slotc));
}

AINLINE Node* NodeCreate(uint8_t slotc, void* value) {
  Node* n = NodeAlloc(slotc);
  n->slotc = slotc;
  n->flags = 0;
  n->value = value;
  return n;
}

// Returns a mutable copy of `n`
static AINLINE Node* NodeCopy1(const Node* n1) {
  uint8_t slotc = n1->slotc;
  Node* n2 = NodeAlloc(slotc);
  return (Node*)memcpy((void*)n2, (const void*)n1, NodeSize(slotc));
}

// Returns a mutable copy of `n` with same or larger slot count `slotc`
static AINLINE Node* NodeCopy2(const Node* n1, uint8_t slotc) {
  Node* n2 = NodeAlloc(slotc);
  assert(slotc >= n1->slotc);
  memcpy((void*)n2, (const void*)n1, NodeSize(n1->slotc));
  n2->slotc = slotc;
  return n2;
}

static AINLINE IPVec* IPVecAlloc() {
  return (IPVec*)malloc(sizeof(IPVec));
}

// Find the last (rightmost) node in the tree, starting at `n`
const Node* NodeTail(const Node* n) {
  while (n->slotc != 0) {
    size_t i = n->slotc-1;
    if (NodeSlotIsBr(n, i)) {
      // Note: This assumes that the rightmost slot is never NULL
      n = (const Node*)n->slotv[i];
    } else {
      break;
    }
  }
  return n;
}

// Find highest index in tree `n`
// static size_t _NodeMaxIndex(const Node* n) {
//   uint8_t si = n->slotc;
//   size_t subdepth;
//   size_t maxi = 0;

//   while (si-- != 0) {
//     if (NodeSlotIsBr(n, i)) {
//       maxi = _NodeMaxIndex(n);
//     } else if (maxi == 0 && n->slotv[si] != 0) {
//       maxi = si;
//     }
//   }
// }

// Length of vector `v` (or: next index)
size_t IPVecLen(const IPVec* v) {
  return v->length;
  //return _NodeMaxIndex(v->root);
}

// static void NodeDebugPrintPath(uint8_t* path, uint8_t pathlen) {
//   do {
//     printf("%u%s", path[--pathlen], pathlen == 0 ? "" : ", ");
//   } while (pathlen != 0);
//   printf("\n");
// }

void NodeDebugPrintTree(const Node* n, int level) {
  uint8_t i = 0;
  if (n->value != 0) {
    printf("('%s' @%p", n->value, n);
  } else {
    printf("(0 @%p", n);
  }

  // allocated slots
  for (; i != n->slotc; ++i) {
    printf("\n%*s[%u] = ", ((level+1)*2), "", i);
    if (NodeSlotIsBr(n, i)) {
      NodeDebugPrintTree(n->slotv[i], level+1);
    } else if (n->slotv[i] != 0) {
      printf("'%s'", (const char*)n->slotv[i]);
    } else {
      printf("0");
    }
  }

  // unallocated slots
  for (; i != BR_FACTOR; ++i) {
    printf("\n%*s[%u] x", ((level+1)*2), "", i);
  }

  printf("\n%*s)", (level*2), "");
  if (level == 0) printf("\n");
}

void IPVecDebugPrint(const IPVec* v) {
  NodeDebugPrintTree(v->root, 0);
}

AINLINE void NodeSlotZero(Node* n, uint8_t start, uint8_t end) {
  // [A B] -> [A B - - E -]
  //  0 1      0 1 2 3 4 5
  // start = 2
  // end   = 4
  //memset((void*)(n->slotv + start), 0, end-start);
  void** v = n->slotv;
  while (start < end) {
    v[start++] = 0;
  }
}

// Internal "put" operation used by "set" and "append" operations when operating
// on branches below the root slots.
void _NodePut(
    const Node* root,   // Tree root
    uint8_t slotc,      // Tree root slot count (op)
    Node** root2,       // Resulting tree root
    size_t index,       // Target index
    void* value         // Target value
  ) {
  // Precondition: All slots in root are allocated

  // Find path to index
  size_t i = index;
  uint8_t path[DEPTH_MAX];
  uint8_t pathlen = 0;
  NodePath(i, path, pathlen);
  //printf("IPVecAppend path: "); NodeDebugPrintPath(path, pathlen);

  // Traverse tree
  Node* parent = 0;
  const Node* branch = root; // current branch we are traversing
  void* evalue = 0; // value moved from slot to new branch
  uint8_t parentsi;

  do {
    uint8_t si = path[--pathlen];
    Node* newbranch;

    if (branch == 0) {
      SDB("1");
      // We are creating a new branch

      // Create a new node that can accomodate slot `si`
      newbranch = NodeCreate(si+1, 0);

      // Set as parent or insert into parent
      if (parent == 0) {
        SDB("11 1");
        // Entirely new path
        *root2 = newbranch;

      } else {
        SDB("11 2");
        // We are making a new path on to an existing parent

        if (evalue != 0) {
          SDB("11 21");
          // Existing value that needs to be set as the branch's value
          newbranch->value = evalue;
          evalue = 0;
        }
        
        parent->slotv[parentsi] = (void*)newbranch;
        NodeSlotEnableBr(parent, parentsi);
      }

      if (pathlen == 0) {
        SDB("12 1");
        // Last slot is the value
        newbranch->slotv[si] = value;
      } else {
        SDB("12 2");
        // Set node as parent
        parent = newbranch;
      }

      // Zero any extra slots created
      parentsi = si;
      while (si-- != 0) {
        newbranch->slotv[si] = 0;
      }
      continue; // avoid assigning now zero si to parentsi

    } else {
      SDB("2");
      // We are copying an existing branch

      if (NodeSlotIsBr(branch, si)) {
        SDB("21 1");
        // The slot already represents a branch

        // Make a copy of the current branch with identical slots
        newbranch = NodeCopy1(branch);

        // Traverse the branch
        branch = (const Node*)newbranch->slotv[si];

        assert(pathlen != 0); // FIXME TODO last item

      } else {
        SDB("21 2");
        // The slot represents a value

        // Make a copy of the current branch
        uint8_t slotc2 = branch->slotc;

        if (si >= slotc2) {
          SDB("21 21 1");
          // Need to allocate additional slots to fit `si`
          newbranch = NodeCopy2(branch, si+1);

          if (si > slotc2) {
            SDB("21 21 11");
            // Zero out intermediate filler slots
            while (slotc2 != si) {
              newbranch->slotv[++slotc2] = 0;
            }
          }

        } else {
          SDB("21 21 2");
          // Enough slots exists. 1:1 copy.
          newbranch = NodeCopy1(branch);
        }

        if (pathlen == 0) {
          SDB("21 22 1");
          // Last path component

          if (newbranch->slotv[si] != 0) {
            printf("NOTE: Replacing value at si=%u\n", si);
          }

          // Assign the value to the slot
          assert(newbranch->slotc > si);
          newbranch->slotv[si] = value;
          //printf("ASSIGN value (%p '%s')\n", value, (const char*)value);

        } else {
          SDB("21 22 2");
          // Nth path component. Create a new branch starting here.

          // Enable branch at slot `si`
          //NodeSlotEnableBr(newbranch, si);
          // ^__ ? handled by "new branch" codepath

          // Store pointer to value held at slot si in evalue, used by
          // "new branch" codepath to transfer the slot value into a branch
          // value.
          evalue = newbranch->slotv[si];

          // We are creating a new branch. Mark by setting n to null
          branch = 0;
        }
      }

      // Set parent to point to newbranch
      if (parent == 0) {
        SDB("22 1");
        *root2 = newbranch;
      } else {
        SDB("22 2");
        parent->slotv[parentsi] = (void*)newbranch;
        assert(NodeSlotIsBr(parent, parentsi));
      }
      parent = newbranch;

    }

    parentsi = si;
  } while (pathlen != 0);
}


IPVec* IPVecPut(const IPVec* v, size_t index, void* value) {
  // Create resulting vector
  IPVec* v2 = IPVecAlloc();

  // Increment length
  if (index >= v->length) {
    v2->length = index + 1;
  } else {
    v2->length = v->length;
  }

  // Get the current root node.
  const Node* root = v->root;
  uint8_t slotc = root->slotc;

  // This will eventually point to the new root
  Node* root2;

  if (index < BR_FACTOR) {
    SDB("1");
    // Special common case: index is at root.

    if (index < slotc) {
      SDB("11");
      // index points to an allocated slot. Make a copy of the branch.
      root2 = NodeCopy1(root);

      if (NodeSlotIsBr(root, index)) {
        SDB("111");
        // index points to a branch. Make a copy of the branch and set its value
        const Node* n2 = (const Node*)root2->slotv[index];
        Node* n3 = NodeCopy1(n2);
        n3->value = value;
        root2->slotv[index] = n3;

      } else {
        SDB("112");
        // index points to an existing value slot. Set the slot's value.
        root2->slotv[index] = value;
      }

    } else {
      SDB("12");
      // index points to an unallocated slot. Allocate a slot and set the value.
      root2 = NodeCopy2(root, index+1);
      root2->slotv[index] = value;

      // Zero out intermediate filler slots
      NodeSlotZero(root2, slotc, index);
    }

  } else {
    SDB("2");
    // Deep "put"
    _NodePut(root, slotc, &root2, index, value);
  }

  // Assign the new root to the new vector
  v2->root = root2;

  return v2;
}


IPVec* IPVecAppend(const IPVec* v, void* value) {
  // Create resulting vector
  IPVec* v2 = IPVecAlloc();
  v2->length = v->length + 1;

  // Get the current root node.
  const Node* root = v->root;
  uint8_t slotc = root->slotc;

  // This will eventually point to the new root
  Node* root2;

  if (slotc < BR_FACTOR) {
    SDB("1");
    // Special common case: root has unallocated slots.
    root2 = NodeCopy2(root, slotc+1);
    root2->slotv[slotc] = value;

  } else {
    SDB("2");
    // All slots in root are allocated. Deep "put"
    _NodePut(root, slotc, &root2, v->length, value);
  }

  // Assign the new root to the new vector
  v2->root = root2;

  return v2;
}

// Retrieve value at index
void* IPVecAt(const IPVec* v, size_t index) {
  const Node* branch = v->root;

  if (index < BR_FACTOR) {
    SDB("1");
    // Optimization: We know the value is in the root

    if (branch->slotc > index) {
      // check if value is a value or a branch
      if (NodeSlotIsBr(branch, index)) {
        // The slot is a branch. Return the value of the node at the branch
        return ((const Node*)branch->slotv[index])->value;
      } else {
        // The slot holds a value. Return that value.
        return branch->slotv[index];
      }
    } else {
      // Out of bounds
      printf("IPVecAt Error: Out of bounds\n");
      return 0;
    }

  } else {
    SDB("2");
    // The item is not in the root but lives deeper into the tree

    // Find path
    uint8_t path[DEPTH_MAX];
    uint8_t pathlen = 0;
    NodePath(index, path, pathlen);
    //printf("IPVecAt path: "); NodeDebugPrintPath(path, pathlen);

    // Traverse tree
    do {
      uint8_t si = path[--pathlen];

      if (NodeSlotIsBr(branch, si)) {
        SDB("21");
        // Slot x is a branch

        if (pathlen == 0) {
          SDB("211");
          // Ends on a branch node. Return the value of that node.
          return ((const Node*)branch->slotv[si])->value;
        } else {
          SDB("212");
          // Dig deeper
          branch = (const Node*)branch->slotv[si];
        }

      } else {
        SDB("22");
        // Slot si is not a branch (it's a value if it exists)

        if (branch->slotc > si) {
          SDB("221");
          // Slot exists

          if (pathlen == 0) {
            SDB("2211");
            // Ends on a value. Return that value.

            return branch->slotv[si];

          } else {
            SDB("2212");
            // Error: Premature end
            printf("IPVecAt Error: Premature end\n");
            return 0;
          }
        } else {
          SDB("222 (branch->slotc <= si | %u <= %u)", branch->slotc, si);
          printf("branch->slotv[0]: %p ('%s')\n", branch->slotv[0],
            (const char*)branch->slotv[0]);
          // Error: Out of bounds
          printf("IPVecAt Error: Out of bounds\n");
          return 0;
        }
      }

    } while (pathlen != 0);

    // Error: Out of bounds
    return 0;
  }
}
