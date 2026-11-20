// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_STAGECOMPARISON_H
#define USG_GEOM_STAGECOMPARISON_H

#include "usg/api.h"

#include <memory>
#include <string>

namespace usg
{
  class Path;
  using StageRef = std::shared_ptr<class Stage>;

  class Attribute;
  class Prim;
  class Relationship;

  //! class StageComparison
  //! A class for comparing a stage A to a stage B.
  class USG_API StageComparison {
  public:
    //! Indicates the cause for a Prim change.
    enum class PrimChangeType {
      PrimAdded,          //!< Prim was added.
      PrimRemoved,        //!< Prim was removed.
      PrimTypeChanged     //!< Prim type changed.
    };

    //! Comunicates how a prim changed between two stages.
    struct PrimChangeInfo
    {
      PrimChangeType type;  //! Cause for change.
      const Prim& primA;    //! Prim in stage A.
      const Prim& primB;    //! Prim in stage B.
    };

    //! Indicates the cause for an Attribute change.
    enum class AttributeChangeType {
      AttributeAdded,     //!< Attribute was added.
      AttributeRemoved,   //!< Attribute was removed.
      NumTimeSamples,     //!< Number of time samples changed.
      TimeSample,         //!< Time sample mismatch.
      NumFields,          //!< Number of attribute fields changed.
      Field,              //!< Field value changed.
      InvalidValue,       //!< Either but not both values is not valid.
      ValueChanged        //!< Value changed.
    };

    //! Communicates how an attribute changed between two stages.
    struct AttributeChangeInfo
    {
      AttributeChangeType type; //!< Cause for change.
      const Attribute& attrA;   //!< Attribute in stage A.
      const Attribute& attrB;   //!< Attribute in stage B.
    };

    //! Indicates the cause for a Relationship change.
    enum class RelationshipChangeType {
      Added,     //!< Relationship was added.
      Removed,   //!< Relationship was removed.
      Changed,   //!< Relationship changed.
    };

    //! Communicates how a Relationship changed between two stages.
    struct RelationshipChangeInfo
    {
      RelationshipChangeType type;  //!< Cause for change.
      const Relationship& relA;     //!< Relationship in stage A.
      const Relationship& relB;     //!< Relationship in stage B.
    };

    //! Class used to communicate the changes to the calling code.
    class USG_API Diff {
    public:
      virtual ~Diff();
      //! Called if a prim has been added, removed or changed.
      virtual void primChanged(const usg::Path&, const PrimChangeInfo&) = 0;
      //! Called if a prim attribute has been added, removed or changed.
      virtual void attributeChanged(const usg::Path&, const AttributeChangeInfo&) = 0;
      //! Called if a prim relationship has been added, removed or changed.
      virtual void relationshipChanged(const usg::Path&, const RelationshipChangeInfo&) = 0;
    };

    //! Which method to use to compare AssetPaths.
    //! This is useful when importing and exporting stages results in
    //! AssetPaths becoming absolute paths.
    enum class AssetPathCompareMethod {
      FullPath,   //!< Compare the full path
      NameOnly    //!< Compare only the name part (i.e. after the last '/')
    };

    //! Sets the AssetPath comparison method.
    void setAssetPathCompareMethod(AssetPathCompareMethod method) { _assetPathMethod = method; }
    //! Returns the AssetPath comparison method.
    AssetPathCompareMethod getAssetPathCompareMethod() const { return _assetPathMethod; }

    //! Sets the comparison tolerance for floating point based values.
    void setFloatingPointTolerance(double tolerance) { _floatingPointTolerance = tolerance; }
    //! Returns the floating point comparison tolerance.
    double getFloatingPointTolerance() const { return _floatingPointTolerance; }

    //! Compares \a stageA with \a stageB.
    //! If \a diff is provided, the actual differences between the stages are
    //! communicated by calling the relevant methods.
    //! Returns true if no differences are found between the stages.
    bool compare(const StageRef& stageA, const StageRef& stageB, Diff* diff);

  private:
    AssetPathCompareMethod _assetPathMethod{AssetPathCompareMethod::FullPath};
    double _floatingPointTolerance{0.0};
  };
}
#endif
