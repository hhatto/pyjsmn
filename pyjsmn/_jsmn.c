#include <Python.h>
#include <string.h>

#include "jsmn.h"

#define DEFAULT_TOKEN_SIZE 1024

#ifdef DEBUG
#define _debug(format,...) printf("[DEBUG]" format,__VA_ARGS__)
#define _info(format,...) printf("[INFO]" format,__VA_ARGS__)
#endif

typedef struct {
    PyObject *stack;
    unsigned int size;
    unsigned int used;
} _pyjsmn_stack;

typedef struct {
    _pyjsmn_stack elements[64];
    _pyjsmn_stack keys[64];
    PyObject *root;
    jsmntype_t root_type;
    int offset;
} _pyjsmn_ctx;

static _pyjsmn_ctx _ctx;
static jsmn_parser parser;

/* The module doc strings */
PyDoc_STRVAR(pyjsmn__doc__, "Python binding for jsmn");
PyDoc_STRVAR(pyjsmn_loads__doc__, "Decoding JSON");

static inline void
_set_object(_pyjsmn_ctx *ctx, PyObject *parent, PyObject *child)
{
    if (child && !parent) {
        if (ctx->offset == -1) {
            ctx->offset = 0;
        }
        ctx->root = child;
        ctx->elements[ctx->offset].stack = child;
        return;
    }
    else if (!parent || !child)
        return;

    if (PyList_Check(parent)) {
        PyList_Append(parent, child);
        if (child && child != Py_None) {
            Py_XDECREF(child);
        }
    }
    else if (PyDict_Check(parent)) {
        PyObject *key = ctx->keys[ctx->offset].stack;
        PyDict_SetItem(parent, key, child);
        Py_DECREF(key);
        if (child && child != Py_None) {
            Py_XDECREF(child);
        }
        ctx->keys[ctx->offset].used--;
    }

    return;
}

static inline PyObject*
_get_pyobject(_pyjsmn_ctx *ctx, jsmntok_t *token, char *jsontext)
{
    int offset;
    int is_float;
    PyObject *object;
    void *tmp_string;

    switch (token->type) {

        case JSMN_OBJECT:
            ctx->offset++;
            if (ctx->offset >= 1)
                ctx->elements[ctx->offset - 1].used++;
            object = PyDict_New();
            ctx->elements[ctx->offset].size += token->size;
            ctx->root_type = JSMN_OBJECT;
            ctx->root = object;
            break;

        case JSMN_ARRAY:
            ctx->offset++;
            if (ctx->offset >= 1)
                ctx->elements[ctx->offset - 1].used++;
            object = PyList_New(0);
            ctx->elements[ctx->offset].size += token->size;
            ctx->root_type = JSMN_ARRAY;
            ctx->root = object;
            break;

        case JSMN_STRING:
            //object = PyUnicode_FromStringAndSize(jsontext + token->start,
            //                                     token->end - token->start);
            object = PyString_FromStringAndSize(jsontext + token->start,
                                                token->end - token->start);
            // TODO: error handling
            ctx->elements[ctx->offset].used++;
            break;

        case JSMN_PRIMITIVE:
            switch (jsontext[token->start]) {
                case 't':
                    object = PyBool_FromLong(1);
                    break;
                case 'f':
                    object = PyBool_FromLong(0);
                    break;
                case 'n':
                    Py_INCREF(Py_None);
                    object = Py_None;
                    break;
                default:  // number
                    is_float = 0;
                    for (offset = 0; (token->end-token->start) > offset; offset++) {
                        switch (jsontext[token->start+offset]) {
                            case '.':
                            case 'e':
                            case 'E':
                                is_float = 1;
                                break;
                        }
                        if (is_float) break;
                    }
                    tmp_string = PyString_FromStringAndSize(
                            jsontext+token->start, token->end - token->start);
                    if (is_float) object = PyFloat_FromString(tmp_string, NULL);
                    else          object = PyInt_FromString(PyString_AS_STRING(tmp_string), NULL, 10);
                    Py_DECREF(tmp_string);
                    break;
            }
            ctx->elements[ctx->offset].used++;
            break;

        default:
            // TODO: error handling
            return NULL;
    }

    return object;
}

static PyObject *
_build_value(_pyjsmn_ctx *ctx, jsmntok_t *token, char *jsontext)
{
    int i;
    PyObject *object;

    for (i = 0;
         token[i].start != 0 || token[i].size != 0 || token[i].end != 0; i++) {
#ifdef DEBUG
        char tmp[10240] = {0};
        _debug("=== size:%d, st:%d, ed:%d ===\n%s\n",
               token[i].size, token[i].start, token[i].end,
               strncat(tmp, jsontext+token[i].start, token[i].end-token[i].start));
#endif

        object = NULL;
        object = _get_pyobject(ctx, token + i, jsontext);

        if (!object) continue;

#ifdef DEBUG
        if ((token+i)->type == JSMN_OBJECT)
            _debug("type             :%d HASH\n", (token+i)->type);
        else if ((token+i)->type == JSMN_ARRAY)
            _debug("type             :%d LIST\n", (token+i)->type);
        else if ((token+i)->type == JSMN_STRING)
            _debug("type             :%d STRING\n", (token+i)->type);
        else
            _debug("type             :%d PRIMITIVE\n", (token+i)->type);
        if (ctx->root_type == JSMN_OBJECT)
            _debug("root_type        :%d HASH\n", ctx->root_type);
        else if (ctx->root_type == JSMN_ARRAY)
            _debug("root_type        :%d LIST\n", ctx->root_type);
        else if (ctx->root_type == JSMN_STRING)
            _debug("root_type        :%d STRING\n", ctx->root_type);
        else
            _debug("root_type        :%d PRIMITIVE\n", ctx->root_type);
        _debug("elements[%2d] %d/%d\n", ctx->offset,
                                        ctx->elements[ctx->offset].used,
                                        ctx->elements[ctx->offset].size);
        _debug("keys    [%2d] %d/%d\n", ctx->offset,
                                        ctx->keys[ctx->offset].used,
                                        ctx->keys[ctx->offset].size);
        _debug("ctx->root:%p, object:%p\n", ctx->root, object);
#endif

        //if (ctx->root && PyDict_Check(ctx->root) && !PyDict_Check(object) &&
        if (ctx->root && (ctx->root_type == JSMN_OBJECT) && !PyDict_Check(object) &&
                (ctx->keys[ctx->offset].used % 2) == 0) {
            ctx->keys[ctx->offset].stack = object;
            ctx->keys[ctx->offset].used++;
            continue;
        }

        _set_object(ctx, ctx->elements[ctx->offset].stack, object);
        ctx->root = ctx->elements[ctx->offset].stack;

        for (; ctx->offset!=0; ) {
            if (ctx->elements[ctx->offset].used == ctx->elements[ctx->offset].size &&
                    ctx->offset != 0) {

                ctx->offset--;
                _set_object(ctx, ctx->elements[ctx->offset].stack,
                            ctx->elements[ctx->offset+1].stack);
                ctx->root = ctx->elements[ctx->offset].stack;

                if (PyDict_Check(ctx->root)) {
                    ctx->root_type = JSMN_OBJECT;
                }
                else {
                    ctx->root_type = JSMN_ARRAY;
                }

                memset(&ctx->elements[ctx->offset+1], 0, sizeof(_pyjsmn_stack));
                memset(&ctx->keys[ctx->offset+1], 0, sizeof(_pyjsmn_stack));
            }
            else {
                break;
            }
        }
    }

    return ctx->root;
}

static PyObject *
pyjsmn_loads(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"text"};
    char *text;
    int ret;
    jsmntok_t token[DEFAULT_TOKEN_SIZE];

    /* Parse arguments */
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", kwlist, &text))
        return NULL;

    memset(&_ctx, 0, sizeof(_pyjsmn_ctx));
    memset(token, 0, sizeof(token));
    _ctx.offset = -1;

    jsmn_init(&parser);

    ret = jsmn_parse(&parser, text, token, DEFAULT_TOKEN_SIZE);
    switch (ret) {
        case JSMN_SUCCESS:
            break;
        case JSMN_ERROR_NOMEM:
            PyErr_SetString(PyExc_RuntimeError, "not enough tokens error.");
            return NULL;
        case JSMN_ERROR_INVAL:
            PyErr_SetString(PyExc_RuntimeError, "invalid json string.");
            return NULL;
        case JSMN_ERROR_PART:
        default:
            PyErr_SetString(PyExc_RuntimeError, "parse error.");
            return NULL;
    }

    return _build_value(&_ctx, token, text);
}


static PyMethodDef PyjsmnMethods[] = {
    {"loads",  (PyCFunction) pyjsmn_loads, METH_VARARGS | METH_KEYWORDS,
     pyjsmn_loads__doc__},
    {NULL, NULL, 0, NULL} /* Sentinel */
};


PyMODINIT_FUNC
initpyjsmn(void)
{
    PyObject *module;

    /* The module */
    module = Py_InitModule3("pyjsmn", PyjsmnMethods, pyjsmn__doc__);
    if (module == NULL)
        return;
}
