// MultiTileIopEngineDefinitions.h
// Copyright (c) 2017 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_MultiTileIopEngineDefinitions_h
#define DDImage_MultiTileIopEngineDefinitions_h

#include "DDImage/ChannelSet.h"
#include "DDImage/Row.h"

#include "DDImage/BufferIopTile.h"
#include "DDImage/DirectGeneralTile.h"
#include "DDImage/RawGeneralTile.h"
#include "DDImage/RowCacheTile.h"

//! Macro defining the engine replacement functions for a MultiTileIop subclass.
//!
//! This calls the doEngine() templated function with a different type of Tile
//! depending upon the type of images available on the Iop returned by
//! MultiTileIop::inputToRead().  If the Iop is planar it will use
//! RawGeneralTile, or DirectGeneralTile if it is packed. BufferIopTiles are
//! used for special Iops that cache their data internally, to avoid duplicating
//! data in the row cache.
//!
//! This should be placed AFTER the doEngine() template function has been
//! defined to allow doEngine() functions to be inlined.
//!
//! This macro is used to avoid identical, boilerplate code in classes; it can't
//! be done directly with virtuals as C++ doesn't support templated virtuals.
//!
//! See macro mFnDDImageMultiTileIop_Template_DeclareFunction_DoEngine() for the
//! signature required for the doEngine() template function.

#define mFnDDImageMultiTileIop_DefineFunctions_engine(CLASS)                            \
  void CLASS::engine_BufferIopTile(int y, int x, int r, ChannelMask m, Row& row) {  \
    doEngine<BufferIopTile>(y, x, r, m, row);                                       \
  }                                                                                     \
  void CLASS::engine_DirectGeneralTile(int y, int x, int r, ChannelMask m, Row& row) {  \
    doEngine<DirectGeneralTile>(y, x, r, m, row);                                       \
  }                                                                                     \
  void CLASS::engine_RawGeneralTile(int y, int x, int r, ChannelMask m, Row& row) {     \
    doEngine<RawGeneralTile>(y, x, r, m, row);                                          \
  }                                                                                     \
  void CLASS::engine_RowCacheTile(int y, int x, int r, ChannelMask m, Row& row) {       \
    doEngine<RowCacheTile>(y, x, r, m, row);                                            \
  }

#endif
