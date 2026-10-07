// Bindings
#include "init.h"

// Lib
#include <procmetrix/version.h>

static PyModuleDef moduleDef = {
    PyModuleDef_HEAD_INIT, "pyprocmetrix", NULL, -1, NULL,
};

PyMODINIT_FUNC PyInit_pyprocmetrix()
{
    // Register the module
    PyObject* module = PyModule_Create(&moduleDef);
    if (!module)
        return NULL;

    // Add version constants
    PyModule_AddIntConstant(
        module, "version_major", procmetrix_version_major());

    PyModule_AddIntConstant(
        module, "version_minor", procmetrix_version_minor());

    PyModule_AddIntConstant(
        module, "version_patch", procmetrix_version_patch());

    // Register each topic
    pyprocmetrix_init_mem(module);
    pyprocmetrix_init_cpu(module);

    return module;
}
