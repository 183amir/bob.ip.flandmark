/**
 * @author Andre Anjos <andre.anjos@idiap.ch>
 * @date Thu 20 Sep 2012 14:46:35 CEST
 *
 * @brief Bob/Python extension to flandmark
 */

#include <bob.blitz/cppapi.h>
#include <bob.blitz/cleanup.h>
#include <bob.io.base/api.h>
#include <structmember.h>

#include <bob.extension/documentation.h>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#include <cstring>

#include <Flandmark.h>
#include <CImg.h>

/******************************************
 * Implementation of Localizer base class *
 ******************************************/

#define CLASS_NAME "Flandmark"

static auto s_class = bob::extension::ClassDoc(
  BOB_EXT_MODULE_PREFIX "." CLASS_NAME,
  "A key-point localization for faces using Flandmark",
  "This class can be used to locate facial landmarks on pre-detected faces. "
  "You input an image and a bounding-box specification and it returns you the positions for multiple key-points for the given face image.\n\n"
  "Consult http://cmp.felk.cvut.cz/~uricamic/flandmark/index.php for more information."
)
.add_constructor(
  bob::extension::FunctionDoc(
    CLASS_NAME,
    "Constructor",
    "Initializes the key-point locator with a model."
    )
  .add_prototype("[model]", "")
  .add_parameter("model", "str (path), optional", "Path to the localization model. If not set (or set to ``None``), then use the default localization model, stored on the class variable ``__default_model__``)")
)
;

typedef struct {
  PyObject_HEAD
  clandmark::Flandmark* flandmark;
  char* filename;
} PyBobIpFlandmarkObject;

static int PyBobIpFlandmark_init (PyBobIpFlandmarkObject* self, PyObject* args, PyObject* kwds) {

  /* Parses input arguments in a single shot */
  static const char* const_kwlist[] = {"model", 0};
  static char** kwlist = const_cast<char**>(const_kwlist);

  PyObject* model = 0;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O&", kwlist, &PyBobIo_FilenameConverter, &model)) return -1;

  if (!model) { //use what is stored in __default_model__
    PyObject* default_model = PyObject_GetAttrString((PyObject*)self, "__default_model__");
    if (!default_model) {
      PyErr_Format(PyExc_RuntimeError, "`%s' needs a model to properly initialize, but the user has not passed one and `__default_model__' is not properly set", Py_TYPE(self)->tp_name);
      return -1;
    }

    auto ok = PyBobIo_FilenameConverter(default_model, &model);
    Py_DECREF(default_model);

    if (!ok || !model) return -1;
  }

  const char* c_filename = 0;

# if PY_VERSION_HEX >= 0x03000000
  c_filename = PyBytes_AS_STRING(model);
# else
  c_filename = PyString_AS_STRING(model);
# endif
  Py_DECREF(model);

  //now we have a filename we can use
  if (!c_filename) return -1;

  self->flandmark = clandmark::Flandmark::getInstanceOf(c_filename);
  if (!self->flandmark) {
    PyErr_Format(PyExc_RuntimeError, "`%s' could not initialize from model file `%s'", Py_TYPE(self)->tp_name, c_filename);
    return -1;
  }

  //flandmark is now initialized, set filename
  self->filename = strndup(c_filename, 256);

  // all good, flandmark is ready
  return 0;

}

static void PyBobIpFlandmark_delete (PyBobIpFlandmarkObject* self) {
  // not sure if this is the best thing to do...
  delete self->flandmark;
  self->flandmark = 0;
  free(self->filename);
  self->filename = 0;
  Py_TYPE(self)->tp_free((PyObject*)self);
}

/**
 * Returns a list of key-point annotations given an image and an iterable over
 * bounding boxes.
 */
static PyObject* call(PyBobIpFlandmarkObject* self, boost::shared_ptr<cimg_library::CImg<unsigned char>> image, int nbbx, boost::shared_array<int> bbx) {

  PyObject* retval = PyTuple_New(nbbx);
  if (!retval) return 0;
  auto retval_ = make_safe(retval);

  for (int i=0; i<nbbx; ++i) {
    // detect face in the given bounding boc
    Py_BEGIN_ALLOW_THREADS
    self->flandmark->detect(image.get(), &bbx[4*i]);
    Py_END_ALLOW_THREADS

    double* detected = self->flandmark->getLandmarks();

    Py_ssize_t shape[2] = {self->flandmark->getLandmarksCount(), 2};
    PyBlitzArrayObject* landmarks = (PyBlitzArrayObject*)PyBlitzArray_SimpleNew(NPY_FLOAT64, 2, shape);
    blitz::Array<double,2> lm = *PyBlitzArrayCxx_AsBlitz<double,2>(landmarks);

    for (int i = 0; i < self->flandmark->getLandmarksCount(); ++i){
      // y -> 0
      lm(i, 0) = detected[2*i+1];
      // x -> 1
      lm(i, 1) = detected[2*i];
    }

    PyTuple_SET_ITEM(retval, i, PyBlitzArray_AsNumpyArray(landmarks, 0));
  }

  Py_INCREF(retval);
  return retval;

}

static auto s_call = bob::extension::FunctionDoc(
    "locate",
    "Locates keypoints on a **single** facial bounding-box on the provided image. "
    "This function returns a list of detected keypoints, which depends on the specified model, "
    "For the default models, the keypoints are organized in this way:\n\n"
    "0. Face center\n"
    "1. Canthus-rl (inner corner of the right eye).\n\n"
    ".. note:: The \"right eye\" means the right eye at the face w.r.t. the person on the image. "
    "That is the left eye in the image, from the viewer's perspective.\n\n"
    "2. Canthus-lr (inner corner of the left eye)\n"
    "3. Mouth-corner-r (right corner of the mouth)\n"
    "4. Mouth-corner-l (left corner of the mouth)\n"
    "5. Canthus-rr (outer corner of the right eye)\n"
    "6. Canthus-ll (outer corner of the left eye)\n"
    "7. Nose\n\n"
    "Each point is returned as tuple defining the pixel positions in the form (y, x)."
    )
    .add_prototype("image, offset, size", "landmarks")
    .add_parameter("image", "array-like (2D, uint8)",
      "The image Flandmark will operate on")
    .add_parameter("offset", "(int, int)", "The top left-most corner of the bounding box containing the face image you want to locate keypoints on.")
    .add_parameter("size", "(int, int)", "The dimensions accross y (height) and x (width) for the bounding box, in number of pixels.")
    .add_return("landmarks", "array (2D, float64)", "Each row in the output array contains the locations of keypoints in the format ``(y, x)``")
    ;

static PyObject* PyBobIpFlandmark_call_single(PyBobIpFlandmarkObject* self, PyObject *args, PyObject* kwds) {

  /* Parses input arguments in a single shot */
  static char* kwlist[] = {const_cast<char*>("image"), const_cast<char*>("offset"), const_cast<char*>("size"), 0};

  PyBlitzArrayObject* image = 0;
  blitz::TinyVector<int,2> position, size;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O&(ii)(ii)", kwlist, &PyBlitzArray_Converter, &image, &position[0], &position[1], &size[0], &size[1])) return 0;

  auto image_ = make_safe(image);

  // check
  if (image->type_num != NPY_UINT8 || image->ndim != 2) {
    PyErr_Format(PyExc_TypeError, "`%s' input `image' data must be a 2D array with dtype `uint8' (i.e. a gray-scaled image), but you passed a %" PY_FORMAT_SIZE_T "d array with data type `%s'", Py_TYPE(self)->tp_name, image->ndim, PyBlitzArray_TypenumAsString(image->type_num));
    return 0;
  }

  blitz::Array<uint8_t,2> blitz_image = *PyBlitzArrayCxx_AsBlitz<uint8_t, 2>(image);

	boost::shared_ptr<cimg_library::CImg<unsigned char>> cimg_image( new cimg_library::CImg<unsigned char>(blitz_image.extent(1), blitz_image.extent(0)) );

	for (int x = 0; x < blitz_image.extent(1); ++x)
		for (int y = 0; y < blitz_image.extent(0); ++y)
			(*cimg_image)(x, y) = blitz_image(y, x);

  // prepares the bbx vector
  boost::shared_array<int> bbx(new int[4]);
  bbx[0] = position[1];
  bbx[1] = position[0];
  bbx[2] = position[1] + size[1];
  bbx[3] = position[0] + size[0];

  PyObject* retval = call(self, cimg_image, 1, bbx);
  if (!retval) return 0;

  // gets the first entry, return it
  PyObject* retval0 = PyTuple_GET_ITEM(retval, 0);
  if (!retval0) return 0;

  Py_INCREF(retval0);
  Py_DECREF(retval);

  return retval0;

};

static PyMethodDef PyBobIpFlandmark_methods[] = {
  {
    s_call.name(),
    (PyCFunction)PyBobIpFlandmark_call_single,
    METH_VARARGS|METH_KEYWORDS,
    s_call.doc()
  },
  {0} /* Sentinel */
};

PyObject* PyBobIpFlandmark_Repr(PyBobIpFlandmarkObject* self) {

  /**
   * Expected output:
   *
   * <bob.ip.flandmark(model='...')>
   */

  PyObject* retval = PyUnicode_FromFormat("<%s(model='%s')>",
      Py_TYPE(self)->tp_name, self->filename);

#if PYTHON_VERSION_HEX < 0x03000000
  if (!retval) return 0;
  PyObject* tmp = PyObject_Str(retval);
  Py_DECREF(retval);
  retval = tmp;
#endif

  return retval;
}

PyTypeObject PyBobIpFlandmark_Type = {
  PyVarObject_HEAD_INIT(0, 0)
  0
};

bool init_BobIpFlandmark(PyObject* module){
  // initialize the type struct
  PyBobIpFlandmark_Type.tp_name = s_class.name();
  PyBobIpFlandmark_Type.tp_basicsize = sizeof(PyBobIpFlandmarkObject);
  PyBobIpFlandmark_Type.tp_flags = Py_TPFLAGS_DEFAULT;
  PyBobIpFlandmark_Type.tp_doc = s_class.doc();

  // set the functions
  PyBobIpFlandmark_Type.tp_new = PyType_GenericNew;
  PyBobIpFlandmark_Type.tp_init = reinterpret_cast<initproc>(PyBobIpFlandmark_init);
  PyBobIpFlandmark_Type.tp_dealloc = reinterpret_cast<destructor>(PyBobIpFlandmark_delete);
  PyBobIpFlandmark_Type.tp_methods = PyBobIpFlandmark_methods;
  PyBobIpFlandmark_Type.tp_call = reinterpret_cast<ternaryfunc>(PyBobIpFlandmark_call_single);

  // representations
  PyBobIpFlandmark_Type.tp_repr = (reprfunc)PyBobIpFlandmark_Repr;
  PyBobIpFlandmark_Type.tp_str = (reprfunc)PyBobIpFlandmark_Repr;

  // check that everything is fine
  if (PyType_Ready(&PyBobIpFlandmark_Type) < 0)
    return false;

  // add the type to the module
  Py_INCREF(&PyBobIpFlandmark_Type);
  return PyModule_AddObject(module, CLASS_NAME, (PyObject*)&PyBobIpFlandmark_Type) >= 0;
}
