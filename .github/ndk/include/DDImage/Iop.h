// Iop.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Iop_h
#define DDImage_Iop_h

#include <assert.h>

#include "Op.h"
#include "Format.h"
#include "ChannelSet.h"
#include "GPUContext.h"
#include "IopInfo.h"

#ifdef __INTEL_COMPILER
#pragma warning(disable:444)  
#endif

namespace DD
{
  namespace Image
  {

    class Row;
    class Vector2;
    class Vector4;
    class Filter;
    class Pixel;
    class Scene;
    class VertexContext;
    class GeoInfo;
    class Cache;
    class Transform;
    class ImagePlane;
    class VArray;
    class PlanarI;
    class IopImpl;
    class ExtendedOpProperties;
    struct GeoInfoRenderState;

    //! IopInfo holder
    class DDImage_API IopInfoOwner : public Info2DProvider
    {
    public:
      typedef IopInfo Info;

    protected:

      /*! \var Info IopInfoOwner::info_
         This is the structure returned by info().  It is protected so that
         _validate() can fill it in, but outside code cannot modify it.
       */
      Info info_;
      
    public:

      /*! \fn const Info& IopInfoOwner::info() const
         Returns a reference to the data filled in by _validate().
         See the description of class Info for what the available data is.
       */
      const Info& info() const
      {
        /// info is only useful if validate(true) has been called
        /// otherwise it might change behind your back.
        //    mFnAssert(valid());
        return info_;
      }

      /*! \fn int IopInfoOwner::x() const
         Same as info().x();
       */
      int x() const { return info().x(); }

      /*! \fn int IopInfoOwner::y() const
         Same as info().y();
       */
      int y() const { return info().y(); }

      /*! \fn int IopInfoOwner::r() const
         Same as info().r();
       */
      int r() const { return info().r(); }

      /*! \fn int IopInfoOwner::t() const
         Same as info().t();
       */
      int t() const { return info().t(); }

      /*! \fn int IopInfoOwner::w() const
         Same as info().w(), same as r()-l().
       */
      int w() const { return info().w(); }

      /*! \fn int IopInfoOwner::h() const
         Same as info().h(), same as t()-y().
       */
      int h() const { return info().h(); }
      bool is_constant() const { return info().is_constant(); }

      /*! \fn int IopInfoOwner::clampx(int x) const
         Same as info().clampx(x), limits x to the range x() to r()-1.
       */
      int clampx(int x) const { return info().clampx(x); }

      /*! \fn int IopInfoOwner::clampy(int y) const
         Same as info().clampy(y), limits y to the range y() to t()-1.
       */
      int clampy(int y) const { return info().clampy(y); }

      /*! \fn const Format& IopInfoOwner::format() const
         Same as info().format().  */
      const Format& format() const { return info().format(); }

      /*! \fn const Format& IopInfoOwner::full_size_format() const
         Same as info().format();
       */
      const Format& full_size_format() const { return info().full_size_format(); }

      /*! \fn int IopInfoOwner::ydirection() const;
         Same as info().ydirection();
       */
      int ydirection() const { return info().ydirection(); }

      /*! \fn bool IopInfoOwner::black_outside() const;
         Same as info().black_outside();
       */
      bool black_outside() const { return info().black_outside(); }

      /*! \fn ChannelMask IopInfoOwner::channels() const;
         Same as info().channels();
       */
      ChannelMask channels() const { return info().channels(); }

      const GeneralInfo& generalInfo() const override
      {
        return info_;
      }
      const Info2D& info2D() const override
      {
        return info_;
      }

      /*! \fn int IopInfoOwner::first_frame() const
         Same as info().first_frame().
       */
      int first_frame() const { return info().first_frame(); }

      /*! \fn int IopInfoOwner::last_frame() const
         Same as info().last_frame().
       */
      int last_frame() const { return info().last_frame(); }

      void format(const Format& f) { info_.format(f); }
      void full_size_format(const Format& f) { info_.full_size_format(f); }
    };
    
    enum class SampleFunction;

    /*! \class DD::Image::Iop
       \brief Iop is the base class for all the image operators.

       The base class is not usable.  Several pure virtual functions must
       be defined by a subclass to make an operator that works.
     */
    class DDImage_API Iop : public Op, public IopInfoOwner
    {
      friend class Cache;
      friend class Interest;
      friend class GeneralTile;
      template<SampleFunction, typename, typename...> friend struct MultiTileCallDelegate;

      Cache* cache_;                                //!< Points to cache if it is turned on

      Box         requested_;                       //!< Union of all calls to request() clamped to info_
      Box         requested_unclamped_;             //!< Union of all calls to request() without clamping (used by NukeEngine to reduce Row repeating)
      ChannelSet  requested_channels_;              //!< Union of all calls to request()
      ChannelSet  requested_channels_unclamped_;    //!< Union of all calls to request() without clamping (used by NukeEngine to reduce Row repeating)
      int         requested_count_;                 //!< Sum of all counts passed to request()
      ChannelSet  multi_request_channels;           //!< Union of all intersections of calls to request()

      /* open the operator and intersect the channels with the set provided
         by the operator. This also checks if there was a request() call for
         at least these channels and produces an error if false.
         Returns false if there was an error, it aborted, or if there are
         no channels in the result.
      */
      bool maskChannels(ChannelSet& channels, const char* fname);

      //! Update the requested_unclamped_ box with the newly requested area. It
      //! is assumed that x,y,r,t represent an unclamped request.
      void updateUnclampedRequest(int x, int y, int r, int t);
      void updateUnclampedRequestedChannels(ChannelSet channels);

    public:
      //! Fetch an image plane.  Ops that want an image plane should pass an empty
      //! image plane with the appropriate size and channels, and then this will
      //! be filled.  This is a relatively low-level function.  See PlanarI and
      //! PlanarIop for a larger structure built around this.
      //!
      //! Note that the area must be a subset of the region of definition as
      //! specified in info().
      //!
      //! This function may perform caching, and calls doFetchPlane() to fetch
      //! the actual.
      void fetchPlane(ImagePlane& imagePlane);

      /**
       * whether this Iop prefers planar or row-based access.
       *
       * if it prefers planar access return a pointer to the interface to
       * do planar size queries
       */
      virtual PlanarI* getPlanarI() { return nullptr; }


      /*! Acts as though valid() is off and does validate(). Thus you can be
         certain that _validate() is called. 

         This is re-implemnted from Op, and calls the base class version, as well as doing
         some extra Iop-specific work to propagate Info from input to output when the Op is in error.
       */
      void force_validate(bool for_real = true) override;

      IopImpl* pImpl()  { return _pImpl.get(); }

      /*!
         Calculate the image for this line and write the result to the
         \a row.

         \a y, \a x, \a r, and \a channels have already been clipped to the
         requested() box and requested_channels() (which also means they
         are clipped to the box and channels set by _validate()).

         This is not called if the intersection of channels() and out_channels()
         is Mask_None.

         You must fill all the requested channels. If your operator does
         not modify a channel then you should copy it unchanged from
         input0().

         The default version prints an error. Unless your operator sets
         out_channels to none always (this is true in some cases) you have
         to implement this.
       */
      virtual void engine(int y, int x, int r, ChannelMask, Row &) = 0;

      //! Cast to an Iop. This is much cheaper and safer than using dynamic_cast.
      Iop* iop() override;
      const Iop* iop() const override;

    protected:

      /*!
         The constructor is protected so only subclasses can create the
         base class. The info() is filled in with default values for
         an empty image with no channels and format Format::None.
       */
      Iop(Node * node);

      void _invalidate() override;

      /*! The subclass-dependent portion of validate(). This is called by
         validate() after checking to see if it is needed.

         This must set the contents of info_.  Usually this is done by
         calling validate() on some input and then assigning that to
         info_, and then possibly modifying that information. You should
         use the copy_info(), merge_info(), and init_merge_info() methods
         if possible to set the info, or maybe Iop::Info::clear(), and
         then modify the results as needed:
         \code
         copy_info();
         \\ sample modifications of data, see plugin source code for more:
         info_.turn_on(Mask_RGB);
         info_.merge(*(info_.format));
         \endcode

         An operator can also detect that it will not do anything to some
         channels and call set_out_channels(). If you set this to Mask_None
         then this will prevent _request(), _open() and engine() from being
         called. Besides being faster this often makes coding a plugin
         easier because you don't have to worry about the degenerate cases
         except in _validate():
         \code
         if (gain == 1) {
          set_out_channels(Mask_None);
         } else {
          set_out_channels(Mask_All);
         }
         \endcode

         If something is wrong you can call error(). This will throw an
         exception and _validate() will not be called again until invalidate()
         is done.

         \a for_real is true if Nuke is going to ask for image data from the
         operator. Otherwise it is only doing this to get the frame range,
         format or other information from the info_. If it is false you should
         try very hard to avoid anything expensive such as opening files and
         you must avoid producing errors that don't happen on every frame since
         that would prevent dissolves operators from fading out inputs during
         the time they produce errors.

         For many operators the action is the same whether \a for_real is
         true or false.  If you don't use any inputs and cannot avoid
         opening a file and want to avoid opening it twice, recursively
         call yourself like this. This is what Read operators do if they
         don't have the format and channels set yet:
         \code if (!for_real) {validate(true); return;} \endcode

         The default version merges the info from all the inputs and then
         turns on the channels in out_channels() if it is not Mask_All.
       */
      void _validate(bool for_real) override;
      
      /*!
         The subclass-dependent portion of request(). This is called by
         request() if the new request is larger than what has already been
         requested this pass. _validate(true) will already have been called.

         This should call request() on any inputs that are going to be used.
         Usually it will modify the arguments slightly:
         \code input0().request(x-1, y-1, r+1, t+1, channels, count); \endcode

         If you are going to randomly access the input, you should request
         it's entire bounding box:
         \code input0().request(channels, count); \endcode

         \a count indicates approximately how many times engine() will
         be called for each value of y. You should pass it unchanged or multiply
         it by a value greater or equal to 1 if your engine() asks for more
         than one input line. This is used to set up caching by knowing how
         many times a line will be needed. \a count may be zero to indicate
         that a previous request is being expanded with more area or more
         channels.

         The default implementation requests the \e same area from each
         input. You will need to change this if your operator does random
         access, possibly to call the request() method that takes no area
         and thus accesses the format. It uses in_channels() to figure out
         what channels to ask for.
       */
      virtual void _request(int x, int y, int r, int t, ChannelMask, int count);

      /*! This used to do something but does not now. It is still recommended
          that any subclasses call this in their _open().
       */
      void _open() override;

      /*! At one time this destroyed the cache. That's no good anymore as
          it throws away data when the automatic file close is done by
          nuke. Instead it only deletes it if valid is false.
       */
      void _close() override;

      //! Fetch an image plane.  This is the function that Iops that should override. 
      //! This can be called for any plane with any arbitrary bounds, channel.  Either
      //! implement all of these (ImagePlane::copyIntersection helps here), or call the default
      //! implementation for a fallback.
      //!
      //! By default this is not cached, so that multiple identical calls to fetchPlane will 
      //! result in multiple identical calls to doFetchPlane.  Use PlanarIop if you want a
      //! base class to manage caching of these for you.
      virtual void doFetchPlane(ImagePlane& imagePlane);

      bool inUse() const override;

      /*! Same as copy_info(n, Mask_All), indicating you are interested in all
         the channels the operator provides.
       */
      void copy_info(int input);

      /*!
         Most _validate() functions can call this to get the information from
         one of their inputs. They can then modify the info_ (such as changing
         the bounding box or adding or removing channels) or use merge_info()
         to get information from other inputs.

         \a channels is what channels of this input you use. The bounding box
         and ydirection are only changed if these intersect the channels available
         from the input. Also if it is none it will avoid opening files or testing
         for errors on that input.

         Equivalent code that you can put into _validate() is:
         \code
         input(n)->validate(channels && for_real);
         info_ = input0().info();
         channels = intersect(channels, info_.channels());
         info_.channels(channels);
         if (!channels) {
          info_.set(0,0,1,1);
          info_.ydirection_(0);
         }
         \endcode
       */
      void copy_info(int input, ChannelMask);

      /*! Same as copy_info(0, Mask_All). */
      void copy_info();

      /*!
         If you call this, then the first merge_info() you call will
         instead do a copy_info(). This is useful for an operator that has to
         iterate through all its inputs and is not sure which ones it is
         using.  Warning: the value in info_ is bad after this call; you must
         call copy_info(), merge_info(), or assign some new value to info_
         before your _validate() function returns.
       */
      void clear_info();

      /*! Same as mergeInfo(input, Mask_All). It assumes all channels are acceptable. */
      void merge_info(int input = 0);

      /*!
         Merge the info from input \a n into this info. This is for use by
         _validate() in operators that take several inputs.

         This will call validate() on the input, passing true if for_real
         is on and \a channels in non-zero.

         If the output of that input intersects channels it will merge
         those channels and the bounding box and ydirection in.

         The formats are unchanged.

         If you have called clear_info() this instead does copy_info().
         This is for conveniece for writing a loop that merges a large
         number of inputs.
       */
      void merge_info(int input, ChannelMask);

      /*!
         Union the current and this input's frame ranges (ignoring either one
         if they are "empty" which is indicated by the last frame being less
         or equal to the first). This is the normal action by Nuke operators
         when merging sequences of different frame ranges.
       */
      void mergeFrameRange(int input);

      ChannelSet out_channels_; //!< protected so validate() can directly modify it
      int  raw_channels_from_;

      /*! \fn void Iop::set_out_channels(ChannelMask mask)
         Same as set_out_channels() = mask.
       */
      void set_out_channels(ChannelMask m) { out_channels_ = m; }
      void set_out_channels(ChannelSetInit m) { out_channels_ = m; }
      void set_out_channels(Channel z) { out_channels_ = z; }
      void raw_channels_from(int n) { raw_channels_from_ = n; }
      void set_out_channels(ChannelMask m, int n) { out_channels_ = m;
                                                    raw_channels_from_ = n; }
      Iop* asIop(Op* op) const {
          mFnAssert(!op || dynamic_cast<Iop*>(op) );
          return static_cast<Iop*>(op);
      }

      /**
        Ops which shouldn't break transform concatenation could call this to propagate input0's 
        transform to the output Ops. See the documentation for getTransform() for more details.
       */
      Transform* getDefaultTransform() const
      {
        Iop* const pInput = input(0);
        if( pInput ) {
          return pInput->getTransform(); // propagate input0's transform to the output.
        }

        return nullptr;
      }

      /**
       * helper function for Iop::sample
       */
      template<class TileType> void doSample(float cx, float cy, float w, float h, Filter*, const ChannelSet& get, Pixel& out);
      template<class TileType> void doSample(float cx, float cy, float w, float h, Filter*, Pixel& out);

      /**
       * helper function for Iop::do_sample_shadowmap
       */
      template<class TileType> float do_sample_shadowmap(Iop* map, bool map_is_one_over_z, float cx, float cy, float W, float H, Filter* filter, float Pz);

    public:

      /*! The destructor destroys the cache if there is one. */
      ~Iop() override;

      /*! If valid, returns the pass through Iop */
      Iop* getPassthroughIop() const;

      /*! Dedicated test for special 'Black' empty Iop subclass. Avoids the need
          to string test Class().
      */
      virtual bool isBlackIop() const { return false; }

      /*! By default Iops only allow other Iops to be hooked to the input.
         You can override this if you have an image operation that takes
         data other than images as input. Make sure you cast back after
         calling input0() or other things that assume the inputs are Iops.
       */
      bool test_input(int, Op*) const override;

      /*!
         Returns a Black operator with the format set to the Nuke user's
         choice for default format. This means that by default an Iop with a
         disconnected input gets a black image of the expected size.

         If the library is used by a program other than Nuke this will return null!
         Most operators assume inputs are not null, so you must connect them yourself.
       */
      Op* default_input(int) const override;

      /*!
         This static function returns the same op that default_input(int) returns.
         This is useful so that non-Iop subclasses can get the same Black iop,
         which is mostly useful for finding out the user's default format or
         frame range settings.
       */
      static Iop* default_input(const OutputContext&);

      /*! \fn Op* Iop::default_input(const Op* op);
         Returns default_input(op->outputContext()). Provided for back-compatibility.
       */
      static Op* default_input(const Op* op) { return default_input(op->outputContext()); }

      /*! \fn Iop* Iop::input(int n) const
         Returns Op::input(n) cast to an Iop. If you override test_input()
         and allow non-Iop inputs, you should be careful when using this
         function.
       */
      Iop* input(int n) const { return asIop(Op::input(n)); }
      Iop* input(int n, int m) const { return asIop(Op::input(n, m)); }

      /*! \fn Iop& Iop::input0() const
         Fast \e inline method of getting input(0). This casts on the
         assumption that the input is another Iop. Also notice that this
         returns a reference rather than a pointer, this is for back
         compatibility.
         Note: This function potentially derefences a null pointer. In
         C++ standard that is considered as an Undefined Behaviour. If
         the input(0) can be null pointer, do not use this function!
       */
      Iop& input0() const { return *(asIop(Op::input0())); }

      /*! \fn Iop& Iop::input1() const
         Fast \e inline method of getting input(1). This casts on the
         assumption that the input is another Iop. Also notice that this
         returns a reference rather than a pointer, this is for back
         compatibility.
         Note: This function potentially derefences a null pointer. In
         C++ standard that is considered as an Undefined Behaviour. If
         the input(1) can be null pointer, do not use this function!
       */
      Iop& input1() const { return *(asIop(Op::input1())); }

      /*! \fn DD::Image::Cache Iop::cache() const;

          Return a pointer to the cache object being used by the Iop, or
          null if there is not cache object. This object is created automatically
          by open() if cached() is true or if there are multiple requests
          for data, or by at() or Tile or Interest objects.

          Calling invalidate() does not destroy the Cache object, but does
          destroy all the stored pixels. This is the majority of the memory
          used by a Cache, but the object itself is approximately the
          size of one row of pixels and should be destroyed if you really
          are done using the Iop.

          The Cache object is destroyed and the pointer cleared when the
          Iop is destroyed, or when _close() is called.
       */
      Cache* cache() const { return cache_; }

      /*! \fn bool Iop::cached() const;
          Returns the last value sent to cached(bool).
       */
      bool cached() const { return Op::cached(); }

      /*! \fn void Iop::cached(bool);

          If this is true then a cache is created by _open() when the
          first pixel is requested. The cache is modified so lines are never
          thrown away (?).

          If you turn this off the old data will remain in the cache and may
          still be returned, but new data will not be added and lines in the
          cache may be thrown away when the memory is needed. To free it
          either call invalidate() or call close().
       */
      void cached(bool b) { Op::cached(b); }

      /*!
         Return the set of channels needed from input \a n to output the
         channels in \a mask. This is done by modifying \a mask in-place
         to the new value. This is only used by PixelIop and by the default
         versions of _request() currently, though it is recommended that
         an Iop implement it.

         To make it easier to write, you can assume that at least one of
         the channels in out_channels() is set in \a mask.

         The default version returns mask unchanged. <i>This is only correct for
         operators where there is not cross-talk between channels</i>. A typical
         case is where the red output depends on all three of red, green,
         and blue, and same for green and blue. In this case you would implement
         it like this:

         \code
         void MyIop::in_channels(int, ChannelSet& mask) const {
         mask += (Mask_RGB);
         }
         \endcode

         If you completely replace a channel, you don't need it from the
         input, so turn it off in the mask. For instance if you write your
         own rgb channels, do this:

         \code
         void MyIop::in_channels(int, ChannelSet& mask) const {
         mask -= (Mask_RGB);
         }
         \endcode
       */
      virtual void in_channels(int, ChannelSet&) const;

      /*! \fn ChannelMask Iop::out_channels() const

         This is the set of channels that the operator modifies, all other
         channels are assumed to be copied unchanged from the
         raw_channels_from() input. Nuke will skip calling engine() if the
         requested channels does not intersect this set. Note that it \e will
         call engine() with channels that are not in this set, as long as \e
         some channels are in it.

         Subclasses use set_out_channels() to change this. They can
         either do it in the constructor, or in _validate().

         By default this returns Mask_All.
       */
      ChannelMask out_channels() const { return out_channels_; }
      int raw_channels_from() const { return raw_channels_from_; }

      /*!
         Return true if it is okay to move a transformation that is before
         this operator to after it. This is true of color correctors or any
         other operator where pixels don't move and the calculation does
         not depend on the position of pixels. Nuke may use this to
         concatenate multiple TransformIop into a single one.

         This must work without _validate() being called! It probably
         should return a constant true or false.

         The base class returns false.
       */
      virtual bool pass_transform() const;


      /*! Call this to indicate the area that will be requested from this
          Iop.  This calls validate(), and then calls _request() which will
          usually recursively call request() on each input.

          All calls to request() are unioned together and intersected with
          the info() produced by validate().  All subsequent calls to get
          pixels (by get(), Tile and Interest constructors, or by calling
          at()) are clamped to this area. Missing channels return zero,
          positions outside the box return the pixel from the nearest edge
          of the box.

          Although usually you want the request box to cover everything you
          will ask for, an operator may limit the size to avoid causing huge
          amounts of data to be cached. For instance some transforms request
          extremely large areas but reduce them to smaller than a single
          pixel, limiting the sampled area to only the non-scaled part of the
          input image and replicating the edge pixels will not degrade the
          transformed image significantly and will use far less memory.

          This is used to set up the automatic caching. The region is used
          to determine how much data to store in the cache. The \a count
          argument is used to indicate that engine() will be called more
          than once for a given y value. A count of N and calling request() N
          times have identical behavior. A count of zero indicates that a
          previous request() call is being expanded in area (zero acts like
          1 if this is the first call). Caches are placed by analyizing the
          slowness() and resulting counts of all the Iops (the current
          algorithim is simple and just turns on the cache on all operators
          with a count greater than one).

          Calling Op::forget_request() will make the next request() reset
          the accumulated area.  You may also want to call cache_new_pass()
          to indicate that all caches can be reused even if the data in them
          is still good.
       */
      void request(int x, int y, int r, int t, ChannelMask channels, int count);
      void request(DD::Image::Box box, ChannelMask channels, int count);

      /*! Use this to request that you will be randomly-accessing the input
          image and don't really know the area needed yet.

          The current version will request a rectangle of the size of the
          format, but it will be moved to contain the bounding box if
          possible. This seems to match what is expected from random access
          in most cases. If you know you need a larger area you can calculate
          it yourself and call the other version of request().

       */
      void request(ChannelMask channels, int count);

      /*! do not call this, it is part of Op::copyState() */
      void _copyState(Iop* input);

      /*! \fn const Box& Iop::requestedBox() const
          The union of all the boxes passed to request(), intersected
          with the box from info(). This is the area that will be stored
          in a cache, also the engine() will never be called for data
          outside this box.
       */
      const Box& requestedBox() const;

      const Box& unclampedRequestedBox() const;
      /*! \fn ChannelMask Iop::requested_channels() const
          The union of all channels passed to request() intersected
          with the channels reported in info(). These are the channels
          that will be stored in a cache.
       */
      ChannelMask requested_channels() const;
      ChannelMask multi_requested_channels() const;
      ChannelMask unclamped_requested_channels() const;

      /*! \fn int Iop::requested_count() const
         The sum of all the count arguments passed to request().
       */
      int requested_count() const { return requested_count_; }

      /*! Fill the \a row with the output of the Iop.
          <i>At least</i> the range \a x through \a r of each of \a channels
          is set to the output of the operator.

          <i>WARNING: the other channels, and data outside x..r, may
          change!!!</i> The implementation can change pointers to point at
          cached data, reuse the row to read input data, and the row may be
          reallocated to make it's space larger.  Do not assume any data,
          or any pointers to data, is preserved or valid after calling
          this. <i>THIS IS NOT A MERGE!!!</i> If you wish to merge data
          you must use a second row and then copy the data yourself.

          If possible, buffer pointers are changed to point at the cached
          data, so no copying is done. If this is not possible, writable()
          is called for each channel needed and the data is calculated and
          written to these locations (note that writable() will reuse any
          buffer already assigned there).

          The region is clamped to the requested() box before being passed
          to the engine, and the pixels at the edges of this area are
          replicated to fill the rest of the region.

          If aborted() is true this clears the row to zero and returns
          quickly.
       */
      virtual void get(int y, int x, int r, ChannelMask, Row &);

      /*!
         Returns several channels of a pixel in the image. The channels to
         compute are specified in the Pixel object. Unrequested channels
         (including Chan_Black) are \e unchanged.

         The x and y are clamped to the requested() area, and any
         channels not in requested_channels() will return zero.

         It can be faster to create and reuse an Interest object
         for this, as that avoids the overhead of locking/unlocking the
         cache for each call.

         It may be faster to use sample() as some operators, such as
         Transform, implement their own versions that back-translate the
         coordinates and call their input directly.
       */
      void at(int x, int y, Pixel& out);
      //! Same except retrieves only channels in \a get, ignoring \a out channels.
      void at(int x, int y, const ChannelSet& get, Pixel& out);

      /*!
         Return a single channel of a pixel in the image.

         The x and y are clamped to the requested() area, and any
         channels not in requested_channels() will return zero.

         This is the slowest method of getting data from the image,
         but it is easy to call and does not require any range checks.
         For faster methods use get() to read an entire row, or use
         an Interest() for random access.
       */
      float at(int x, int y, Channel);

      /*! Return the average of any cached pixels found in the specified
          rectangle. This is used by the viewer color-sampling code. The
          passed array is indexed by channel number and filled in with the
          values from the image. Unrequested channels are not changed.

          If the operator has been executed recently this will return the
          average of any pixels that were produced by the last execution.
          If it has not been executed recently then older pixel values may
          still be located in memory and the average of these are returned.
          If it was never executed or the cache memory has been freed or the
          requested box does not intersect the cache memory then the output
          value is set to zero and the corresponding bit in the return value
          is turned off.

          The return value is \a channels with any channels that had no
          pixels found turned off.
       */
      ChannelSet peek(int x, int y, int r, int t, Pixel& out);

      /*! Interface type for peek API to process every pixel found in the specified
        rectangle by peek call.
          Input:
          - pixel holding valid channel values
      */
      class PixelCallbackI
      {
      public:
        virtual ~PixelCallbackI() {}
        virtual void process(const Pixel&) = 0;
      };

      /*! For every pixel found in the specified rectangle call the given interface's
        process function. Return true of at least one pixel was given to the callback.

        This is used by the viewer color-sampling code for more-expensive image pixels
        analysis such as
        - min color
        - max color
        - median
        - mode
      */
      bool peek(int x, int y, int w, int h, ChannelMask pixelChannels, PixelCallbackI & callback);

      /*! If this image operation involves a transformation, return a pointer to the
          object that represents it.

        This is used by transform nodes to enable transform concatenation so that e.g.
        scaling down and then up an image doesn't result in quality loss to aliasing
        artefacts and repeated application of filtered sampling.

        If an Op doesn't generate or modify a transform, and it's safe to propagate the Op's
        input transform to the Op's output, then the Op should override getTransform() and 
        pass the appropriate input transform.

        For Ops with multiple inputs, this will depend on the use case. For example, if one 
        input represents the image and another input represents some additional data (e.g. extra 
        metadata) then you would pass the transform from the image input. 

        For Ops with single inputs, you could just call the getDefaultTransform() function.
      */

      virtual Transform* getTransform() { return nullptr; }

      // Shade the 3D OpenGL preview of a 3D object:

      /*! Texture resolution information.
          Note that a texture can be sized differently in X & Y to maximize the usage of the
          buffer from the Iop source, so do not assume the texels have a 1:1 aspect ratio. See
          the getTextureImageInfo() method for more info.
      */
      struct TextureImageInfo
      {
        int        width = 0;                 //!< How wide the texture buffer is in texels
        int        height = 0;                //!< How tall the texture buffer is in texels
        int        borderSize = 0;            //!< The border size in texels, around the texture
        fdk::Vec2i origin = fdk::Vec2i(0);    //!< Location of buffer left/bottom in Iop pixels
        fdk::Vec2f scale = fdk::Vec2f(1.0f);  //!< Scaling of texture w/h from Iop pixels to texels

        struct UVTransform
        {
          fdk::Vec2f translate = fdk::Vec2f(0.0f);  //!< Left/Bottom UV coordinate
          fdk::Vec2f scale = fdk::Vec2f(1.0f);      //!< Top/Right UV scaling factor
        } uvTransform;                        //!< UV coordinates of texture corners
      };

      /*! Calculate texture buffer resolution information used for producing Iop texturemaps
          for 3D objects or postagestamp images drawn on Nodes in the node graph.

          Given the maximum buffer W/H dimensions and formatting preferences (\a extractBbox
          and \a keepImageAspect,) derive the preferred target buffer W/H, pixel origin offsets,
          pixel scaling factors, pixel border width, and finally the texture UV transform values
          to map the (possibly) distorted texture buffer back to undistorted UVs coords.

          \a extractBbox true directs the method to use the Iop's info() bbox instead of the
          Format's bbox as the source pixel region to sample into the texture buffer. Since the
          info() bbox can be at any arbitrary pixel location and size the \a uvTransform.translate
          and \a uvTransform.scale values must be used to conform the texture buffer to the correct
          UV offset and scale otherwise the resulting texture will likely be distorted and in the
          wrong position. Postagestamp images will typically want \a extractBbox to be false so the
          Format is used and clipping any extended info() bbox to its edges.

          \a keepImageAspect true forces the texture buffer contents to conform to the Iop's
          Format aspect ratio (including its pixel aspect ratio.) Texturemaps will typically want
          \a keepImageAspect to be false to maximize filling the texture buffer in both X & Y.

          Defaults to 'texture mode' which is extractBbox=true & keepImageAspect=false.
      */
      TextureImageInfo getTextureImageInfo(int maxWidth, int maxHeight, bool extractBbox = true, bool keepImageAspect = false) const;

      /*! Same as getTextureImageInfo but assumes texture max W/H is an arbitrary 1000 pixels in
          size and the Iop's info() bbox is being extracted into a non-square buffer.
          The derived texture transform info will allow mapping of the possibly distorted texture
          buffer back to an undistorted UV rectangle at the correct UV origin.
      */
      TextureImageInfo::UVTransform getTextureTransform() const;

      /*!
         Set the OpenGL current 2D texture map to draw a reduced-resolution
         version of this image. Returns false if the texture map cannot be
         created or is not yet available, in which case you should draw
         your OpenGL preview some other way, such as with solid colors.

         The first time this is called this will probably return false.
         However this will signal Nuke to produce the down-rez image needed
         by the texture map using parallel threads. When this image is
         done, Nuke will redraw the viewer, and your drawing code can
         call this and it will get the texture map.

         If \a gl is false, this tests for the existence and triggers the
         generation of the texturemap, but does not do any OpenGL calls.
         This can be used to test if the texture will work or to trigger the
         generation of the texture before it is actually needed.

         To allow arbitrary bounding boxes and the output of Transform to be
         texturemapped, this function may mess with the OpenGL texture
         matrix.  Be sure to reset this and disable texture mapping by
         calling unset_texturemap(). To reproduce the treatment of edge
         pixels by Iop, this will set GL_CLAMP_TO_BORDER (GL_CLAMP on
         Windows, unfortunately).

         If DDImage is not linked with Nuke, this will return false always.
       */
      virtual bool set_texturemap(ViewerContext*, bool gl = true);

      /*!
         Holds texture information as created by get_texturemap.
       */
      struct TextureImage
      {
        int width{0};                                   //!< How wide the texture is
        int height{0};                                  //!< How tall the texture is
        int depth{0};                                   //!< How many components a texel has.
        int border{0};                                  //!< The border around the texture.
        float ux{0};                                    //!< Left UV coordinate.
        float uy{0};                                    //!< Bottom UV coordinate.
        float uw{0};                                    //!< Right UV coordinate.
        float uh{0};                                    //!< Top UV coordinate.
        std::shared_ptr<unsigned char> buffer{nullptr}; //!< Points to the texture data.
        int32_t state{0};                               //!< State of texture buffer (unfilled, being filled, error, complete, etc)
      };

      /*!
         Retrieves the texture map created by set_texturemap. Returns an
         invalid TextureImage object (TextureImage::buffer set to nullptr) if
         the texturemap is not yet available or if DDImage is not linked with
         Nuke.

         \see set_texturemap
       */
      virtual TextureImage get_texturemap(ViewerContext* ctx);

      /*!
         Undo anything set_texturemap did. The default one turns off
         texturemapping and resets the texture matrix to the identity.
       */
      virtual void unset_texturemap(ViewerContext*);

      /*!
         Shade the provided geometry object using this Iop as a material.
         Return false if it cannot correctly draw this object in OpenGL, in
         which case a solid-shaded version will be drawn.

         This function is usually only overridden by the Material subclass,
         but it is a method on Iop so that an Iop can be used as a shader
         to texture-map it's image on the object.

         The default version does:

         \code
         if (set_texturemap(ctx)) {
          geo.draw_solid(ctx);
          unset_texturemap(ctx);
          return true;
         } else {
          return false;
         }
         \endcode

         \see set_texturemap, vertex_shader, fragment_shader
       */
      virtual bool shade_GL(ViewerContext*, GeoInfo&);

      // Shade 3D objects when rendering:

      /*!
         Modify the vertex of any 3D geometry that this image is being
         applied as a shader to. This can change channels such as uv and xyz
         to achieve special effects like point displacement, uv projection,
         normal warping, etc. Or it can add extra channels to interpret by
         the fragment_shader.

         This function is usually only overridden by the Material subclass,
         but it is a method on Iop so that an Iop can be used as a shader to
         texture-map it's image on the object.  The default version does
         nothing.

         \see shade_GL, fragment_shader
       */
      virtual void vertex_shader(VertexContext&);

      /*!
         Change the value of the out Pixel as though the result of the surface
         shading.

         This function is usually only overridden by the Material subclass,
         but it is a method on Iop so that an Iop can be used as a shader
         to texture-map its image on the object. The default version
         takes the UV, if any, scales it to the format, and calls
         sample().  If any lights exist in the scene, a default diffuse
         shading is applied.

         \see shade_GL, vertex_shader
       */
      virtual void fragment_shader(const VertexContext&, Pixel& out);

      // Displacement functions:

      /*!
       Do the displacement...
      */
      virtual void displacement_shader(const VertexContext& vtx, VArray& out);

      /*!
       Return the maximum displacement bound
      */
      virtual float displacement_bound() const;

      // Blending a surface shader sample with the background

      /*!
         Perform the composition of the surface sample over the background pixel.
         This function is called for every sample from the back to the front sample.
         
         The default version implement the pre-multiplied 
         alpha compositing (Pixel::over).
      */
      virtual void blending_shader(const Pixel& in, Pixel& out);

      // Change the internal render state of a geoinfo
      virtual void render_state(GeoInfoRenderState& state);

      // Filtering functions:

      /*!
         Sample a rectangular area. Note that \a cx,cy is the \e center
         of the rectangle, not the corner.
         The channels requested by \a get are sampled and put into \a out.
       */
      virtual void sample(float cx, float cy, float w, float h, Filter*, const ChannelSet& get, Pixel& out);

      /*! Same except samples out.channels. */
      virtual void sample(float cx, float cy, float w, float h, Filter*, Pixel& out);

      /*! Same except the filter argument is a default Cubic filter. */
      void sample(float cx, float cy, float w, float h, Pixel& out);

      /*!
         Sample a parallelogram. It is centered on \a center and the length
         of the two axis are given by \a dU and \a dV. The channels requested
         by \a channels are put into \a out.

         Note that the Transform subclass has its own implementation that
         skips any motion blur. This is so that the 3D renderer can add motionblur
         and it does not get doubled.

         The default implementation turns the parallelogram into a rectangle
         of the same area and calls the rectangle sample() function.

         The channels requested by \a get are sampled and put into \a out.
       */
      virtual void sample(const Vector2& center, const Vector2& dU, const Vector2& dV, Filter*, const ChannelSet& get, Pixel& out);

      /*! Same except samples out.channels. */
      virtual void sample(const Vector2& center, const Vector2& dU, const Vector2& dV, Filter* filter, Pixel& out);

      /*! Same except the filter argument is a default Cubic filter. */
      void sample(const Vector2& center, const Vector2& dU, const Vector2& dV, Pixel& out);

      /* Sample a rectangular area of a shadowmap using the Percentage Closer Filtering method (Pixar 1987). */
      virtual float sample_shadowmap(Iop* map, bool map_is_one_over_z, float cx, float cy, float W, float H, Filter* filter, float Pz);

      /*! \fn Channel Iop::channel(const char* name);
         Same as DD::Image::getChannel(), this method is provided for
         back-compatibility.
       */
      static Channel channel(const char* name) { return DD::Image::getChannel(name); }

      /*! \fn const char* Iop::channel_name(Channel c);
         Same as DD::Image::getName(Channel), this method is provided for
         back-compatibility.
       */
      static const char* channel_name(Channel c) { return DD::Image::getName(c); }

      void build_handles(ViewerContext*) override;

      /*! Not implemented in the library but you can define it */
      friend std::ostream& operator<<(std::ostream&, const Iop&);

      virtual void inputsAttached() { }

      //! GPU support code
      
      /*!
         Interface to tell the op whether to transform texture coordinates if it happens to
         sample any textures in its fragment shader.
       
         The base class implementation is empty, it's up to derived classes that might
         want to do this to provide an implementation appropriate to them (which may just
         mean setting some flag).
       
         \sa gpuEngine_setupTextureTransform
       */
      virtual void gpuEngine_useTextureTransform(bool useTransform) {}
      
      /*!
         Interface for setting up the optional texture coordinate transform.
         This provides a matrix that's assumed to contain the concatenation of any transforms
         that exist in the op tree *after* this op.
       
         The base class implementation is empty, it's up to derived classes that might want
         to use texture coordinate transforms to provide an appropriate implementation.
       
         \sa gpuEngine_useTextureTransform
       */
      virtual void gpuEngine_setupTextureTransform(const Matrix4& postOpTransform) {}

      /*!
         Return the declaration portion of the OpenGL implementation.
         Returning NULL is the same as empty string.  When this string is
         used, the sequence "$$" is replaced by a prefix to de-clash the string
         against other instances of the same Op class (or instances of other
         classes that happen to use the same name for a uniform, etc).
       
         The sequence $name$ is replaced with the value of the knob of
         that name (currently only single floating point values are
         supported, or $name.x$ for a single field of a multi-value knob).
       */
      virtual const char* gpuEngine_decl() const;

      /*!
         Return the body portion of the OpenGL implementation.
         This is a modified version of GLSL.
         Returning NULL means the Op has no GPU implementation and has
         to be run on the main processor. When this string is
         used, the sequence "$$" is replaced by a prefix to de-clash the string
         against other instances of the same Op class (or instances of other
         classes that happen to use the same name for a uniform, etc).
       
         The sequence $name$ is replaced with the value of the knob of
         that name (currently only single floating point values are
         supported, or $name.x$ for a single field of a multi-value knob).
       
         There are also two custom variables: IN and OUT. IN is a sampler
         containing the data computed in the main processor. OUT is the
         cumulative result of all GPU operations and the effective output
         of the Op (gl_FragColor will be overwritten). An example is
         "OUT = OUT * $gain$;".
       */
      virtual const char* gpuEngine_body() const;
      
      /*!
         Returns a hash dependent on the shader source code, for the specified time.
         Note that this is not necessarily a hash _of_ the source code, the guarantee
         is merely that it varies deterministically as the source code varies.
         In practice, the shader source may depend on some of the Op's knobs, so the
         hash may just be of those knob values, evaluated at the specified time, and
         the function will not in general need to generate/know the full source.
       
         The default implementation just returns a default constructed Hash object,
         sub-classes should override this appropriately.
       
         Ideally, for performance reasons, the source shouldn't be time or even knob
         dependent and knob values should be handled with GL uniforms. However, there
         are situations where that's impractical and may even harm render performance
         merely to cater for knobs that in practice are rarely animated. Hence this
         function's support for time dependence.
       
         For an example override see the Grade plugin code in the NDK examples.
       */
      virtual Hash gpuEngine_shader_hash_at(double time);
      
      /*!
         This method returns the number of texture units that the iop
         requires for GPU processing.  The base class version is zero;
         override if your op uses more.  Note that not setting this value
         correctly can result in the GPU processing unexpectedly running
         out of texture units and causing rendering problems.
       */
      virtual int gpuEngine_getNumRequiredTexUnits() const;

      /*!
         Called before rendering this node when the gpuEngine_body() will
         be used. This can be used to bind textures, etc.
         If an error occurs in this method, it can throw an exception,
         which will be caught by the GPU setup code.
       */
      virtual void gpuEngine_GL_begin(GPUContext* context);

      /*!
         Called after rendering this node when the gpuEngine_body() will
         be used. This can be used to unbind textures, etc.
       */
      virtual void gpuEngine_GL_end(GPUContext* context);

      //! override this to return input0() if you want your Iop to be disregarded when
      //! directly connected to a Viewer.  The Viewer will follow the chain of Iops
      //! until it reaches one that returns NULL.  For implementation by the DiskCache
      //! node etc.
      virtual Iop* viewerPassThrough() const
      {
        return nullptr;
      }

      //! Return hints to control when and how this op will be evaluated by the top-down system.
      OpHints opHints() const override;

      //! Sets the pointer to object implementing extended properties
      virtual void setExtendedOpProperties(std::shared_ptr<ExtendedOpProperties> extendedOpProperties);

      //! Returns the pointer to object implementing extended properties
      virtual std::shared_ptr<ExtendedOpProperties> getExtendedOpProperties() const;

      static bool sPreserveCaches;

    private:

      std::unique_ptr<IopImpl> _pImpl;
    };
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of Iop.h
