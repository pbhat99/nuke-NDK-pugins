// (c) 2010 The Foundry Visionmongers Ltd.

#ifndef GEOSELECTKNOBI_H_
#define GEOSELECTKNOBI_H_

#include "DDImage/DDImage_API.h"
#include "DDImage/GeoInfo.h"
#include "DDImage/GeometryList.h"
#include "DDImage/GeoSelection.h"
#include "DDImage/Op.h"

#include <vector>

namespace DD {
  namespace Image {


    class DDImage_API GeoSelect_KnobI
    {
    protected:
      virtual ~GeoSelect_KnobI() {}

    public:

      /// Get the selection for geometry from this node. This is a view backed
      /// by the set of all selected items; the memory is managed by Nuke, so
      /// you shouldn't try to free it yourself.
      ///
      /// Note that adding something to the selection returned by this will NOT
      /// add it to Nuke's selection. However manipulating the data already
      /// contained in it WILL affect the current selection.
      virtual GeoSelection getSelection() = 0;

      /// Turn the GeoSelection structure into a flat vector of selected vertices or faces per GeoInfo,
      /// so that we can achieve fast access to the items.
      virtual bool getSelectedItems(SelectionMode3D selMode, Op::ItemSelectionList& selectionList) = 0;

      /// Turn the GeoSelection structure into a flat vector of weighted vertices or faces per GeoInfo,
      /// so that we can achieve fast access to the items.
      virtual bool getWeightedItems(SelectionMode3D selMode, Op::ItemSelectionList& selectionList) = 0;

      //----------------------------------------------------------------------------
      // Deprecated methods
      //----------------------------------------------------------------------------
      /// Get the geometry from the node.
      mFnDeprecatedLegacy3DSystemInNuke14("getGeometry() has been deprecated, use getScene() instead.")
        virtual GeometryList* getGeometry() = 0;
      //----------------------------------------------------------------------------
      //----------------------------------------------------------------------------
    };


  } // namespace Image
} // namespace DD

#endif /* GEOSELECTKNOBI_H_ */
