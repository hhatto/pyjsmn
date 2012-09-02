#include <Python.h>
#include <string.h>

#include "jsmn.h"

#define DEFAULT_TOKEN_SIZE 256

#ifdef DEBUG
#define _debug(format,...) printf("[DEBUG]" format,__VA_ARGS__)
#define _info(format,...) printf("[INFO]" format,__VA_ARGS__)
#endif

typedef struct {
    PyObject *stack[128];
    unsigned int size;
    unsigned int used;
} _pyjsmn_stack;

typedef struct {
    _pyjsmn_stack elements;
    _pyjsmn_stack keys;
    PyObject *root;
    jsmntype_t root_type;
} _pyjsmn_ctx;

static _pyjsmn_ctx _ctx;
static jsmn_parser parser;

/* The module doc strings */
PyDoc_STRVAR(pyjsmn__doc__, "Python binding for jsmn");
PyDoc_STRVAR(pyjsmn_loads__doc__, "Decoding JSON");

static int
_set_object(_pyjsmn_ctx *ctx, PyObject *parent, PyObject *child)
{
    if (child && !parent) {
        ctx->root = child;
        ctx->elements.stack[ctx->elements.used - 1] = child;
        return 0;
    }
    else if (!parent || !child)
        return -1;

    if (PyList_Check(parent)) {
        PyList_Append(parent, child);
        if (child && child != Py_None) {
            Py_XDECREF(child);
        }
    }
    else if (PyDict_Check(parent)) {
        PyObject *key = ctx->keys.stack[ctx->keys.used - 1];
#ifdef DEBUG
        _debug("used:%d\n", ctx->keys.used);
        _debug("parent:%p, key:%p, child:%p\n", parent, key, child);
#endif
        PyDict_SetItem(parent, key, child);
        Py_DECREF(key);
        if (child && child != Py_None) {
            Py_XDECREF(child);
        }
        ctx->keys.used--;
    }

    return 0;
}

static PyObject*
_get_pyobject(_pyjsmn_ctx *ctx, jsmntok_t *token, char *jsontext)
{
    int offset;
    int is_float;
    PyObject *object;
    void *tmp_string;

    switch (token->type) {
        case JSMN_OBJECT:
            object = PyDict_New();
            ctx->elements.size += token->size;
            ctx->root_type = JSMN_OBJECT;
            ctx->elements.used++;
            break;
        case JSMN_ARRAY:
            object = PyList_New(0);
            ctx->elements.size += token->size;
            ctx->root_type = JSMN_ARRAY;
            ctx->elements.used++;
            break;
        case JSMN_STRING:
            object = PyUnicode_FromStringAndSize(jsontext+token->start,
                                                 token->end - token->start);
            // TODO: error handling
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
                    Py_XDECREF(tmp_string);
                    break;
            }
            break;
        default:
            // TODO: error handling
            printf("not support type!!\n");
            return NULL;
    }

    return object;
}

static PyObject *
_build_value(_pyjsmn_ctx *ctx, jsmntok_t *token, char *jsontext)
{
    int i;
    PyObject *object;

    for (i = 0; token[i].start != -1; i++) {
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
        _debug("type         :%d\n", (token+i)->type);
        _debug("root_type    :%d\n", ctx->root_type);
        _debug("elements.used:%d\n", ctx->elements.used);
        _debug("elements.size:%d\n", ctx->elements.size);
        _debug("keys.used    :%d\n", ctx->keys.used);
        _debug("keys.size    :%d\n", ctx->keys.size);
#endif

        if (!PyDict_Check(object) &&
            (ctx->root_type == JSMN_OBJECT) && ((ctx->keys.used % 2) == 0)) {
            ctx->keys.stack[ctx->keys.used] = object;
            ctx->keys.used++;
            continue;
        }

        _set_object(ctx, ctx->root, object);
        //_set_object(ctx, ctx->elements.stack[ctx->elements.used - 1], object);
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
