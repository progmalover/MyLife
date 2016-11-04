/*
 * =====================================================================================
 *
 *       Filename:  spammodule.c
 *
 *    Description:  demo of extending python modules, also avilable in
 *                  Demo/embed/demo.c in the Python source distribution.
 *
 *        Version:  1.0
 *        Created:  03/08/2016 03:39:56 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <python.h>

/* Notice, we have no need to define whole class include main function to let this module
 * work. As the python virtual mechina helps us to do this.
 * Classes, functions or some symbols in python-c modules always starts with Py or PY, so
 * we have to obay it.
 * The PyXXX function is python module API from python.h
 * */

static PyObject *
spam_system(PyObject *self, PyObject *args)
{
    const char *command;
    int status;

    if (!PyArg_ParseTuple(args, "s", &command)) {
        return NULL;
    }
    status = system(command);   // call system API in linux to run command
    return Py_BuildValue("i", sts);
}


/* Same as OVS command define method.
 * PyMethodDef, METH_VARARGS is what we must use.
 * */
static PyMethodDef SpamMethods[] = {
    ...
    {"system",  spam_system, METH_VARARGS,
    "Execute a shell command."},
    ...
    {NULL, NULL, 0, NULL}        /*  Sentinel */
};*/


/* The method table must be passed to the interpreter in the module’s initialization function. Which
 * means we have to register functions using methods bellow.
 * */

PyMODINIT_FUNC
initspam(void)
{
    (void) Py_InitModule("spam", SpamMethods);
}
