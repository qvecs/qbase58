#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "base58.h"

/* 
 * Python method: encode(data: bytes) -> str
 *  - Takes raw bytes, returns a textual Base58 representation.
 */
static PyObject* py_encode(PyObject* self, PyObject* args) {
    const unsigned char* input_data = NULL;
    Py_ssize_t input_length = 0;

    // Parse a Python bytes object
    if (!PyArg_ParseTuple(args, "y#", &input_data, &input_length)) {
        return NULL;  // Raises TypeError if not bytes
    }

    char* encoded = NULL;
    size_t encoded_len = 0;
    int ret = base58_encode(input_data, (size_t)input_length, &encoded, &encoded_len);
    if (ret != 0 || !encoded) {
        PyErr_SetString(PyExc_RuntimeError, "Base58 encoding failed.");
        if (encoded) {
            free(encoded);
        }
        return NULL;
    }

    // Convert the char* (ASCII) into a Python str
    // Since Base58 is guaranteed ASCII, this is safe.
    PyObject* result = PyUnicode_FromStringAndSize(encoded, (Py_ssize_t)encoded_len);

    free(encoded);
    return result;  // a Python str
}

/* 
 * Python method: decode(data: bytes|str) -> bytes
 *  - Takes a textual Base58 string (or bytes) and returns raw bytes.
 */
static PyObject* py_decode(PyObject* self, PyObject* args) {
    PyObject* input_obj = NULL;

    // Accept one argument, which can be bytes or str
    if (!PyArg_ParseTuple(args, "O", &input_obj)) {
        return NULL;
    }

    // If it's str, encode to ASCII bytes first
    PyObject* bytes_obj = NULL;
    if (PyUnicode_Check(input_obj)) {
        bytes_obj = PyUnicode_AsASCIIString(input_obj); 
        // PyUnicode_AsASCIIString() returns a new reference or NULL on failure
        if (!bytes_obj) {
            return NULL;  // Raises UnicodeEncodeError, etc.
        }
    } 
    else if (PyBytes_Check(input_obj)) {
        // Just use the existing bytes
        bytes_obj = input_obj;
        Py_INCREF(bytes_obj); // so we can safely Py_DECREF later
    } 
    else {
        PyErr_SetString(PyExc_TypeError, "decode() requires a str or bytes object.");
        return NULL;
    }

    // Extract raw bytes
    const char* input_data = PyBytes_AsString(bytes_obj);
    Py_ssize_t input_length = PyBytes_Size(bytes_obj);
    if (!input_data || input_length < 0) {
        Py_XDECREF(bytes_obj);
        return NULL;
    }

    // Convert to null-terminated char[] for base58_decode
    char* cbuf = (char*)malloc(input_length + 1);
    if (!cbuf) {
        Py_XDECREF(bytes_obj);
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate buffer");
        return NULL;
    }
    memcpy(cbuf, input_data, input_length);
    cbuf[input_length] = '\0';

    unsigned char* decoded = NULL;
    size_t decoded_len = 0;
    int ret = base58_decode(cbuf, &decoded, &decoded_len);
    free(cbuf);

    Py_XDECREF(bytes_obj);

    if (ret != 0 || !decoded) {
        free(decoded);
        PyErr_SetString(PyExc_ValueError, "Invalid Base58 string.");
        return NULL;
    }

    // Return a bytes object with the decoded binary data
    PyObject* result = PyBytes_FromStringAndSize((const char*)decoded, (Py_ssize_t)decoded_len);
    free(decoded);
    return result;  // a Python bytes
}

/* Module method definitions */
static PyMethodDef py_base58_methods[] = {
    {"encode", (PyCFunction)py_encode, METH_VARARGS, "Encode raw bytes into Base58 string (ASCII)."},
    {"decode", (PyCFunction)py_decode, METH_VARARGS, "Decode Base58 data (bytes/str) into raw bytes."},
    {NULL, NULL, 0, NULL}
};

/* Module definition */
static struct PyModuleDef py_base58_module = {
    PyModuleDef_HEAD_INIT,
    "py_base58",
    NULL,
    -1,
    py_base58_methods
};

PyMODINIT_FUNC PyInit_py_base58(void) {
    return PyModule_Create(&py_base58_module);
}
