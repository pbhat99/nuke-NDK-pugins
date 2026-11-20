// Execute.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Execute_h
#define DDImage_Execute_h

/*! \class DD::Image::Execute

   An "execution context". It is used to ask Nuke to generate a tree of
   Ops that will remain locked (ie not changed by any other execution
   contexts, though maybe shared by them) until the Execute is
   destroyed.

   This is used to look at multiple frames (a "clip") of an
   operator's input. To do this a DD::Image::Op should:
 * Unless you want to compare each frame to the current one, make
    Op::split_input() return -1. This indicates that Nuke
    should not lock ops for that input when building the tree, so
    they can be reused for this.
 * If the output does not depend on the current frame (such as an
    operator that averages a fixed range together) you should
    also make Op::input_frame() return a constant. Otherwise the
    hash will vary depending on the frame number, causing it to
    recalculate as the user changes frames.
 * To look at all the frames, the open() or execute() should create
    an Execute (usually as a local variable), then inside the loop it
    should call reset() and then generateOp() for each frame.
 */

#include "DDImage/Op.h"
#include "DDImage/OutputContext.h"
#include "DDImage_API.h"

namespace DD
{
  namespace Image
  {

    class Op;
    class OutputContext;

    class DDImage_API Execute
    {
    public:
      typedef unsigned ExecuteVersion;

    private:

      // Glue to call code inside Nuke:
      friend class ExecuteImplementation;
      enum Action { CONSTRUCT, DESTRUCT, GENERATEOP, RESET };
      static void* (*doStuff)(Execute *, Action what, void*);
      static void* (*doStuffConst)(const Execute *, Action what, void*);
      struct GenerateOpArgs { Op* op;
                              int inputnumber;
                              const OutputContext& outputContext; };

      // Dummy functions to initialise the above. Safer when destroying.
      static void* doStuffNull(Execute*, Action what, void*) { return nullptr; }
      static void* doStuffNullConst(const Execute*, Action what, void*) { return nullptr; }

    public:

      Execute() { doStuff(this, CONSTRUCT, nullptr); }

      ~Execute() { doStuff(this, DESTRUCT, nullptr); }

      //! Generate an Op graph for the node that owns \a op at \a outputContext.
      //! This Op graph will be "locked" and will not be altered until the Execute is
      //! destroyed or reset() is called.
      OpGraphLocation generateOpGraphAt(Op* op, const OutputContext& outputContext);

      //! Generate an Op graph for the input on the node that owns \a op at \a outputContext.
      //! This Op graph will be "locked" and will not be altered until the Execute is
      //! destroyed or reset() is called.
      OpGraphLocation generateOpGraphForInput(Op* op, int nodeInputNumber, const OutputContext& outputContext);

      /** Produce input n of \a op at frame \a frame. This Op will be
          "locked" and will not be altered until the Execute is destroyed
          or reset() is called.
       */
      Op* generateOp(Op* op, int inputnumber, double frame, int view = 0)
      {
        OutputContext oc = op->uiContext();
        oc.setFrame(frame);
        oc.setView(view);
        return generateOp(op, inputnumber, oc);
      }

      /** Produce input n of \a op using values from output context \a outputContext. 
          This Op will be "locked" and will not be altered until the Execute is destroyed
          or reset() is called.
       */
      Op* generateOp(Op* op, int inputnumber, const OutputContext& outputContext)
      {
        GenerateOpArgs a = { op, inputnumber, outputContext};
        return (Op*)(doStuff(this, GENERATEOP, &a));
      }

      /** Same as destroy+create of a new Execute, but slightly more
          efficient. All previous Ops from generateOp
          calls are unlocked and can be reused by this or other calls.
          In a typical loop you would call this after processing each
          frame number.
       */
      void reset() { doStuff(this, RESET, nullptr); }

      void setVersion(int newVersion)
      {
        version = newVersion;
      }

      int getVersion() const
      {
        return version;
      }

    private:
      Execute(int) { prev = nullptr;
                     version = 0; }
      ExecuteVersion version; // number that increments for each instance to id them
      Execute* prev; // down in stack
    };

  }
}

#endif
