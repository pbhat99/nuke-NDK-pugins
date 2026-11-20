#ifndef DDIMAGE_FRAMEEXTENTPROVIDERI_H
#define DDIMAGE_FRAMEEXTENTPROVIDERI_H
#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {
    /*!
     * This interface is used for Ops that can provide frame extent information,
     * defining the frame range over which they operate.  It is called in order to
     * determine clip range bars in the dope sheet.
     */
    class DDImage_API FrameExtentProviderI
    {
    public:
      virtual ~FrameExtentProviderI();

      /*!
       * Returns the first frame in the frame extent
       */
      virtual double getFirstFrame() const = 0;

      /*!
       * Returns the last frame in the frame extent
       */
      virtual double getLastFrame() const = 0;

      /*!
       * Sets the new position of the first frame
       */
      virtual void setFirstFrame(double value) = 0;

      /*!
       * Sets the new position of the last frame
       */
      virtual void setLastFrame(double value) = 0;

      /*!
       * Sets the new position of the centre of the range.
       * Typically this means moving the whole clip,
       * so the first and last frames will move accordingly.
       */
      virtual void setCentre(double value) = 0;
    };
  }
}

#endif
