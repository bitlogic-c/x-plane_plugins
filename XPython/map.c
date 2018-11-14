#define _GNU_SOURCE
#include <Python.h>
#include <stdio.h>
#include <stdbool.h>
#define XPLM200
#define XPLM210
#define XPLM300
#include <XPLM/XPLMDefs.h>
#include <XPLM/XPLMMap.h>
#include "plugin_dl.h"

PyObject *mapDict;
intptr_t mapCntr;
PyObject *mapRefDict;
PyObject *mapCreateDict;
intptr_t mapCreateCntr;

static inline void mapCallback(int inCallbackIndex, XPLMMapLayerID inLayer, const float *inMapBoundsLeftTopRightBottom, float zoomRatio,
                        float mapUnitsPerUserInterfaceUnit, XPLMMapStyle mapStyle, XPLMMapProjectionID projection,
                        void *inRefcon)
{
  PyObject *layerObj, *boundsObj, *projectionObj, *refconObj, *callback;
  PyObject *ref = PyLong_FromVoidPtr(inRefcon);
  PyObject *callbackInfo = PyDict_GetItem(mapDict, ref);
  Py_XDECREF(ref);
  if(callbackInfo == NULL){
    printf("Couldn't find map callback with id = %p.", inRefcon); 
    return;
  }

  layerObj = PyLong_FromVoidPtr(inLayer);
  projectionObj = PyLong_FromVoidPtr(projection);
  refconObj = PySequence_GetItem(callbackInfo, 9);
  callback = PySequence_GetItem(callbackInfo, inCallbackIndex);
  
  boundsObj = PyTuple_New(4);
  PyTuple_SET_ITEM(boundsObj, 0, PyFloat_FromDouble((double)inMapBoundsLeftTopRightBottom[0]));
  PyTuple_SET_ITEM(boundsObj, 1, PyFloat_FromDouble((double)inMapBoundsLeftTopRightBottom[1]));
  PyTuple_SET_ITEM(boundsObj, 2, PyFloat_FromDouble((double)inMapBoundsLeftTopRightBottom[2]));
  PyTuple_SET_ITEM(boundsObj, 3, PyFloat_FromDouble((double)inMapBoundsLeftTopRightBottom[3]));

  PyObject *pRes = PyObject_CallFunction(callback, "(OOffiOO)", layerObj, boundsObj, zoomRatio,
                                         mapUnitsPerUserInterfaceUnit, mapStyle, projectionObj, refconObj);
  if(!pRes){
    printf("MapCallback callback failed.\n");
    PyObject *err = PyErr_Occurred();
    if(err){
      PyErr_Print();
    }
  }
  Py_DECREF(boundsObj);
  Py_DECREF(layerObj);
  Py_DECREF(projectionObj);
  Py_XDECREF(pRes);
}

static inline void mapPrepareCacheCallback(XPLMMapLayerID inLayer, const float *inMapBoundsLeftTopRightBottom,
                                           XPLMMapProjectionID projection, void *inRefcon)
{
  PyObject *layerObj, *boundsObj, *projectionObj, *refconObj, *callback;
  PyObject *ref = PyLong_FromVoidPtr(inRefcon);
  PyObject *callbackInfo = PyDict_GetItem(mapDict, ref);
  Py_XDECREF(ref);
  if(callbackInfo == NULL){
    printf("Couldn't find map callback with id = %p.", inRefcon); 
    return;
  }

  layerObj = PyLong_FromVoidPtr(inLayer);
  projectionObj = PyLong_FromVoidPtr(projection);
  refconObj = PySequence_GetItem(callbackInfo, 9);
  callback = PySequence_GetItem(callbackInfo, 3);
  
  boundsObj = PyTuple_New(4);
  PyTuple_SET_ITEM(boundsObj, 0, PyFloat_FromDouble((double)inMapBoundsLeftTopRightBottom[0]));
  PyTuple_SET_ITEM(boundsObj, 1, PyFloat_FromDouble((double)inMapBoundsLeftTopRightBottom[1]));
  PyTuple_SET_ITEM(boundsObj, 2, PyFloat_FromDouble((double)inMapBoundsLeftTopRightBottom[2]));
  PyTuple_SET_ITEM(boundsObj, 3, PyFloat_FromDouble((double)inMapBoundsLeftTopRightBottom[3]));

  PyObject *pRes = PyObject_CallFunction(callback, "(OOOO)", layerObj, boundsObj, projectionObj, refconObj);
  if(!pRes){
    printf("MapPrepareCacheCallback callback failed.\n");
    PyObject *err = PyErr_Occurred();
    if(err){
      PyErr_Print();
    }
  }
  Py_DECREF(boundsObj);
  Py_DECREF(layerObj);
  Py_DECREF(projectionObj);
  Py_XDECREF(pRes);
}

static inline void mapWillBeDeletedCallback(XPLMMapLayerID inLayer, void *inRefcon)
{
  PyObject *layerObj, *refconObj, *callback;
  PyObject *ref = PyLong_FromVoidPtr(inRefcon);
  PyObject *callbackInfo = PyDict_GetItem(mapDict, ref);
  Py_XDECREF(ref);
  if(callbackInfo == NULL){
    printf("Couldn't find map callback with id = %p.", inRefcon); 
    return;
  }

  layerObj = PyLong_FromVoidPtr(inLayer);
  refconObj = PySequence_GetItem(callbackInfo, 9);
  callback = PySequence_GetItem(callbackInfo, 2);
  
  PyObject *pRes = PyObject_CallFunction(callback, "(OO)", layerObj, refconObj);
  if(!pRes){
    printf("MapWillBeDeletedCallback callback failed.\n");
    PyObject *err = PyErr_Occurred();
    if(err){
      PyErr_Print();
    }
  }
  Py_DECREF(layerObj);
  Py_XDECREF(pRes);
}

static inline void mapCreatedCallback(const char *mapIdentifier, void *inRefcon)
{
  PyObject *mapIdentifierObj, *refconObj, *callback;
  PyObject *ref = PyLong_FromVoidPtr(inRefcon);
  PyObject *callbackInfo = PyDict_GetItem(mapCreateDict, ref);
  Py_XDECREF(ref);
  if(callbackInfo == NULL){
    printf("Couldn't find map created callback with id = %p.", inRefcon); 
    return;
  }
  mapIdentifierObj = PyUnicode_DecodeUTF8(mapIdentifier, strlen(mapIdentifier), NULL);
  callback = PySequence_GetItem(callbackInfo, 0);
  refconObj = PySequence_GetItem(callbackInfo, 1);
  
  PyObject *pRes = PyObject_CallFunction(callback, "(OO)", mapIdentifierObj, refconObj);
  if(!pRes){
    printf("mapCreatedCallback callback failed.\n");
    PyObject *err = PyErr_Occurred();
    if(err){
      PyErr_Print();
    }
  }
  Py_DECREF(mapIdentifierObj);
  Py_XDECREF(pRes);
}

static void mapDrawingCallback(XPLMMapLayerID inLayer, const float *inMapBoundsLeftTopRightBottom, float zoomRatio,
                        float mapUnitsPerUserInterfaceUnit, XPLMMapStyle mapStyle, XPLMMapProjectionID projection,
                        void *inRefcon)
{
  mapCallback(4, inLayer, inMapBoundsLeftTopRightBottom, zoomRatio,
              mapUnitsPerUserInterfaceUnit, mapStyle, projection, inRefcon);
}

static void mapIconDrawingCallback(XPLMMapLayerID inLayer, const float *inMapBoundsLeftTopRightBottom, float zoomRatio,
                        float mapUnitsPerUserInterfaceUnit, XPLMMapStyle mapStyle, XPLMMapProjectionID projection,
                        void *inRefcon)
{
  mapCallback(5, inLayer, inMapBoundsLeftTopRightBottom, zoomRatio,
              mapUnitsPerUserInterfaceUnit, mapStyle, projection, inRefcon);
}

static void mapLabelDrawingCallback(XPLMMapLayerID inLayer, const float *inMapBoundsLeftTopRightBottom, float zoomRatio,
                        float mapUnitsPerUserInterfaceUnit, XPLMMapStyle mapStyle, XPLMMapProjectionID projection,
                        void *inRefcon)
{
  mapCallback(6, inLayer, inMapBoundsLeftTopRightBottom, zoomRatio,
              mapUnitsPerUserInterfaceUnit, mapStyle, projection, inRefcon);
}

static PyObject *XPLMCreateMapLayerFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *pluginSelf, *params;
  XPLMCreateMapLayer_t inParams;

  if(!XPLMCreateMapLayer_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMCreateMapLayer is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "OO", &pluginSelf, &params)){
    return NULL;
  }

  void *ref = (void *)++mapCntr;
  inParams.structSize = sizeof(inParams);
  inParams.mapToCreateLayerIn = PyUnicode_AsUTF8(PySequence_GetItem(params, 0));
  inParams.layerType = PyLong_AsLong(PySequence_GetItem(params, 1));
  inParams.willBeDeletedCallback = mapWillBeDeletedCallback;
  inParams.prepCacheCallback = mapPrepareCacheCallback;
  inParams.drawCallback = mapDrawingCallback;
  inParams.iconCallback = mapIconDrawingCallback;
  inParams.labelCallback = mapLabelDrawingCallback;
  inParams.showUiToggle = PyLong_AsLong(PySequence_GetItem(params, 7));
  inParams.layerName = PyUnicode_AsUTF8(PySequence_GetItem(params, 8));
  inParams.refcon = ref;

  XPLMMapLayerID res = XPLMCreateMapLayer_ptr(&inParams);
  if(!res){
    return NULL;
  }
  PyObject *resObj = PyLong_FromVoidPtr(res);
  PyObject *refObj = PyLong_FromVoidPtr(ref);
  PyDict_SetItem(mapDict, refObj, params);
  PyDict_SetItem(mapRefDict, resObj, refObj);

  return resObj;
}

static PyObject *XPLMDestroyMapLayerFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *pluginSelf, *layer;

  if(!XPLMDestroyMapLayer_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMDestroyMapLayer is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "OO", &pluginSelf, &layer)){
    return NULL;
  }

  XPLMMapLayerID inLayer = PyLong_AsVoidPtr(layer);
  int res = XPLMDestroyMapLayer_ptr(inLayer);
  if(res){
    PyObject *ref = PyDict_GetItem(mapRefDict, layer);
    PyDict_DelItem(mapDict, ref);
    PyDict_DelItem(mapRefDict, layer);
  }

  return PyLong_FromLong(res);
}

static PyObject *XPLMRegisterMapCreationHookFun(PyObject *self, PyObject *args)
{
  (void) self;
  if(!XPLMRegisterMapCreationHook_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMRegisterMapCreationHook is available only in XPLM300 and up.");
    return NULL;
  }
  void *refcon = (void *)++mapCreateCntr;
  PyDict_SetItem(mapCreateDict, PyLong_FromVoidPtr(refcon), args);
  XPLMRegisterMapCreationHook_ptr(mapCreatedCallback, refcon);
  Py_RETURN_NONE;
}

static PyObject *XPLMMapExistsFun(PyObject *self, PyObject *args)
{
  (void) self;
  const char *mapIdentifier;

  if(!XPLMMapExists_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMMapExists is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "s", &mapIdentifier)){
    return NULL;
  }
  int res = XPLMMapExists_ptr(mapIdentifier);
  return PyLong_FromLong(res);
}

static PyObject *XPLMDrawMapIconFromSheetFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *layerObj;
  const char *inPngPath;
  int s, t, ds, dt;
  float mapX, mapY, rotationDegrees, mapWidth;
  XPLMMapOrientation orientation;

  if(!XPLMDrawMapIconFromSheet_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMDrawMapIconFromSheet is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "Osiiiiffiff", &layerObj, &inPngPath, &s, &t, &ds, &dt, &mapX, &mapY,
                       &orientation, &rotationDegrees, &mapWidth)){
    return NULL;
  }
  XPLMMapLayerID layer = PyLong_AsVoidPtr(layerObj);
  XPLMDrawMapIconFromSheet_ptr(layer, inPngPath, s, t, ds, dt, mapX, mapY,
                       orientation, rotationDegrees, mapWidth);
  Py_RETURN_NONE;
}

static PyObject *XPLMDrawMapLabelFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *layerObj;
  const char *inText;
  float mapX, mapY, rotationDegrees;
  XPLMMapOrientation orientation;

  if(!XPLMDrawMapLabel_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMDrawMapLabel is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "Osffif", &layerObj, &inText, &mapX, &mapY,
                       &orientation, &rotationDegrees)){
    return NULL;
  }
  XPLMMapLayerID layer = PyLong_AsVoidPtr(layerObj);
  XPLMDrawMapLabel_ptr(layer, inText, mapX, mapY, orientation, rotationDegrees);
  Py_RETURN_NONE;
}

static PyObject *XPLMMapProjectFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *projectionObj, *outXObj, *outYObj;
  double latitude, longitude;

  if(!XPLMMapProject_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMMapProject is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "OddOO", &projectionObj, &latitude, &longitude, &outXObj, &outYObj)){
    return NULL;
  }
  XPLMMapProjectionID projection = PyLong_AsVoidPtr(projectionObj);
  float outX, outY;
  XPLMMapProject_ptr(projection, latitude, longitude, &outX, &outY);
  if(outXObj != Py_None){
    PyList_Append(outXObj, PyFloat_FromDouble(outX));
  }
  if(outYObj != Py_None){
    PyList_Append(outYObj, PyFloat_FromDouble(outY));
  }
  Py_RETURN_NONE;
}

static PyObject *XPLMMapUnprojectFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *projectionObj, *outLatitudeObj, *outLongitudeObj;
  float mapX, mapY;

  if(!XPLMMapUnproject_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMMapUnproject is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "OffOO", &projectionObj, &mapX, &mapY, &outLatitudeObj, &outLongitudeObj)){
    return NULL;
  }
  XPLMMapProjectionID projection = PyLong_AsVoidPtr(projectionObj);
  double outLongitude, outLatitude;
  XPLMMapUnproject_ptr(projection, mapX, mapY, &outLatitude, &outLongitude);
  if(outLatitudeObj != Py_None){
    PyList_Append(outLatitudeObj, PyFloat_FromDouble(outLatitude));
  }
  if(outLongitudeObj != Py_None){
    PyList_Append(outLongitudeObj, PyFloat_FromDouble(outLongitude));
  }
  Py_RETURN_NONE;
}

static PyObject *XPLMMapScaleMeterFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *projectionObj;
  float mapX, mapY;

  if(!XPLMMapScaleMeter_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMMapScaleMeter is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "Off", &projectionObj, &mapX, &mapY)){
    return NULL;
  }
  XPLMMapProjectionID projection = PyLong_AsVoidPtr(projectionObj);
  float res = XPLMMapScaleMeter_ptr(projection, mapX, mapY);
  return PyFloat_FromDouble(res);
}

static PyObject *XPLMMapGetNorthHeadingFun(PyObject *self, PyObject *args)
{
  (void) self;
  PyObject *projectionObj;
  float mapX, mapY;

  if(!XPLMMapGetNorthHeading_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMMapGetNorthHeading is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "Off", &projectionObj, &mapX, &mapY)){
    return NULL;
  }
  XPLMMapProjectionID projection = PyLong_AsVoidPtr(projectionObj);
  float res = XPLMMapGetNorthHeading_ptr(projection, mapX, mapY);
  return PyFloat_FromDouble(res);
}

static PyObject *cleanup(PyObject *self, PyObject *args)
{
  (void) self;
  (void) args;
  PyDict_Clear(mapDict);
  Py_DECREF(mapDict);
  PyDict_Clear(mapRefDict);
  Py_DECREF(mapRefDict);
  PyDict_Clear(mapCreateDict);
  Py_DECREF(mapCreateDict);
  Py_RETURN_NONE;
}

static PyMethodDef XPLMMapMethods[] = {
  {"XPLMCreateMapLayer", XPLMCreateMapLayerFun, METH_VARARGS, ""},
  {"XPLMDestroyMapLayer", XPLMDestroyMapLayerFun, METH_VARARGS, ""},
  {"XPLMRegisterMapCreationHook", XPLMRegisterMapCreationHookFun, METH_VARARGS, ""},
  {"XPLMMapExists", XPLMMapExistsFun, METH_VARARGS, ""},
  {"XPLMDrawMapIconFromSheet", XPLMDrawMapIconFromSheetFun, METH_VARARGS, ""},
  {"XPLMDrawMapLabel", XPLMDrawMapLabelFun, METH_VARARGS, ""},
  {"XPLMMapProject", XPLMMapProjectFun, METH_VARARGS, ""},
  {"XPLMMapUnproject", XPLMMapUnprojectFun, METH_VARARGS, ""},
  {"XPLMMapScaleMeter", XPLMMapScaleMeterFun, METH_VARARGS, ""},
  {"XPLMMapGetNorthHeading", XPLMMapGetNorthHeadingFun, METH_VARARGS, ""},
  {"cleanup", cleanup, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL}
};

static struct PyModuleDef XPLMMapModule = {
  PyModuleDef_HEAD_INIT,
  "XPLMMap",
  NULL,
  -1,
  XPLMMapMethods,
  NULL,
  NULL,
  NULL,
  NULL
};

PyMODINIT_FUNC
PyInit_XPLMMap(void)
{
  if(!(mapCreateDict = PyDict_New())){
    return NULL;
  }
  if(!(mapRefDict = PyDict_New())){
    return NULL;
  }
  if(!(mapDict = PyDict_New())){
    return NULL;
  }
  PyObject *mod = PyModule_Create(&XPLMMapModule);
  if(mod){
    PyModule_AddIntConstant(mod, "xplm_MapStyle_VFR_Sectional", xplm_MapStyle_VFR_Sectional);
    PyModule_AddIntConstant(mod, "xplm_MapStyle_IFR_LowEnroute", xplm_MapStyle_IFR_LowEnroute);
    PyModule_AddIntConstant(mod, "xplm_MapStyle_IFR_HighEnroute", xplm_MapStyle_IFR_HighEnroute);
    PyModule_AddIntConstant(mod, "xplm_MapLayer_Fill", xplm_MapLayer_Fill);
    PyModule_AddIntConstant(mod, "xplm_MapLayer_Markings", xplm_MapLayer_Markings);
    PyModule_AddStringConstant(mod, "XPLM_MAP_USER_INTERFACE", XPLM_MAP_USER_INTERFACE);
    PyModule_AddStringConstant(mod, "XPLM_MAP_IOS", XPLM_MAP_IOS);
    PyModule_AddIntConstant(mod, "xplm_MapOrientation_Map", xplm_MapOrientation_Map);
    PyModule_AddIntConstant(mod, "xplm_MapOrientation_UI", xplm_MapOrientation_UI);
  }
  return mod;
}




