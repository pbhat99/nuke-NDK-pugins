// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef CONTROLPOINTCOLLECTIONKNOBI_H
#define CONTROLPOINTCOLLECTIONKNOBI_H

#include "DDImage/Knob.h"
#include "DDImage/Vector2.h"
#include "DDImage/Matrix4.h"
#include "DDImage/RippleKnobI.h"

#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    struct ConvolveArray;

    class DDImage_API ControlPointCollectionInterface
    {
    public:
      virtual ~ControlPointCollectionInterface();
      
      virtual void resetToDefault(ControlPointCollection_KnobI* knob) = 0;
    };

    class DDImage_API ControlPointCollection_KnobI
    {
    public:
      virtual ~ControlPointCollection_KnobI();
      
      enum eAction
      {
        eNull = -1,
        eMoveMainControlPoints
      };

      struct ControlPoint
      {
        Vector2 pos;
        Vector2 tangents[4];
      };

      enum ControlPointsFlags
      {
        eDataPosition = 1 << 0,
        eDataTangent0 = 1 << 1,
        eDataTangent1 = 1 << 2,
        eDataTangent2 = 1 << 3,
        eDataTangent3 = 1 << 4,
        eDataTangents = eDataTangent0 | eDataTangent1 | eDataTangent2 | eDataTangent3,
        eDataAll = 0xFFFFFFFF
      };
      
      enum ClipboardData
      {
        eNone,      // nothing in the clipboard
        eForeign,   // a format we don't understand
        eValues,    // data for single frame values
        eAnimations // data for animation curves
      };
      
    public:
      //! resizes the 2D array of ControlPoints to the requested size. This will invalidate (reset) all Control Point
      //! data if reset it set to true, otherwise it will try and interpolate new positions for the resulting Control Points
      //! based on the topology of the current control points
      virtual void resizeCollection(unsigned int xPoints, unsigned int yPoints = 1, bool reset = true) = 0;
      
      //! resets the positions and the tangents of each control point to fit the specified rectangle dimensions
      virtual void scaleToFitRect(int x, int y, int r, int t) = 0;

      virtual void  previewMode(bool mode) = 0;

      virtual bool previewMode() const = 0;

      virtual void changed( bool save ) = 0;

      virtual void startUndo( const char* name ) = 0;

      virtual void extraUndo() = 0;

      virtual void clean_undo() = 0;

      //! indicates whether or not an undo happened
      virtual bool undoHappened() = 0;

      //! indicates whether or not an undo happened and resets the flag
      virtual bool undoHappenedAndReset() = 0;
      
      virtual unsigned int xPointCount() = 0;
      virtual unsigned int yPointCount() = 0;

      virtual unsigned int previewXPointCount() = 0;
      virtual unsigned int previewYPointCount() = 0;

      //! converts a 1-dimensional array index of a point to the corresponding X,Y coordinate indexes
      virtual void convertSingleIndexToIndexes(unsigned int index, unsigned int& xIndex, unsigned int& yIndex, bool previewMode = false) const = 0;
      //! converts 2-dimensional indexes (X,Y) to a single 1-dimensional array index
      virtual unsigned int convertIndexesToSingleIndex(unsigned int xIndex, unsigned int yIndex, bool previewMode = false) const = 0;
      
      virtual void getAllControlPoints(std::vector<int>& indexes) = 0;
      
      //! set the position of the control points / tangents for the current frame
      virtual void setControlPointPos(unsigned int x, unsigned int y, float xValue, float yValue) = 0;      
      virtual void setControlPointTangentPos(unsigned int x, unsigned int y, unsigned int index, float xValue, float yValue) = 0;
      
      //! set the position of the control point at the given frame - will key if there isn't a key on the frame specified
      virtual void setControlPointPosForFrame(unsigned int x, unsigned int y, float xValue, float yValue, float frame) = 0;

      virtual void cuspPoints(std::vector<int>& indexes) = 0;      
      virtual void smoothPoints(std::vector<int>& indexes) = 0;

      //! sets the expression of the AnimationCurve that represents the X value of the specified Control Point.
      //! this will set the Control Point to be animated if it currently is not
      virtual void setControlPointXCurveExpression(unsigned int x, unsigned int y, const std::string& expression) = 0;
      
      //! sets the expression of the AnimationCurve that represents the Y value of the specified Control Point.
      //! this will set the Control Point to be animated if it currently is not
      virtual void setControlPointYCurveExpression(unsigned int x, unsigned int y, const std::string& expression) = 0;
      
      //! fills in the supplied ControlPoint data struct for the specified control point with the items requested based on flags
      virtual void getControlPointData(const DD::Image::OutputContext& oc, ControlPoint& cp, unsigned int x, unsigned int y,
                                       unsigned int flags = eDataAll) = 0;

      virtual void getControlPointPreviewData(const DD::Image::OutputContext& oc, ControlPoint& cp, unsigned int x, unsigned int y,
                                       unsigned int flags = eDataAll) = 0;
      
      //! the following three functions will be faster than individually getting each control point required using getControlPointData()
      
      //! gets data for all control points and adds them to the std::vector<DD::Image::ControlPointCollection_KnobI::ControlPoint> provided
      //! points are added on a row, column basis, starting from 0, so (0,0), (0,1), (0,2), (1,0), etc
      //! Takes an OutputContext which can handle views and frames correctly
      virtual void getAllControlPointsData(const DD::Image::OutputContext& oc,
                                           std::vector<DD::Image::ControlPointCollection_KnobI::ControlPoint>& points, unsigned int flags = eDataAll) = 0;
      
      //! gets data for all control points and adds them to the std::vector<DD::Image::ControlPointCollection_KnobI::ControlPoint> provided
      //! points are added on a row, column basis, starting from 0, so (0,0), (0,1), (0,2), (1,0), etc
      //! This only takes a frame, and is only designed to work with the current UI view for handles in viewer
      virtual void getAllControlPointsData(float frame,
                                           std::vector<DD::Image::ControlPointCollection_KnobI::ControlPoint>& points, unsigned int flags = eDataAll) = 0;
      
      //! gets preview data for all control points and adds them to the std::vector<DD::Image::ControlPointCollection_KnobI::ControlPoint> provided
      //! points are added on a row, column basis, starting from 0, so (0,0), (0,1), (0,2), (1,0), etc
      //! This only takes a frame, and is only designed to work with the current UI view for handles in viewer
      virtual void getAllControlPointsPreviewData(float frame,
                                           std::vector<DD::Image::ControlPointCollection_KnobI::ControlPoint>& points, unsigned int flags = eDataAll) = 0;

      //! sets the properties (specified by flags) of the specified ControlPoint from the passed in data struct
      virtual bool setControlPointData(ControlPoint& cp, unsigned int x, unsigned int y, unsigned int flags = eDataAll) = 0;
      
      //! translates a single control point's position by the specified x and y amounts. Will only do it for the current frame
      virtual void translateControlPointPos(unsigned int xIndex, unsigned int yIndex, const DD::Image::Vector2& delta) = 0;
      //! translates a single control point's position by the specified x and y amounts, taking into account the rippleInfo
      //! passed in. If the state of the ripple mode is off, it will only translate the point for the current frame
      virtual void translateControlPointPosRipple(unsigned int xIndex, unsigned int yIndex, const DD::Image::Vector2& delta,
                                                  const DD::Image::Ripple_KnobInfo& rippleInfo) = 0;
      
      //! translates a single control point's tangent by the specified x and y amounts. Will only do it for the current frame
      virtual void translateControlPointTangent(unsigned int xIndex, unsigned int yIndex, unsigned int tangent, const DD::Image::Vector2& delta) = 0;
      //! translates a single control point's tangent by the specified x and y amounts, taking into account the rippleInfo
      //! passed in. If the state of the ripple mode is off, it will only translate the tangent for the current frame
      virtual void translateControlPointTangentRipple(unsigned int xIndex, unsigned int yIndex, unsigned int tangent,
                                                      const DD::Image::Vector2& delta, const DD::Image::Ripple_KnobInfo& rippleInfo) = 0;
            
      //! copy the values (on the current frame) of the specified indexes to the clipboard
      virtual void copyControlPointValuesToClipboard(const std::vector<int>& indexes) const = 0;
      virtual void copyControlPointValuesToClipboard(const std::vector<int>& indexes, int xRes, int yRes) const = 0;
      //! copy the animations of the specified indexes to the clipboard
      virtual void copyControlPointAnimationsToClipboard(const std::vector<int>& indexes) const = 0;
      virtual void copyControlPointAnimationsToClipboard(const std::vector<int>& indexes, int xRes, int yRes) const = 0;
      
      //! paste the values (on the current frame) of the specified indexes from the clipboard. Returns
      //! true if valid data was in the clipboard and control points were modified
      virtual bool pasteControlPointValuesFromClipboard(const std::vector<int>& indexes) = 0;
      //! paste the animations of the specified indexes from the clipboard. Returns
      //! true if valid data was in the clipboard and control points were modified
      virtual bool pasteControlPointAnimationsFromClipboard(const std::vector<int>& indexes) = 0;

      //! copies the current collection to the preview
      virtual void copyCollectionToPreview() = 0;
      
      //! returns what is in the clipboard currently
      virtual ClipboardData detectClipboardData() const = 0;
      //! returns the number of control points in the clipboard
      virtual int getClipboardNumberOfControlPoints(int& xRes, int& yRes) const = 0;
      
      //! copy the values (on the current frame) of the specified indexes to a string
      virtual std::string copyControlPointValuesToString(const std::vector<int>& indexes, int xRes, int yRes) const = 0;
      //! copy the animations of the specified indexes to a string
      virtual std::string copyControlPointAnimationsToString(const std::vector<int>& indexes, int xRes, int yRes) const = 0;
      //! paste the values (on the current frame) of the specified indexes from a string. Returns
      //! true if valid data was in the clipboard and control points were modified
      virtual bool pasteControlPointValuesFromString(const std::string& data, const std::vector<int>& indexes) = 0;
      //! paste the animations of the specified indexes from a string. Returns
      //! true if valid data was in the clipboard and control points were modified
      virtual bool pasteControlPointAnimationsFromString(const std::string& data, const std::vector<int>& indexes) = 0;
      //! returns what is in the string.
      virtual ClipboardData detectStringData(const std::string& data) const = 0;

      virtual Vector2 transform(float x, float y, bool isAbsolutePosition = true) const = 0;
      virtual Vector2 inverseTransform(float x, float y, bool isAbsolutePosition = true) const = 0;
      
      //! these functions control keys/animation on all ControlPoints in the collection
      
      //! returns all keyframes that have been set for any control point (and its tangents) in the collection
      virtual void getKeyFrames(std::set<float>& keyFrames) = 0;
      
      virtual void setKeys() = 0;
      virtual void deleteAnimations() = 0;
      
      virtual void showEditExpressionDialogForControlPoint(unsigned int x, unsigned int y) = 0;
      virtual void removeExpressionsForControlPoint(unsigned int x, unsigned int y) = 0;
      
      virtual bool controlPointOrTangentsHaveExpressions(unsigned int x, unsigned int y) = 0;
      virtual bool controlPointHasExpressions(unsigned int x, unsigned int y) = 0;
      virtual bool controlPointTangentHasExpressions(unsigned int x, unsigned int y, unsigned int tangent) = 0;
      
      //! returns whether the requested control point position (and optionally tangents) has keys set on the frame provided
      virtual bool controlPointOrTangentsHaveKeys(unsigned int x, unsigned int y, float frame, bool includeTangents) = 0;
      
      //! returns whether the requests control point position (and optionally tangents) are using animated values or static values
      virtual bool controlPointIsAnimated(unsigned int x, unsigned int y, bool includeTangents) const = 0;
      
      //! these set and remove keys for a single control point (and its tangents) on the current frame
      virtual void setKeysOnControlPoint(unsigned int x, unsigned int y) = 0;
      virtual void removeKeysOnControlPoint(unsigned int x, unsigned int y) = 0;
      virtual void deleteAnimationsOnControlPoint(unsigned int x, unsigned int y) = 0;
      
      //! set the key for a single control point's position (not its tangents)
      virtual void setKeyOnControlPointPosition(unsigned int x, unsigned int y, float frame) = 0;
      virtual void setKeyOnControlPointTangents(unsigned int x, unsigned int y, float frame) = 0;
            
      //! sets keys for the specified control point on the current frame, and keys on the current frame any other control
      //! point which is animated
      virtual void setKeysOnControlPointAndKeyOtherAnimated(unsigned int x, unsigned int y) = 0;
      
      //! these set and remove keys for specified control points on the current frame
      virtual void setKeysOnControlPoints(const std::vector<int>& cpIndexes) = 0;
      virtual void removeKeysOnControlPoints(const std::vector<int>& cpIndexes) = 0;
      virtual void deleteAnimationsOnControlPoints(const std::vector<int>& cpIndexes) = 0;
      
      //! these set and remove keys for all control points on the current frame
      virtual void setKeysOnAllControlPoints() = 0;
      //! remove the keys on all the control points on the current frame
      virtual void removeKeysOnAllControlPoints() = 0;
      //! remove the keys on all the control points at the specified frame
      virtual void removeKeysOnAllControlPoints(double time) = 0;
      //! remove all the keys on all the control points on all frames
      virtual void removeAllKeysOnAllControlPoints() = 0;
      virtual void deleteAnimationsOnAllControlPoints() = 0;
      
      //! find the fraction of the co-ordinates between the control points
      //! the skip option makes the function assume that the co-ordinates already contains a row/column, so
      //! will check against the one after
      virtual float findRowFractionOfPosition(float posX, float posY, unsigned int xIndex, unsigned yIndex, bool skip = false) = 0;
      virtual float findColumnFractionOfPosition(float posX, float posY, unsigned int xIndex, unsigned yIndex, bool skip = false) = 0;

      virtual bool createRow(float posX, float posY, unsigned int xIndex, unsigned int yIndex, std::vector<ControlPoint>& out,
                             bool forceNoAnimation = false) = 0;
      virtual bool createColumn(float posX, float posY, unsigned int xIndex, unsigned int yIndex, std::vector<ControlPoint>& out,
                                bool forceNoAnimation = false) = 0;
      
      //! adds a new set of control points at the specified index
      virtual void insertRow(unsigned int index, float fraction) = 0;
      virtual void insertColumn(unsigned int index, float fraction) = 0;
      
      //! removes a particular set of control points at the specified index
      //! optionally alters tangents on the surrounding points based on the fraction of the position
      virtual void removeRow(unsigned int index, bool alterSurroundingTangents) = 0;
      virtual void removeColumn(unsigned int index, bool alterSurroundingTangents) = 0;
      
      enum ShowItem
      {
        ePosition, eTangents, eBoth
      };
      
      virtual void showAnimationCurvesForControlPoints(const std::vector<int>& cpIndexes, ShowItem show, bool dopeSheet) = 0;

      virtual void updateAnimationVisibility( bool visible ) = 0;
      
      virtual void append(DD::Image::Hash& hash, const DD::Image::OutputContext* context = nullptr) = 0;
      virtual DD::Image::Hash editVersionHash() const = 0;
      
      virtual void changed() = 0;
      
      virtual DD::Image::Matrix4& getTransformMatrix() = 0;
      virtual DD::Image::ConvolveArray& getWorldTransformMatrix() = 0;
      
      virtual void setUseOtherTransforms(bool useOtherTransforms) = 0;
      virtual void setOtherTransforms(DD::Image::Matrix4* transformMatrix, DD::Image::ConvolveArray* worldTransform) = 0;
      
      virtual void setOwner(DD::Image::ControlPointCollectionInterface* pOwner) = 0;
      
      virtual DD::Image::Knob* realKnob() = 0;
      virtual DD::Image::Knob& knob() = 0;
    };
  }
}

#endif // CONTROLPOINTCOLLECTIONKNOBI_H
