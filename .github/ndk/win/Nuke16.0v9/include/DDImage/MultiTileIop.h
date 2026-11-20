#ifndef MultiTileIop_h
#define MultiTileIop_h

#include "DDImage/Iop.h"

namespace DD {
  namespace Image {

    /**
     * Base class for an Iop which efficiently handles inputs of multiple
     * different tile types.
     *
     * In addition to implementing the inputToRead() function detailed below;
     *
     * Subclass definitions should:
     *
     *  1.  Include the macro
     *        mFnDDImageMultiTileIop_Template_DeclareFunction_DoEngine
     *      or declare the function
     *        template<class TileType> inline void doEngine(params)
     *      where the params match the engine() function.
     *
     *  2.  Include the macro
     *        mFnDDImageMultiTileIop_DeclareFunctions_engine
     *      to declare the engine() replacement functions (see below).
     *
     * Subclass implementations should:
     *
     *  1.  Include the macro
     *        mFnDDImageMultiTileIop_DefineFunctions_engine
     *      from MultiTileIopEngineDefinitions.h to define the engine()
     *      replacement functions AFTER the implementation of the doEngine()
     *      template function (to allow inlining).
     *
     */
    class DDImage_API MultiTileIop : public Iop
    {
    protected:
      /*!
         The constructor is protected so only subclasses can create the
         base class. The info() is filled in with default values for
         an empty image with no channels and format Format::None.
       */
      MultiTileIop(Node * node) : Iop(node) {}

    public:
      /** engine() replacement functions.
       *
       * It is recommended that these are implemented in subclasses using the
       * mFnDDImageMultiTileIop_DefineFunctions_engine() macro defined in
       * MultiTileIopEngineDefinitions.h, allowing engine() to be implemented as
       * a template function for all the available input tile types.
       */
      virtual void engine_BufferIopTile(int y, int x, int r, ChannelMask m, Row& row) = 0;
      virtual void engine_DirectGeneralTile(int y, int x, int r, ChannelMask m, Row& row) = 0;
      virtual void engine_RawGeneralTile(int y, int x, int r, ChannelMask m, Row& row) = 0;
      virtual void engine_RowCacheTile(int y, int x, int r, ChannelMask m, Row& row) = 0;

      /** Callback function which returns the input whose tile will be read in
       * the engine() call.
       *
       * Note that this function will be called directly after the inputs are
       * validated so it can be assumed that all inputs are available.
       */
      virtual Iop* inputToRead() const = 0;

      /** force_validate() may be called in the same way as
       * Op::force_validate().
       *
       * This function has the additional effect of setting the tile type of
       * the input so that calls are made to the correct _engine*() function.
       */
      void force_validate(bool for_real = true) override;

      /** Implementation of engine().  DO NOT OVERRIDE.
       *
       * This calls through to the appropriate _engine*() function dependent on
       * the tile type of the validated input.
       */
      void engine(int y, int x, int r, ChannelMask m, Row& row) override;

    private:
      // The engine function to call
      void (MultiTileIop::*_engineCallback)(int y, int x, int r, ChannelMask m, Row& row);
    };

  }
}

//! Macro to declare a template doEngine() function for each tile type.
//!
//! The function should be parameterised on TileType, and use
//! typename TileType::RowPtr instead of const float* for pointers to image
//! data.  In the case where RowCacheTile is used then this will just be a
//! typedef to const float* and it should produce equivalent code.
//!
//! NOTE: This declares a template function which is used by
//! mFnDDImageMultiTileIop_DefineFunctions_engine() in
//! MultiTileIopEngineDefinitions.h.
//! The parameters must match engine, i.e.
//! int y, int x, int r, ChannelMask m, Row& row.
//!
//! Alternatively, the template function doEngine() may be declared manually.
//!
//! For performance reasons, it is recommended that the template functions are
//! marked for inlining.

#define mFnDDImageMultiTileIop_Template_DeclareFunction_DoEngine(...) \
  template<class TileType> inline void doEngine(__VA_ARGS__)

//------------------------------------------------------------------------------
// Using the macro below will allow DDImage to maintain source level
// compatibility if new tile type are added.
//------------------------------------------------------------------------------

//! Macro declaring the engine replacement functions for a subclass.

#define mFnDDImageMultiTileIop_DeclareFunctions_engine(...) \
  void engine_BufferIopTile(__VA_ARGS__) override;         \
  void engine_DirectGeneralTile(__VA_ARGS__) override;         \
  void engine_RawGeneralTile(__VA_ARGS__) override;          \
  void engine_RowCacheTile(__VA_ARGS__) override

#endif
