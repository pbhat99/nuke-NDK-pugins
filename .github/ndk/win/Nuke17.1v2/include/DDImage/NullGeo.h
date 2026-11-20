// NullGeo.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_NullGeo_h
#define DDImage_NullGeo_h

#include "DDImage/GeoOp.h"

#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

namespace DD
{
  namespace Image
  {

    /*!
       This is a stub GeoOp, intended to be the dead-end in a geometry
       tree.  It contains a single empty cache and does no work.
     */
    class DDImage_API NullGeo : public GeoOp
    {
    protected:

      /*! Do nothing - don't call any inputs. */
      void get_geometry_hash() override;

      /*! Do nothing - don't call any inputs. */
      void geometry_engine(Scene&, GeometryList& out) override;

    public:

      /*! Base class sets inputs to zero. */
      NullGeo(Node * node);
      int minimum_inputs() const override { return 0; }
      int maximum_inputs() const override { return 0; }

      const char* Class() const override;
      const char* node_help() const override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
