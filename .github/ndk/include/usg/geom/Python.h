// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_PYTHON_H
#define USG_GEOM_PYTHON_H

#include "usg/geom/Layer.h"
#include "usg/geom/Stage.h"

// Abstract the PyObject pointer to avoid including Python here.
// Cast the returned PyObjectPtr to a PyObject*.
using PyObjectPtr = void*;

namespace usg
{
  //! Returns the native Python object for a wrapped stage (a pxr.Usd.Stage).
  //! If USD is configured without Python bindings, this will return nullptr.
  //! Cast the returned PyObjectPtr to a PyObject* before use.
  USG_API PyObjectPtr getPythonObject(const StageRef& stage);

  //! Returns the native Python object for a wrapped layer (an pxr.Sdf.Layer).
  //! If USD is configured without Python bindings, this will return nullptr.
  //! Cast the returned PyObjectPtr to a PyObject* before use.
  USG_API PyObjectPtr getPythonObject(const LayerRef& layer);

  //! Returns the native Python object for a wrapped layer (a pxr.Sdf.Path).
  //! If USD is configured without Python bindings, this will return nullptr.
  //! Cast the returned PyObjectPtr to a PyObject* before use.
  USG_API PyObjectPtr getPythonObject(const Path& path);

  //! Returns the native Python object for a wrapped SdfPrimSpecHandle/UsdPrim.
  //! If USD is configured without Python bindings, this will return nullptr.
  //! Cast the returned PyObjectPtr to a PyObject* before use.
  USG_API PyObjectPtr getPythonObject(const Prim& prim);

  //! Returns a wrapped stage ref (a pxr.Usd.Stage) for a USD stage Python object.
  //! If USD is configured without Python bindings, this will return an empty reference.
  //! Takes a PyObject* cast to PyObjectPtr as argument.
  USG_API StageRef getStageRefFromPythonObject(PyObjectPtr pyObject);
}

#endif
