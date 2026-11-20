// Transform.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Transform_h
#define DDImage_Transform_h

#include "DDImage/Iop.h"
#include "DDImage/RowCacheTile.h"
#include "DDImage/Filter.h"
#include "DDImage/Shutter.h"
#include "Matrix4.h"

#define FN_ALLOW_MOTIONBLURSAMPLE_INTEREST
#ifdef FN_ALLOW_MOTIONBLURSAMPLE_INTEREST
  #include "DDImage/Interest.h"
#endif

#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

namespace DD
{
  namespace Image
  {
    class TransformI;

    /*! \class DD::Image::Transform

       Transforms the input image by a 4x4 transformation matrix.

     */
    class DDImage_API Transform : public Iop
    {
      friend class TransformI;
      TransformI* tp;

    protected:
      Matrix4 matrix_;

      /*!
         _validate() calls this when the output image is invisible or otherwise
         cannot be calculated. It deletes the tp object to indicate this.
       */
      bool degenerate();
      Matrix4 concat_matrix_;
      Iop* concat_input_;
      bool clamp_;
      bool black_outside_;
      bool concat_clamp_;
      bool concat_black_outside_;
      bool can_concat;

      float _motionblur;      ///< amount of motion blur to do, '1' being heuristically'enough'
      ShutterControls _shutter; ///< shutter controls for motion blurring

      Filter filter_;

      /*! For implementing motion blur, return the matrix at various times.
         Default version changes the knobs and calls validate(), but a subclass
         can replace this if there is a faster way.
       */
      virtual void matrixAt(const OutputContext&, Matrix4& dest);
      void concatMatrixAt(const OutputContext&, Matrix4& dest);

      /*!
       * Since _validate is fetching out values of knobs at nearby times
       * to do motionblurring, we have to hash those in here, otherwise
       * changing the velocity of the curve without changing its value
       * will not result in the hash changing.
       */
      void append(Hash &hash) override;

      /// update the concat_matrix_ / concat_input_ / concat_clamp_ / concat_black_outside_
      void updateConcat(bool setInfo);

    public:

      /*!
         _validate() will combine all adjacent Transform operators into
         a single one. When you call engine() it will skip the earlier
         operators, read from the input to the first one (concat_input()),
         and process by a concatenated transformation matrix (concat_matrix()).
       */
      void _validate(bool) override;

      /*!
         After _validate() is called a subclass can call this to force the
         Transform to clip the output to a rectangle and put black outside it.
         The rectangle is exactly 1 pixel inside the passed Box (ie the box
         includes the black edge). The info() is set to this box.
         This is used by Reformat.
       */
      void setclip(const Box&);
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;
      void engine(int y, int x, int r, ChannelMask allchan, Row& row) override;

      /*! The constructor sets the transformation to the identity matrix. */
      Transform(Node * node = nullptr);
      ~Transform() override;

      Transform* getTransform() override
      {
        return this;
      }

      void setInputsValid(bool) override;

      HandlesMode doAnyHandles(ViewerContext* ctx) override;

      /*!
         draw_handle() will draw the input texture map. We only want this
         to happen for the first operator, so that if the user has several
         open at once the texture is only drawn once.
       */
      void build_handles(ViewerContext* ctx) override;

      /*! Draws a rectangle the size of the input format, filled with
         the input as a texture map if possible. */
      void draw_handle(ViewerContext* ctx) override;

      /*! Evaluate a Mitchell filter at position x. This can be used by Nuke
         to view the filter shapes. */
      static double mitchell(double b, double c, double x);

      /*!
         This virtual function should return the matrix used. A subclass
         can call reset() and then scale, translate, etc to get the matrix
         wanted and then return Transform::matrix(). You can also skip
         doing anything and return Transform::matrix() if valid() is
         true, since you know the matrix has been calculated then.
       */
      Matrix4* matrix();
      virtual void matrix(const Matrix4& m)
      {
        matrix_ = m;
      }

      /*!
         Transform an input \a uv position into an output \a xyw.

         Returns false for w <= 0. Such a point is not visible and has no
         output x/y position. However this version makes a "guess" as to
         what direction visible pixels that are near \a uv are and returns
         very large values for x,y in that direction. This is useful for
         calculating resulting bounding boxes.
       */
      bool transform(double x, double y, double& X, double& Y, double& Z);

      /*!
         Same as transform() but it uses the concatenated matrix, thus
         transforming coordinates from concat_input(). This only works after
         validate() has been called.
       */
      bool concat_transform(double x, double y, double& X, double& Y, double& Z);

      /*! \fn void Transform::reset()
         Set the transformation to the identity. */
      void reset()
      {
        matrix_.makeIdentity();
      }

      /*! \fn void Transform::multiply(const Matrix4 &)
         Transform further by an arbitrary transformation matrix.  Like OpenGL
         all transformation operations are done by post-multiply, so this
         transformation is applied in the \e object's space.  For instance
         a rotate(1) followed by a translate(1,0) will move the center of
         the object along a diagonal line.
       */
      void multiply(const Matrix4& m)
      {
        matrix_ *= m;
      }
      void multiply(const Transform& t)
      {
        matrix_ *= t.matrix_;
      }
      void premultiply(const Matrix4& m)
      {
        matrix_ = m * matrix_;
      }
      void premultiply(const Transform& t)
      {
        matrix_ = t.matrix_ * matrix_;
      }
      void scale(float s)
      {
        matrix_.scale(s);
      }
      void scale(float x, float y, float z = 1.0)
      {
        matrix_.scale(x, y, z);
      }
      void rotateX(float radians)
      {
        matrix_.rotateX(radians);
      }
      void rotateY(float radians)
      {
        matrix_.rotateY(radians);
      }
      void rotateZ(float radians)
      {
        matrix_.rotateZ(radians);
      }
      void rotate(float radians)
      {
        rotateZ(radians);
      }
      void rotate(float a, float x, float y, float z)
      {
        matrix_.rotate(a, x, y, z);
      }
      void skew(float a)
      {
        matrix_.skew(a);
      }
      void translate(float x, float y)
      {
        matrix_.translate(x, y);
      }
      void translate(float x, float y, float z)
      {
        matrix_.translate(x, y, z);
      }

      const Filter* filter() const
      {
        return &filter_;
      }
      int filter_type() const
      {
        return filter_.type();
      }
      void filter_type(int t)
      {
        filter_.type(t);
      }

      bool clamp() const
      {
        return clamp_;
      }

      /*! \fn void Transform::clamp(bool v)

         If true then any negative intermediate results are set to zero.
         This can improve the results from high-contrast images and filters
         with negative lobes if you know nothing in the input is negative,
         by deleting a lot of the ringing in black areas.

         The default value is false.
       */
      void clamp(bool v)
      {
        clamp_ = v;
      }

      bool black_outside() const
      {
        return black_outside_;
      }

      /*! \fn void Transform::black_outside(bool v)

         If true the transform will act as though any data outside
         the incoming bounding box is black. It will also produce an alpha
         channel if there is none in the input.

         If this is false, Transform will replicate the pixels outside the
         bounding box instead. This will prevent any antialiasing on the
         edge. This is used by the Reformat operator, and may be useful for
         tile or camera shake type operators.

         The default value is true.
       */
      void black_outside(bool v)
      {
        black_outside_ = v;
      }

      const char* Class() const override;
      const char* node_help() const override;

      /*! All transform operators default to a purpleish color. */
      unsigned node_color() const override;

      /*! \fn const Matrix4& Transform::concat_matrix() const
         After _validate() is done, this returns the transformation between
         the concat_input() and the output of this operator.
       */
      const Matrix4& concat_matrix() const
      {
        return concat_matrix_;
      }

      /*! \fn Iop* Transform::concat_input() const
         After _validate() is done this returns the operator that input
         will actually be read from.
       */
      Iop* concat_input() const
      {
        return concat_input_;
      }

      /*!
         Subclasses should call this to insert a filter-selection control
         into their control panels. This adds the "filter type" and "clamp"
         controls.
       */
      void filter_knobs(Knob_Callback);

      /*! Adds the motionblur amount and shutter knobs */
      void motionblur_knobs(Knob_Callback cb);

      /*! Does filter_knobs, adds the "black outside" knob, and does motionblur_knobs() */
      void knobs(Knob_Callback) override;

      float getMotionBlur() const { return _motionblur; }
      bool isMotionBlurring() const { return _motionblur > 0 && _shutter.getDuration(); }

      /*! Do motion blurring for the pixel, returns the guestimated path length of the sample */
      float motionBlurSample(int x, int y, RowCacheTile& i,  ChannelMask iChannels, Pixel& out);

      /*! Texture filtering function. Disables motion blur!
          The channels requested by \a get are sampled and put into \a out.
      */
      void sample(const Vector2& center, const Vector2& dU, const Vector2& dV,
                  Filter* filter, const ChannelSet& get, Pixel& out) override;
      /*! Same except samples out.channels. */
      void sample(const Vector2& center, const Vector2& dU, const Vector2& dV,
                  Filter* filter, Pixel& out) override;

      /*! Disables motion blur!
          The channels requested by \a get are sampled and put into \a out.
      */
      void sample(float cx, float cy, float w, float h,
                  Filter* filter, const ChannelSet& get, Pixel& out) override;
      /*! Same except samples out.channels. */
      void sample(float cx, float cy, float w, float h,
                  Filter* filter, Pixel& out) override;

      /*! Uses the input's texture map, but changes the OpenGL texture
         matrix to show it correctly translated.
       */
      bool set_texturemap(ViewerContext*, bool) override;
      Op* realParent();
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
