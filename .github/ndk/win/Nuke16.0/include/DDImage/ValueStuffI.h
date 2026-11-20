// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ValueStuffI_H
#define DD_Image_ValueStuffI_H

#include <string>

#include "DDImage/DDImage_API.h"
#include "DDImage/GraphScope.h"
#include "DDImage/OutputContext.h"

namespace DD
{
  namespace Image {
    class OutputContext;

    //! Container for values
    class DDImage_API ValueStuffI {
    protected:

      virtual ~ValueStuffI();

      /**
       * contains structures pointed to by the 2nd arg of value_stuff
       */
      union ValueStuffArg {
        struct SetValueArg { double v;
                             int channel; } set_value;
        struct SetValueAtArg { double v, t;
                               int channel; } set_value_at;
        struct SetValuesArg { const double* array;
                              int n; } set_values;
        struct SetValuesAtArg { const double* array;
                                int n;
                                double time; } set_values_at;
        struct SetValuesFloatArg { const float* array;
                                   int n; } set_values_float;
        struct SetValuesAtFloatArg { const float* array;
                                     int n;
                                     double time; } set_values_at_float;
        struct SetValuesIntArg { const int* array;
                                   int n; } set_values_int;
        struct SetValuesAtIntArg { const int* array;
                                     int n;
                                     double time; } set_values_at_int;
        struct SetValuesInt64Arg { const U64* array;
                                   int n; } set_values_int64;
        struct SetValuesAtInt64Arg { const U64* array;
                                     int n;
                                     double time; } set_values_at_int64;
        struct SetValueAtViewArg { double v, t;
                                   int view;
                                   int channel; } set_value_at_view;
        struct SetKeyArg { int channel;
                           int view; } set_key;

        struct SetKeyAtArg {  double t; 
                              int channel;
                              int view; } set_key_at;

        struct RemoveKeyAtArg { double t;
                                int channel;
                                int view; } remove_key_at;
        struct SetRangeArg { double a, b;
                             bool force; } set_range;
        struct SetAnimationArg { const char* a;
                                 int channel; } set_animation;
        struct IsKeyAtArg { double t;
                            int channel;
                            int view; } is_key_at;
        struct GetValueArg { int channel;
                             double r; } get_value;
        struct GetValueAtViewArg { double t;
                                   int view;
                                   int channel;
                                   double r; } get_value_at_view;
        struct GetValueAtContextArg { double t;
                                      int view;
                                      U64 graphScopeHash;
                                      int channel;
                                      double r;
                                      GraphScopePtr graphScopePtr() {
                                        return OutputContext::graphScope(GraphScopeId(graphScopeHash));
                                      };
                                    } get_value_at_context;
        struct SetExpressionArg { const char* v;
                                  int channel;
                                  int view; } set_expression;
        struct GetNumKeysArg { int view;
          int channel; } get_num_keys;
        struct GetKeyIndexArg { int view;
                                double t;
                                int channel; } get_key_index;
        struct GetKeyTimeArg { int view;
                               int keyindex;
                               int channel;
                               double r; } get_key_time;
        struct GetDerivativeArg  { int view;
                                   double t;
                                   int channel;
                                   int n;
                                   double r; } get_derivative;
        struct GetInverseArg { int view;
                               double value;
                               int channel;
                               int n;
                               double r; } get_inverse;
        struct GetIntegralArg  { int view;
                                 double t1;
                                 double t2;
                                 int channel;
                                 double r; } get_integral;
        struct SplitViewArg { int view; } split_view;
        struct UnsplitViewArg { int view; } unsplit_view;
        struct ClearAnimatedViewArg { int view;
                                      int index; } clear_animated_view;
        struct SetAnimatedViewArg { int view;
                                    int index; } set_animated_view;
        struct IsAnimatedViewArg { int view;
                                   int index; } is_animated_view;

        struct GetKeyFrameArg  { int view;
                                 int keyindex;
                                 int channel;
                                 double x, y, lslope, rslope, la, ra;
                                 unsigned char interp, extrap; } get_keyframe;
        struct SetKeyFrameArg  { int view;
                                 int keyindex;
                                 int channel;
                                 double x, y, lslope, rslope, la, ra;
                                 unsigned char interp, extrap; } set_keyframe;
        struct AddKeyFrameArg  { int view;
                                 int channel;
                                 double x, y, lslope, rslope, la, ra;
                                 unsigned char interp, extrap; } add_keyframe;
        struct GetExpressionArg { int view;
                                  std::string* expr;
                                  int channel; } get_expression;
      };

      enum { // arguments to value_stuff
        W_SETVALUES_DBL, 
        W_SETVALUE, 
        W_SETRANGE, 
        W_SETVALUESAT_DBL, 
        W_SETVALUES_FLT, 
        W_SETVALUES_INT, 
        W_SETVALUES_INT64,
        W_SETANIMATION, 
        W_SETVALUEAT,         
        W_SETKEY,
        W_SETKEYAT,
        W_ISKEYAT,         
        W_GETVALUE, 
        W_SETEXPRESSION, 
        W_SETVALUESAT_FLT, 
        W_SETVALUESAT_INT, 
        W_SETVALUESAT_INT64, 
        W_GETVALUEAT,
        W_REMOVEKEY, 
        W_REMOVEKEYAT, 
        W_ISKEY,
        W_GETNUMKEYS,
        W_GETKEYINDEX, 
        W_GETKEYTIME,
        W_GETDERIVATIVE, 
        W_GETINTEGRAL, 
        W_GETVALUEATVIEW,
        W_SET_ANIMATED, 
        W_IS_ANIMATED, 
        W_CLEAR_ANIMATED,
        W_TOGGLE, 
        W_SET_INPUT,
        W_SETVALUEAT_VIEW,
        W_SPLIT_VIEW,
        W_UNSPLIT_VIEW,
        W_CLEAR_ANIMATED_VIEW,
        W_IS_ANIMATED_VIEW,
        W_SET_ANIMATED_VIEW,
        W_GETKEYFRAME, 
        W_SETKEYFRAME, 
        W_ADDKEYFRAME,
        W_GETEXPRESSION,
        W_GETINVERSE,
        W_GETINVERSE_GENERAL,
        W_GETVALUEATCONTEXT
      };

    public:

      /*!
         Used to pass most of the get/set value functions to the Array_Knob
         subclass implemented inside Nuke. You may be able to make your knob
         act like Array Knob by overriding this.
         The default version returns false.
       */
      virtual int value_stuff(int what, ValueStuffArg* user = nullptr) const;

      /*! bool ValueStuffI::set_value(double value, int index);
       
       Change the value of the knob to this value. If the knob stores
       multiple channels, \a index is which one to change, or use -1 to
       change them all. If the channel is animated this will set a
       keyframe. Returns true if this value is different than the old one.
       */
      bool set_value(double v, int channel = -1)
      {
        ValueStuffArg::SetValueArg Data = { v, channel };
        return value_stuff( W_SETVALUE, (ValueStuffArg*)&Data) != 0;
      }
      
      /*! bool ValueStuffI::set_value_at(double value, double time, int index);
       
       Change the value of the knob at the given time. If the knob is not
       animated this is the same as set_value(). If you want to make sure
       the knob is animated call set_animated() first.
       */
      bool set_value_at(double v, double time, int channel = -1)
      {
        ValueStuffArg::SetValueAtArg Data = { v, time, channel };
        return value_stuff( W_SETVALUEAT, (ValueStuffArg*)&Data) != 0;
      }

      bool set_value_at_view(double v, double time, int view, int channel = -1)
      {
        ValueStuffArg::SetValueAtViewArg Data = { v, time, view, channel };
        return value_stuff( W_SETVALUEAT_VIEW, (ValueStuffArg*)&Data) != 0;
      }

      bool set_key(int channel = -1, int view = -1)
      {
        ValueStuffArg::SetKeyArg Data = { channel, view };
        return value_stuff( W_SETKEY, (ValueStuffArg*)&Data) != 0;
      }

      bool set_key_at(double time, int channel = -1, int view = -1)
      {
        ValueStuffArg::SetKeyAtArg Data = { time, channel, view };
        return value_stuff( W_SETKEYAT, (ValueStuffArg*)&Data) != 0;
      }

      bool set_values(const double* array, int n)
      {
        ValueStuffArg::SetValuesArg Data = { array, n };
        return value_stuff( W_SETVALUES_DBL, (ValueStuffArg*)&Data) != 0;
      }

      bool set_values_at(const double* array, int n, double time)
      {
        ValueStuffArg::SetValuesAtArg Data = { array, n, time };
        return value_stuff( W_SETVALUESAT_DBL, (ValueStuffArg*)&Data) != 0;
      }

      bool set_values(const float* array, int n)
      {
        ValueStuffArg::SetValuesFloatArg Data = { array, n };
        return value_stuff( W_SETVALUES_FLT, (ValueStuffArg*)&Data) != 0;
      }

      bool set_values_at(const float* array, int n, double time)
      {
        ValueStuffArg::SetValuesAtFloatArg Data = { array, n, time };
        return value_stuff( W_SETVALUESAT_FLT, (ValueStuffArg*)&Data) != 0;
      }
      
      bool set_values(const int* array, int n)
      {
        ValueStuffArg::SetValuesIntArg Data = { array, n };
        return value_stuff( W_SETVALUES_INT, (ValueStuffArg*)&Data) != 0;
      }

      bool set_values_at(const int* array, int n, double time)
      {
        ValueStuffArg::SetValuesAtIntArg Data = { array, n, time };
        return value_stuff( W_SETVALUESAT_INT, (ValueStuffArg*)&Data) != 0;
      }

      bool set_values(const U64* array, int n)
      {
        ValueStuffArg::SetValuesInt64Arg Data = { array, n };
        return value_stuff( W_SETVALUES_INT64, (ValueStuffArg*)&Data) != 0;
      }

      bool set_values_at(const U64* array, int n, double time)
      {
        ValueStuffArg::SetValuesAtInt64Arg Data = { array, n, time };
        return value_stuff( W_SETVALUESAT_INT64, (ValueStuffArg*)&Data) != 0;
      }


      /*! bool ValueStuffI::remove_key(int index)
       Removes a keyframe, if it exists, at the current frame. Does nothing
       if the knob is not animated. If the knob stores multiple channels,
       then \a index picks which one, or use -1 to do all channels. Returns
       true if any keyframes were deleted.
       */
      bool remove_key(int channel = -1)
      {
        return value_stuff( W_REMOVEKEY, (ValueStuffArg*)(long long)channel) != 0;
      }
      
      /*! bool ValueStuffI::remove_key_at(double time, int index)
       Removes all keyframes at \a time.
       */
      bool remove_key_at(double time, int channel = -1, int view = -1)
      {
        ValueStuffArg::RemoveKeyAtArg Data = { time, channel, view };
        return value_stuff( W_REMOVEKEYAT, (ValueStuffArg*)&Data) != 0;
      }
      
      /*! void ValueStuffI::set_range(double a, double b, bool force=false);
       Set the range displayed by the GUI slider to \a a..b.
       \a force turns on the FORCE_RANGE flag.
       */
      void set_range(double a, double b, bool force = false)
      {
        ValueStuffArg::SetRangeArg Data = { a, b, force };
        value_stuff( W_SETRANGE, (ValueStuffArg*)&Data);
      }
      
      /*! void ValueStuffI::set_animation(const char *, int index);
       Turn on the animation and replace the curve and expression with a
       new one parsed from the string. See Animation::from_script() for
       the format of the string. If index is -1 it puts the same curve
       on all channels.
       */
      void set_animation(const char* a, int channel = -1)
      {
        ValueStuffArg::SetAnimationArg Data = { a, channel };
        value_stuff( W_SETANIMATION, (ValueStuffArg*)&Data);
      }
      
      /*! bool ValueStuffI::is_key_at(double time, int index);
       Return true if there is a keyframe at \a time.
       */
      bool is_key_at(double time, int channel = -1, int view = -1) const
      {
        ValueStuffArg::IsKeyAtArg Data = { time, channel, view };
        return value_stuff( W_ISKEYAT, (ValueStuffArg*)&Data) != 0;
      }
      
      /*! bool ValueStuffI::is_key(int index)
       Returns true if there is a keyframe at the current frame. If there
       are multiple channels then \a index picks which to check, or use -1
       for all channels.
       */
      bool is_key(int channel = -1) const
      {
        return value_stuff(W_ISKEY, (ValueStuffArg*)(long long)channel) != 0;
      }
      
      /*! double ValueStuffI::get_value(int index);
       Return the numeric value. If it is animated the value at the current
       frame is returned. If the knob stores multiple channels then \a
       index indicates which one (defaults to 0).
       */
      double get_value(int channel = 0) const
      {
        ValueStuffArg::GetValueArg Data = { channel, 0.0 };
        value_stuff( W_GETVALUE, (ValueStuffArg*)&Data);
        return Data.r;
      }
      
      /*! double ValueStuffI::get_value_at(double time, int index);
       If it is animated, return the value at the given \a time. If not
       animated this is the same as get_value().
       */
      double get_value_at(double time, int channel = 0) const
      {
        ValueStuffArg::GetValueAtViewArg Data = { time, 0, channel, 0.0 };
        value_stuff( W_GETVALUEAT, (ValueStuffArg*)&Data);
        return Data.r;
      }
      
      double get_value_at_view(double time, int view, int channel = 0) const
      {
        ValueStuffArg::GetValueAtViewArg Data = { time, view, channel, 0.0 };
        value_stuff( W_GETVALUEATVIEW, (ValueStuffArg*)&Data);
        return Data.r;
      }

      double get_value_at_context(double time, int view, const GraphScopePtr graphScope, int channel = 0) const
      {
        ValueStuffArg::GetValueAtContextArg Data = { time, view, graphScope->hash().value(), channel, 0.0 };
        value_stuff( W_GETVALUEATCONTEXT, reinterpret_cast<ValueStuffArg*>(&Data));
        return Data.r;
      }

      double get_value_at(const OutputContext& oc, int channel = 0) const;
      
      /*! void ValueStuffI::set_expression(const char *expr, int index);
       Turn on the animation if it is not on, and replace the expression
       with a new one. If index is -1 it puts the same expression in
       all channels.
       */
      bool set_expression(const char* v, int channel, int view = -1)
      {
        ValueStuffArg::SetExpressionArg Data = { v, channel, view };
        return value_stuff( W_SETEXPRESSION, (ValueStuffArg*)&Data) != 0;
      }

      int getNumKeys(int channel = -1, int view = -1) const
      {
        ValueStuffArg::GetNumKeysArg Data = { view, channel };
        return value_stuff(W_GETNUMKEYS, (ValueStuffArg*)&Data);
      }

      int getKeyIndex(double& time, int channel = -1, int view = -1) const
      {
        ValueStuffArg::GetKeyIndexArg Data = { view, time, channel };
        return value_stuff(W_GETKEYINDEX, (ValueStuffArg*)&Data);
      }

      bool split_view(int view)
      {
        ValueStuffArg::SplitViewArg Data = { view };
        return value_stuff(W_SPLIT_VIEW, (ValueStuffArg*)&Data) != 0;
      }

      bool unsplit_view(int view)
      {
        ValueStuffArg::UnsplitViewArg Data = { view };
        return value_stuff(W_UNSPLIT_VIEW, (ValueStuffArg*)&Data) != 0;
      }

      void getKeyFrame( int keyindex, double& x, double& y, double& lslope, double& rslope, double& la, double& ra, unsigned char& interp, unsigned char& extrap, int channel, int view = -1 ) const
      {
        ValueStuffArg::GetKeyFrameArg Data = { view, keyindex, channel, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0 };
        value_stuff(W_GETKEYFRAME, (ValueStuffArg*)&Data);
        x = Data.x;
        y = Data.y;
        lslope = Data.lslope;
        rslope = Data.rslope;
        la = Data.la;
        ra = Data.ra;
        interp = Data.interp;
        extrap = Data.extrap;
      }

      void setKeyFrame( int keyindex, double x, double y, double lslope, double rslope, double la, double ra, unsigned char interp, unsigned char extrap, int channel, int view = -1 ) const
      {
        ValueStuffArg::SetKeyFrameArg Data = { view, keyindex, channel, x, y, lslope, rslope, la, ra, interp, extrap };
        value_stuff(W_SETKEYFRAME, (ValueStuffArg*)&Data);
      }

      void addKeyFrame( double x, double y, double lslope, double rslope, double la, double ra, unsigned char interp, unsigned char extrap, int channel, int view = -1 ) const
      {
        ValueStuffArg::AddKeyFrameArg Data = { view, channel, x, y, lslope, rslope, la, ra, interp, extrap };
        value_stuff(W_ADDKEYFRAME, (ValueStuffArg*)&Data);
      }

      void get_expression(std::string& expr, int channel = -1, int view = -1)
      {
        ValueStuffArg::GetExpressionArg Data = { view, &expr, channel };
        value_stuff( W_GETEXPRESSION, (ValueStuffArg*)&Data);
      }

      double getKeyTime(int keyindex, int channel = -1, int view = -1) const
      {
        ValueStuffArg::GetKeyTimeArg Data = { view, keyindex, channel, 0.0 };
        value_stuff(W_GETKEYTIME, (ValueStuffArg*)&Data);
        return Data.r;
      }

      double getDerivative(double time, int channel = 0, int view = -1) const
      {
        ValueStuffArg::GetDerivativeArg Data = { view, time, channel, 1, 0.0 };
        value_stuff( W_GETDERIVATIVE, (ValueStuffArg*)&Data);
        return Data.r;
      }

      double getNthDerivative(int n, double time, int channel = 0, int view = -1) const
      {
        ValueStuffArg::GetDerivativeArg Data = { view, time, channel, n, 0.0 };
        value_stuff( W_GETDERIVATIVE, (ValueStuffArg*)&Data);
        return Data.r;
      }

      double getIntegral(double t1, double t2, int channel = 0, int view = -1) const
      {
        ValueStuffArg::GetIntegralArg Data = { view, t1, t2, channel, 0.0 };
        value_stuff( W_GETINTEGRAL, (ValueStuffArg*)&Data);
        return Data.r;
      }
      
      double getInverse(double value, int channel = 0, int view = -1) const
      {
        ValueStuffArg::GetInverseArg Data = { view, value, channel, 1, 0.0 };
        value_stuff( W_GETINVERSE, (ValueStuffArg*)&Data);
        return Data.r;
      }

      double getInverseGeneral(double value, int channel = 0, int view = -1) const
      {
        ValueStuffArg::GetInverseArg Data = { view, value, channel, 1, 0.0 };
        value_stuff( W_GETINVERSE_GENERAL, (ValueStuffArg*)&Data);
        return Data.r;
      }

      /*! void ValueStuffI::*set_animated(int index);
       Turn on the animation. Does nothing and returns false if already
       animated. If there are multiple channels, \a index picks which to
       animate, or use -1 for all of them.
       */
      bool set_animated(int channel = -1) const
      {
        return value_stuff(W_SET_ANIMATED, (ValueStuffArg*)(long long)channel) != 0;
      }
      
      /*! bool ValueStuffI::is_animated(int index);
       Return true if the knob is animated. Use \a index to only check a
       certain channel, or use -1 for all of them.
       */
      bool is_animated(int channel = -1)
      {
        return value_stuff(W_IS_ANIMATED, (ValueStuffArg*)(long long)channel) != 0;
      }
      
      /*! void ValueStuffI::clear_animated(int index);
       Delete the animation, if any. The knob is left at a constant value
       equal to whatever value the animation had at the current frame.
       Use \a index to only delete a channel, or -1 to delete all of them.
       */
      bool clear_animated(int channel = -1)
      {
        return value_stuff(W_CLEAR_ANIMATED, (ValueStuffArg*)(long long)channel) != 0;
      }

      bool clear_animated_view(int view, int channel = -1)
      {
        ValueStuffArg::ClearAnimatedViewArg Data = { view, channel };
        return value_stuff(W_CLEAR_ANIMATED_VIEW, (ValueStuffArg*)&Data) != 0;
      }

      bool set_animated_view(int view, int channel = -1)
      {
        ValueStuffArg::SetAnimatedViewArg Data = { view, channel };
        return value_stuff(W_SET_ANIMATED_VIEW, (ValueStuffArg*)&Data) != 0;
      }

      bool is_animated_view(int view, int channel = -1)
      {
        ValueStuffArg::IsAnimatedViewArg Data = { view, channel };
        return value_stuff(W_IS_ANIMATED_VIEW, (ValueStuffArg*)&Data) != 0;
      }

      bool toggle()
      {
        return value_stuff(W_TOGGLE, (ValueStuffArg*)nullptr) != 0;
      }

      bool set_input(int input)
      {
        return value_stuff(W_SET_INPUT, (ValueStuffArg*)(long long)input) != 0;
      }
    };
  }
}

#endif
