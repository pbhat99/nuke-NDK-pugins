// ProxyContext.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.
// 
// Storage for the proxy aspects of OutputContext, should not be instantiated directly.
//

#ifndef DDImage_ProxyContext_h
#define DDImage_ProxyContext_h

#include "DDImage_API.h"
#include "Vector2.h"
#include "math.h"
#include <string>
#include "Box.h"

namespace DD
{
  namespace Image
  {
    class Format;
    class Hash;
    class Matrix4;


    /*! \class DD::Image::ProxyContext

        See also class DD::Image::OutputContext

        This is the part out OutputContext which deals with proxy and
        should NOT be instantiated.

        It is intentionaly non-virtual in order to not loose any performance
        and does NOT contain any allocations that would otherwise need cleaning
        up.

        It contains a ProxySetting to denote how to Scale and a representation 
        of a 3x2 matrix with b,c missing, to denote Scale and Translation
        properties.
    */
    class DDImage_API ProxyContext
    {
    public:

      enum ProxySetting {
        PROXY_OFF = 0,    //!< Nuke's proxy mode is turned off
        PROXY_NEVER,      //!< Always use full-size image
        PROXY_SCALE_DOWN, //!< Use smallest image >= desired size, or largest if none
        PROXY_NEAREST,    //!< Use image closest to the desired size
        PROXY_ALWAYS      //!< Always use proxy image
      };

    protected:

      /**! Protected C'tor
       *   Should only be constructed by children such as OutputContext
       */
      ProxyContext()
        : _scale(1.0f, 1.0f)
        , _translate(0.0f, 0.0f)
        , _proxy(PROXY_OFF)
      {}


    public :

      ProxySetting proxy() const { return ProxySetting(_proxy); }

      /*! \fn void OutputContext::setProxy(OutputContext::ProxySetting)
         Change how proxy vs full-size files are chosen in the Read operator.
         This decision is actually done by code in Nuke. This is actually an
         independent setting from setProxyscale() but normally the the scale
         is not set unless this is turned on as well.
       */
      void setProxy(ProxySetting v) { _proxy = int(v); }

      float scale_x() const { return _scale.x; }
      float scale_y() const { return _scale.y; }
      float translate_x() const { return _translate.x; }
      float translate_y() const { return _translate.y; }

      //! Set all proxy parameters.
      void set(ProxySetting v, float sx, float sy, float tx, float ty) { _proxy = int(v);
                                                                         _scale.x = sx;
                                                                         _scale.y = sy;
                                                                         _translate.x = tx;
                                                                         _translate.y = ty; }

      /*! \fn void OutputContext::setProxyscale(float scale);
         Set to_proxy() to scale coordinates by the given factor (which
         should be less or equal to 1) and reset the translation (to zero).
       */
      void setProxyscale(float scale) { _scale.x = _scale.y = scale;
                                        _translate.x = _translate.y = 0; }
      void setProxyscale(float sx, float sy) { _scale.x = sx;
                                               _scale.y = sy;
                                               _translate.x = _translate.y = 0; }

      /*!
         Set the scaling to the conversion from \a full to \a proxy.
       */
      void setProxyscale(const Format& fullsize, const Format& proxysize);

      /*!
         Set the scaling to the transform in the matrix (which had better only
         have scale and translations).
       */
      void setProxyscale(const Matrix4&);

      /*! \fn void OutputContext::scale(float scale);
         Multiply the proxy scaling by the factor (which is normally less or equal
         to 1). The picture output by the operator will be scaled by this factor.
       */
      void scale(float scale) { _scale.x *= scale;
                                _scale.y *= scale;
                                _translate.x *= scale;
                                _translate.y *= scale; }
      void scale(float sx, float sy) { _scale.x *= sx;
                                       _scale.y *= sy;
                                       _translate.x *= sx;
                                       _translate.y *= sy; }

      /*! \fn void OutputContext::translate(float x, float y);
         Translate the to_proxy_xy() by the given number of pixels. The picture
         output by the operator will be translated by this factor.
       */
      void translate(float x, float y) { _translate.x += x;
                                         _translate.y += y; }
      bool isIdentity() const { return _scale.x == 1 && _scale.y == 1 && !_translate.x && !_translate.y; }

      /*!
         Multiply the matrix by the transformation from full-size space to
         proxy space. If the matrix was the identity, it will now be a
         transform from full size to proxy.
       */
      void to_proxy(Matrix4&) const;

      /*! \fn void OutputContext::to_proxy_wh(double& w, double& h) const
         Transform a vector/distance from full size to proxy space. This is
         different than to_proxy_xy if there is a translation of the origin. */
      void to_proxy_wh(double& w, double& h) const { w *= _scale.x;
                                                     h *= _scale.y; }
      void to_proxy_wh(float& w, float& h) const { w *= _scale.x;
                                                   h *= _scale.y; }

      double to_proxy_w(double w) const { return w * _scale.x; }
      double to_proxy_h(double w) const { return w * _scale.y; }

      /*! \fn void OutputContext::to_proxy_xy(double& x, double& y) const
         Transform an xy position from full size to proxy space. */
      void to_proxy_xy(double& x, double& y) const { x = x * _scale.x + _translate.x;
                                                     y = y * _scale.y + _translate.y; }
      void to_proxy_xy(float& x, float& y) const { x = x * _scale.x + _translate.x;
                                                   y = y * _scale.y + _translate.y; }

      double to_proxy_x(double w) const { return w * _scale.x + _translate.x; }
      double to_proxy_y(double w) const { return w * _scale.y + _translate.y; }

      double from_proxy_x(double x) const { return (x - _translate.x) / _scale.x; }
      double from_proxy_y(double y) const { return (y - _translate.y) / _scale.y; }

      void to_proxy_box(DD::Image::Box& box) const
      {
        double x = box.x();
        double y = box.y();
        double r = box.r();
        double t = box.t();
        to_proxy_xy(x, y);
        to_proxy_xy(r, t);
        box.x((int)x);
        box.y((int)y);
        box.r((int)r + 1);
        box.t((int)t + 1);
      }

      /*! \fn void OutputContext::to_proxy_area(double& w, double& h) const;
         Return the size of a rectangle with the same area as the 0,0,w,h
         rectangle transformed to proxy scale, and approximatly the same shape.
         This is the same as to_proxy_wh as long as there is no
         rotation. The shape exactly matches as long as rotations are 90 degrees.
       */
      void to_proxy_area(double& w, double& h) const { w *= _scale.x;
                                                       h *= _scale.y; }

      /*! \fn double OutputContext::to_proxy_area(double area) const;
         Return the area of a shape of the given area when converted from
         full size to proxy space.
       */
      double to_proxy_area(double area) const { return fabs(area * _scale.x * _scale.y); }

      /*!
         Replace the contents of \a to with the scaled version of \a from.
         Integer values are rounded to the nearest match. The name of the
         format is set to null.
       */
      void to_proxy(const Format& from, Format& to) const;

      /*!
         \a from is the result of rel.to_proxy. Undoes the rel.to_proxy, and
         then applies this.to_proxy to the result. If this results in \a from
         then \a from is returned. Otherwise the result is written over \a to
         and that is returned. This is a weird action used by Nuke.
       */
      const Format* to_proxy_rel(const Format& from, Format& to, const ProxyContext&) const;

      /*! \fn double OutputContext::to_proxy_pixel_aspect(double pa) const
         Convert a pixel aspect. I.e if an image with pixel aspect of \a pa
         is rescaled by the proxy scaling, it's pixel aspect will be this.
       */
      double to_proxy_pixel_aspect(double pa) const { return pa * _scale.y / _scale.x; }

      /*!
         Multiply the matrix by the transformation from proxy space to
         full-size space. If the matrix was the identity, it will now be
         a transform from proxy to full-size;
       */
      void from_proxy(Matrix4&) const;

      /*! \fn void OutputContext::from_proxy_xy(double& x, double& y) const
         Transform an xy position from proxy space to full size. */
      void from_proxy_xy(double& x, double& y) const { x = (x - _translate.x) / _scale.x;
                                                       y = (y - _translate.y) / _scale.y; }

      /*! \fn void OutputContext::from_proxy_wh(double& w, double& h) const {
         Transform a vector/distance from proxy space to full size. This is
         different than from_proxy_xy if there is a translation of the origin. */
      void from_proxy_wh(double& w, double& h) const { w /= _scale.x;
                                                       h /= _scale.y; }
      void from_proxy_xy(float& x, float& y) const { x = (x - _translate.x) / _scale.x;
                                                     y = (y - _translate.y) / _scale.y; }
      void from_proxy_wh(float& w, float& h) const { w /= _scale.x;
                                                     h /= _scale.y; }

      /*! \fn double OutputContext::from_proxy_area(double area) const;
         Return the area of a shape of the given area when converted from
         proxy space to full size.
       */
      double from_proxy_area(double area) const { return fabs(area / (_scale.x * _scale.y)); }

      double from_proxy_pixel_aspect(double pa) const { return pa / _scale.y * _scale.x; }


      //! Append the values affecting proxy scaling to the hash (i.e. not frame or view)
      void appendProxy(Hash& hash) const;

    protected :

      // not overloading operator== as OutputContext provides that functionality
      bool isEqualTo(const ProxyContext& other) const
      {
        return _proxy == other._proxy &&
               _scale.x == other._scale.x &&
               _scale.y == other._scale.y &&
               _translate.x == other._translate.x &&
               _translate.y == other._translate.y;
      }

      // not overloading operator< as OutputContext provides that functionality
      bool isLessThan(const ProxyContext& other) const
      {
        if (_scale.x < other._scale.x)
          return true;

        if (_scale.x > other._scale.x)
          return false;

        if (_scale.y < other._scale.y)
          return true;

        if (_scale.y > other._scale.y)
          return false;

        if (_translate.x < other._translate.x)
          return true;

        if (_translate.x > other._translate.x)
          return false;

        if (_translate.y < other._translate.y)
          return true;

        if (_translate.y > other._translate.y)
          return false;

        if (_proxy < other._proxy)
          return true;

        if (_proxy > other._proxy)
          return false;

        return false;
      }

    private :

      bool operator<(const ProxyContext& other) const; // private and not implemented. You should go via OutputContext (or use isLessThan() instead).
      bool operator>(const ProxyContext& other) const; // private and not implemented. You should go via OutputContext (or use isLessThan() instead).
      bool operator==(const ProxyContext& other) const; // private and not implemented. You should go via OutputContext (or use isEqualTo() instead).
      bool operator!=(const ProxyContext& other) const; // private and not implemented. You should go via OutputContext (or use isEqualTo() instead).

      friend DDImage_API std::ostream& operator << (std::ostream& o, const ProxyContext&);

    private:
      DD::Image::Vector2 _scale; // x = a, y= d
      DD::Image::Vector2 _translate; // x = e, y = f
      int _proxy;       // not an enum so the structure is packed for hash

    };

  }
}

#endif // DDImage_ProxyContext_h

