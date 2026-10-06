#include "init.h"

static PyModuleDef moduleDef = {
    PyModuleDef_HEAD_INIT, "pyprocmetrix", NULL, -1, NULL,
};

PyMODINIT_FUNC PyInit_pyprocmetrix()
{
    // Register the module
    PyObject* module = PyModule_Create(&moduleDef);
    if (!module)
        return NULL;

    // Register each topic
    pyprocmetrix_init_mem(module);

    return module;
}
