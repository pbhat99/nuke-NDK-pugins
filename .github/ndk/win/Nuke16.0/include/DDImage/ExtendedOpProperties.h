// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef EXTENDEDOPPROPERTIES_H
#define EXTENDEDOPPROPERTIES_H

#include "DDImage/Knobs.h"

namespace DD
{
  namespace Image
  {
    //! \class DD::Image::ExtendedOpProperties
    //! This class encapsulates the knobs functionality for the extended properties tab.
    //! Shared pointer to object of this class should be passed in to any Iops
    //! which wish to add an extended properties tab.
    //! Calling extendedPropertiesKnobs creates the extended properties tab in the node properties panel.
    //! That tab contains a number of String knob pairs that represent key/value pairs.
    //! Those pairs can be retrieved by the node using keyValuePair function.
    class DDImage_API ExtendedOpProperties
    {
    public:
      using KeyValuePair = std::pair<std::string, std::string>;

      ExtendedOpProperties(int numKeyValuePairs);
    
      virtual ~ExtendedOpProperties() = default;

      //! Creates/Stores the extended properties tab.
      void extendedPropertiesKnobs(Knob_Callback f, const char* tabName, const char* dividerLabel, const char* tooltip);

      //! Returns the key/value pair denoted by a given index
      const KeyValuePair& keyValuePair(int index) const { return _keyValuePairs[index]; }

      //! Returns the number of key/value pairs
      int numKeyValuePairs() const { return _keyValuePairs.size(); }

    private:
      //! Property key/value pairs.
      std::vector<KeyValuePair> _keyValuePairs;
    };
  }
}

#endif // EXTENDEDOPPROPERTIES_H
