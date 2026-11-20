// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef METAKEYFRAMEKNOBI_H
#define METAKEYFRAMEKNOBI_H

#include <set>

#include "DDImage/Knob.h"
#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    class MetaKeyFrame_KnobI;
    
    class DDImage_API MetaKeyFrameDelegate
    {
    public:
      virtual ~MetaKeyFrameDelegate();
      
      enum KeyPressed
      {
        eSetKeyFrame,
        eDeleteKeyFrame,
        eCopy,
        ePaste,
        eDeleteAllKeys,
        eCopyAllKeys
      };

      /**! Function to get the key-frames from the given knob via the delegate
       */
      virtual void getKeyFrames(const MetaKeyFrame_KnobI* knob, std::set<float>& keyFrames) = 0;
      
      virtual void metaKeyFrameKeyPressed(const MetaKeyFrame_KnobI* knob, KeyPressed key, float frame) = 0;
    };
    
    struct DDImage_API MetaKeyFrameOptions
    {
      enum MetaKeyFrameFlags
      {
        DEFAULT = 0,
        COPY_PASTE_BUTTONS = 1 << 0,
        IGNORE_KEY_NUMBERS = 1 << 1,
        DELETE_ALL_KEYS_BUTTON = 1 << 2,
        COPY_ALL_PASTE_BUTTONS = 1 << 3
      };
      
      MetaKeyFrameOptions() : flags(DEFAULT)
      {
      }

      MetaKeyFrameOptions(int flgs) : flags(flgs)
      {
      }      

      int flags;
    };
  
    class DDImage_API MetaKeyFrame_KnobI
    {
    protected:
      virtual ~MetaKeyFrame_KnobI();
      
    public:            
      virtual void setDelegate(MetaKeyFrameDelegate* delegate) = 0;
    };    
  }
}    

#endif // METAKEYFRAMEKNOBI_H
