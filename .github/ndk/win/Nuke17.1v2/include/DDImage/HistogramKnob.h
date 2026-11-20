// Histogram_Knob.h

/*
** Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.
*/

#ifndef DD_Image_Histogram_Knob_H
#define DD_Image_Histogram_Knob_H
#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {
    /*!
     * The definition of the data as used by the Histogram_Knob. This class is
     * reference counted internally, so you should call removeUser() not delete.
     */
    class DDImage_API Histogram_Data
    {
      int version;
      int user; //!< Reference count, for smart pointing.
      int lumIn[258];
      int lumOut[258];
      double* cc;
      //! This class is reference counted.
      ~Histogram_Data();
      public:
      Histogram_Data(double* CC = nullptr) 
      { 
        version = 1;
        user = 1;
        cc = CC;
        clear(); 
      }
      //! Increment the reference count.
      void addUser() { user++; }
      /*!
       * Decrement the reference count. If this reached zero the object is 
       * destroyed.
       */
      void removeUser() { 
        user--;
        if (user == 0)
          delete this;
      }
      //! Blank out the entire histogram.
      void clear() 
      { 
        for (int i = 0; i < 258; i++)
          lumIn[i] = lumOut[i] = 0;
      }
      void addLumIn(float y) 
      { 
        int ix = (int)(y * 255.0f + 0.5f);
        if (ix > 255)
          ix = 257;
        if (ix < 0)
          ix = 256;
        lumIn[ix]++; 
      }
      //! Get the input value at the given index.
      int getLumIn(int ix) { return lumIn[ix]; }
      //! Set an output luminance.
      void addLumOut(float y) 
      { 
        int ix = (int)(y * 255.0f + 0.5f);
        if (ix > 255)
          ix = 257;
        if (ix < 0)
          ix = 256;
        lumOut[ix]++; 
      }
      //! Get the output luminance at the given index.
      int getLumOut(int ix) { return lumOut[ix]; }
      //! Get the maximum input value.
      int getMaxIn() 
      { 
        int m = 0, ix = 0;
        for (int i = 0; i < 256; i++)
          if (lumIn[i] > m) { m = lumIn[i];
            ix = i; 
          }
        return ix; 
      }
      //! Get the maximum output value.
      int getMaxOut() 
      { 
        int m = 0, ix = 0;
        for (int i = 0; i < 256; i++)
          if (lumOut[i] > m) { m = lumOut[i];
            ix = i; 
          }
        return ix; 
      }
      //! Get the maximum output luminance.
      int getMaxLum()
      {
        int m = 0, i;
        for (i = 0; i < 256; i++)
          if (lumIn[i] > m)
            m = lumIn[i];
        for (i = 0; i < 256; i++)
          if (lumOut[i] > m)
            m = lumOut[i];
        return m;
      }
      //! Get the number of in things.
      int getNIn() 
      { 
        int m = 0;
        for (int i = 0; i < 256; i++)
          m += lumIn[i];
        return m; 
      }
      //! Return the list of correction values.
      double* corrector() { return cc; }
      /*!
       * Return a specific correction factor at the given index. If no
       * correction array was specified, it will return 0.0.
       */
      double corrector(int i) { return cc ? cc[i] : 0.0; }
    };
  }
}


#endif
