// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_EXECUTABLE_H
#define DDIMAGE_EXECUTABLE_H

#include "DDImage/Iop.h"

namespace DD
{
  namespace Image
  {

    class Op;

    //! Class to wrap up node graph execution
    class DDImage_API Executable
    {

      // Be sure to change this symbol if the ABI changes.
#define DDImage_Executable_ABI abi6_0_1
      virtual int DDImage_Executable_ABI();

      Op* _op;
      std::set<int> _viewsToExecute;

      bool _useLimit;
      int _limitFirst;
      int _limitLast;

    protected:

      virtual ~Executable();

    public:
      Executable(Op * op);

      virtual void beginExecuting();
      virtual void execute() = 0;

      //! Whether the execute() function on this Executable object
      //! can be safely run in a parallel thread.  If you return true
      //! from this you cannot use any knob setting functions, in particular.
      virtual bool isExecuteThreadSafe() const
      {
        return true;
      }

      //! The execute() function calls this function to get a progress update [0;100] 
      //! on the frame being executed. This allows a refined rendering progress to be displayed if overridden.
      virtual unsigned int getFrameProgress() const 
      { 
        return 0; 
      }
      
      //! The execute() function calls this function to get a progress message update
      //! on the frame being executed. This allows a custom progress message to be displayed if overridden.
      virtual std::string getFrameProgressMessage() const 
      { 
        return {}; 
      }

      virtual void endExecuting();

      void setViewsToExecute(const std::set<int>& view);
      void clearViewsToExecute();

      const std::set<int>& viewsToExecute()
      {
        return _viewsToExecute;
      }

      enum ExecMode {
        /// The Op can only be usefully executed for one view/frame unless
        /// a filename knob changes when the view or frame changes
        WriteOne,

        /// The Op can be usefully executed for various views/frames,
        /// even though no knobs change when the view/frame changes.
        /// A movie file can do this for frames, for instance.
        WriteAny,

        /// The op, when executed, will ignore the views/frames passed to it,
        /// and instead write out a set of views or frames that it chooses.
        /// (exr writer with multiple views, for example)
        WriteMany,
      };

      /// can this node be usefully executed for more than one view in sequence
      
      /// Default is WriteAny, indicating this can be called over and over
      /// with different views. Note that the Write Op overrides this and
      /// returns WriteOne by default.
      virtual ExecMode execViewMode();

      /// can this node be usefully executed for more then one frame in sequence
      
      /// Default is WriteAny, indicating this can be called over and over
      /// with different frames. Note that the Write Op overrides this and
      /// returns WriteOne by default.
      virtual ExecMode execFrameMode();

      /// return the views that this node will execute
      virtual const std::set<int>& views();

      /// return true if this should be picked up by an execute of everything
      /*!
        Return true if this node should be executed when the user asks for
        the group or script containing this node to be executed. Typically
        true only for Write nodes.
      */
      virtual bool isWrite() const;

      //! provide frame range knobs and a 'limit to frame range'
      void frameRangeKnobs(Knob_Callback);

      //! return true if this node should be skipped for this context.
      //! default implementation is used for the 'limit to frame range'
      //! facility by Write/WriteGeo; if you call frameRangeKnobs in your
      //! knobs function you get that behaviour, but it can also be overriden
      virtual bool skipForContext(const OutputContext& context) const;

      //! If you use frameRangeKnobs your knob_changed should call this 
      //! and return 1 if it returns 1, to handle enabling/disabling of
      //! the limit knobs.  It is interested in both the knob 'use_limit'
      //! and &Knob::showPanel
      int frameRangeKnobsChanged(DD::Image::Knob* knob);

      virtual Executable* executable() = 0;

      //! Get the filename that this wishes to write to (if any), otherwise
      //! NULL.
      virtual const char* filename() const
      {
        return nullptr;
      }
    };
  }
}

#endif
