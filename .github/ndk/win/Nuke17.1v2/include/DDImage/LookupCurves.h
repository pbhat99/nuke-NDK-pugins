// Convolve.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_LookupCurves_h
#define DDImage_LookupCurves_h

#include "DDImage_API.h"

#include <vector>
#include <string>

class LookupCurvesKnob; // class in Nuke that does all the work

namespace DD
{
  namespace Image
  {

    class Knob;
    class OutputContext;
    class Hash;

    /*! \relates DD::Image::LookupCurves
       Table entry describing one of the curves. You must create a static table of these
       and use it in the constructor of your LookupCurves object. Warning: the pointer
       to the table is copied into Nuke and it must remain in existence even if the
       LookupCurves object is destroyed.
     */
    struct CurveDescription
    {

      const char* name; //!< name of curve (should be short). NULL ends the table
      std::string defaultValue; //!< string to parse to get the default curve

      typedef void (* BuildCallback)(void* curve); //!< Ony for internal use, callback used to build the curve

      BuildCallback buildCallback; //!< Ony for internal use, callback used to build the curve

      enum {
        eNormal   = 0,
        eReadOnly = 1
      };

      int flags; //!< bitmask with above values in it
      const char* tooltip;
    };

    //! Type for the lookup curves editor.  Mostly affects background and 
    //! framing.  On the hue types the first and last keyframes will be synchronised.
    enum LookupCurvesType {
      eLookupCurvesTypeTime = 0,
      eLookupCurvesTypePlot,
      eLookupCurvesTypeRGBLut,
      eLookupCurvesTypeYLut,
      eLookupCurvesTypeALut,
      eLookupCurvesTypeHue1,
      eLookupCurvesTypeHue2
    };

    /*! \class DD::Image::LookupCurves

       Provides user-editable lookup curves to a plugin.

       Your plugin must include an instance of this object, and must also call
       LookupCurves_knob() in the knobs() method. The user will then get a mini curve
       editor in the control panel where they can edit the lookup curves.

       This fairly opaque object replaces the array of "Animation" objects that were in
       the previous Nuke api. This was done to allow more freedom to reimplement the
       animation editor, and to allow lookup curves that can vary over time.

       Although many plugins call evaluate() directly in their engine()
       method, it is recommended that for maximum speed you should build your
       own indexed table during _validate() by calling evaluate() at that
       time to fill it in. For absolute maximum speed, use the append()
       method to check if the lookup curves have changed since you last
       filled in your table.  A plugin will always have a better idea about
       the domain, accuracy, and interpolation it needs, so Nuke does not
       bother even trying to provide an indexed table.

     */
    class DDImage_API LookupCurves
    {
    public:

      //! Represents a point on a curve
      struct SKey
      {
        float x; //!> X coordinate
        float y; //!> Y coordinate
        float slope; //!> curve slope at (X,Y)
      };

      /**
       * Constructor.  T should point to an array of CurveDescriptions; these should be 
       * terminated by a sentinel CurveDescriptor with a NULL name pointer.
       */
      LookupCurves(const CurveDescription* t);

      /*!
         Return how many curves the user can edit. This is defined by the table passed
         to the constructor. All functions that take an index number can only be called
         with a number less than this value.
       */
      unsigned size() const;

      /*! \fn const char* LookupCurves::curveName(unsigned index) const
         Return the name of the indexed curve, from the table passed to the constructor.
       */
      const char* curveName(unsigned index) const { return curveDescriptions[index].name; }

      /*! \fn double LookupCurves::getValue(unsigned index, double x) const
         Return the value of lookup curve \a index at the location \a x.
       */
      double getValue(unsigned index, double x) const { return evaluateCallback(this, index, x); }

      /*! fn const void LookupCurves::getOuterKeys(unsigned index, SKey& leftKey, SKey& rightKey) const
          Return the first and last keys for the curve
       */
      void getOuterKeys(unsigned index, SKey& leftKey, SKey& rightKey) const { getOuterKeysCallback(this, index, leftKey, rightKey); }

      /*! \fn bool LookupCurves::hasExpression(unsigned index) const
         Does the curve have an expression? (expressions may not be safely evaluated on worker threads) */
      bool hasExpression(unsigned index) const { return otherCallback(this, HASEXPRESSION, index, 0, 0) != 0; }

      /*! \fn bool LookupCurves::isIdentity(unsigned index) const
         Return true if the curve is the identity: evaluate(index,x)==x for all x. */
      bool isIdentity(unsigned index) const { return otherCallback(this, ISIDENTITY, index, 0, 0) != 0; }

      /*! \fn bool LookupCurves::isConstant(unsigned index) const
         Return true if the curve is constant: evaluate(index,x)==evaluate(index,0) for all x. */
      bool isConstant(unsigned index) const { return otherCallback(this, ISCONSTANT, index, 0, 0) != 0; }

      /*! \fn double LookupCurves::getDerivative(unsigned index, double x) const
         Return the first derivative of the curve at x. */
      double getDerivative(unsigned index, double x) const { return otherCallback(this, GETDERIVATIVE, index, 1, x); }

      /*! \fn double LookupCurves::getNthDerivative(unsigned index, int n, double x) const
         Return the n'th derivative of the curve at x. If n is zero this acts like evaluate() */
      double getNthDerivative(int n, unsigned index, double x) const { return otherCallback(this, GETDERIVATIVE, index, n, x); }

      /*! \fn double LookupCurves::getInverse(unsigned index, double y) const

         Return x such that evaluate(index,x)==y. This is fairly slow!  This
         is designed to invert color lookup tables. It only works if the
         derivative is zero or positive everywhere. If the Newton's
         approximation fails it will do a binary search but it then assumes
         the answer is between 0 and 1.
       */
      double getInverse(unsigned index, double y) const { return otherCallback(this, GETINVERSE, index, y, 0); }

      /*! \fn double LookupCurves::getInverseGeneral(unsigned index, double y) const

         Return x such that evaluate(index,x)==y. This is fairly slow!
         It is well defined for monotonically increasing/decreasing curves,
         where the inverse lies within the Root node's frame range.
       */
      double getInverseGeneral(unsigned index, double y) const { return otherCallback(this, GETINVERSEGENERAL, index, y, 0); }

      /*! \fn double LookupCurves::getIntegral(unsigned index, double x1, double x2) const
         Integrate the curve between x1 and x2.
       */
      double getIntegral(unsigned index, double x1, double x2) const { return otherCallback(this, GETINTEGRAL, index, x1, x2); }

      /*!
         Modify the hash to reflect the values of all the lookup curves. This can
         be used to determine if you need to refill the tables.
       */
      void append(Hash&) const;

      //! Returns the knob that uses this object, may be null.
      Knob* getKnob() const { return knob; }

    private:

      friend class ::LookupCurvesKnob;

      std::vector<CurveDescription> curveDescriptions;
      Knob* knob; // points at the Knob that stored into it
      const OutputContext* context; // points at the Op's outputContext
      double (* evaluateCallback)(const LookupCurves*, unsigned, double x);
      static double nocrash(const LookupCurves*, unsigned, double);
      enum Op { GETINVERSE, ISIDENTITY, ISCONSTANT, GETDERIVATIVE, GETINTEGRAL, HASEXPRESSION, GETINVERSEGENERAL };
      double (* otherCallback)(const LookupCurves*, Op, unsigned index, double a, double b);
      static double nocrash2(const LookupCurves *, Op, unsigned index, double a, double b);
      void (* getOuterKeysCallback)(const LookupCurves*, unsigned index, SKey& leftKey, SKey& rightKey);
      static void nocrash3(const LookupCurves *, unsigned index, SKey& leftKey, SKey& rightKey);

    };

    /**
     * interface class for Lookup Curves knobs
     */
    class DDImage_API LookupCurves_KnobI {
    public:
      virtual ~LookupCurves_KnobI();

      /**
       * set the curve /i/ to /curveDescription/
       */
      virtual void setCurve(int curveNo, const CurveDescription& curveDescription) = 0;
    };

  }
}

#endif
