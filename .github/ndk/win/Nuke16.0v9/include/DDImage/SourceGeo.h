// SourceGeo.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_SourceGeo_h
#define DDImage_SourceGeo_h

#include "DDImage/NullGeo.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::SourceGeo
       Subclass of GeoOp that does not take another GeoOp as input. This
       alters the control panel knobs and tooltips slightly to avoid
       references to the input geometry.
     */
    class DDImage_API SourceGeo : public NullGeo
    {
      FrameRange _frameRange;

    public:

      /*! Constructs a single texture input. */
      SourceGeo(Node*);

      /*! Returns the Iop connected to the texture input. */
      Iop* input_iop() const;
      virtual Iop* default_material_iop() const { return input_iop(); }
      int minimum_inputs() const override { return 1; }
      int maximum_inputs() const override { return 1; }

      Op* default_input(int) const override;
      bool test_input(int input, Op* op) const override;
      const char* input_label(int input, char* buffer) const override;

      /*! Calls the GeoOp baseclass with \b source_geo=true so that
          the display knobs are correct for a Source object.
       */
      void knobs(Knob_Callback) override;

      /*! Constructs the transform to apply before the iop input is followed.
       */
      void build_input_handles(ViewerContext*);

      const char* Class() const override;
      const char* node_help() const override;


      /*! Assign all the base parameters for each GeoInfo in the output GeometryList.
       */
      virtual void init_geoinfo_parms(Scene&, GeometryList&);

      bool render_checkmark() const { return render_; }

    protected:
      //! Only constructors should call this, unless knobs() is not called
      void render_checkmark(bool v) { render_ = v; }

      //! build_handles with the context model matrix multiplied by the local
      //! matrix so nodes up the tree are displayed correctly
      //! Deprecated.  Use GeoOp::build_matrix_handles with a local matrix pointer instead.
      void build_matrix_handles(ViewerContext* ctx, Matrix4 localMatrix);


      /*! Hash up knobs that affect the geo's material.
       */
      void get_geometry_hash() override;


      /*! SourceGeo subclass implementation calls geometry_engine() on each
          GeoInfo in the local cache list.
       */
      void geometry_engine(Scene&, GeometryList& out) override;

      virtual void create_geometry(Scene&, GeometryList& out) = 0;

      void updateGeneralInfo() override;

    private:
      bool render_; //!< GUI render setting

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
