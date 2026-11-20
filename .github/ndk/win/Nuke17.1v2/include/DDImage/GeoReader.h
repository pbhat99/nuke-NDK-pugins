//------------------------------------------------------------------------------
// GeoReader.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_GEOREADER_H
#define DDIMAGE_GEOREADER_H

#include "DDImage/ReadGeo.h"

namespace DD
{
  namespace Image
  {

    class GeoDescription;

    /*! \class DD::Image::GeoReader
        Reads data from a geometry file.

        The ReadGeo creates one of these for each file it reads. Each
        different file type is a different subclass. Each subclass defines
        a Reader_Description object, all of these are searched to figure
        out what subclass to use.

        The base class for GeoReader both provides default implementations and
        serves as the "NullReader" that is used when the type cannot be
        figured out.

     */
    class DDImage_API GeoReader
    {
    protected:
      // Allow access to GeoReder to configure internals:
      friend class ReadGeo;

    protected:
      ReadGeo* geo; // Pointer to parent owner

    public:

      typedef GeoDescription Description;

      /*! Subclasses should have constructors that match
         Reader_Description::constructor.
       */
      GeoReader(ReadGeo*);

      /*! The destructor must close any files (even though the ReadGeo may
          have opened them).
       */
      virtual ~GeoReader();

      /*! This is called before the first call to geometry_engine().
          It should do any expensive setup.
       */
      virtual void _open();


      /*! The filename */
      const char* filename() const { return geo->fname(); }

      /*! read texture texture w coord */
      bool readTextureWCoord() { return geo->readTextureWCoord(); }

      /*! enable the axis knob or knot for the current plugin
          This is a temporary hack fix until per-mesh animation over-rides
          can be introduced. */
      virtual bool enableAxisKnob() { return true; }

      /*! Get the geometry hash. */
      virtual void get_geometry_hash(Hash* geo_hash) = 0;

      /*! Implements the knob_changed callback for readers. */
      virtual int knob_changed(Knob* k)  = 0;

      /*! Implements the _validate callback for readers. */
      virtual void _validate(const bool for_real) = 0;

      /*! Has the state changed? */
      virtual void append(Hash& newHash) = 0;

      /*! Default material */
      virtual Iop* default_material_iop() const { return geo->input_iop(); }


    public:
      //! Override this method to build the geometry
      virtual void geometry_engine(Scene&, GeometryList& out);
    };

  } // namespace Image
} // namespace DD

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
