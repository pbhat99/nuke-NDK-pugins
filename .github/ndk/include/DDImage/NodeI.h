// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_NodeI_h
#define DDImage_NodeI_h

#include "DDImage/DDImage_API.h"
#include "DDImage/KnobChangedObserverI.h"
#include "DDImage/Hash.h"
#include "DDImage/GeneralInfo.h"
#include "DDImage/OutputContext.h"

#include "ndk/graph/GsvTypes.h"
#include "fdk/base/Time.h"

#include <vector>
#include <string>

class QImage;
class Node;

namespace DD
{
  namespace Image
  {
    using GsvSets = ndk::GsvSets;
    class LinkableI;
    class GsvKnobI;
    class Description;
    class Op;


    class DDImage_API NodeI
    {
    public:
      virtual ~NodeI();

    public:

      //! Return the Node's current UI Context. It is important to note that the node
      //! itself doesn't really have a UI Context of its own and that this value may
      //! change if Nuke is rendering multiple ops for the node in different contexts.
      //! As such, the value returned here should not be relied upon for anything other
      //! than short term operations.
      virtual const OutputContext& uiContext() const = 0;

      //! Return the Node object attached to this interface.
      virtual Node* nodeObj() const { return nullptr; }

      //! Return the local Node name, without parent path.
      virtual const char* name() const = 0;

      /*! Return the full path of the node which is the Node's path with parents separated
          using the optional separator character. The default separator is a period '.' which
          is typically used for Nuke expressions.

          However periods can cause problems with some path parsers so underscore '_'
          and colon ':' are often good options.

          All printable (std::isprint) characters are allowed, which include space ' '.

          Note that forward-slash '/' is handled specially and the first character of the
          returned path will have a '/' prefix to form an absolute path.
      */
      virtual std::string fullPath(char separator = '.') const = 0;

      //! Returns the full path of the node, including any parents, separated by periods '.'
      virtual std::string getNodeName() const = 0;

      //! Return the GsvKnob if this Node has one.
      virtual GsvKnobI* getGsvKnob() const = 0;

      //! Return the GsvSets from a GsvKnob if exists, or empty otherwise. The
      //! returned GsvSets will be used to construct a GraphScope that will be
      //! parented to the GraphScope set in the OutputContext during generate_op,
      //! effectively extending/overriding that scope.
      virtual GsvSets getGsvSets() const = 0;

      //! Return the Node's GraphScope.
      virtual GraphScopePtr getGraphScope() const = 0;

      //! Find the Node at the given full path, if it exists, and return a pointer to it. If
      //! the \a instanceIndex provided is >= 0 then a specific clone instance (index 1+), or
      //! the 'original'(base) instance (index 0) is returned. If \a instanceIndex value is not
      //! valid for the retrieved Node's context then null is returned.
      //!
      //! \a nodePath is -always- an absolute path and periods '.' are assumed as the hierarchy
      //! separator, conforming to the standard Nuke Node path convention.
      //!
      //! However, embedded expressions are -NOT- supported, unlike the \a toNode() method. For
      //! example you cannot retrieve a Node using the path string 'Group1.FooNode.parent',
      //! as the trailing '.parent' is an expression.
      static NodeI* GetNodeAtPath(const std::string& nodePath, int32_t instanceIndex = -1);

      //! Convert an abstract Node* to a node interface.
      static NodeI* GetInterface(Node*);

      //-----------------------------------------------------------------------------------

      //! Return the parent node. If this is non-null the returned node will always be a Group.
      virtual NodeI* getParent() const = 0;

      /*! Hash of input Node pointers unaffected by Knob values, to identify if Node graph topology
          has changed.
          This hashes the name of the Node, the input indices order and each input's topologyHash(),
          so the topology hash will also reflect the names of all upstream nodes and can be used to
          indicate when those names are changed.
      */
      virtual Hash topologyHash() = 0;

      //! Node's global edit version state hash, formed from the input and local knob edit
      //! version hashes, and the Node name(plus clone info.) This state hash should only
      //! change when the contents of a Knob is changed (i.e. edited) or the Node graph
      //! topology is changed (i.e. input connections edited) since the combination of input
      //! knobs will change.
      //!
      //! Op::treeVersion() provides a similar global change notification except that it is
      //! global to -all- of Nuke, so Node::editVersionHash() allows the individual Node
      //! global edit version state to be tracked.
      //!
      //! Note - since editVersionHash is built from Knob editVersion integers which are
      //! incremented when Knob contents are edited, the editVersionHash() should never
      //! repeat even when Knobs are set back to a previously used value.
      virtual Hash editVersionHash() = 0;

      //! Return whether we are currently in node creation or not
      virtual bool creatingNewNode() const { return false; }

      //-----------------------------------------------------------------------------------

      //! Can this Node class be cloned?
      virtual bool isCloneable() const { return true; }

      //! Is this Node a clone? If so use baseNode() to retrieve the 'original' Node.
      virtual bool isClone() const { return false; }

      //! Return the 'original'(base) Node if this is a clone, or 'this' if not cloned.
      virtual NodeI* baseNode() { return this; }

      //! Return the number of clone instances NOT including the 'original'(base) Node.
      virtual size_t numCloneInstances() const { return 0; }

      //! Return the instance index of this Node where 0 is the 'original'(base) Node, or -1 if
      //! Node is not cloned. Clones will start at index 1.
      virtual int32_t cloneInstanceIndex() const { return -1; }

      //! Return the NodeI pointer for a valid instance index, otherwise null.
      virtual NodeI* cloneInstanceNode(int32_t instanceIndex) const { return nullptr; }

      //! Return a list of all instances for this Node. If the Node has not been cloned then nothing is
      //! returned. Otherwise a list starting with the 'original'(base) Node followed by all clones is
      //! returned.
      //! The instance index returned by the cloneInstance() method will match the ordering of the Nodes
      //! in this list (assuming the index is >= 0).
      virtual std::vector<NodeI*> cloneInstances() const { return std::vector<NodeI*>(); }

      //-----------------------------------------------------------------------------------

      //! Return the name from the Node_Description that created this Node. Note that this name may
      //! -not- be the same as the Node Class() which is usually the script command name. Some Nodes
      //! are not created by a script command or Op class name, so this method provides the creator
      //! name for those Node types.
      virtual const char* nodeDescriptionName() const = 0;

      //! Returns the base class name for the Node if it manages Ops, usually 'OpNode'.
      virtual const char* opManagerClass() const = 0;

      //! A single Node can create many Op instances however the first one it creates is special as it
      //! is never deleted until the Node is. See Op::firstOp() for more info.
      virtual Op* firstOp() const = 0;

      //! Return a specific Op owned by this Node, uniquely identified by its OutputContext.
      virtual Op* findOp(const OutputContext& context) const { return nullptr; }

      /*! Return the Op hash of a Node at a specific OutputContext if the Node contains one, otherwise
          a default Hash is returned.
          Note that just because the Node contains an Op with that hash does not mean the Op is useable,
          always check the Op is valid and constructed.
      */
      virtual Hash findOpHash(const OutputContext& context) const { return Hash(); }

      /*! Retrieve an Op owned by the Node with a matching hash, or null if none found.
          You must check that the returned Op is useable, valid and constructed.
      */
      virtual Op* findOpByHash(const DD::Image::Hash& hash) const { return nullptr; }

      //! Get the hashes of all Ops owned by this Node.
      virtual std::vector<DD::Image::Hash> opHashes() const { return std::vector<DD::Image::Hash>(); }

      //-----------------------------------------------------------------------------------

      //! Calculate a frame range for the given OutputContext.
      //! Implemented by Nodes that want to modify the frame range flowing down the graph.
      virtual void getFrameRangeAt(const OutputContext& context, FrameRange& frameRangeOut) const = 0;

      //! Calculate the input frame for the given OutputContext.
      //! Implemented by Nodes that want to modify the time flowing down the graph.
      //! If Node is not changing anything returns the frame in \a context.
      virtual double getInputFrameAt(const DD::Image::OutputContext& context) const = 0;

      //! Apply Node out->in time remapping function, if any, otherwise outTimes are copied to inTimes.
      void applyTimeRemapping(const fdk::TimeValueSet& outTimes, const OutputContext& baseContext, fdk::TimeValueSet& inTimes) const;

      //! Same but with a time list source.
      void applyTimeRemapping(const fdk::TimeValueList& outTimes, const OutputContext& baseContext, fdk::TimeValueSet& inTimes) const;

      //-----------------------------------------------------------------------------------

      //! Returns an std::vector of all knobs that can be linked to
      virtual std::vector<LinkableI*> getLinkableKnobs() = 0;

      //! Specify a custom QImage icon to decorate this Node, along with an x/y offset and scale.
      virtual void setCustomIcon(const QImage* image, const float scale, const int offsetX, const int offsetY) {}

      //! Observer mechanism to allow objects inheriting DD::Image::KnobChangedObserverI to be notified of knob_changed events
      virtual void registerKnobChangedObserver(DD::Image::KnobChangedObserverI* observer) = 0;
      virtual void unregisterKnobChangedObserver(DD::Image::KnobChangedObserverI* observer) = 0;

      //! Request a help system update for this Node.
      virtual void checkExternalDocs() const = 0;

      //! Returns a Python object for this node, making a new one if one didn't already exist. The return value is a PyObject*.
      //! The reference count for the object will be incremented.
      virtual void* getPythonObject() = 0;
    };
  }
}

#endif // DDImage_NodeI_h
