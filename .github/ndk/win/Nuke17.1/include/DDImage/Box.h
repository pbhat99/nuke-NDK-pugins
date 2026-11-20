// Box.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Box_h
#define DDImage_Box_h

#include "DDImage_API.h"

#include <cstdint>

namespace DD
{
  namespace Image
  {

    class Vector2;


    /*! \class DD::Image::Box
       \brief Describes a rectangle with integer boundaries.

       This class is used as a base for everything in DD::Image that
       describes a rectangle of pixels. It contains 4 integers storing
       the left, bottom, right, and top edges of the box.

       Boxes must have a size of at least 1 in both dimensions. This
       is so they always contain at least one pixel. This pixel is
       used to provide a value when locations are indexed outside
       the box. If you set a box to a size less than one then the
       result is unpredictable.

       In DD::Image integer coordinates are considered to be at the
       \e corners of the pixels, not in their centers. When addressing
       a pixel by integers the pixel to the upper-right of the integer
       coordinate is chosen. This is why the right and top edge appear
       to indicate a pixel outside the box. In fact all 4 edges are
       treated the same and they represent the positions around the
       outside edge of the pixels.
     */
    class DDImage_API Box
    {
      int x_; /*!< Left edge */
      int y_; /*!< Bottom edge */
      int r_; /*!< Right edge */
      int t_; /*!< Top edge */
    public:

      /* box iterator, used to iterate over the area of a box.
         /y/ and /x/ are defined at each position.  Ordering will
         be bottom-to-top, left-to-right */
      class DDImage_API iterator {
      public:
        int y;
        int x;
        
        iterator(int y, int x, int l, int r) : y(y), x(x), l(l), r(r) { }
        
        void operator++() {
          x++;
          if (x == r) {
            x = l;
            y++;
          }
        }
        
        void operator++(int) {
          x++;
          if (x == r) {
            x = l;
            y++;
          }
        }
        
        bool operator!=(const iterator& other) const
        {
          return y != other.y || x != other.x;
        }

      private:
        int l;
        int r;
      };

      typedef iterator const_iterator;

      iterator begin() const
      {
        return iterator(y_, x_, x_, r_);
      }

      iterator end() const
      {
        return iterator(t_, x_, x_, r_);
      }

      Box() : x_(0), y_(0), r_(1), t_(1) {}

      Box(int x, int y, int r, int t) : x_(x), y_(y), r_(r), t_(t) {}

      Box(const Box &b) { *this = b; }

      int x() const { return x_; } /*!< Location of left edge */
      void x(int v) { x_ = v; } /*!< Set left edge */

      int y() const { return y_; } /*!< Location of bottom edge */
      void y(int v) { y_ = v; } /*!< Set bottom edge */

      int r() const { return r_; } /*!< Location of right edge */
      void r(int v) { r_ = v; } /*!< Set right edge */

      int t() const { return t_; } /*!< Location of top edge */
      void t(int v) { t_ = v; } /*!< Set top edge */

      int w() const { return r_ - x_; } /*!< width, r()-x() */
      void w(int v) { r_ = x_ + v; } /*!< Set width by moving right edge */

      int h() const { return t_ - y_; } /*!< height, t()-y() */
      void h(int v) { t_ = y_ + v; } /*!< Set height by moving top edge */

      float center_x() const { return (x_ + r_) / 2.0f; } /*!< center in x */

      float center_y() const { return (y_ + t_) / 2.0f; } /*!< center in y */

      /*! Set all four sides at once. */
      void set(int x, int y, int r, int t) { x_ = x;
                                             y_ = y;
                                             r_ = r;
                                             t_ = t; }

      /*! Set all four sides at once. */
      void set(const Box& b) { *this = b; }

      /*! True if box is 1x1 in both directions. */
      bool is_constant() const { return r_ <= x_ + 1 && t_ <= y_ + 1; }

      void clear() { x_ = y_ = 0;
                     r_ = t_ = 1; }        /*!< Set to is_constant() */

      /*! Move all the sides and thus the entire box by the given deltas */
      void move(int dx, int dy) { x_ += dx;
                                  r_ += dx;
                                  y_ += dy;
                                  t_ += dy; }

      /*! Move x(), y(), r(), t() by the given deltas, except if a
         dimension is 1 it does not move those edges, because the color is
         constant in that direction. This is used to pad out an input image
         with the spill that a filter will create. Notice that \a dx and \a
         dy are negative to make the box bigger. */
      void pad(int dx, int dy, int dr, int dt)
      {
        if (r_ > x_ + 1) { x_ += dx;
                           r_ += dr; }
        if (t_ > y_ + 1) { y_ += dy;
                           t_ += dt; }
      }

      /*! Same as pad(-dx,-dy,dx,dy), add the same amount on all sides. */
      void pad(int dx, int dy) { pad(-dx, -dy, dx, dy); }

      /*! Same as pad(-d,-d,d,d), add the same amount on all sides. */
      void pad(int d) { pad(-d, -d, d, d); }

      /*! Return x restricted to pointing at a pixel in the box. */
      int clampx(int x) const { return x <= x_ ? x_ : x >= r_ ? r_ - 1 : x; }

      /*! Return y restricted to pointing at a pixel in the box. */
      int clampy(int y) const { return y <= y_ ? y_ : y >= t_ ? t_ - 1 : y; }


      /*!
         Change this box to be the union of itself and the passed box.

         This is for bounding boxes where it is assumed the edge pixels
         extend out to infinity. 1-wide or 1-tall boxes cause the other's
         dimensions to be used unchanged because these indicate an infinite
         area of constant color and thus their position is irrelevant. If
         both boxes are 1 wide or 1 tall then the passed box's position
         is used.

         The output is never an "empty" box which is now illegal for
         bounding boxes. If both boxes are <= 0 wide or tall then the passed
         box's postition is used and the right/top edge are set to x/y
         plus one.
       */
      void merge(const Box&);


      /*!
         Similar to merge(x,y,x+1,y+1) but it always expands the box, even
         if it describes a constant. This allows you to build a box out of
         a bunch of pixels that you plan to index.
       */
      void merge(int x, int y);


      /*! Same as merge(Box(x,y,r,t)) */
      void merge(int x, int y, int r, int t);
      
      /*!
        Expand the bbox by the given amount in all directions around the centre.
        Negative values will shrink the bbox.
      */
      void expand(int amount);
      
      /*!
        Expand the bbox by the given amount for both dimensions around the centre.
        Negative values will shrink the bbox.
      */
      void expand(int widthAmount, int heightAmount);
      
      /*!
        Scale the bbox by the ratio for both dimensions around the centre.
        1.0 is the current size.
      */
      void scale(float scaleAmount);
      
      /*!
         Change this box to be the intersection of itself and the passed
         box.

         DD::Image replicates pixels from the nearest edge of the bounding
         box when selecting coordinates outside it. This affects how boxes
         intersect, because a box with zero dimensions is not allowed. If
         the boxes don't intersect the result will still have 1 pixel
         in it. The resulting location of this box could be anywhere, but
         this function places it inside the edge of the passed box \a v.
       */
      void intersect(const Box&);


      /*! Same as intersect(Box(x,y,r,t)) */
      void intersect(int x, int y, int r, int t);

      /*!
        Check if this box intersects the other box
      */
      bool intersects(const Box& other) const;

      /*! 
        Same as intersects(Box(x,y,r,t)) 
        */
      bool intersects(int x, int y, int r, int t) const;

      /*!
        Check if the other box is completely inside this box
      */
      bool contains(const Box& other) const;

      int64_t area() const {
        return static_cast<int64_t>(w()) * static_cast<int64_t>(h());
      }
    };

    /// equality of boxes
    inline bool operator==(const DD::Image::Box& b1, const DD::Image::Box& b2)
    {
      return b1.x() == b2.x() &&
        b1.y() == b2.y() &&
        b1.r() == b2.r() &&
        b1.t() == b2.t();
    }

    /// inequality of boxes
    inline bool operator!=(const DD::Image::Box& b1, const DD::Image::Box& b2)
    {
      return b1.x() != b2.x() ||
        b1.y() != b2.y() ||
        b1.r() != b2.r() ||
        b1.t() != b2.t();
    }

  }
}

#endif

// end of Box.h
