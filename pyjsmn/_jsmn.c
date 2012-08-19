#include <Python.h>
#include <string.h>

#include "jsmn.h"

static jsmn_parser parser;


/* The module doc strings */
PyDoc_STRVAR(pyjsmn__doc__, "Python binding for jsmn");
PyDoc_STRVAR(pyjsmn_loads__doc__, "Decoding JSON");


static PyObject *
pyjsmn_loads(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"text"};
    char *text;
    int ret;
    jsmntok_t token[10];
    PyObject *object;

    /* Parse arguments */
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", kwlist, &text))
        return NULL;

    ret = jsmn_parse(&parser, text, token, 10);
    if (ret != JSMN_SUCCESS) {
        PyErr_SetString(PyExc_TypeError, "parse error.");
        return NULL;
    }

    int i;
    for (i=0; i<10; i++) {
        printf("%d, %d, %d, %d\n",
               token[i].type, token[i].start, token[i].end, token[i].size);
    }

    return Py_BuildValue("i", 1);
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

    jsmn_init(&parser);
}
