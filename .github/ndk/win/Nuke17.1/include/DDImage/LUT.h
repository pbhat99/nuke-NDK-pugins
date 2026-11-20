// LUT.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_LUT_h
#define DDImage_LUT_h

#include "DDImage_API.h"
#include "Hash.h"
#include "math.h"
#include "Thread.h"
#include "DDImage/Deprecation.h"

namespace DD
{
  namespace Image
  {

    class LUTInfo;
    class Reader;
    class Writer;

    /*! \class DD::Image::LUT

       A LUT defines a mapping function. One side is the floating point
       numbers used in Nuke. The other side (the side it goes "to" and "from")
       is generally values that are used in a file or by a piece of
       hardware.

       The LUT class can very quickly convert between an array of floating
       point number in Nuke and arrays of 8 bit or 16 bit numbers. It can
       also convert float<->float and may be faster depending on the
       complexity of the function. All this is done by using lookup tables
       and linear interpolation between the table entries. When converting
       to integers, error diffusion is used to hide posterization.

       Any subclass must implement to_byte() and from_byte().  For
       historical reasons these convert to the range 0..255 rather than
       0..1.

       There is also the pseudo-subclass Linear, which has the same API but
       does straight line transfer only, avoiding the if statement used to
       detect this in the LUT class functions.

     */
    class DDImage_API LUT
    {
      mutable U16 to_byte_table[0x10000];
      float from_byte_table[257];
      mutable char initialized_;
      mutable bool linear_;
      mutable bool zero_;

      static Lock gInitialisationLock;

    public:
      typedef int BuiltinLUTId;

      /* DataType is externally exposed and immutable due to reference by value
       * (see the 'addDefaultColorspaceMapper' python callback for more details).
       * It can be used to access LUTs directly via Get/SetLut
       */
      enum DataType {
        INVALID        = -1,
        MONITOR        = 0,
        VIEWER         = 1,
        INT8           = 2,
        INT16          = 3,
        LOG            = 4,
        FLOAT          = 5,
        GAMMA1_8       = 6,
        GAMMA2_2       = 7,
        GAMMA2_4       = 8,
        PANALOG        = 9,
        REDLOG         = 10,
        VIPERLOG       = 11,
        ALEXAV3LOGC    = 12,
        PLOGLIN        = 13,
        SLOG           = 14,
        SLOG1          = 15,
        SLOG2          = 16,
        SLOG3          = 17,
        CLOG           = 18,
        PROTUNE        = 19,
        GAMMA2_6       = 20,
        LOG3G10        = 21,
        LOG3G12        = 22,
        BT1886         = 23,
        HDR2084        = 24,
        HYBRIDLOGGAMMA = 25,
        ST2084         = 26,
        BLACKMAGIC_GEN5 = 27,
        ARRILOGC4       = 28,
        TYPES_END
      };

    protected:
      Hash hash;
      // Marking this const so that it can be called from const functions.
      // It updates the mutable to_byte_table. We need this because we're
      // deferring initialisation of the table 'til its first use, which is
      // generally inside a const function.

      /*!
         Fill in the table used by toFloatFast() and thus by all the
         to functions. This is done by calling to_byte(). This can be used
         by a from_byte() implementation that works by using interpolation
         to invert the to_byte() function.
         Does nothing if called a second time.
       */
      void fillToTable() const;

      /*!
         Fill in the table used by fromFloatFast() and thus all the
         from functions. This is done by calling from_byte(). This can be
         used by a to_byte() implementation taht works by using interpolation
         to invert the from_byte() function.
         Does nothing if called a second time.
       */
      void fillFromTable();

      /*!
         Forces fillToTable() and fillFromTable() to fill the tables again
         and calls them, and calculates a value for hash(). The constructor
         calls this, but subclasses may want to call this directly if the LUT
         has other controls that can change it's results.
       */
      void validate();

      static void InitialiseBuiltin(BuiltinLUTId id);

    public:
      LUT() : initialized_(0), linear_(false), zero_(false) {}
      virtual ~LUT() {}

      /*!
        Returns true if the LUT table used by the to functions is valid. It becomes valid
        after the first time one of the to functions is used.
      */
      bool isToTableValid() const { return initialized_ & 1; }

      /*!
         Return the value that to_byte(uchar...) is being calculated to
         by the lookup table.
       */
      float test(float) const;

      /*! \fn bool LUT::linear() const
         Returns true if to_byte(x) appears to return x*255. This is
         tested when the table is filled in. If true some operators are
         much faster.
       */
      bool linear() const { fillToTable(); return linear_; }

      /*! \fn bool LUT::zero() const
         Returns true if to_byte(0) returns a value <= 0. This is
         tested when the table is filled in. If true some operators are
         much faster.
       */
      bool zero() const { fillToTable(); return zero_; }

      /*!
         Add the contents of this LUT to the hash. This is pretty fast (it
         just appends a single internal Hash) because it needs to be called
         a lot to determine if images have changed.
       */
      void append(Hash&) const;

      /*! \fn float LUT::to_byte(float) const

         This virtual function must be overridden to do the math that
         converts floating point values to byte values in the range
         0-255. Notice that you want as accurate as a result as possible,
         do not round to the nearest integer. LUT
         assumes that for any values in the range 0-1.0 that this function
         is the inverse of to_byte(float).
       */
      virtual float to_byte(float) const = 0;

      /*! Convert an array of floating point pixel values to an array of
         bytes, with error diffusion. This is very well optimized and
         quite fast, and is used by the Nuke viewer when drawing it's
         display.

         \a W is the number of pixels to convert.

         \a delta is how far apart to put the resulting bytes, allowing
         you to write to an interlaced buffer.

         The input and output buffers must not overlap in memory.
       */
      void to_byte(uchar*, const float*, int W, int delta = 1) const;

      /*! Convert an array of floating point pixel values to an array of
         bytes, with error diffusion.

         \a alpha is the unconverted alpha for a premultiplied image.
         This is used to correct
         the premultiply so it matches how premultiply is usually stored
         on disk (the stored values are the converted values multiplied
         by the alpha, rather than the linear values multiplied by the
         alpha. The alpha channel itself should be converted with
         Linear::to_byte().

         \a W is the number of pixels to convert.

         \a delta is how far apart to put the resulting bytes, allowing
         you to write to an interlaced buffer.

         The input and output buffers must not overlap in memory.
       */
      void to_byte(uchar*, const float*, const float* alpha, int W, int delta = 1) const;

      /*! Fill an array of \a W shorts with n-bit conversion of the floating
          point pixels. Input values less than zero turn into zero, input
          values greater than 1 turn into all 1's (even though larger numbers
          can be represented when bits < 16). See to_byte for how
          all the other arguments are treated.

          Currently no error diffusion is done.
       */
      void to_short(U16*, const float*, int W, int bits = 16, int delta = 1) const;

      /*! Fill an array of \a W shorts with n-bit conversion of the floating
          point pixels. Input values less than zero turn into zero, input
          values greater than 1 turn into all 1's (even though larger numbers
          can be represented when bits < 16). See to_byte for how
          all the other arguments are treated.
       */
      void to_short(U16*, const float*, const float* alpha, int W, int bits = 16, int delta = 1) const;

      /*! \fn float LUT::from_byte(float v) const
         This virtual function should be overridden to do the math that
         converts byte values in the range 0-255 to floating point. LUT
         assumes that this function is the inverse of to_byte(float).
       */
      virtual float from_byte(float) const = 0;

      /*! Converts bytes through the from_byte(float) virtual function.
         A 256-entry lookup table filled in by validate() is used for
         maximum speed.

         The input data is \a W bytes in the range 0-255 pointed to by \a
         from. \a delta is the distance between the input bytes (for the
         common case where the input is interlaced rgba samples).

         The output is written to the first \a W floats of \a buf.

         The input and output may be the same memory array as long as \a
         delta is less or equal to 4. The conversion is done in backwards
         order so this is possible.
       */
      void from_byte(float*, const uchar*, int W, int delta = 1) const;

      /*! Similar to the previous from_byte() except this divides by the \a
         alpha channel then does the conversion, then multiplies by the
         alpha channel again. This corrects the values for premultiplied
         pixels in the common case where the renderer multiplied them in
         8-bit space rather than in linear light levels. If dividing by
         the alpha produces a result greater than 1 it assumes it is 1.

         The alpha channel should be run through Linear::from_byte() to
         get the correct alpha channel.

         \a delta is assumed to apply to both the \a from and \a alpha arrays.

         The input, alpha, and output may be the same memory array as long as \a
         delta is less or equal to 4. The conversion is done in backwards
         order so this is possible.
       */
      void from_byte(float*, const uchar*, const uchar* alpha, int W, int delta = 1) const;

      /*!
         Converts 16-bit data to linear floating point. \a n 1's is considered
         1.0. The resulting floating point numbers are from_byte(255*x/(2^n-1)).
         The current implementation is not really fast and calls the virtual
         from_byte() function for each sample.

         The input data is \a W 16-bit words pointed to by \a
         from. \a delta is the distance between each (for the
         common case where the input is interlaced rgba samples).

         The output is written to the first \a W floats of \a buf.

         The input and output may be the same memory array as long as \a
         delta is less or equal to 2. The conversion is done in backwards
         order so this is possible.
       */
      void from_short(float*, const U16*, int W, int bits = 16, int delta = 1) const;

      /*! Similar to the previous from_byte() except this divides by the \a
         alpha channel then does the conversion, then multiplies by the
         alpha channel again. This corrects the values for premultiplied
         pixels in the common case where the renderer multiplied them in
         8-bit space rather than in linear light levels. If dividing by
         the alpha produces a result greater than 1 it assumes it is 1.

         The alpha channel should be run through Linear::from_byte() to
         get the correct alpha channel.

         \a delta is assumed to apply to both the \a from and \a alpha arrays.

         The input, alpha, and output may be the same memory array as long as \a
         delta is less or equal to 2. The conversion is done in backwards
         order so this is possible.
       */
      void from_short(float*, const U16*, const U16* alpha, int W, int bits = 16, int delta = 1) const;

      /*!
         Convert an array of \a W floating-point values by calling toFloatFast()
         and write them to \a to separated by \a delta.
       */
      void to_float(float*, const float*, int W, int delta = 1) const;

      /*!
         Convert an array of floating-point values from the 0-1 linear range
         by calling toFloatFast(x/a)*a where a is the value from the alpha
         arrays. I.e. unpremultliply by alpha, convert, and then
         multiply back.
       */
      void to_float(float*, const float*, const float* alpha, int W, int delta = 1) const;

      /*!
         Convert an array of \a W floating point values separated by \a delta
         using fromFloatFast() and write them to \a dst, separated by 1. \a src
         and \a dst may point to the same memory.
       */
      void from_float(float*, const float*, int W, int delta = 1) const;

      /*!
         Convert using fromFloatFast(x/a)*a, where a is the values from the
         alpha array. I.e. unpremultiply, convert, then multiply again.
         \a src and \a dst may point to the same memory.
       */
      void from_float(float*, const float*, const float* alpha, int W, int delta = 1) const;

      /*! \fn float LUT::fromFloat(float) const
         Calls from_byte(x*255), and thus converts a single floating point
         value accurately to linear. This may be slow, see fromFloatFast().
       */
      float fromFloat(float v) const { return from_byte(v * 255); }

      /*!
         Converts a single floating point value to linear by using the
         lookup tables. The value is clamped to the 0-1 range as that is
         the range of the tables!
       */
      float fromFloatFast(float v) const;

      /*! \fn float LUT::toFloat(float) const
         Calls to_byte(x)*255, and thus converts a single floating point
         linear value accurately. This may be slow, see toFloatFast().
       */
      float toFloat(float v) const { return to_byte(v) / 255; }

      /*!
         Converts a single floating point value from linear to the LUT
         space by using the lookup tables.
       */
      float toFloatFast(float v) const;

      /*! Return a pointer to a LUT that converts colors linear. */
      static LUT* Linear();

      //*! Modify the array of luts that builtin() returns
      static void setBuiltin(BuiltinLUTId n, const char* name, LUT* lut);

      /*!
         Return LUT based on enumeration. If you create an Enumeration_knob
         using the builtin_names then you can pass the index it stores
         as an argument to this function to get the chosen LUT.

         If you pass zero then getLut(MONITOR) is returned, however this is
         supposed to indicate that you are to intelligently choose based on
         data type and call getLut().
       */
      static LUT* GetBuiltinLUT(BuiltinLUTId id);

      /*! Returns the BuiltinLUT named "name". 
      If no match is found, returns the LUT 
      associated with the MONITOR datatype.
      */
      static LUT* GetBuiltinLutByName(const char* name);

      static LUT* Builtin(BuiltinLUTId n, Reader*);
      static LUT* Builtin(const char* name, Reader*);

      static LUT* Builtin(BuiltinLUTId n, Writer*);
      static LUT* Builtin(const char* name, Writer*);
      /*!
         Return a null-terminated array of string names of the built-in LUT
         types as used by builtin(). This can be used to provide labels for
         an Enumeration_knob.  The first entry says "auto-detect", but you
         can skip this by adding 1 to the returned value.
       */
      static const char* builtin_names[];

      /*!
         Same as builtin(n)->append(hash), except if n==0 it will append
         the hash for all the types of data, on the assumption that the one
         to be chosen will be one of them. This is to generate a hash that
         will change at the right times for file readers.
       */
      static void append(BuiltinLUTId n, Hash&);

      /*!
         Return the LUT to use by default for the given data type.
       */
      static LUT* GetDefaultLutForType(DataType);
      static LUT* GetLut(DataType, const Reader*);
      static LUT* GetLut(DataType, const Writer*);

      //*! Modify above table and hint to the Reader which LUT it wants to use
      static void SetLut(DataType, LUT*);

      static LUT* sRGB() { return GetDefaultLutForType(MONITOR); } // for back-compatibility

    private:
      static LUT* GetLut(DataType, const LUTInfo*);

      static LUT* Builtin(BuiltinLUTId, const LUTInfo*);
      static LUT* Builtin(const char*, const LUTInfo*);
    };

    /*! \class DD::Image::Linear
        \relates DD::Image::LUT

        This class is designed to resemble the interface to the class LUT,
        but simply multiplies or divides by 255. This can be done more
        efficiently than the lookup tables in the class LUT, which was
        designed for gamma functions that take longer to calculate.
     */
    class DDImage_API Linear
    {
    public:
      static float from_byte(float f) { return f * (1.0f / 255.0f); }

      /*! Converts \a W bytes to float by dividing them by 255. \a delta
          is the distance between the source bytes to allow them to be in
          an interlaced buffer.
       */
      static void from_byte(float*, const uchar*, int W, int delta = 1);

      /*! Converts \a W 16-bit samples to float by dividing them by 2^n-1. \a delta
          is the distance between the source words to allow them to be in
          an interlaced buffer.
       */
      static void from_short(float*, const U16*, int W, int bits = 16, int delta = 1);
      static void from_float(float*, const float*, int W, int delta = 1);

      static float to_byte(float f) { return f * 255.0f; }

      /*!
         Converts \a W floats to bytes by doing clamp(float)*255.
         Error diffusion is applied so that a much smoother and more accurate
         representation of the original values is stored, getting rid of
         posterizing effects.

         \a W is the number of pixels to convert.  \a delta is the distance
         between the output bytes (useful for interlacing them into a buffer
         for screen display).

         The input and output buffers must not overlap in memory.
       */
      static void to_byte(uchar*, const float*, int W, int delta = 1);

      /*!
         Converts \a W floats to clamp(float)*(2^n-1) and stores the result
         in \a buf. \a delta is the distance apart to store the results,
         allowing the output buffer to be interlaced.

         This should do error diffusion but that is not yet implemented.
       */
      static void to_short(U16*, const float*, int W, int bits = 16, int delta = 1);
      static void to_float(float*, const float*, int W, int delta = 1);
      static void to_float_cust(float* dst, const float* src, int W, unsigned int destDelta = 1, unsigned int srcDelta = 1);

      static float fromFloat(float v) { return v; }
      static float fromFloatFast(float v) { return v; }
      static float toFloat(float v) { return v; }
      static float toFloatFast(float v) { return v; }

    };

  }
}

#endif // DDImage_LUT_h

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
