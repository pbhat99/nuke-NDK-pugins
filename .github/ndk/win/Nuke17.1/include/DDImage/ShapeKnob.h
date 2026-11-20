// ShapeKnob.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ShapeKnob_h
#define DD_Image_ShapeKnob_h

#include "Knob.h"
#include "OutputContext.h"

//------------------------------------------------------------------------------

class ShapeController_Knob;
class ShapeController_Widget;

//------------------------------------------------------------------------------

namespace DD
{
  namespace Image
  {

    //------------------------------------------------------------------------------

    /*! \class DD::Image::ShapeKnob

        This subclass of Knob can be used to manipulate a piece of data
        that changes over time, such as an animated shape. The Bezier
        operator uses this. There is also a demo Polygon plugin that you
        can refer to for an example of the use of this knob.

        You must write code to store a set of "key" shapes. You should
        also store a "current" shape that may be different from any of the
        keys. You also need to write methods to set the current shape to
        an interpolation between two shapes, and to add and delete keys
        and to set a key to the current shape.
     */
    class DDImage_API ShapeKnob : public Knob
    {
      friend class ::ShapeController_Knob;
    public:
      Knob* i; //!< points at a ShapeController_Knob
    private:
      int keys_;
      bool animated_;
      bool autokey_;

      // You must implement these, but don't call them!!!

      /*! \fn void ShapeKnob::set_key(int index)

          Your subclass must implement this. It should replace key \a index
          with the current shape. \a index will be between 0 and keys()-1.
       */
      virtual void set_key(int) = 0;

      /*! \fn void ShapeKnob::add_key(int index)

          Your subclass must implement this. It should create a new key
          at \a index, moving any keys at \a index and higher up one.
          \a index will be between 0 and keys()-1. keys() will already
          be incremented to the new value. You can leave the key filled
          with garbage because set_key() will be called immediately
          after this.
       */
      virtual void add_key(int) = 0;

      /*! \fn void ShapeKnob::delete_keys(int index, int count)

          Your subclass must implement this. It should delete keys from \a
          index through \a index+count-1. You can assume that \a index is
          greater than zero, \a count is greater than zero, and \a
          index+count is less or equal to the former value of keys(). keys()
          will already be decremented by \a count before calling this.  You
          can also assume the resulting number of keys will be at least
          one.
       */
      virtual void delete_keys(int first, int number) = 0;
      virtual bool is_animated(int) const;

    protected:

      /*! Do one of DELETE_KEY, MAYBE_SET_KEY, SET_KEY, ADD_KEY,
          NO_ANIMATIONS, AUTOKEY_ON, or AUTOKEY_OFF, depending on the \a
          event. This is useful to make a menu() that does these actions.  */
      bool handle(int event, double frame);

      /*! \fn bool ShapeKnob::goto_shape(float shape, const DD::Image::OutputContext& context)

          Your subclass must implement this. It should set the "current"
          shape to an interpolation between two keys. Return true if the
          new current shape is different than the previous current shape.

          Which keys and how much to interpolate them is determined by
          shape().
       */
      virtual bool goto_shape(float shape, const DD::Image::OutputContext& context) = 0;

      bool goto_context(const DD::Image::OutputContext& oc)
      {
        return goto_shape(shape(oc.frame()), oc);
      }

      bool goto_frame(float frame)
      {
        DD::Image::OutputContext oc = uiContext();
        oc.setFrame(frame);
        return goto_shape(shape(frame), oc);
      }

      /*! Change the number of keys stored in the shape curve and the value
          returned by keys(). You must reallocate your own storage of keys
          to match before calling this (it does not callback add_key() or
          delete_keys()).

          If \a n is less than keys() then the trailing keys are deleted.
          If \a n is greater than keys() then \a delta is used as a spacing
          in time between each of them and between the first new one and the
          last old key. If no time has been set for any keys than delta is
          used as the time for the first one.

          Usually you call this in from_script() to set the number of keys.

          Returns true if \a n is different than keys().
       */
      bool keys(int n, float delta = 1);

      /*! \fn bool ShapeKnob::set_key_at(double t);
         Stores the current shape as a new key at given time. NYI...
       */
      bool set_key_at(double time) { return handle(MAYBE_SET_KEY, time); }

      /*! void ShapeKnob::maybe_set_key()

          When the user changes the current shape using the GUI you should
          call this.  If is_animated() is off this will call set_key(0). If
          is_animated() is on and autokey() is on this calls set_key() (which
          will either set the current key or create a new one). If
          is_animated() is on but autokey() is off this does nothing.

          This calls Knob::changed() so that Nuke knows that the display
          in the viewer must update.
       */
      bool maybe_set_key() { return handle(MAYBE_SET_KEY, uiContext().frame()); }

      /*! \fn bool ShapeKnob::no_animation()

          Make the node be non-animated by deleting all but one keys,
          setting the remaining key to the current shape, destroying the
          shape curve so it is not in the curve editor, and turning off
          autokey() and animated(). This pops up an "are you sure?" question
          if it looks like this will destroy useful animation.  */
      bool no_animation() { return handle(NO_ANIMATION, 0.0); }

      /*   bool delete_key() {return handle(DELETE_KEY);} */
      /*   bool set_key_now() {return handle(SET_KEY);} */
      /*   bool add_key_at() {return handle(ADD_KEY);} */
      /*   bool autokey(bool v) {return handle(v?AUTOKEY_ON:AUTOKEY_OFF);} */
      /*   bool copy_key() {return handle(COPY_KEY);} */
      /*   bool paste_key() {return handle(PASTE_KEY);} */
      /*   bool copy_all() {return handle(COPY_ALL);} */
      /*   bool paste_all() {return handle(PASTE_ALL);} */

    public:

      /*! Event numbers for pop-up menu */
      enum {
        DELETE_KEY = 20, MAYBE_SET_KEY, SET_KEY, ADD_KEY,
        NO_ANIMATION, AUTOKEY_ON, AUTOKEY_OFF, EDIT_CURVE,
        COPY_KEY, PASTE_KEY, COPY_ALL, PASTE_ALL,
        SPLIT, UNSPLIT
      };

      /*! This creates a child knob that holds the shape animation.
          The second argument is the name of this knob (and thus the
          name of the curve for the shape in the shape editor). The
          third argument is a label for the shape knob, if not given
          the shapename will be used.
       */
      ShapeKnob(Knob_Closure * cb,
                const char* name,
                const char* shapename = "shape",
                const char* label = nullptr);
      ~ShapeKnob() override;


      /*! \fn int ShapeKnob::keys() const
          Returns how many keys there are. This will be at least 1.
       */
      int keys() const { return keys_; }

      /*! Return the time that key n is at. This can be used to draw
          frame numbers next to a    "path" of a point in the GUI, or can
          be used to vary how the interpolation is done, for instance
          to make the motion smooth in time rather than in key numbers.
       */
      float time(int key) const;

      /*!
         \brief Evaluate the shape curve at time. If the time is between two
         keys, this will return a number between those two key numbers.
         If there are less than 2 keys this always returns 0.
       */
      /*! \fn float ShapeKnob::shape(double time) const;

          Return a number indicating which two keys to interpolate and
          how much to interpolate between them. This is set before goto_shape()
          is called.

          If there are less than 2 keys this will always return 0.0.

          If shape() is an integer (and in range) then the result should be
          exactly equal to some key. Otherwise do a mix between keys
          floorf(shape()) and ceilf(shape()), using
          (shape()-floorf(shape())) as a mixing factor.

          The value may be out of the range from 0 to keys()-1. If this is
          true you should extrapolate the last two or first two keys. If
          that does not make sense for your data you should pretend it
          is equal to the first or the last key.
       */
      float shape(double time) const;

      /*!
          If this returns false then keys() is always 1, shape() is always
          zero, and any changes you make call set_key(0) no matter what
          the frame is. This is the initial state.

          If this returns true than there is a shape curve that the user
          can edit in the curve editor. keys() is greater or equal to 1.

          To turn it on, call set_key() (or call maybe_set_key() while
          autokey is on, or turn on autokey).

          To turn it off call no_animation(). This will pop up a question
          for the user if Nuke thinks something valuable is going to be
          destroyed.
       */
      bool is_animated() const;
      bool isAnimated() const override { return is_animated(); }
      virtual bool keyable() const = 0;

      /*! \fn bool ShapeKnob::autokey() const
          Return true if the autokey checkmark is turned on. */
      bool autokey() const { return autokey_; }

      /*! \fn bool ShapeKnob::from_script(const char* v)

          You will need to implement this. It should figure out from the
          text how many keys there are and call keys() with that number, and
          allocate and store all the data.

          The time of each key is done by the internal shape curve knob.  In
          a normal script this will be parsed afterwards. If the shape curve
          has the wrong number of keys an error will be produced and it will
          be brute-forced to match.
       */
      bool from_script(const char* v) override = 0;

      /*! \fn void ShapeKnob::to_script(std::ostream& o, const OutputContext*, bool quote) const;

          You will need to implement this as well, to write all the
          data in each key in a way that can be parsed by from_script
          and the number of keys can be determined.

          The easiest way is to put {...} around each key so that an
          outer ScriptList will return the number of keys.
       */
      void to_script(std::ostream&, const OutputContext*, bool quote) const override = 0;
      int get_key_list(std::set<int>&) const override;

    };

    //------------------------------------------------------------------------------

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
