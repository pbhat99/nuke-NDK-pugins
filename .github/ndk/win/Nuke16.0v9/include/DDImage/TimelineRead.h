// Copyright (c) 2015 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Private_TimelineRead_H
#define DDImage_Private_TimelineRead_H

#include "DDImage/Read.h"

namespace DD
{
  namespace Image
  {
    class TimelineReadInternals;
    
    /*! \class DD::Image::TimelineRead
     *
     *  \brief Sub-class of DD::Image::Read used by NukeStudio/Hiero for timeline reads.
     *
     *  NukeStudio and Hiero code that uses DDImage plugins for reading source footage will create
     *  TimelineRead rather than just Read instances. This is for internal use, though if readers
     *  want to behave in some different when being used for the timeline they can test a dynamic_cast
     *  of their owning Read op to see if it's actually a TimelineRead.
     */
    class DDImage_API TimelineRead : public Read
    {
    public:
      
      /*! We only need a default constructor, as by definition there's no node to pass through to the Read
       *  constructor.
       */
      TimelineRead();
      
      ~TimelineRead() override;

      //! These intentionally shadow the non-virtual base class methods. Setting
      //! the file name will also create the reader format, which is needed for
      //! timeline reads
      void filename(const char* f);
      const char* filename() const;
      
      //! Writable access to the TimelineReadInternals member - for internal use only.
      TimelineReadInternals* internals();
      
      //! Read-only acces to the TimelineReadInternals member - for internal use only.
      const TimelineReadInternals* internals() const;

    private:
      
      // Member containing functionality for internal use only.
      TimelineReadInternals* _internals;
    };

  } // namespace Image
}   // namespace DD

#endif // DDImage_Private_TimelineRead_H
