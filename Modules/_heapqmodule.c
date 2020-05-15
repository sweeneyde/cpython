/* Drop in replacement for heapq.py

C implementation derived directly from heapq.py in Py2.3
which was written by Kevin O'Connor, augmented by Tim Peters,
annotated by François Pinard, and converted to C by Raymond Hettinger.

*/

#include "Python.h"

#include "clinic/_heapqmodule.c.h"

/*[clinic input]
module _heapq
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=d7cca0a2e4c0ceb3]*/

static int
siftdown(PyListObject *heap, Py_ssize_t startpos, Py_ssize_t pos)
{
    PyObject *newitem, *parent, **arr;
    Py_ssize_t parentpos, size;
    int cmp;

    assert(PyList_Check(heap));
    size = PyList_GET_SIZE(heap);
    if (pos >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return -1;
    }

    /* Follow the path to the root, moving parents down until finding
       a place newitem fits. */
    arr = _PyList_ITEMS(heap);
    newitem = arr[pos];
    while (pos > startpos) {
        parentpos = (pos - 1) >> 1;
        parent = arr[parentpos];
        Py_INCREF(newitem);
        Py_INCREF(parent);
        cmp = PyObject_RichCompareBool(newitem, parent, Py_LT);
        Py_DECREF(parent);
        Py_DECREF(newitem);
        if (cmp < 0)
            return -1;
        if (size != PyList_GET_SIZE(heap)) {
            PyErr_SetString(PyExc_RuntimeError,
                            "list changed size during iteration");
            return -1;
        }
        if (cmp == 0)
            break;
        arr = _PyList_ITEMS(heap);
        parent = arr[parentpos];
        newitem = arr[pos];
        arr[parentpos] = newitem;
        arr[pos] = parent;
        pos = parentpos;
    }
    return 0;
}

static int
siftup(PyListObject *heap, Py_ssize_t pos)
{
    Py_ssize_t startpos, endpos, childpos, limit;
    PyObject *tmp1, *tmp2, **arr;
    int cmp;

    assert(PyList_Check(heap));
    endpos = PyList_GET_SIZE(heap);
    startpos = pos;
    if (pos >= endpos) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return -1;
    }

    /* Bubble up the smaller child until hitting a leaf. */
    arr = _PyList_ITEMS(heap);
    limit = endpos >> 1;         /* smallest pos that has no child */
    while (pos < limit) {
        /* Set childpos to index of smaller child.   */
        childpos = 2*pos + 1;    /* leftmost child position  */
        if (childpos + 1 < endpos) {
            PyObject* a = arr[childpos];
            PyObject* b = arr[childpos + 1];
            Py_INCREF(a);
            Py_INCREF(b);
            cmp = PyObject_RichCompareBool(a, b, Py_LT);
            Py_DECREF(a);
            Py_DECREF(b);
            if (cmp < 0)
                return -1;
            childpos += ((unsigned)cmp ^ 1);   /* increment when cmp==0 */
            arr = _PyList_ITEMS(heap);         /* arr may have changed */
            if (endpos != PyList_GET_SIZE(heap)) {
                PyErr_SetString(PyExc_RuntimeError,
                                "list changed size during iteration");
                return -1;
            }
        }
        /* Move the smaller child up. */
        tmp1 = arr[childpos];
        tmp2 = arr[pos];
        arr[childpos] = tmp2;
        arr[pos] = tmp1;
        pos = childpos;
    }
    /* Bubble it up to its final resting place (by sifting its parents down). */
    return siftdown(heap, startpos, pos);
}

/*[clinic input]
_heapq.heappush

    heap: object
    item: object
    /

Push item onto heap, maintaining the heap invariant.
[clinic start generated code]*/

static PyObject *
_heapq_heappush_impl(PyObject *module, PyObject *heap, PyObject *item)
/*[clinic end generated code: output=912c094f47663935 input=7913545cb5118842]*/
{
    if (!PyList_Check(heap)) {
        PyErr_SetString(PyExc_TypeError, "heap argument must be a list");
        return NULL;
    }

    if (PyList_Append(heap, item))
        return NULL;

    if (siftdown((PyListObject *)heap, 0, PyList_GET_SIZE(heap)-1))
        return NULL;
    Py_RETURN_NONE;
}

static PyObject *
heappop_internal(PyObject *heap, int siftup_func(PyListObject *, Py_ssize_t))
{
    PyObject *lastelt, *returnitem;
    Py_ssize_t n;

    if (!PyList_Check(heap)) {
        PyErr_SetString(PyExc_TypeError, "heap argument must be a list");
        return NULL;
    }

    /* raises IndexError if the heap is empty */
    n = PyList_GET_SIZE(heap);
    if (n == 0) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }

    lastelt = PyList_GET_ITEM(heap, n-1) ;
    Py_INCREF(lastelt);
    if (PyList_SetSlice(heap, n-1, n, NULL)) {
        Py_DECREF(lastelt);
        return NULL;
    }
    n--;

    if (!n)
        return lastelt;
    returnitem = PyList_GET_ITEM(heap, 0);
    PyList_SET_ITEM(heap, 0, lastelt);
    if (siftup_func((PyListObject *)heap, 0)) {
        Py_DECREF(returnitem);
        return NULL;
    }
    return returnitem;
}

/*[clinic input]
_heapq.heappop

    heap: object
    /

Pop the smallest item off the heap, maintaining the heap invariant.
[clinic start generated code]*/

static PyObject *
_heapq_heappop(PyObject *module, PyObject *heap)
/*[clinic end generated code: output=e1bbbc9866bce179 input=9bd36317b806033d]*/
{
    return heappop_internal(heap, siftup);
}

static PyObject *
heapreplace_internal(PyObject *heap, PyObject *item, int siftup_func(PyListObject *, Py_ssize_t))
{
    PyObject *returnitem;

    if (!PyList_Check(heap)) {
        PyErr_SetString(PyExc_TypeError, "heap argument must be a list");
        return NULL;
    }

    if (PyList_GET_SIZE(heap) == 0) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }

    returnitem = PyList_GET_ITEM(heap, 0);
    Py_INCREF(item);
    PyList_SET_ITEM(heap, 0, item);
    if (siftup_func((PyListObject *)heap, 0)) {
        Py_DECREF(returnitem);
        return NULL;
    }
    return returnitem;
}


/*[clinic input]
_heapq.heapreplace

    heap: object
    item: object
    /

Pop and return the current smallest value, and add the new item.

This is more efficient than heappop() followed by heappush(), and can be
more appropriate when using a fixed-size heap.  Note that the value
returned may be larger than item!  That constrains reasonable uses of
this routine unless written as part of a conditional replacement:

    if item > heap[0]:
        item = heapreplace(heap, item)
[clinic start generated code]*/

static PyObject *
_heapq_heapreplace_impl(PyObject *module, PyObject *heap, PyObject *item)
/*[clinic end generated code: output=82ea55be8fbe24b4 input=e57ae8f4ecfc88e3]*/
{
    return heapreplace_internal(heap, item, siftup);
}

/*[clinic input]
_heapq.heappushpop

    heap: object
    item: object
    /

Push item on the heap, then pop and return the smallest item from the heap.

The combined action runs more efficiently than heappush() followed by
a separate call to heappop().
[clinic start generated code]*/

static PyObject *
_heapq_heappushpop_impl(PyObject *module, PyObject *heap, PyObject *item)
/*[clinic end generated code: output=67231dc98ed5774f input=eb48c90ba77b2214]*/
{
    PyObject *returnitem;
    int cmp;

    if (!PyList_Check(heap)) {
        PyErr_SetString(PyExc_TypeError, "heap argument must be a list");
        return NULL;
    }

    if (PyList_GET_SIZE(heap) == 0) {
        Py_INCREF(item);
        return item;
    }

    PyObject* top = PyList_GET_ITEM(heap, 0);
    Py_INCREF(top);
    cmp = PyObject_RichCompareBool(top, item, Py_LT);
    Py_DECREF(top);
    if (cmp < 0)
        return NULL;
    if (cmp == 0) {
        Py_INCREF(item);
        return item;
    }

    if (PyList_GET_SIZE(heap) == 0) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }

    returnitem = PyList_GET_ITEM(heap, 0);
    Py_INCREF(item);
    PyList_SET_ITEM(heap, 0, item);
    if (siftup((PyListObject *)heap, 0)) {
        Py_DECREF(returnitem);
        return NULL;
    }
    return returnitem;
}

static Py_ssize_t
keep_top_bit(Py_ssize_t n)
{
    int i = 0;

    while (n > 1) {
        n >>= 1;
        i++;
    }
    return n << i;
}

/* Cache friendly version of heapify()
   -----------------------------------

   Build-up a heap in O(n) time by performing siftup() operations
   on nodes whose children are already heaps.

   The simplest way is to sift the nodes in reverse order from
   n//2-1 to 0 inclusive.  The downside is that children may be
   out of cache by the time their parent is reached.

   A better way is to not wait for the children to go out of cache.
   Once a sibling pair of child nodes have been sifted, immediately
   sift their parent node (while the children are still in cache).

   Both ways build child heaps before their parents, so both ways
   do the exact same number of comparisons and produce exactly
   the same heap.  The only difference is that the traversal
   order is optimized for cache efficiency.
*/

static PyObject *
cache_friendly_heapify(PyObject *heap, int siftup_func(PyListObject *, Py_ssize_t))
{
    Py_ssize_t i, j, m, mhalf, leftmost;

    m = PyList_GET_SIZE(heap) >> 1;         /* index of first childless node */
    leftmost = keep_top_bit(m + 1) - 1;     /* leftmost node in row of m */
    mhalf = m >> 1;                         /* parent of first childless node */

    for (i = leftmost - 1 ; i >= mhalf ; i--) {
        j = i;
        while (1) {
            if (siftup_func((PyListObject *)heap, j))
                return NULL;
            if (!(j & 1))
                break;
            j >>= 1;
        }
    }

    for (i = m - 1 ; i >= leftmost ; i--) {
        j = i;
        while (1) {
            if (siftup_func((PyListObject *)heap, j))
                return NULL;
            if (!(j & 1))
                break;
            j >>= 1;
        }
    }
    Py_RETURN_NONE;
}

static PyObject *
heapify_internal(PyObject *heap, int siftup_func(PyListObject *, Py_ssize_t))
{
    Py_ssize_t i, n;

    if (!PyList_Check(heap)) {
        PyErr_SetString(PyExc_TypeError, "heap argument must be a list");
        return NULL;
    }

    /* For heaps likely to be bigger than L1 cache, we use the cache
       friendly heapify function.  For smaller heaps that fit entirely
       in cache, we prefer the simpler algorithm with less branching.
    */
    n = PyList_GET_SIZE(heap);
    if (n > 2500)
        return cache_friendly_heapify(heap, siftup_func);

    /* Transform bottom-up.  The largest index there's any point to
       looking at is the largest with a child index in-range, so must
       have 2*i + 1 < n, or i < (n-1)/2.  If n is even = 2*j, this is
       (2*j-1)/2 = j-1/2 so j-1 is the largest, which is n//2 - 1.  If
       n is odd = 2*j+1, this is (2*j+1-1)/2 = j so j-1 is the largest,
       and that's again n//2-1.
    */
    for (i = (n >> 1) - 1 ; i >= 0 ; i--)
        if (siftup_func((PyListObject *)heap, i))
            return NULL;
    Py_RETURN_NONE;
}

/*[clinic input]
_heapq.heapify

    heap: object
    /

Transform list into a heap, in-place, in O(len(heap)) time.
[clinic start generated code]*/

static PyObject *
_heapq_heapify(PyObject *module, PyObject *heap)
/*[clinic end generated code: output=11483f23627c4616 input=872c87504b8de970]*/
{
    return heapify_internal(heap, siftup);
}

static int
siftdown_max(PyListObject *heap, Py_ssize_t startpos, Py_ssize_t pos)
{
    PyObject *newitem, *parent, **arr;
    Py_ssize_t parentpos, size;
    int cmp;

    assert(PyList_Check(heap));
    size = PyList_GET_SIZE(heap);
    if (pos >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return -1;
    }

    /* Follow the path to the root, moving parents down until finding
       a place newitem fits. */
    arr = _PyList_ITEMS(heap);
    newitem = arr[pos];
    while (pos > startpos) {
        parentpos = (pos - 1) >> 1;
        parent = arr[parentpos];
        Py_INCREF(parent);
        Py_INCREF(newitem);
        cmp = PyObject_RichCompareBool(parent, newitem, Py_LT);
        Py_DECREF(parent);
        Py_DECREF(newitem);
        if (cmp < 0)
            return -1;
        if (size != PyList_GET_SIZE(heap)) {
            PyErr_SetString(PyExc_RuntimeError,
                            "list changed size during iteration");
            return -1;
        }
        if (cmp == 0)
            break;
        arr = _PyList_ITEMS(heap);
        parent = arr[parentpos];
        newitem = arr[pos];
        arr[parentpos] = newitem;
        arr[pos] = parent;
        pos = parentpos;
    }
    return 0;
}

static int
siftup_max(PyListObject *heap, Py_ssize_t pos)
{
    Py_ssize_t startpos, endpos, childpos, limit;
    PyObject *tmp1, *tmp2, **arr;
    int cmp;

    assert(PyList_Check(heap));
    endpos = PyList_GET_SIZE(heap);
    startpos = pos;
    if (pos >= endpos) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return -1;
    }

    /* Bubble up the smaller child until hitting a leaf. */
    arr = _PyList_ITEMS(heap);
    limit = endpos >> 1;         /* smallest pos that has no child */
    while (pos < limit) {
        /* Set childpos to index of smaller child.   */
        childpos = 2*pos + 1;    /* leftmost child position  */
        if (childpos + 1 < endpos) {
            PyObject* a = arr[childpos + 1];
            PyObject* b = arr[childpos];
            Py_INCREF(a);
            Py_INCREF(b);
            cmp = PyObject_RichCompareBool(a, b, Py_LT);
            Py_DECREF(a);
            Py_DECREF(b);
            if (cmp < 0)
                return -1;
            childpos += ((unsigned)cmp ^ 1);   /* increment when cmp==0 */
            arr = _PyList_ITEMS(heap);         /* arr may have changed */
            if (endpos != PyList_GET_SIZE(heap)) {
                PyErr_SetString(PyExc_RuntimeError,
                                "list changed size during iteration");
                return -1;
            }
        }
        /* Move the smaller child up. */
        tmp1 = arr[childpos];
        tmp2 = arr[pos];
        arr[childpos] = tmp2;
        arr[pos] = tmp1;
        pos = childpos;
    }
    /* Bubble it up to its final resting place (by sifting its parents down). */
    return siftdown_max(heap, startpos, pos);
}


/*[clinic input]
_heapq._heappop_max

    heap: object
    /

Maxheap variant of heappop.
[clinic start generated code]*/

static PyObject *
_heapq__heappop_max(PyObject *module, PyObject *heap)
/*[clinic end generated code: output=acd30acf6384b13c input=62ede3ba9117f541]*/
{
    return heappop_internal(heap, siftup_max);
}

/*[clinic input]
_heapq._heapreplace_max

    heap: object
    item: object
    /

Maxheap variant of heapreplace.
[clinic start generated code]*/

static PyObject *
_heapq__heapreplace_max_impl(PyObject *module, PyObject *heap,
                             PyObject *item)
/*[clinic end generated code: output=8ad7545e4a5e8adb input=6d8f25131e0f0e5f]*/
{
    return heapreplace_internal(heap, item, siftup_max);
}

/*[clinic input]
_heapq._heapify_max

    heap: object
    /

Maxheap variant of heapify.
[clinic start generated code]*/

static PyObject *
_heapq__heapify_max(PyObject *module, PyObject *heap)
/*[clinic end generated code: output=1c6bb6b60d6a2133 input=cdfcc6835b14110d]*/
{
    return heapify_internal(heap, siftup_max);
}

/* merge object *************************************************************/

typedef struct node {
    PyObject *key;
    PyObject *value;
    Py_ssize_t source_index;
} merge_node;

typedef struct mergeobject{
    PyObject_HEAD
    Py_ssize_t next_index;
    PyObject **iterators;   /* fixed-size array */
    Py_ssize_t num_iters;
    PyObject *keyfunc;      /* called to compare items */
    merge_node *tree;       /* tree of compared items stored as an array */
    PyObject *first_sentinel;
    PyObject *last_sentinel;
    int reverse;
    int status; /* Enum: 0=not yet heapified; 1=in progress; 2=finished */
} mergeobject;

static PyTypeObject merge_type;

static void reprint(PyObject *obj) {
    PyObject_Print(obj, stdout, 0);
}

/* Is a before b? */
static int
node_less_than(mergeobject *mo, merge_node *node_a, merge_node *node_b)
{
    PyObject *a, *b;
    int cmp;

    if (mo->keyfunc) {
        a = node_a->key;
        b = node_b->key;
    } else {
        assert(node_a->key == NULL);
        assert(node_b->key == NULL);
        a = node_a->value;
        b = node_b->value;
    }

    assert(a != NULL);
    assert(b != NULL);

/*    printf("Comparing ");
    reprint(a);
    printf(" to ");
    reprint(b);
    printf("..."); */

    if (a == mo->first_sentinel || b == mo->last_sentinel) {
        // reprint(a);
        // printf("wins! \n");
        return 1;
    }
    if (a == mo->last_sentinel || b == mo->first_sentinel) {
        // reprint(b);
        // printf("wins! \n");
        return 0;
    }

    if (!mo->reverse) {
        if (node_a->source_index < node_b->source_index) {
            cmp = PyObject_RichCompareBool(b, a, Py_LT);
            if (cmp < 0) {
                return -1;
            }
            // reprint((!cmp) ? a : b); printf("wins! \n");
            return !cmp;
        }
        else {
            cmp = PyObject_RichCompareBool(a, b, Py_LT);
            if (cmp < 0) {
                return -1;
            }
            // reprint((cmp) ? a : b); printf("wins! \n");
            return cmp;
        }
    }
    else {
        if (node_a->source_index < node_b->source_index) {
            cmp = PyObject_RichCompareBool(a, b, Py_LT);
            if (cmp < 0) {
                return -1;
            }
            // reprint((!cmp) ? a : b); printf("wins! \n");
            return !cmp;
        }
        else {
            cmp = PyObject_RichCompareBool(b, a, Py_LT);
            if (cmp < 0) {
                return -1;
            }
            // reprint((cmp) ? a : b); printf("wins! \n");
            return cmp;
        }
    }
}

static int
init_node(mergeobject *mo, Py_ssize_t iter_index, merge_node *holding)
{
    PyObject *it = mo->iterators[iter_index];

    holding->source_index = iter_index;
    holding->key = NULL;
    holding->value = NULL;

    if (it == NULL) {
        if (mo->keyfunc) {
            holding->value = mo->last_sentinel;
            Py_INCREF(mo->last_sentinel);
            holding->key = mo->last_sentinel;
            Py_INCREF(mo->last_sentinel);
        }
        else {
            holding->value = mo->last_sentinel;
            Py_INCREF(mo->last_sentinel);
            holding->key = NULL;
        }
        return 0;
    }
    
    assert(PyIter_Check(it));
    PyObject *new = PyIter_Next(it);

    if (new == NULL) {
        mo->iterators[iter_index] = NULL;
        Py_DECREF(it);
        if (PyErr_Occurred()) {
            goto error;
        }
        if (mo->keyfunc) {
            holding->value = mo->last_sentinel;
            Py_INCREF(mo->last_sentinel);
            holding->key = mo->last_sentinel;
            Py_INCREF(mo->last_sentinel);
        }
        else {
            holding->value = mo->last_sentinel;
            Py_INCREF(mo->last_sentinel);
            holding->key = NULL;
        }
        return 0;
    }

    holding->value = new;

    if (mo->keyfunc) {
        holding->key = _PyObject_CallOneArg(mo->keyfunc, new);
        if (holding->key == NULL) {
            goto error;
        }
    }
    else {
        holding->key = NULL;
    }

    return 0;

error:
    Py_CLEAR(holding->key);
    Py_CLEAR(holding->value);
    return -1;
}


static int
replay_games(mergeobject *mo, Py_ssize_t iter_index, merge_node *holding) {
    Py_ssize_t n = mo->num_iters;
    merge_node *traverse;
    merge_node temp;
    merge_node *tree = mo->tree;
    Py_ssize_t tree_index;
    int res, cmp;

    res = init_node(mo, iter_index, holding);
    if (res < 0) {
        return -1;
    }

    for (tree_index = (iter_index + n) / 2 - 1;
         tree_index >= 0;
         tree_index = (tree_index + 1) / 2 - 1
    ) {
        traverse = &tree[tree_index];
        // printf("holding: ");
        // reprint(holding->value);
        // printf(", traverse: ");
        // reprint(traverse->value);
        // printf("...\n");
        cmp = node_less_than(mo, holding, traverse);
        if (cmp < 0) {
            Py_CLEAR(holding->key);
            Py_CLEAR(holding->value);
            return -1;
        }
        /* if holding is not better than traverse... */
        if (!cmp) {
            temp = tree[tree_index];
            tree[tree_index] = *holding;
            *holding = temp;
        }
        // printf("now holding: ");
        // reprint(holding->value);
        // printf("\n");
    }
    return 0;
}

static PyObject *
merge_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *keyfunc = NULL;
    int reverse = 0;
    mergeobject *mo;
    PyObject **iterators = NULL;
    merge_node *tree = NULL;
    Py_ssize_t num_iters, i;
    PyObject *first_sentinel = NULL, *last_sentinel = NULL;

    if (kwds != NULL) {
        char *kwlist[] = {"key", "reverse", 0};
        PyObject *tmpargs = PyTuple_New(0);
        if (tmpargs == NULL) {
            return NULL;
        }
        if (!PyArg_ParseTupleAndKeywords(tmpargs, kwds, "|Op:merge",
                                         kwlist, &keyfunc, &reverse)) {
            Py_DECREF(tmpargs);
            return NULL;
        }
        Py_DECREF(tmpargs);
    }

    if (keyfunc == Py_None) {
        keyfunc = NULL;
    }
    Py_XINCREF(keyfunc);

    assert(PyTuple_CheckExact(args));
    num_iters = PyTuple_GET_SIZE(args);

    if (num_iters == 0) {
        mo = (mergeobject *)type->tp_alloc(type, 0);
        if (mo == NULL) {
            return NULL;
        }
        mo->iterators = NULL;
        mo->num_iters = 0;
        mo->tree = NULL;
        mo->keyfunc = NULL;
        mo->reverse = 0;
        mo->status = 2;
        return (PyObject *) mo;
    }

    iterators = PyMem_Calloc(num_iters, sizeof(PyObject *));
    if (iterators == NULL) {
        goto error;
    }

    /* Store the tree as a list with n-1 internal nodes and n leaves.
       If a key is being used, store keys next to original items. */;
    tree = PyMem_Calloc(num_iters - 1, sizeof(merge_node));
    if (tree == NULL) {
        goto error;
    }

    first_sentinel = PyObject_New(PyObject, &PyBaseObject_Type);
    if (first_sentinel == NULL) {
        goto error;
    }

    last_sentinel = PyObject_New(PyObject, &PyBaseObject_Type);
    if (last_sentinel == NULL) {
        goto error;
    }

    /* Now make the shifted list of iterators. */
    for (i = 0; i < num_iters; i++) {
        PyObject *iterable = PyTuple_GET_ITEM(args, i);
        PyObject *iterator = PyObject_GetIter(iterable);
        if (iterator == NULL) {
            goto error;
        }
        iterators[i] = iterator;
    }

    for (i = 0; i < num_iters - 1; i++) {
        tree[i].source_index = -1;
        tree[i].value = first_sentinel;
        Py_INCREF(first_sentinel);
        if (keyfunc) {
            tree[i].key = first_sentinel;
            Py_INCREF(first_sentinel);
        }
        else {
            tree[i].key = NULL;
        }
    }

    mo = (mergeobject *)type->tp_alloc(type, 0);
    if (mo == NULL) {
        goto error;
    }
    mo->next_index = -1;
    mo->iterators = iterators;
    mo->num_iters = num_iters;
    mo->tree = tree;
    mo->keyfunc = keyfunc;
    mo->reverse = reverse;
    mo->status = 0;
    mo->first_sentinel = first_sentinel;
    mo->last_sentinel = last_sentinel;

    return (PyObject *) mo;

error:
    if (iterators) {
        for (i = 0; i < num_iters; i++) {
            Py_CLEAR(iterators[i]);
        }
        PyMem_Free(iterators);
    }
    if (tree) {
        for (i = 0; i < num_iters - 1; i ++) {
            Py_CLEAR(tree[i].key);
            Py_CLEAR(tree[i].value);
        }
        PyMem_Free(tree);
    }
    Py_XDECREF(keyfunc);
    Py_XDECREF(first_sentinel);
    Py_XDECREF(last_sentinel);
    return NULL;
}

static int
merge_clear(mergeobject *mo)
{
    Py_ssize_t n = mo->num_iters;
    Py_ssize_t i;

    if (mo->tree) {
        for (i = 0; i < n - 1; i++) {
            Py_CLEAR(mo->tree[i].key);
            Py_CLEAR(mo->tree[i].value);
        }
    }
    PyMem_Free(mo->tree);
    mo->tree = NULL;

    if (mo->iterators) {
        for (i = 0; i < n; i++) {
            Py_CLEAR(mo->iterators[i]);
        }
    }
    PyMem_Free(mo->iterators);
    mo->tree = NULL;

    Py_CLEAR(mo->keyfunc);
    Py_CLEAR(mo->first_sentinel);
    Py_CLEAR(mo->last_sentinel);
    return 0;
}

static void
merge_dealloc(mergeobject *mo)
{
    PyObject_GC_UnTrack(mo);
    merge_clear(mo);
    Py_TYPE(mo)->tp_free(mo);
}

static PyObject *
merge_sizeof(mergeobject *mo, void *unused)
{
    Py_ssize_t n = mo->num_iters;
    Py_ssize_t treesize = n - 1;
    Py_ssize_t res = _PyObject_SIZE(Py_TYPE(mo));

    if (mo->iterators) {
        res += n * sizeof(PyObject*);
    }
    if (mo->tree) {
        res += treesize * sizeof(merge_node);
    }
    return PyLong_FromSsize_t(res);
}

static int
merge_traverse(mergeobject *mo, visitproc visit, void *arg)
{
    Py_ssize_t n = mo->num_iters;
    Py_ssize_t i;

    if (mo->tree) {
        for (i = 0; i < n - 1; i++) {
            Py_VISIT(mo->tree[i].key);
            Py_VISIT(mo->tree[i].value);
        }
    }

    if (mo->iterators) {
        for (i = 0; i < n; i++) {
            Py_VISIT(mo->iterators[i]);
        }
    }

    Py_VISIT(mo->keyfunc);
    Py_VISIT(mo->first_sentinel);
    Py_VISIT(mo->last_sentinel);
    return 0;
}

static PyObject *
merge_next(mergeobject *mo) {
    PyObject *result;
    merge_node *tree = mo->tree;
    merge_node holding;
    Py_ssize_t i, n = mo->num_iters;

    switch (mo->status) {
    case 0:
        for (i = 1; i < n; i++) {
            if (replay_games(mo, i, &holding) < 0) {
                Py_CLEAR(holding.key);
                Py_CLEAR(holding.value);
                goto stop;
            }
            Py_CLEAR(holding.key);
            Py_CLEAR(holding.value);
        }
        mo->next_index = 0;
        mo->status = 1;
    /* Fallthrough */
    case 1:
        // printf("tree[0]: ");
        // reprint(tree[0].value);
        // printf("\n");
        if (replay_games(mo, mo->next_index, &holding) < 0) {
            goto stop;
        }
        mo->next_index = holding.source_index;
        Py_CLEAR(holding.key);
        result = holding.value;
        if (result == mo->last_sentinel) {
            Py_DECREF(result);
            goto stop;
        }
        return holding.value;
    case 2:
        return NULL;
    default:
        Py_UNREACHABLE();
    }
stop:
    mo->status = 2;
    return NULL;
}


PyDoc_STRVAR(sizeof_doc, "Returns size in memory, in bytes.");

static PyMethodDef merge_methods[] = {
    {"__sizeof__", (PyCFunction)merge_sizeof, METH_NOARGS, sizeof_doc},
    {NULL, NULL}
};

PyDoc_STRVAR(merge_doc,
"merge(*iterables, key=None, reverse=False) --> merge object\n\
\n\
Merge multiple sorted inputs into a single sorted output.\n\
\n\
Similar to sorted(itertools.chain(*iterables)) but returns a generator,\n\
does not pull the data into memory all at once, and assumes that each of\n\
the input streams is already sorted (smallest to largest).\n\
\n\
>>> list(merge([1,3,5,7], [0,2,4,8], [5,10,15,20], [], [25]))\n\
[0, 1, 2, 3, 4, 5, 5, 7, 8, 10, 15, 20, 25]\n\
\n\
If *key* is not None, applies a key function to each element to determine\n\
its sort order.\n\
\n\
>>> list(merge(['dog', 'horse'], ['cat', 'fish', 'kangaroo'], key=len))\n\
['dog', 'cat', 'fish', 'horse', 'kangaroo']");

static PyTypeObject merge_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "heapq.merge",
    .tp_basicsize = sizeof(mergeobject),
    .tp_dealloc = (destructor)merge_dealloc,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_doc = merge_doc,
    .tp_traverse = (traverseproc)merge_traverse,
    .tp_clear = (inquiry)merge_clear,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc)merge_next,
    .tp_methods = merge_methods,
    .tp_new = merge_new,
    .tp_free = PyObject_GC_Del,
};

static PyMethodDef heapq_methods[] = {
    _HEAPQ_HEAPPUSH_METHODDEF
    _HEAPQ_HEAPPUSHPOP_METHODDEF
    _HEAPQ_HEAPPOP_METHODDEF
    _HEAPQ_HEAPREPLACE_METHODDEF
    _HEAPQ_HEAPIFY_METHODDEF
    _HEAPQ__HEAPPOP_MAX_METHODDEF
    _HEAPQ__HEAPIFY_MAX_METHODDEF
    _HEAPQ__HEAPREPLACE_MAX_METHODDEF
    {NULL, NULL}           /* sentinel */
};

PyDoc_STRVAR(module_doc,
"Heap queue algorithm (a.k.a. priority queue).\n\
\n\
Heaps are arrays for which a[k] <= a[2*k+1] and a[k] <= a[2*k+2] for\n\
all k, counting elements from 0.  For the sake of comparison,\n\
non-existing elements are considered to be infinite.  The interesting\n\
property of a heap is that a[0] is always its smallest element.\n\
\n\
Usage:\n\
\n\
heap = []            # creates an empty heap\n\
heappush(heap, item) # pushes a new item on the heap\n\
item = heappop(heap) # pops the smallest item from the heap\n\
item = heap[0]       # smallest item on the heap without popping it\n\
heapify(x)           # transforms list into a heap, in-place, in linear time\n\
item = heapreplace(heap, item) # pops and returns smallest item, and adds\n\
                               # new item; the heap size is unchanged\n\
\n\
Our API differs from textbook heap algorithms as follows:\n\
\n\
- We use 0-based indexing.  This makes the relationship between the\n\
  index for a node and the indexes for its children slightly less\n\
  obvious, but is more suitable since Python uses 0-based indexing.\n\
\n\
- Our heappop() method returns the smallest item, not the largest.\n\
\n\
These two make it possible to view the heap as a regular Python list\n\
without surprises: heap[0] is the smallest item, and heap.sort()\n\
maintains the heap invariant!\n");


PyDoc_STRVAR(__about__,
"Heap queues\n\
\n\
[explanation by Fran\xc3\xa7ois Pinard]\n\
\n\
Heaps are arrays for which a[k] <= a[2*k+1] and a[k] <= a[2*k+2] for\n\
all k, counting elements from 0.  For the sake of comparison,\n\
non-existing elements are considered to be infinite.  The interesting\n\
property of a heap is that a[0] is always its smallest element.\n"
"\n\
The strange invariant above is meant to be an efficient memory\n\
representation for a tournament.  The numbers below are `k', not a[k]:\n\
\n\
                                   0\n\
\n\
                  1                                 2\n\
\n\
          3               4                5               6\n\
\n\
      7       8       9       10      11      12      13      14\n\
\n\
    15 16   17 18   19 20   21 22   23 24   25 26   27 28   29 30\n\
\n\
\n\
In the tree above, each cell `k' is topping `2*k+1' and `2*k+2'.  In\n\
a usual binary tournament we see in sports, each cell is the winner\n\
over the two cells it tops, and we can trace the winner down the tree\n\
to see all opponents s/he had.  However, in many computer applications\n\
of such tournaments, we do not need to trace the history of a winner.\n\
To be more memory efficient, when a winner is promoted, we try to\n\
replace it by something else at a lower level, and the rule becomes\n\
that a cell and the two cells it tops contain three different items,\n\
but the top cell \"wins\" over the two topped cells.\n"
"\n\
If this heap invariant is protected at all time, index 0 is clearly\n\
the overall winner.  The simplest algorithmic way to remove it and\n\
find the \"next\" winner is to move some loser (let's say cell 30 in the\n\
diagram above) into the 0 position, and then percolate this new 0 down\n\
the tree, exchanging values, until the invariant is re-established.\n\
This is clearly logarithmic on the total number of items in the tree.\n\
By iterating over all items, you get an O(n ln n) sort.\n"
"\n\
A nice feature of this sort is that you can efficiently insert new\n\
items while the sort is going on, provided that the inserted items are\n\
not \"better\" than the last 0'th element you extracted.  This is\n\
especially useful in simulation contexts, where the tree holds all\n\
incoming events, and the \"win\" condition means the smallest scheduled\n\
time.  When an event schedule other events for execution, they are\n\
scheduled into the future, so they can easily go into the heap.  So, a\n\
heap is a good structure for implementing schedulers (this is what I\n\
used for my MIDI sequencer :-).\n"
"\n\
Various structures for implementing schedulers have been extensively\n\
studied, and heaps are good for this, as they are reasonably speedy,\n\
the speed is almost constant, and the worst case is not much different\n\
than the average case.  However, there are other representations which\n\
are more efficient overall, yet the worst cases might be terrible.\n"
"\n\
Heaps are also very useful in big disk sorts.  You most probably all\n\
know that a big sort implies producing \"runs\" (which are pre-sorted\n\
sequences, which size is usually related to the amount of CPU memory),\n\
followed by a merging passes for these runs, which merging is often\n\
very cleverly organised[1].  It is very important that the initial\n\
sort produces the longest runs possible.  Tournaments are a good way\n\
to that.  If, using all the memory available to hold a tournament, you\n\
replace and percolate items that happen to fit the current run, you'll\n\
produce runs which are twice the size of the memory for random input,\n\
and much better for input fuzzily ordered.\n"
"\n\
Moreover, if you output the 0'th item on disk and get an input which\n\
may not fit in the current tournament (because the value \"wins\" over\n\
the last output value), it cannot fit in the heap, so the size of the\n\
heap decreases.  The freed memory could be cleverly reused immediately\n\
for progressively building a second heap, which grows at exactly the\n\
same rate the first heap is melting.  When the first heap completely\n\
vanishes, you switch heaps and start a new run.  Clever and quite\n\
effective!\n\
\n\
In a word, heaps are useful memory structures to know.  I use them in\n\
a few applications, and I think it is good to keep a `heap' module\n\
around. :-)\n"
"\n\
--------------------\n\
[1] The disk balancing algorithms which are current, nowadays, are\n\
more annoying than clever, and this is a consequence of the seeking\n\
capabilities of the disks.  On devices which cannot seek, like big\n\
tape drives, the story was quite different, and one had to be very\n\
clever to ensure (far in advance) that each tape movement will be the\n\
most effective possible (that is, will best participate at\n\
\"progressing\" the merge).  Some tapes were even able to read\n\
backwards, and this was also used to avoid the rewinding time.\n\
Believe me, real good tape sorts were quite spectacular to watch!\n\
From all times, sorting has always been a Great Art! :-)\n");


static int
heapq_exec(PyObject *m)
{
    PyObject *about = PyUnicode_FromString(__about__);
    if (PyModule_AddObject(m, "__about__", about) < 0) {
        Py_DECREF(about);
        return -1;
    }
    if (PyModule_AddType(m, &merge_type) < 0) {
        return -1;
    }
    return 0;
}

static struct PyModuleDef_Slot heapq_slots[] = {
    {Py_mod_exec, heapq_exec},
    {0, NULL}
};

static struct PyModuleDef _heapqmodule = {
    PyModuleDef_HEAD_INIT,
    "_heapq",
    module_doc,
    0,
    heapq_methods,
    heapq_slots,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit__heapq(void)
{
    return PyModuleDef_Init(&_heapqmodule);
}
