// DDImage/KnobChangedObserverI.h
// Copyright (c) 2018 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_KnobChangedObserverI_h
#define DDImage_KnobChangedObserverI_h

namespace DD {
  namespace Image {
    
    class Knob;

    //! Contains an interface for objects that would like to act as observers
    //! of knob changed events on the node associated with a NodeI.
    class KnobChangedObserverI
    {
    public:
      KnobChangedObserverI() {}
      virtual ~KnobChangedObserverI() {}

      // Calls to this function are made for _all_ knob changes, regardless of whether the
      // NO_KNOB_CHANGED flags is set on the changed knob. This is in contrast to calls to the virtual
      // Op::knob_changed, which are _not_ made if the NO_KNOB_CHANGED flag is set.
      //
      // Note that this is primarily intended for watching changes to the Root Node's Knobs. There is currently
      // no way to obtain pointers to other Nodes inside the NDK.
      virtual void observedKnobChanged(DD::Image::Knob* knob) = 0;
    };
  }
} // namespace DD::Image

#endif // DDImage_KnobChangedObserverI_h

