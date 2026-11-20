#ifndef DDImage_Info2D_h
#define DDImage_Info2D_h

#include "DDImage_API.h"

#include "DDImage/GeneralInfo.h"
#include "DDImage/Format.h"
#include "DDImage/Box.h"

namespace DD
{
  namespace Image
  {

      /*! \class DD::Image::Info2D

         This structure contains some of the information that Iop::_validate() and DeepOp::_validate()
         must calculate. Because this information is often copied unchanged
         or with tiny variations from the input operator, it is put into
         this structure so that it can be copied with a single assignment.

         This is a subclass of Box and that provides x,y,r,t
         values that describe the bounding box around all the interesting
         pixels (that is, the region of definition). It also provides a merge
         and intersect operators that are useful when _validate is combining
         the info from two inputs.
       */
    class DDImage_API Info2D : public GeneralInfo, public Box
    {
    public:
      /** constructor
       */
      Info2D();

      void setFormats(const FormatPair& formats)
      {
        _format = formats.format();
        _fullSizeFormat = formats.fullSizeFormat();
      }

      FormatPair formats() const
      {
        return FormatPair(_format, _fullSizeFormat);
      }

      /*! \fn const Format& DD::Image::IopInfo::format() const
        
        The image size/format. Operators that care (such as file writers)
        should use this as the size of the image. It can also be used
        as a boundary for an operator that draws data that logically should
        extend to infinity.
        
        The format also has a name and pixel aspect ratio that may be
        useful. Note that Nuke will have already taken the pixel aspect
        ratio into account when storing WH_knob or other dimensional
        knobs.
        
        This is also set by the constructor and can be referred to to
        initialize knob values. For instance if you want an x value to
        default to half way across the image, set it to
        (format().x()+format.r())/2. This format is a \e guess and it
        may be different when the user hooks up the node or finally
        stores values in the knobs.
      */
      const Format& format() const { return *_format; }
      
      /*! \fn void Info2D::format(const Format& f)
        Sets info_.format() to the given format. Some operators expect
        this to tell them how to size their output. Most operators will
        overwrite this in their _validate() method.
      */
      void format(const Format& v) { _format = &v; }
      
      /*! \fn const Format& DD::Image::Info2D::full_size_format() const
        
        If the output of this operator is a proxy for a full size image,
        this is the full size. Nuke assumes all coordinates are given in
        the full size and scales them to the format() when storing them
        into the knob locations. Because Nuke does the scaling most
        operators can ignore this, it is occasionally useful for
        undoing the scaling to get the user's original numbers.
        
        If this is not a proxy then this should point at the same location
        as format().
      */
      const Format& full_size_format() const { return *_fullSizeFormat; }
      
      /*! \fn void Info2D::full_size_format(const Format& f)
        Sets info_.full_size_format() to the given format. Some operators
        expect this to tell them how to size their output. Most operators
        will overwrite this in their _validate() method.
      */
      void full_size_format(const Format& v) { _fullSizeFormat = &v; }
            
      const DD::Image::Box& box() const
      {
        return *this;
      }

      DD::Image::Box& box()
      {
        return *this;
      }

      void setBox(const DD::Image::Box& box)
      {
        set(box);
      }

      /*! \fn int DD::Image::Info2D::ydirection() const

        Indicates a preferred direction to ask for the lines in. Some
        operators (ie file readers) can produce the lines in one direction
        much faster, usually because reading line N requires reading all
        lines from 0..N-1 first. Thus if you go by increasing N the first
        line is returned immediately, but if you go backwards there is a
        long delay before the first line is produced. Notice that positive
        means "up" as line 0 is at the bottom.

        Larger numbers indicate how "bad" it is. For sgi files it uses +/-
        1 because reading in the wrong direction just means it will do
        seek() between each line. For jpeg it is -100 because it has to
        decode all lines above line N.

        If your operator flips the picture upsidedown it should negate the
        y direction, otherwise leave it unchanged. If it reads from
        several inputs it should add all the ydirections together.
      */
      int  ydirection() const { return _ydirection; }
      void ydirection(int v) { _ydirection = v; }

      enum AccessPreference { eAccessPreferNone, eAccessPreferLinear } ;

      void setAccessPreference(AccessPreference ap) { _accessPreference = ap; }

      /*! \fn AccessPreference& DD::Image::Info2D::accessPreference() const

        Indicates a preferred access pattern to ask for the lines in. Some
        operators operate full frame and prefer to be asked for lines from
        the very top of the request area rather than in the middle.

        Operators can call Info2D::setAccessPreference to set their access
        preference.  eAccessPreferLinear will cause the viewer to ask for
        lines always from the top or bottom of the request area first, ignoring
        any user mouse click.

        The default is eAccessPreferNone, indicating this operator has no
        access preference.
      */
      const AccessPreference& accessPreference() const { return _accessPreference; }

    private:
      const Format* _format;
      const Format* _fullSizeFormat;

      int _ydirection;
      AccessPreference _accessPreference;
    };

    class DDImage_API Info2DProvider : public GeneralInfoProvider
    {
    public:
      ~Info2DProvider() override;

      virtual const Info2D& info2D() const = 0;
    };

  }
}

#endif
