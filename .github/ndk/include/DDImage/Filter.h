// Filter.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Filter_h
#define DDImage_Filter_h

#include "DDImage_API.h"
#include "DDImage/Op.h"

namespace DD
{
  namespace Image
  {

    /*! This is the delta for a 1.0 width filter and is used to layout
        the static storage of filter coefficients. */
#define FILTER_BINS 64

    /*! \class DD::Image::Filter

       Generates and returns 1-dimensional filter arrays. These are
       designed to be multiplied by source pixels to calculate an output
       pixel for a transformation or texture mapping operation.

       This object is actually just a small holder of an enumeration
       describing the type of filter. The filter coefficients are actually
       in static memory and shared between all instances of the same filter
       type.

       You can declare a static one of these if you want a filter. To allow
       the user to choose the type of filter, put one of these into your Op
       and then call the knobs() method inside your own Op::knobs() method
       to allow the user to change the type.

     */
    class DDImage_API Filter
    {
    public:
      // Built-in filter types:
      enum { Impulse = 0, Cubic, Keys, Simon, Rifman, Mitchell, Parzen, Notch, Lanczos4, Lanczos6, Sinc4, NumFilters };
      static const char* NAMES[]; //!< Name text array for menus
      static const char* HELP; //!< Help text for tooltips

    protected:
      int type_;        //!< Enumeration in built-in list
      float* filter_array;    //!< Filter data in an XY array
      Knob* filter_knob_;

    public:

      int type() const { return type_; }
      void type(int t) { type_ = t;
                         initialize(); }

      Knob* filterKnob() { return filter_knob_; }

      /*! The integer \a t indicates what type of filter is wanted. It is
         a number designed to be stored in an Enumeration_knob, you should
         use the Filter::NAMES to get the names of all the defined filters.
       */
      Filter(int type = Cubic);

      /*! Coefficients for the filter at a particular position & width. */
      struct Coefficients
      {
        const float* array; //!< coefficients
        int delta; //!< distance between cefficients in array
        int count; //!< number of coefficients
        int first; //!< index to multiply by array[0]
        float normalize; //!< common multiplier to make it sum to 1

        /*! Multiply an array of pixels by the filter coefficients and return the sum
           of the results. It is equivalent to the following code:

           \code
           float sum = pixels[first]*array[0];
           for (int i = 1; i < count; i++) sum += pixels[first+i]*array[delta*i];
           return sum*normalize;
           \endcode

           Note that in many cases your input is a bounded array or is not an array
           at all. You can either write equivalent code to read your source, or
           copy it to an array and call this.
         */
        float apply(const float* array) const;
        float apply(const float* array, int left, int right) const;
      };


      /*! This must be called before get() can be called. It fills in the
         static source for the filter arrays. If anybody has called this for
         the same type before, this will return immediately.
       */
      void initialize();

      /*! Return a filter float array.

          \a position is the coordinate the filter is centered on.

          \a width is the derivative of the transform. This must be positive
          so take the absolute value before calling this.

          \a result is set to the desired values. The actual coefficients
          are in static memory and the \a result.array will point at them.
          See Filter::Coefficients::apply() for how to use this.

          The return value is true if the filter is equivalent to a 1.0-wide
          one. This is very common and you can use this to apply the filter
          faster by special-casing the code. In this case:
          - delta == FILTER_BINS (a constant that is 64 currently)
          - count can be assumed to be 4
          - normalize is 1.0
       */
      bool get(float position, float width, Coefficients&) const;

      bool impulse() const { return type_ == Impulse; }

      /*! Returns true if the 1-wide centered filter has a value of zero at
       +1 and -1.  This indicates that the image does not change if there
         is only integer translation.
       */
      bool interpolative() const;

      /*! Filter-selection control. */
      void knobs(Knob_Callback, const char* name = "filter", const char* label = nullptr);
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
