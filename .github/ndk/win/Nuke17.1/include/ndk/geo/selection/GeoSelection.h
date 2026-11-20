// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_SELECTION_GEOSELECTION_H
#define NDK_GEO_SELECTION_GEOSELECTION_H

#include "ndk/api.h"

#include "usg/geom/Path.h"
#include "usg/geom/Stage.h"

#include <array>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace ndk {

  namespace Selection {

    //! User-selected prims are explicit selections, those prim's children are implicitly selected
    enum class NDK_API PrimSelectionStatus
    {
      Unselected,
      Explicit,
      Implicit
    };

    using PrimSelectionStatusMap = std::unordered_map<usg::Path, PrimSelectionStatus>;

    //! Selection status with weighting of one part of the object (e.g. vertices or faces)
    struct NDK_API WeightedPartSelection
    {
      std::vector<float> selections;
      std::vector<float> weights;

      void clear()
      {
        selections.clear();
        weights.clear();
      }

      void clearSelections()
      {
        std::fill(selections.begin(), selections.end(), 0.0f);
      }

      void clearWeights()
      {
        std::fill(weights.begin(), weights.end(), 0.0f);
      }

      bool empty() const {
        mFnAssert(selections.size() == weights.size());
        return selections.empty();
      }

      size_t size() const
      {
        mFnAssert(selections.size() == weights.size());
        return selections.size();
      }

      void resize(size_t size)
      {
        selections.resize(size);
        weights.resize(size);
      }

      void add(const WeightedPartSelection& rhs)
      {
        if (size() < rhs.size()) {
          resize(rhs.size());
        }
        std::transform(selections.cbegin(), selections.cend(), rhs.selections.cbegin(), selections.begin(), std::plus<float>());
        std::transform(weights.cbegin(), weights.cend(), rhs.weights.cbegin(), weights.begin(), std::plus<float>());
      }

      bool operator==(const WeightedPartSelection& rhs) const
      {
        return (this->selections == rhs.selections) && (this->weights == rhs.weights);
      }
    };

    //! A part of an object's geometry
    enum class NDK_API PartType
    {
      Vertices,
      Faces,
      NumParts,
      Object = NumParts // Special value for the object's selection status. There is no corresponding part object!
    };

    using PartSelectionArray = std::array<WeightedPartSelection, static_cast<size_t>(PartType::NumParts)>;

    //! Stores the selection status of a single 3D object
    struct NDK_API GeoObjectSelection
    {
      //! Object unselected and part selections empty
      bool isEmpty() const;

      bool isSelected = false;
      PartSelectionArray parts;

      WeightedPartSelection& getPartSelection(PartType type) { return parts[static_cast<size_t>(type)]; }
      const WeightedPartSelection& getPartSelection(PartType type) const { return parts[static_cast<size_t>(type)]; }
    };

    //! Stores the selection status of several 3D objects as a map of paths to GeoObjectSelections
    class NDK_API GeoSelection
    {
      public:
        using ObjectMap = std::unordered_map<usg::Path, GeoObjectSelection>;

        GeoSelection();
        ~GeoSelection();


        // METHODS CONCERNING WHOLE MAP

        //! Removes everything
        void clear();

        //! Removes the selection values but leaves the ObjectMap entries
        void clearValuesOnly();

        //! Total number of entries, selected or not
        size_t size() const;

        //! Add another selection to this one (adds up attributes element-wise)
        void addSelection(const GeoSelection& other);

        //! Deselect the given part of all objects
        void deselect(PartType type);

        //! Remove all entries with GeoObjectSelection.isEmpty() == true
        void deleteEmpty();

        //! Only returns valid values if isPrimSelectionStatusValid() == true
        const PrimSelectionStatusMap& getPrimSelectionStatus() const;

        //! Check status of path, returns unselected for invalid paths, only returns valid values if isPrimSelectionStatusValid() == true
        PrimSelectionStatus getPrimSelectionStatus(const usg::Path& path) const;

        //! If true getPrimSelectionStatus() return value is valid
        bool isPrimSelectionStatusValid() const;

        //! For use by Nuke viewer which will update the status with the current stage
        void updatePrimSelectionStatus(const std::shared_ptr<usg::Stage>& stage);


        // METHODS CONCERNING SINGLE PATHS

        //! Path is present, may be selected or not
        bool isPathPresent(const usg::Path& path) const;

        //! If path not present returns false
        bool isObjectSelected(const usg::Path& path) const;

        //! Will create a new entry if the path not present
        void setObjectSelected(const usg::Path& path, bool selected);

        //! Get write access to part selections. Creates new (unselected) entry if path not present
        WeightedPartSelection& getWritablePartSelection(const usg::Path& path, PartType type);


        // ITERATION AND SEARCH

        //! Find a path, will be equal to end() if not found
        ObjectMap::const_iterator find(const usg::Path& path) const;

        //! Iterator
        ObjectMap::const_iterator begin() const;

        //! Iterator
        ObjectMap::const_iterator end() const;


        // SERIALIZATION

        //! Dump state to text output stream (PrimSelectionStatus is never stored!)
        std::ostream& serialize(std::ostream &out) const;

        //! Set state from text input stream (PrimSelectionStatus is never stored!)
        std::istream& deserialize(std::istream &in);


      private:
        void setPrimSelectionStatusDirty();

        ObjectMap _selectedPaths;
        PrimSelectionStatusMap _primSelectionStatus;
        bool _isPrimSelectionStatusDirty = false;
    };

    NDK_API std::ostream& operator<<(std::ostream& os, const GeoSelection& selection);

  }
}

#endif
