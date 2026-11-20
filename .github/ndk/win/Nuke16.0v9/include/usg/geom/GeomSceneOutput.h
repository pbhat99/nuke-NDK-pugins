// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_GEOMSCENEOUTPUT_H
#define USG_GEOM_GEOMSCENEOUTPUT_H

#include "usg/base/ArgSet.h"

#include <string>

namespace usg
{

  //--------------------------------------------------------------------------------


  /*! Implementation-specific arbitrary data container passed through GeomEngine
      graph via GeomSceneOutput.

      GeomEngines can construct a subclass of this and assign it to the
      GeomSceneOutput prior to calling getScenegraph().
  */
  class USG_API GeomSceneData
  {
  public:
    GeomSceneData() {}
    virtual ~GeomSceneData() {}
  };


  //--------------------------------------------------------------------------------


  /*!
  */
  class USG_API GeomSceneOutput
  {
  public:
    //! Arbitrary user data
    const std::string&   sceneDataID() const { return _scene_data_id; }
    GeomSceneData*       sceneData()   const { return _scene_data;    }
    void                 sceneData(GeomSceneData* p,
                                   const char*    id) { _scene_data_id = id; _scene_data = p; }


  public:
    GeomSceneOutput();

    friend USG_API std::ostream& operator << (std::ostream&, const GeomSceneOutput&);


  public://private:
    // TODO: these are public for now during prototyping - need methods to manipulate them:
    ArgSet          _scene_scope_vars;    //!< Output scene globals, added by GeomEngines in processScene().


  private:
    std::string     _scene_data_id;       //!< String ID for the scene data object
    GeomSceneData*  _scene_data;          //!< Arbitrary data defined and managed by GeomEngine subclasses

  };


  USG_API std::ostream& operator << (std::ostream&, const GeomSceneOutput&);


} // namespace usg

#endif

