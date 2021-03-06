/**
 * @author Andre Anjos <andre.anjos@idiap.ch>
 * @date Fri 25 Oct 16:54:55 2013
 *
 * @brief Bindings to bob::ap
 */

#ifdef NO_IMPORT_ARRAY
#undef NO_IMPORT_ARRAY
#endif
#include <bob.blitz/capi.h>
#include <bob.blitz/cleanup.h>
#include <bob.core/api.h>
#include <bob.io.base/api.h>
#include <bob.extension/documentation.h>

extern PyTypeObject PyBobIpFlandmark_Type;

static auto s_setter = bob::extension::FunctionDoc(
    "__set_default_model__",
    "Internal function to set the default model for the Flandmark class"
    )
    .add_prototype("path", "")
    .add_parameter("path", "str", "The path to the new model file")
    ;

PyObject* set_flandmark_model(PyObject*, PyObject* o) {

  int ok = PyDict_SetItemString(PyBobIpFlandmark_Type.tp_dict,
      "__default_model__", o);

  if (ok == -1) return 0;

  Py_RETURN_NONE;

}

static PyMethodDef module_methods[] = {
  {
    s_setter.name(),
    (PyCFunction)set_flandmark_model,
    METH_O,
    s_setter.doc()
  },
  {0}  /* Sentinel */
};

PyDoc_STRVAR(module_docstr, "Flandmark keypoint localization library");

#if PY_VERSION_HEX >= 0x03000000
static PyModuleDef module_definition = {
  PyModuleDef_HEAD_INIT,
  BOB_EXT_MODULE_NAME,
  module_docstr,
  -1,
  module_methods,
  0, 0, 0, 0
};
#endif

static PyObject* create_module (void) {

  //makes sure that PyBobIpFlandmark_Type has a dictionary on tp_dict
  PyBobIpFlandmark_Type.tp_dict = PyDict_New();
  if (!PyBobIpFlandmark_Type.tp_dict) return 0;

  PyBobIpFlandmark_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobIpFlandmark_Type) < 0) return 0;

# if PY_VERSION_HEX >= 0x03000000
  PyObject* module = PyModule_Create(&module_definition);
  auto module_ = make_xsafe(module);
  const char* ret = "O";
# else
  PyObject* module = Py_InitModule3(BOB_EXT_MODULE_NAME, module_methods, module_docstr);
  const char* ret = "N";
# endif
  if (!module) return 0;

  /* register the types to python */
  Py_INCREF(&PyBobIpFlandmark_Type);
  if (PyModule_AddObject(module, "Flandmark", (PyObject *)&PyBobIpFlandmark_Type) < 0) return 0;

  /* imports dependencies */
  if (import_bob_blitz() < 0) return 0;
  if (import_bob_core_logging() < 0) return 0;
  if (import_bob_io_base() < 0) return 0;

  return Py_BuildValue(ret, module);
}

PyMODINIT_FUNC BOB_EXT_ENTRY_NAME (void) {
# if PY_VERSION_HEX >= 0x03000000
  return
# endif
    create_module();
}
