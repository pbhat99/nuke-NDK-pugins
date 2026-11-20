// LinkableI.h
// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_LINKABLEI_H
#define DDIMAGE_LINKABLEI_H

#include "Transform.h"

namespace DD
{
  namespace Image
  {
    /*!
     * Information about a knob that may be linked. Contains the actual knob
     * as well as some meta information that may be used to make some
     * decisions about it, such as whether it's enabled or not.
     *
     * Additionally, you can specify custom expressions for how the link string is
     * derived. For example, to address a particular cell of a TableKnobI, you
     * would add an expression std::string to the std::vector with syntax:
     * "row.columnname". This also lets you create arbitrary links to non-knob
     * data if desired. For example, you could set knob to NULL, display name to
     * "sin of frame number" and add an expression "sin(t)".
     */
    class DDImage_API LinkableKnobInfo
    {
      private:
        Knob* _knob;
        bool _enabled;
        bool _absolute;
      
        std::string _displayName;
        std::vector<int> _indices;      
      public:
        LinkableKnobInfo(Knob* knob = nullptr, bool enabled = true, bool absolute = true);
        LinkableKnobInfo(std::string displayName, Knob* knob = nullptr, bool enabled = true, bool absolute = true);
      
        Knob* knob() const;
        bool enabled() const;
        bool absolute() const;
        std::string displayName() const;
      
        const std::vector<int>& indices() const;
        void indices(const std::vector<int>& indices);
    };
    /*!
     * This is a node that may be linked to from a place that directly wants
     * to link to another node, i.e. from the Animation Menu for an XY_Knob or
     * the Roto context menu.
     */
    class DDImage_API LinkableI
    {
      public:
        LinkableI();
        virtual ~LinkableI();
        /*!
         * Query the object for the knobs that can be linkable with the given
         * type. This type should be one of those as defined in Knobs.h, e.g.
         * XY_KNOB, ARRAY_KNOB, etc.
         */
        virtual std::vector<LinkableKnobInfo> getLinkableKnobs(int type) const  = 0;
      
        /*!
         * In some cases, the LinkableI object may be asked if it can create
         * a knob that can be linked to. For example, in the Tracker, you may 
         * want to create a new track. The type parameter is the same as 
         * used in getLinkableKnobs(int type), e.g. XY_KNOB, ARRAY_KNOB, etc.
         */
        virtual bool canCreateNewLinkableKnob(int type) const = 0;
        virtual LinkableKnobInfo createNewLinkableKnob(int type) = 0;
    };
  }
}

#endif

