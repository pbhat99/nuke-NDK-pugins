// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_Knob_H
#define DD_Image_Knob_H

#include "DDImage/DDImage_API.h"
#include "DDImage/OpMessage.h"
#include "DDImage/Store.h"
#include "DDImage/ViewSet.h"
#include "DDImage/ViewerContext.h"
#include "DDImage/Geo_KnobI.h"
#include "DDImage/Axis_KnobI.h"
#include "DDImage/PositionVector_KnobI.h"
#include "DDImage/ValueStuffI.h"
#include "DDImage/Memory.h"
#include "DDImage/Deprecation.h"

#include <iostream>
#include <vector>
#include <cstdarg>
#include <memory>
#include <stack>
#include <list>
#include <set>
#include <unordered_set>

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)
#  pragma warning(push)
#  pragma warning( disable : 4251 ) // warning #4251: needs to have dll-interface to be used by clients of <X>
#endif // FN_OS_WINDOWS &&  FN_COMPILER_INTEL

class Node;
class QWidget;
class VersionNumbers;

/// this hides the toolkit details from users of DDImage
typedef QWidget WidgetType;
typedef QWidget* WidgetPointer;

namespace ndk {
  namespace asset {
    class AssetInfo;
    class AssetData;
  }
}

namespace DD
{
  namespace Image
  {
    class Op;
    class OutputContext;
    class Hash;
    class Format;
    class Knob_Closure;
    class Box;
    class Authoring;
    class ArrayKnobI;
    class ResizableArrayKnobI;
    class Enumeration_KnobI;
    class Channel_KnobI;
    class Colorspace_KnobI;
    class SceneView_KnobI;
    class SceneGraph_KnobI;
    class USDSceneGraph_KnobI;
    class MultiArray_KnobI;
    class List_KnobI;
    class Table_KnobI;
    class GeoSelect_KnobI;
    class GsvKnobI;
    class Axis_KnobI;
    class ControlPointCollection_KnobI;
    class MetaKeyFrame_KnobI;
    class TransformJack_KnobI;
    class Transform2d_KnobI;
    class Ripple_KnobI;
    class WidgetContext;
    class File_KnobI;
    class ScriptKnobI;
    class StringKnobI;
    class LookupCurves_KnobI;
    class ColorKnobI;
    class AnimationHolderI;
    class LinkKnobI;
    class Path_KnobI;
    class HandleContextImpl;
    class PathExpression_KnobI;
    
    using AssetInfo = ndk::asset::AssetInfo;
    using AssetData = ndk::asset::AssetData;
    
    // Using this as the label automatically turns on INVISIBLE flag:
    extern DDImage_API const char INVISIBLE[];

    class KnobChangeGroup;
    class Knob;
    class KnobImpl;
    class KnobHelpers;

    using KnobSet = std::unordered_set<Knob*>;

    class PluginPython_KnobI
    {
    protected:
      void* _pythonType;
    public:
      PluginPython_KnobI() : _pythonType(nullptr) {}
      virtual ~PluginPython_KnobI() {}
      virtual void* getPythonType() { return _pythonType; }
      virtual void setPythonType(void* pythonType) { _pythonType = pythonType; }
    };

    /*! \class DD::Image::Knob

        Provides the user interface for a single named "control" of an Op.

        The Op::knobs() function is called to create these and to list
        them in order to store() values into an instance of an Op.

        The to_script() and from_script() methods are used to read and
        write the value as a text stream. This is used for saving scripts
        and also for implementing undo/redo.

        The most common types of knobs produce a widget in the "control
        panel" of a Node in Nuke. However this functionality is private
        to Nuke. Plugins can only make knobs that are manipulated through
        the user clicking and dragging on the viewers. They can also create
        "child" knobs that display widgets.

        Writing a new knob type is quite complex, though I hope it is
        not hopeless. It is recommended that you examine the source code
        to plugins that use it (DustBust, ...).

     */
    class DDImage_API Knob : public ValueStuffI
    {
      friend class DD::Image::KnobChangeGroup;
      std::unique_ptr<KnobImpl> _pImpl;

      /// noncopy and nonassign
      Knob& operator=(const Knob&);
      Knob(const Knob &);

      std::string _claimedName;
      std::string _defaultName;

      DrawHandleTypeMask  _drawHandleTypes;
      int _openLinkCount;

    public:

      virtual bool updateKnobsFromChildAsset(Knob* assetKnob, const AssetData& assetData) { return false; };
      virtual void updateKnobsFromAsset(const AssetData& assetData) {};

      void setAssetInfo(const AssetInfo& assetInfo);
      const AssetInfo* getAssetInfo() const;
      AssetInfo* getAssetInfo();

      typedef std::vector<int> SelectedIndexVector;

      void setClaimedName(const std::string &claimedName)
      {
        _claimedName = claimedName;
      }

      const std::string& claimedName()
      {
        return _claimedName;
      }

      void setDefaultName(const std::string &defaultName)
      {
        _defaultName = defaultName;
      }

      void setDrawHandleTypes(DrawHandleTypeMask drawHandleTypes)
      {
        _drawHandleTypes = drawHandleTypes;
      }

      DrawHandleTypeMask drawHandleTypes() const
      {
        return _drawHandleTypes;
      }

      // The default name will be the same as the name, except in certain circumstances
      // It can be used for knobs that have the same name, but apply to different contexts on the same
      // Node. At the moment, this only applies to Read/Write nodes, which add extra knobs
      // based on the file extension. Using the default name allows us to specify knobs with the
      // same name but handle conflicts with knob default settings between different kinds
      // of files. So a compression knob on an exr specific read node can have a different
      // default as a compression knob on a mov specific read node.
      const std::string& defaultName() const
      {
        if (_defaultName.empty())
          return name();

        return _defaultName;
      }

      /*! The constructor for a knob adds it to the list being built for
          the current Node.
       */
      Knob(Knob_Closure * c, const char* name, const char* label = nullptr);
      ~Knob() override;

#if FN_TRACK_MEMORY_ALLOCATIONS 
      //! Overloads to use the DDImage heap allocator
      void* operator new(size_t size);
      void  operator delete(void* mem);
      void* operator new[](size_t size);
      void  operator delete[](void* mem);
#endif // FN_TRACK_MEMORY_ALLOCATIONS 

      /*! Return a string name for the type of knob. The base class returns
         "Knob". This is currently only used for messages.
       */
      virtual const char* Class() const = 0;

      /*!
         Return the enumeration value from Knobs.h that will create this
         knob. This is used to write instructions for creating this knob to
         a saved script. If no value creates this knob then return 0, and
         the user will not be able to clone the knob. The base class returns
         zero.
       */
      virtual int ClassID() const;

      virtual ArrayKnobI* arrayKnob()
      {
        return nullptr;
      }

      virtual ResizableArrayKnobI* resizableArrayKnob()
      {
        return nullptr;
      }

      virtual StringKnobI* stringKnob()
      {
        return nullptr;
      }

      virtual ScriptKnobI* scriptKnob()
      {
        return nullptr;
      }

      virtual Enumeration_KnobI* enumerationKnob()
      {
        return nullptr;
      }

      virtual Channel_KnobI* channelKnob()
      {
        return nullptr;
      }

      virtual Colorspace_KnobI* colorspaceKnob()
      {
        return nullptr;
      }

      virtual SceneGraph_KnobI* sceneGraphKnob()
      {
        return nullptr;
      }

      virtual USDSceneGraph_KnobI* usdSceneGraphKnob()
      {
        return nullptr;
      }

      virtual SceneView_KnobI* sceneViewKnob()
      {
        return nullptr;
      }

      virtual MultiArray_KnobI* multiArrayKnob()
      {
        return nullptr;
      }

      virtual List_KnobI* listKnob()
      {
        return nullptr;
      }

      virtual Table_KnobI* tableKnob()
      {
        return nullptr;
      }

      virtual Geo_KnobI* geoKnob()
      {
        return nullptr;
      }

      virtual Axis_KnobI* axisKnob()
      {
        return nullptr;
      }

      virtual GsvKnobI* gsvKnob()
      {
        return nullptr;
      }

      virtual PositionVector_KnobI* positionVectorKnob()
      {
        return nullptr;
      }

      virtual PluginPython_KnobI* pluginPythonKnob()
      {
        return nullptr;
      }

      virtual GeoSelect_KnobI* geoSelectKnob()
      {
        return nullptr;
      }
      
      virtual ControlPointCollection_KnobI* controlPointCollectionKnob()
      {
        return nullptr;
      }

      virtual MetaKeyFrame_KnobI* metaKeyFrameKnob()
      {
        return nullptr;
      }

      virtual TransformJack_KnobI* transformJackKnob()
      {
        return nullptr;
      }
      
      virtual Transform2d_KnobI* transform2dKnob()
      {
        return nullptr;
      }
      
      virtual Ripple_KnobI* rippleKnob()
      {
        return nullptr;
      }
      
      virtual File_KnobI* fileKnob()
      {
        return nullptr;
      }

      virtual LookupCurves_KnobI* lookupCurvesKnob()
      {
        return nullptr;
      }

      virtual ColorKnobI* colorKnob()
      {
        return nullptr;
      }

      virtual AnimationHolderI* animationKnob()
      {
        return nullptr;
      }

      virtual LinkKnobI* linkKnob()
      {
        return nullptr;
      }

      virtual Path_KnobI* pathKnob()
      {
        return nullptr;
      }

      virtual PathExpression_KnobI* pathExpressionKnob()
      {
        return nullptr;
      }

      VersionNumbers* version();
      VersionNumbers* version() const;

      Node* node() const { return node_; }
      void set_node(Node* n) { node_ = n; }

      /*! Return an Op that belongs to the control panel this Knob is
          on. You cannot make very many assumptions about this op except
          that it has been constructed. You could for instance get the
          Class() or help().
       */
      Op* op() const;

      const std::string& name() const { return name_; }
      void name(const char* p);

      /*! fn bool Knob::is(const char* name) const
         Return true if the name matches the passed string. You should use this
         in Op::knob_changed() methods to figure out if the changed knob is the one
         you are interested in.
       */
      bool is(const char* v) const { return name_ == v; }

      /*!
         Return true if the name starts with the substring. Useful in Op::knob_changed()
         to detect a set of related knobs.
       */
      bool startsWith(const char* v) const;

      //! Return the label for this knob, regardless of whether the label is set
      const std::string& label() const { return label_; }

      //! Return the label for this knob, or the name if label is not set
      const std::string& labelToUse() const { return label_set ? label_ : name_; }

      //! Checks whether the label (returned by @ref label) was set, i.e. not empty
      bool labelNotNull() const { return label_set; }

      //! Sets the label to the given value.
      //! The value can be null, in which case @ref labelNotNull will return false after this call
      void label(const char*);

      //! Checks whether knob value is transferable from one node to another
      bool canCopyValue() const;

      //! Checks whether knob value can be included into the node's user preset
      bool canSaveValueToPreset() const;
      
      //! returns the fully-qualified name of the knob within the node. This can be useful for expression
      //! linking. "idx" specifies the optional channel number of the sub-knob, leave blank or set to -1 to
      //! get the qualified name of the knob only.
      std::string fullyQualifiedName(int idx = -1) const;

      // these emulate the old name() and label() methods almost exactly:
      const char* label_c_str() const { return labelNotNull() ? label_.c_str() : name_.c_str(); }
      const char* name_c_str() const { return name_.c_str(); }

      /*! Get the tooltip for the knob */
      const std::string& tooltip() const;
      void tooltip(const std::string& p);


      /*! Add a tooltip override to the tooltip overrides table.
          'knob' is expected to be in the form 'Blur.size', and
          tooltip is the text to be displayed instead of the default
          tooltip */
      static void addTooltipOverride(const std::string& knob, const std::string& tooltip);

      // flags - be sure to fix documentation in Knob.C!

      typedef U64 FlagMask;

      // General flags (must not intersect any class-specific flags):
      // General knob flags begin at 0x80. 
      // Usually if you're adding a new General flag you just need to look at the last flag and use the next available value.
      // When adding a general flag, search the file first to make sure the value isn't already in use
      static constexpr const FlagMask  DISABLED                 = 0x0000000000000080; ///< Grey out and lock interface. Prevents copy/paste (see READ_ONLY to allow this).
      static constexpr const FlagMask  NO_ANIMATION             = 0x0000000000000100; ///< Disable right click and button animation menu.
      static constexpr const FlagMask  DO_NOT_WRITE             = 0x0000000000000200; ///< Disables calling to_script. No writing to script file or copy/paste.
      static constexpr const FlagMask  INVISIBLE                = 0x0000000000000400; ///< Disables param and viewer widgets. Cannot be made visible again. See HIDDEN for this.
      static constexpr const FlagMask  RESIZABLE                = 0x0000000000000800; ///< Allows more complex knobs to resize param panel to fill available space.
      static constexpr const FlagMask  STARTLINE                = 0x0000000000001000; ///< Start a new line in the param panel before knob widget.
      static constexpr const FlagMask  ENDLINE                  = 0x0000000000002000; ///< Start a new line in the param panel after knob widget.
      static constexpr const FlagMask  NO_RERENDER              = 0x0000000000004000; ///< Removes knob from Op hash calculation, preventing rerendering on value change.
      static constexpr const FlagMask  NO_HANDLES               = 0x0000000000008000; ///< Disables viewer widget handles from drawing.
      static constexpr const FlagMask  KNOB_CHANGED_ALWAYS      = 0x0000000000010000; ///< Always calls knob_changed, regardless of whether it has previously returned false.
      static constexpr const FlagMask  NO_KNOB_CHANGED          = 0x0000000000020000; ///< Prevents knob_changed being called on value change. Set if prev knob_changed returned false.
      static constexpr const FlagMask  HIDDEN                   = 0x0000000000040000; ///< Disables param panel and viewer widgets. Can be managed dynamically with show/hide.
      static constexpr const FlagMask  NO_UNDO                  = 0x0000000000080000; ///< Disables laying down of undo/redo points.
      static constexpr const FlagMask  ALWAYS_SAVE              = 0x0000000000100000; ///< Forces data to always be written regardless. Deprecated. Override not_default instead.
      static constexpr const FlagMask  NODE_KNOB                = 0x0000000000200000; ///< For internal use only.
      static constexpr const FlagMask  HANDLES_ANYWAY           = 0x0000000000400000; ///< Force viewer widgets to be visible regardless of current node tab.
      static constexpr const FlagMask  INDETERMINATE            = 0x0000000000800000; ///< Presents a blacked out undefined value interface on supporting knobs.
      static constexpr const FlagMask  COLOURCHIP_HAS_UNSET     = 0x0000000001000000; ///< Defines whether a color chip can be in the 'unset' state. Defaults to false.
      static constexpr const FlagMask  SMALL_UI                 = 0x0000000002000000; ///< Switches param panel widget to be more viewer Toolbar friendly in supported knobs (eg Button).
      static constexpr const FlagMask  NO_NUMERIC_FIELDS        = 0x0000000004000000; ///< Disables numeric input box widget on supported knobs.
      static constexpr const FlagMask  KNOB_CHANGED_RECURSIVE   = 0x0000000008000000; ///< Recursive knob_changed calls are prevented unless overriden using this flag.
      static constexpr const FlagMask  READ_ONLY                = 0x0000000010000000; ///< As with DISABLED, except value can be copied from and expression linked against.
      static constexpr const FlagMask  NO_CURVE_EDITOR          = 0x0000000020000000; ///< Disables curve editor.
      static constexpr const FlagMask  NO_MULTIVIEW             = 0x0000000040000000; ///< Disables view menu and splitting when in a multiview script.
      static constexpr const FlagMask  EARLY_STORE              = 0x0000000080000000; ///< Forces early synchronisation of data allowing usage in pre-op calls such as split_input().
      static constexpr const FlagMask  MODIFIES_GEOMETRY        = 0x0000000100000000; ///< Should be set on all knobs which modify geometry or associated transforms.
      static constexpr const FlagMask  OUTPUT_ONLY              = 0x0000000200000000; ///< Similar to READ_ONLY & NO_RERENDER together - data changes don't count as a script change.
      static constexpr const FlagMask  NO_KNOB_CHANGED_FINISHED = 0x0000000400000000; ///< Prevents knob_changed_finished being called on value change. Set if prev call returned false.
      static constexpr const FlagMask  SET_SIZE_POLICY          = 0x0000000800000000; ///< Do not use.
      static constexpr const FlagMask  EXPAND_TO_WIDTH          = 0x0000001000000000; ///< Force knob to expand to fill available space. - only for Enum knobs currently
      static constexpr const FlagMask  NEVER_DRAW_HANDLES       = 0x0000002000000000; ///< Disables viewer widget handles from drawing. Unlike the NO_HANDLES flag, the state of this flag will never change internally within Nuke
      static constexpr const FlagMask  KNOB_CHANGED_RIGHTCONTEXT= 0x0000004000000000; ///< Always call knob_changed on a properly cooked Op, even if KNOB_CHANGED_ALWAYS is on
      static constexpr const FlagMask  DONT_SAVE_TO_NODEPRESET  = 0x0000008000000000; ///< This value of this knob should never be saved to a NodePreset. Can be used, for example, for data knobs.
      static constexpr const FlagMask  RESERVED_COLORCHIP_KNOB  = 0x0000010000000000; ///< DO NOT USE. This value is used by the colorchip knob.
      static constexpr const FlagMask  READ_ONLY_IN_SCRIPTS     = 0x0000020000000000; ///< Prevents knobs from being modified from Python/Tcl
      static constexpr const FlagMask  ALWAYS_ALIGN_LABEL_TOP   = 0x0000040000000000; ///< Label is always aligned to the top of the Knob
      static constexpr const FlagMask  TINY_SLIDER              = 0x0000080000000000; ///< Modifies SLIDER to be a tiny slider underneath lineedit. Should be a numeric knob flag but we've overrun the < 0x80 condition.
      static constexpr const FlagMask  HIDE_ANIMATION_AND_VIEWS = 0x0000100000000000; ///< Prevents Animation Curve_Knob and Views being shown. Animation is still possible, unless NO_ANIMATION is set of course.
      static constexpr const FlagMask  NO_COLOR_DROPDOWN        = 0x0000200000000000; ///< Prevents Color Panel Dropdown from being available. Popup color panel will stil be available.
      static constexpr const FlagMask  NODEGRAPH_ONLY           = 0x0000400000000000; ///< Indicate that this knob should only be displayed when using the NodeGraph, since the Timeline uses gpuEngine, which might not support all the same knobs.
      static constexpr const FlagMask  NO_SCRIPT_EXECUTE        = 0x0000800000000000; ///< Prevents 'execute' being called on the knob
      static constexpr const FlagMask  MODIFIES_TIME            = 0x0001000000000000; ///< Should be set on all knobs which modify timing
      static constexpr const FlagMask  TOOLBAR_BUTTON_DRAWSTYLE = 0x0002000000000000; ///< This knob must be drawn in the style of Viewer toolbar knobs
      static constexpr const FlagMask  FLAGS_LOCKED             = 0x0004000000000000; ///< Used to lock modifications to this knobs flags
      static constexpr const FlagMask  DO_NOT_READ              = 0x0008000000000000; ///< Skip reading the knob from script, must be set before loading from script Enumeration_Knob only
      static constexpr const FlagMask  FROM_ASSET               = 0x0010000000000000; ///< This knob's value is supplied from an asset
      static constexpr const FlagMask  RESERVED_NUMERIC_KNOB    = 0x0020000000000000; ///< DO NOT USE. This value is used by numeric knobs.

      static constexpr const FlagMask  EXPRESSION_NO_ANIM       = 0x0040000000000000; ///< Special case for several root knobs. Enables expressions only on a knob but disables frame variable usage. frame, t and x variables will evaluate to 0 and the expression editor popup will show an error message to the user.

      // 
      // Knob type-specific flags.
      // These should be in the range < 0x80.
      // If you run out of flags for that knob type, use the next generic value, but make sure you also reserve it in the generic section.
      // See COLORCHIP_PRESERVE_ALPHA and RESERVED_COLORCHIP_KNOB as an example.
      //

      // Numeric knobs:
      static constexpr const FlagMask  MAGNITUDE                = 0x0000000000000001; ///< Enables switchable numeric box & slider to multiple boxes (array knob derived numeric knobs).
      static constexpr const FlagMask  SLIDER                   = 0x0000000000000002; ///< Enables slider on single numeric knob, or array knob with MAGNITUDE set (numeric knobs).
      static constexpr const FlagMask  LOG_SLIDER               = 0x0000000000000004; ///< Switches linear slider to log slider, or cubic depending on range (numeric knobs with SLIDER).
      static constexpr const FlagMask  STORE_INTEGER            = 0x0000000000000008; ///< Stores and presents integer value rather than float (numeric knobs).
      static constexpr const FlagMask  FORCE_RANGE              = 0x0000000000000010; ///< Forces stored and presented value to be clamped to range set (numeric knobs).
      static constexpr const FlagMask  ANGLE                    = 0x0000000000000020; ///< Switches widget for angle UI (single value numeric knobs).
      static constexpr const FlagMask  NO_PROXYSCALE            = 0x0000000000000040; ///< Disables proxyscaling on knobs supporting it (XY_Knob & WH_Knob derivatives).
      static constexpr const FlagMask  NO_PIXELASPECTSCALE      = 0x0020000000000000; ///< Disables pixel aspect scaling on knobs supporting it (XY_Knob & WH_Knob derivatives).

      // String Knobs
      static constexpr const FlagMask  GRANULAR_UNDO            = 0x0000000000000001; ///< Disables concatenation of minor undo events (string knobs)
      static constexpr const FlagMask  NO_RECURSIVE_PATHS       = 0x0000000000000002; ///< Badly named. Actually disables relative paths (string knobs).
      static constexpr const FlagMask  NO_TCL_ERROR             = 0x0000000000000004; ///< For strings containing TCL expressions, don't replace with TCL error messages if an error occurs

      // Enumeration
      static constexpr const FlagMask  SAVE_MENU                = 0x0000000002000000; ///< Forces menu entries to be written to script. Used by dynamic menus (enumeration knobs).
      static constexpr const FlagMask  EXPAND_TO_CONTENTS       = 0x0000000000000001; ///< Make Enumeration knobs adjust their width to the size of the largest munu item.
      static constexpr const FlagMask  EXACT_MATCH_ONLY         = 0x0000000000000002; ///< Make Enumeration knobs use exact match when setting a value. If an attempt is made to set an
                                                                            ///< invalid value, the knob will be put into an Error state.
      static constexpr const FlagMask  STRIP_CASCADE_PREFIX     = 0x0000000000000004; ///< Make Cascading Enumeration knobs not serialise out cascading prefixes

      // SceneView / Path knob
      static constexpr const FlagMask SINGLE_SELECTION_ONLY     = 0x0000000000000001; ///< Knob only allows one item to be selected at a time
      static constexpr const FlagMask SHOW_BUTTONS              = 0x0000000000000002; ///< Show Add Layer/Delete Layer buttons
      static constexpr const FlagMask SHOW_SCENE_PICK_BUTTON    = 0x0000000000000008; ///< Show Scene Graph Dialog button to choose Prim(s) from the Scene
      static constexpr const FlagMask SHOW_SELECT_PATHS_BUTTON  = 0x0000000000000010; ///< For PathExpression knobs - show Select Path button to select prims in mask.

      // BeginGroup
      static constexpr const FlagMask  CLOSED                   = 0x0000000000000001; ///< Stores the open/closed state of group knobs (group knobs).
      static constexpr const FlagMask  TOOLBAR_GROUP            = 0x0000000000000002; ///< Make the group into a viewer toolbar. General used via BeginToolbar (group knobs).
      static constexpr const FlagMask  TOOLBAR_LEFT             = 0x0000000000000000; ///< Defines which side of viewer toolbar appears on. Pick one at toolbar construction time (toolbar).
      static constexpr const FlagMask  TOOLBAR_TOP              = 0x0000000000000010; ///< Defines which side of viewer toolbar appears on. Pick one at toolbar construction time (toolbar).
      static constexpr const FlagMask  TOOLBAR_BOTTOM           = 0x0000000000000020; ///< Defines which side of viewer toolbar appears on. Pick one at toolbar construction time (toolbar).
      static constexpr const FlagMask  TOOLBAR_RIGHT            = 0x0000000000000030; ///< Defines which side of viewer toolbar appears on. Pick one at toolbar construction time (toolbar).
      static constexpr const FlagMask  TOOLBAR_POSITION         = 0x0000000000000030; ///< A mask for the position part of the flags

      // ChannelSet/Channel:
      static constexpr const FlagMask  NO_CHECKMARKS            = 0x0000000000000001; ///< Disable individual channel checkbox widgets (channel/channelset knobs).
      static constexpr const FlagMask  NO_ALPHA_PULLDOWN        = 0x0000000000000002; ///< Disable 4th channel pulldown widget (channel/channelset knobs).
      static constexpr const FlagMask  FULL_LAYER_ENABLED       = 0x0000000000000004; ///< channel/channelset knobs will always consider every channel in a layer to be enabled.
      static constexpr const FlagMask  FORCE_RGBA_LAYER         = 0x0000000000000008; ///< Disables the layer selection dropdown. Forces the layer to always be RGBA.

      // Format knob
      static constexpr const FlagMask  PROXY_DEFAULT            = 0x0000000000000001; ///< Sets default knob value from script proxy format rather than full res (format knob).

      // ColorChip knob
      static constexpr const FlagMask COLORCHIP_PRESERVE_ALPHA  = 0x0000010000000000; ///< The ColorChip_knob discards alpha values by default. Set this flag to make it keep them, instead.

      // Colorspace Knob
      static constexpr const FlagMask  ALLOW_NUKE_COLORSPACES   = 0x0000000000000001; ///< Allows the knob to display Nuke's native colorspaces if Color Management is set to Nuke.

      // Text knob
      static constexpr const FlagMask WORD_WRAP                 = 0x0000000000000001; ///< Enable word wrapping-wrapping for the Text_knob

      // Eyedropper knob
      static constexpr const FlagMask CHECKED                   = 0x0000000000000001; ///< Tick checkbox in the ticker

    private:
      static constexpr bool ContainsFlagMask(const FlagMask flags, const FlagMask toQuery) noexcept
      { return (flags & toQuery) != 0; }

      bool lockedFlagMask(const FlagMask args) const noexcept
      {
        const bool knobLocked{ContainsFlagMask(flags_, FLAGS_LOCKED)};
        const bool argsLocked{ContainsFlagMask(args, FLAGS_LOCKED)};
        if (!knobLocked || (knobLocked && argsLocked)) {
          return false;
        }
        return true;
      }

      void setFlagMask(const FlagMask toSet) noexcept
      { if (!lockedFlagMask(toSet)) flags_ |= toSet; }

      void clearFlagMask(const FlagMask toClear) noexcept
      { if (!lockedFlagMask(toClear)) flags_ &= ~toClear; }

      void invertFlagMask(const FlagMask toInvert) noexcept
      { if (!lockedFlagMask(toInvert)) flags_ ^= toInvert; }

    public:
      ////////////////////////////////////////////////////////////////
      // Note -
      // The value, view and animation set/get methods are defined in
      // Knob's ValueStuffI base class. See ValueStuffI.h.
      //
      // For example:
      //    bool set_animated(int channel = -1) const;
      //    bool is_animated(int channel = -1);
      //    bool clear_animated(int channel = -1);
      //    bool clear_animated_view(int view, int channel = -1);
      //    bool set_animated_view(int view, int channel = -1);
      //    bool is_animated_view(int view, int channel = -1);
      //  etc


      //! Assign the knob mask to the given parameter
      void flags(FlagMask f) noexcept { if (!lockedFlagMask(f)) flags_ = f; }

      //! Set the given flag in the knob mask
      void set_flag(FlagMask f) noexcept { setFlagMask(f); }

      //! Clear the given flag in the knob mask
      void clear_flag(FlagMask f) noexcept { clearFlagMask(f); }

      //! Invert the given flag in the knob mask
      void invert_flag(FlagMask f) noexcept { invertFlagMask(f); }

      //! Update the given flag in the knob mask
      //! @param b If true, set the flag, otherwise clear the flag
      void set_flag(FlagMask f, bool b) noexcept { if (b) setFlagMask(f); else clearFlagMask(f); }

      //! Return the knob mask
      FlagMask flags() const noexcept { return flags_; }

      //! Checks whether the given flag is set in the knob mask
      bool flag(FlagMask f) const noexcept { return ContainsFlagMask(flags_, f); }

      //! Checks whether any of the given flags is set in the knob mask
      bool any_of(FlagMask f) const noexcept { return flag(f); }

      //! Checks whether all of the given flags are set in the knob mask
      bool all_of(FlagMask f) const noexcept { return (flags_ & f) == f; }

      virtual bool priority_undo() const {return false;}

      /*! Create the widget to put in a panel for the knob and return
         it. This can return null if no widget should be created, this
         is what the default version does.

          addCallback() must be called to add some function that is
          called when the value changes or when the knob is destroyed,
          so that the widget can respond to it. Nuke source code currently
          wraps this in another function called add_widget_to_knob().
       */
      virtual WidgetPointer make_widget(const WidgetContext& context);


      /*! Create the secondary widget.
          This widget is intended to be be displayed below the existing label, if the label exists.
          The default implementation returns a NULL widget.

          return  WidgetPointer, a pointer to the widget which is to be displayed in the label
                  column, else, NULL if no secondary widget is to be displayed (in which case the label
                  will display as usual)
       */
      virtual WidgetPointer make_secondary_widget(const WidgetContext& context);
      
      /*!
         Cause any update to widgets due to the outputContext() being changed.
         The default version does nothing, but subclasses that display the
         animated value must do this.
       */
      virtual void updateUI(const OutputContext&);

      //! DEPRECATED - Please use NotificationCallbackReason for notifications
      //! or VisibilityCallback for visibility query.
      enum
        //mFnDeprecatedInNuke17("Move to addNotificationCallback/addVisibilityCallback")
        LegacyCallbackReason {
        kDestroying,
        kIsVisible, // Visibility query requires return value
        kUpdateWidgets,
        kChanged
      };

      //! DEPRECATED - Please use NotificationCallbackReason with
      //! addNotificationCallback or addVisibilityCallback.
      //!
      //! CallbackReason is available for backwards compatibility during
      //! deprecation.
      using CallbackReason = LegacyCallbackReason;

      //! Notification callbacks for knob and widget signals
      enum NotificationCallbackReason {
        kNotification_Destroying,
        kNotification_UpdateWidgets,
        kNotification_Changed
      };

      typedef int (*LegacyCallback)(void* closure, LegacyCallbackReason);

      /// Functor from which to derive to provide knob notification callbacks.
      struct NotificationCallbackFunctor {
        /// The destructor will be called when removing callbacks.
        virtual ~NotificationCallbackFunctor() {};
        /// operator== is required to identify callbacks.
        /// This is guaranteed only to be called with the same type so use:
        /// const MyFunctor& myFunctor = static_cast<const MyFunctor&>(cb);
        virtual bool operator==(const NotificationCallbackFunctor& cs) const = 0;
        /// Implement this function to receive callbacks.
        virtual int operator()(NotificationCallbackReason) const = 0;
        /// Add member variables to provide callback parameters.
      };

      /// Functor from which to derive to provide knob visibility callbacks.
      struct VisibilityCallbackFunctor {
        /// The destructor will be called when removing callbacks.
        virtual ~VisibilityCallbackFunctor() {};
        /// operator== is required to identify callbacks.
        /// This is guaranteed only to be called with the same type so use:
        /// const MyFunctor& myFunctor = static_cast<const MyFunctor&>(cb);
        virtual bool operator==(const VisibilityCallbackFunctor& cs) const = 0;
        /// Implement this function to receive callbacks.
        virtual int operator()() const = 0;
        /// Add member variables to provide callback parameters.
      };

      //! DEPRECATED - We allow use of the deprecated interface for now.
      //! Please migrate to the new notification and query callbacks.
      using Callback = LegacyCallback;

      /*!
         DEPRECATED - Please use addNotificationCallback or
         addVisibilityCallback.

         Add a callback to a GUI widget. Changes to the knob's visibility,
         enable, the value, and the destruction of the knob all send
         messages to this.

         This is used to query properties of the knob.  On providing a callback
         you are required to return a valid response for all query reasons. If
         more than one callback is provided, the result is any_of(ret_value).
       */
      mFnDeprecatedInNuke17("Move to addNotificationCallback/addVisibilityCallback")
      void addCallback(LegacyCallback cb, void* closure);

      /*!
         Add a notification callback to a GUI widget.

         This is used for notification of state changes to the knob
         (e.g. destruction, redrawing and value changes).
         The callback return value is not used.
       */
      template<typename CallbackPackageType>
      void addNotificationCallback(const CallbackPackageType& cs);

      /*!
         Add a visibility query callback to a GUI widget.

         Visibility is determined by bool(ret_value).  If more than one callback
         is provided, the result is any_of(ret_value).
       */
      template<typename CallbackPackageType>
      void addVisibilityCallback(const CallbackPackageType& cs);

      /*!
         DEPRECATED - Please use removeNotificationCallback or
         removeVisibilityCallback.

         Remove a callback added with addCallback(). You must not call
         this if your callback got the kDestroying message.
       */
      mFnDeprecatedInNuke17("Move to removeNotificationCallback/removeVisibilityCallback")
      void removeCallback( LegacyCallback cb, void* closure );

      /*!
         Remove a callback added with addNotificationCallback(). You must not
         call this if your callback got the kDestroying message.
       */
      void removeNotificationCallback( const NotificationCallbackFunctor& cs );

      /*!
         Remove a callback added with addVisibilityCallback(). You must not call
         this if your callback got the kDestroying message.
       */
      void removeVisibilityCallback( const VisibilityCallbackFunctor& cs );

      /*!
         DEPRECATED - Please use isNotificationCallbackRegistered or
         isVisibilityCallbackRegistered.

         Check if a callback was registered.
       */
      mFnDeprecatedInNuke17("Move to isNotificationCallbackRegistered/isVisibilityCallbackRegistered")
      bool isCallbackRegistered( LegacyCallback cb, void* closure ) const;

      /*!
         Check if a notification callback was registered.
       */
      bool isNotificationCallbackRegistered( const NotificationCallbackFunctor& cs ) const;

      /*!
         Check if a visibility callback was registered.
       */
      bool isVisibilityCallbackRegistered( const VisibilityCallbackFunctor& cs ) const;


      /*! \fn bool Knob::isEnabled() const;
         Return true if enable() was called or false if disable() was called.
       */
      bool isEnabled() const { return !(flags_ & DISABLED) && !disabledDueToLicensing(); }

      /*! \fn bool Knob::enable();
          Same as enable(true) */
      void enable() { enable(true); }

      /*! \fn bool Knob::disable();
          Same as enable(false) */
      void disable() { enable(false); }

      /*!
         Enable or disable (gray out) the widget.
         For historical reasons this will also show/hide the handles in the
         viewer by setting the NO_HANDLES flag. If you don't want this, set
         the flag back to the way you want after calling this.
       */
      virtual void enable(bool);
      virtual bool disabledDueToLicensing() const;

      /*! Return true if this knob is read only.  The default implementation
       *  tests for the read-only flag.  In read-only mode, values will be 
       *  displayed and can be copied and viewed as usual, but no modifications
       *  through direct user input will be possible.  Changes may still be
       *  made through Tcl/Python.
       *
       *  Not all knobs support this flag.
       */
      virtual bool isReadOnly() const;

      virtual void setReadOnly(bool value);

      /*! Return true if this knob is read-only in scripts.  The default implementation
       *  tests for the READ_ONLY_IN_SCRIPTS flag.  In this mode, attempts to modify
       *  a knob's values will be prevented with an error.  Changes may still be
       *  possible through the Nuke user interface (see READ_ONLY flag to block this)
       */
      bool isReadOnlyInScripts() const
      {
        return flag(Knob::READ_ONLY_IN_SCRIPTS);
      }

      void setReadOnlyInScripts()
      {
        set_flag(Knob::READ_ONLY_IN_SCRIPTS);
      }
      
      /*! \fn bool Knob::isVisible() const;
         Return true if the widget is visible (ie the control panel is open
         and switched to the tab that this knob is on). Knobs with no widgets
         return panel_visible().
       */
      bool isVisible() const;
      bool isHandleVisible() const;

      //! the number of panels open which contain a link knob pointing at this knob
      int openLinkCount() const { return _openLinkCount; }

      //! increase the openLinkCount
      virtual void openLink() { ++_openLinkCount; }

      //! decrease the openLinkCount
      virtual void closeLink()
      {
        if (_openLinkCount > 0) {
          --_openLinkCount;
        }
      }

      /*! \fn void Knob::hide()
         Make the widget disappear. This only works if the widget exists,
         which means you have to defer this until Op::knob_callback() is
         called with the show-panel knob.
       */
      void hide() { visible(false); }

      /*! \fn void Knob::show()
         Make the widget become visible, undoing a hide().
       */
      void show() { visible(true); }

      /*!
         Does show() if true, hide() if false.
       */
      void visible(bool);

      /*! \fn bool Knob::isOpen() const
         Returns true if this hierarchy title is open.
       */
      bool isOpen() const { return !(flags_ & CLOSED); }

      /*! \fn void Knob::open()
         Makes a hierarchy title open, showing all the child knobs.
       */
      void open() { open(true); }

      /*! \fn void Knob::close()
         Makes a hierarchy title close, hiding all the child knobs.
       */
      void close() { open(false); }

      /*! Does open() if true, close() if false. */
      void open(bool);

      /*! \fn bool Knob::isPushed() const;
         Return true if this knob is "pushed". This means that the user is
         either holding down a slider in the control panel or is dragging
         a handle in the viewer. This can be used to decide whether to draw
         a texture map preview or not.
       */
      bool isPushed() const { return _pushed; }

      /*! \fn void Knob::pushed( bool p );
         Turn isPushed() on/off.
       */
      void pushed( bool p ) { _pushed = p; }

      /*! Return another knob on the same control panel as this knob, looked
          up by name. */
      Knob* knob(const char*) const;

      /*! Find a knob by index number. If the index number is larger than
          the number of knobs then null is returned. By starting at zero
          and incrementing until this returns null you will get every Knob
          known about. One of the return values will be this knob itself.
       */
      Knob* knob(int) const;

      /*! Return true if the node this knob is in is disabled. */
      bool node_disabled() const;

      /*! Returns true if the control panel for this node is open. You may
         want to use visible(), which will return true only if the tab
         containing this knob is open.
       */
      bool panel_visible() const;

      /*! Returns true if the node is selected by the user. This can be used
         to turn a wireframe preview on/off.
       */
      bool node_selected() const;

      /*! Returns the color selected by the user to draw things in the viewer.
         The return value is of the form 0xrrggbb00.
       */
      unsigned node_gl_color() const;

      /*! Execute a string in Nuke's scripting language.
         You \e must call script_unlock() after calling this!
         \see Op::script_command
       */
      bool script_command(const char* command, bool py = true, bool eval = true) const;

      /*! \see Op::script_expand().
          The integer argument is unused currently. Use 1 for future
          compatibility.
       */
      bool script_expand(const char* str, int what = 1) const;
      /*! \see Op::script_result(). */
      static const char* script_result(bool py = false);
      /*! \see Op::script_unlock() */
      static void script_unlock();
      /*! \see Op::message_f */
      static int message(char t, const char*, ...);
      /*! \see Op::message_f */
      static int vmessage(char t, const char*, va_list);

      virtual void reset_to_default() { from_script(""); }

      ////////////////////////////////////////////////////////////////
      // For drawing handles into the OpenGL viewer:

      /*! This call is used by viewers to search for objects that need to
          draw "handles" into the viewer. This will be called for all knobs
          on any opened control panel.

          If you want to draw anything you should add a callback with
          ViewerContext::add_draw_handle(). Or you can return true and
          a callback that calls draw_handle() will be added for you.

          Recommended you check that the viewer is in the correct mode
          (ctx->viewer_mode()!=0 indicates a 3D mode) and that visible()
          is true before you add the callbacks.

          The default version returns false.
       */
      virtual bool build_handle(ViewerContext*);

      /*! Convenience function to call ViewerContext::add_draw_handle() with
          a function that will call this->draw_handle().
       */
      void add_draw_handle(ViewerContext*);

      /*! If build_handle() returns true, then this will be called to actually
          draw the handle in OpenGL.
       */
      virtual void draw_handle(ViewerContext*);

      typedef std::vector<int> TKnobIndices;
#     if 1 // def DD_IMAGE_EXPERIMENTAL_HANDLES
      typedef bool (HandleBatch)(ViewerContext* ctx, Knob*, const int focusIndex, const TKnobIndices& indices);
#     endif //def DD_IMAGE_EXPERIMENTAL_HANDLES
      typedef bool (Handle)(ViewerContext* ctx, Knob*, int index);

      /*! Commands passed to begin_handle */
      enum HandleType {
        INVALID = -1,
        POSITION = 0,
        SELECTABLE,
        DISTANCE_FROM_POINT,
        DISTANCE_WITH_SNAP,
        // non-hit-detect commands:
        ANYWHERE = 20,
        ANYWHERE_MOUSEMOVES,
        ANYWHERE_KEY_PRESSED,
        SELECTED_BY_THIS
      };

      enum HandleFlags {
        HOVER                               = mFnBIT(0),
        USE_CURRENT_COLOR                   = mFnBIT(1),
        LOW_PRIORITY                        = mFnBIT(2),  ///< Indicates that this handle is low priority in terms of selecting.
                                                          ///< If multiple handles are contained within the pick area, normal
                                                          ///< priority handles are favoured over low priority ones.
        DO_HANDLE_CALLBACK_ON_SHIFT_SELECT  = mFnBIT(3)   ///< This flag is to get around the fact that, when shift-selectting, 
                                                          ///< Nuke doesn't by default call handle callbacks. Setting this flag 
                                                          ///< will ensure that it will.
      };
      /*! \class DD::Image::Knob::HandleContext
          Context for handles that captures the conditions that the handle should be drawn with and the expected
          behaviour of the handle. */
      class DDImage_API HandleContext
      {
      public:
        HandleContext(HandleType type);
        HandleContext(HandleType type, unsigned int flags);
        HandleContext(const HandleContext& other);

        ~HandleContext();
        HandleContext& operator=(const HandleContext& other);

        HandleType type() const { return _type; }
        void setType( HandleType type ) { _type = type; }
        void setFlags( unsigned int flags ) { _flags = flags; }
        unsigned int flags() const { return _flags; }

        //! Method for internal use only.
        HandleContextImpl* pImpl() const { return _pImpl.get(); }

      private:
        HandleType _type;
        unsigned int _flags{0};
        std::unique_ptr<HandleContextImpl> _pImpl;
      };

      /*! Start drawing a handle. Any OpenGL you draw after this will be
          passed through hit-detection on a PUSH event and if selected then
          the \a cb will be called with events.  draw_handle() should call
          this to indicate that the next thing to be drawn is something the
          user can click on, which we call a "handle".

          The \a command describes the xyz that will be passed back to the
          handle callback. Current values are:

          - POSITION: The xyz passed in plus the distance the mouse dragged,
          with the numbers rounded somewhat to "nice" decimal numbers, and
          if the mouse has not moved very far or if shift is held down to
          restrict the axis, the passed-in xyz unchanged.

          - SELECTABLE: Same as POSITION except this is a selectable
          handle. It is selected by the user clicking on it, by the user
          dragging out a box, or by shift-clicking.  When the user drags any
          selected handle then all of them get PUSH, DRAG, and RELEASE
          events and the first one gets the rounded position and all the
          others move in a locked formation with it.  Any keystrokes are
          sent as KEY events to all the selected handles. If \a cb calls
          ctx->menu() then the menu only appears once, and all other calls
          return the same menu item, so the same action is done to all points.

          - DISTANCE_FROM_POINT: The difference between where the mouse is
          and the xyz position passed in this call (pass 0,0,0 if you want
          the absolute position in space). The xyz are not rounded so you
          probably don't want the numbers shown to the user, as they will
          have ugly extra digits of precision.

          - DISTANCE_WITH_SNAP: Same as DISTANCE_FROM_POINT except the delta
          values are rounded to "nice" numbers. Pass 0,0,0 to get "nice"
          rounded positions in space.

          - ANYWHERE, ANYWHERE_MOUSEMOVES, SELECTED_BY_THIS: these should
          only be passed to make_handle.

          The \a cb function is called when the user causes an event that
          Nuke thinks should be sent to this handle. The function should
          return true if the event is consumed by this handle (returing
          false allows an ANYWHERE handle to get events).
          The \a context->event() says why this is being called:

          - PUSH - mouse was pushed down

          - DRAG - mouse was dragged after being pushed. The mouse position can
          be retrieved from context->x(), y(), and z(). These coordinates are
          in the same transformation that make_handle() was called in. You can
          also get the mouse position in pixels from context->mouse_x() and
          mouse_y(), though I'm not sure if you can do anything useful with
          that.

          - RELEASE - mouse was released after being dragged.

          - KEY - A keyboard key was pushed. You can get it from context->key().

          - KEYUP - A keyboard key was released. You can get it from context->key().

          - WHEEL - Mouse wheel (or other secondary manipulator) was moved. You can
          get the amount it was moved (in pixels) from context->wheel_dx() and
          wheel_dy().

          - MOVE - mouse is being dragged over window with no button pushed.

          - FIRST_MENU - You should use this as the first enumeration assigned
          to items in a Menu. If you call ctx->menu() and an item is picked
          the event is set to the picked item.

          - NO_EVENT - If you call ctx->menu() and either the user does not pick
          an item or they already picked an item from a different menu, then
          the event is set to this. You should not do anything in this case.

          \a index is passed unchanged to the \a cb function. This can be
          used to select what part of a knob is being moved.

          \a x and \a y (and \a z) indicate the initial position of the
          dragged object. Notice that it does not have to be the location
          you are drawing at.

       */
      void begin_handle(HandleContext command, ViewerContext* ctx,
                        Handle* cb, int index, const DD::Image::Vector3& pos, ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
#     ifdef DD_IMAGE_EXPERIMENTAL_HANDLES
      void begin_handle(HandleContext command, ViewerContext* ctx,
                        HandleBatch* cb, int index, const DD::Image::Vector3& pos, ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
#     endif //def DD_IMAGE_EXPERIMENTAL_HANDLES

      /*! \fn void Knob::begin_handle(ViewerContext* ctx, Handle* cb, int index, float x, float y, float z)
         Same as begin_handle(POSITION,...)
       */
      inline
      void begin_handle(ViewerContext* ctx,
                        Handle* cb, int index, const DD::Image::Vector3& pos, ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
#     ifdef DD_IMAGE_EXPERIMENTAL_HANDLES
      inline
      void begin_handle(ViewerContext* ctx,
                        HandleBatch* cb, int index, const DD::Image::Vector3& pos, ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
#     endif //def DD_IMAGE_EXPERIMENTAL_HANDLES

      inline
      void begin_handle(HandleContext command, ViewerContext* ctx,
                        Handle* cb, int index, float x = 0, float y = 0, float z = 0, ViewerContext::Cursor cursor = ViewerContext::kNoCursor );

      inline
      void begin_handle(ViewerContext* ctx,
                        Handle* cb, int index, float x = 0, float y = 0, float z = 0, ViewerContext::Cursor cursor = ViewerContext::kNoCursor );

      /*! draw_handle() should call this after calling begin_handle() and
          drawing the selectable items. It is harmless to call this extra times.
       */
      void end_handle(ViewerContext* ctx);

      /*! Calls begin_handle(), draws a small square (using GL_POINTS), and
          calls end_handle(). The arguments are the same as for begin_handle().

          If you don't want the square to be visible to the user you can
          call this only when ctx->event() is equal to PUSH.

          You can also use this to create some special handles that do not
          use OpenGL hit detection:

          <ul>

          <li>ANYWHERE - A click anywhere in the window, or any keystroke
          not handled by another handle, calls this handle. You do not need
          to draw anything or call end_handle(). You will get PUSH and DRAG
          and RELEASE events with mouse coordinates rounded to nice values
          relative, similar to DISTANCE_WITH_SNAP. You will also get KEY
          events for any keys that the normal handles do not want.

          <li>ANYWHERE_MOUSEMOVES - Similar to ANYWHERE, but additionally
          moving the mouse anywhere in the window will cause MOVE events to
          be generated.

          <li>SELECTED_BY_THIS - This should only be called by a Handle in
          response to a PUSH event. It will cause this new handle to be
          added to the selected set if it is not in there already.  This
          allows one handle to control the selection of several handles, for
          instance clicking on the line of a Bezier will select all the
          points in that line, this is done by having the line Handle
          function call this with every point. Nuke will clear other
          selected handles and toggle all of them on and off depending on
          the state of the shift key and the first one of these.

          </ul> */
#     ifdef DD_IMAGE_EXPERIMENTAL_HANDLES
      void make_handle(HandleContext command, ViewerContext* ctx,
                       HandleBatch* cb, const int index, const DD::Image::Vector3& pos, const ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
#     endif //def DD_IMAGE_EXPERIMENTAL_HANDLES

      /*! \fn sub-optimal vesion of the above, this will call 'cb' once per 'index' whereas
              the above will call cb once with all indexes registered for that knob.
              */
      void make_handle(HandleContext command, ViewerContext* ctx,
                       Handle* cb, const int index, const DD::Image::Vector3& pos, const ViewerContext::Cursor cursor = ViewerContext::kNoCursor );

      inline
      void make_handle(ViewerContext* ctx,
                       Handle* cb, const int index, const DD::Image::Vector3& pos, const ViewerContext::Cursor cursor = ViewerContext::kNoCursor );

      // deprecated
      inline
      void make_handle(HandleContext command, ViewerContext* ctx,
                       Handle* cb, const int index, const float x, const float y, const float z = 0.0f, const ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
      // deprecated
      inline
      void make_handle(ViewerContext* ctx,
                       Handle* cb, const int index, const float x, const float y, const float z = 0.0f, const ViewerContext::Cursor cursor = ViewerContext::kNoCursor );

#     ifdef DD_IMAGE_EXPERIMENTAL_HANDLES
      /*! \fn void Knob::make_handle(ViewerContext* ctx, HandleBatch* cb, int index, float x, float y, float z)
         Same as make_handle(POSITION,...)
       */
      inline
      void make_handle(ViewerContext* ctx,
                       HandleBatch* cb, const int index, const DD::Image::Vector3& pos, const ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
      //! deprecated
      inline
      void make_handle(HandleContext command, ViewerContext* ctx,
                       HandleBatch* cb, const int index, const float x, const float y, const float z = 0.0f, const ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
      //! deprecated
      inline
      void make_handle(ViewerContext* ctx,
                       HandleBatch* cb, const int index, const float x, const float y, const float z = 0.0f, const ViewerContext::Cursor cursor = ViewerContext::kNoCursor );
#     endif //def DD_IMAGE_EXPERIMENTAL_HANDLES

      /*! Return true if the \a cb was flagged as a SELECTABLE one and it is
          currently selected by the user.
       */
      bool is_selected(ViewerContext* ctx, Handle* cb, int index) const;
#     ifdef DD_IMAGE_EXPERIMENTAL_HANDLES
      bool is_selected(ViewerContext* ctx, HandleBatch* cb, int index);
#     endif //def DD_IMAGE_EXPERIMENTAL_HANDLES

      /*! Return true if the main SELECTABLE callback is
          currently selected by the user.
       */
      virtual bool is_selected(ViewerContext* ctx, int index) const { return false; };

      /*! Return a vector of all selected indices for the given cb
       */
      void get_selected_indices(Handle* cb, SelectedIndexVector& allIndices) const;

      /*! Clear the selection list
         Return true if successful.
       */
      bool clear_selected(ViewerContext*);

      /*! Toggle the selection
         Return true if successful.
       */
      bool toggle_selected(ViewerContext* ctx, Handle* cb, int index);
#     ifdef DD_IMAGE_EXPERIMENTAL_HANDLES
      bool toggle_selected(ViewerContext* ctx, HandleBatch* cb, int index);
#     endif //def DD_IMAGE_EXPERIMENTAL_HANDLES

      /*! Toggle the selection with the main SELECTABLE callback.
          Return true if successful.
       */
      virtual bool toggle_selected(ViewerContext* ctx, int index) { return false; };

      /*! Returns true if the \a cb was flagged as HOVER, and it is
          currently in focus by the user
       */
      bool is_focused(ViewerContext*, Handle* cb, int index);
#     ifdef DD_IMAGE_EXPERIMENTAL_HANDLES
      bool is_focused(ViewerContext*, HandleBatch* cb, int index);
#     endif //def DD_IMAGE_EXPERIMENTAL_HANDLES

      /*! Return the selection size
       */
      int selection_size(ViewerContext*);

      ////////////////////////////////////////////////////////////////
      // Handle functions can call these:

      /*!
        Add the current knob into the last undo event. This allows changes
        from different knobs to be grouped into a single event.
       */
      void add_to_undo();

      /*!
        Stop adding to undo
       */
      void stop_add_to_undo();

      /*!
         Same as new_undo(0). If this knob has not been recorded into the
         current Undo, start a new Undo and record the current state.
       */
      void new_undo(const char* name = nullptr);

      /*!
         Closes (possible) previous open undo event and starts a new one
         before recording the current state. This should be called by knobs instead
         of new_undo(...) when the action should not be merged with any
         previous open undo event.
       */
      void force_new_undo(const char* name = nullptr);

      /*!
         Make changes to this knob be part of the current Undo.

         Deprecated. You should not need to call this, Nuke merges undos
         for everything done by an Op::knob_changed() or other functions that
         should look like a single action to the user.

         TODO : mark as deprecated
       */
      void extra_undo();

      /*!
         Start a new Undo and remember the current state in that Undo,
         if either:
         - \a this knob has not been recorded in the current Undo
         - \a widget is non-zero and different than the previous call

         The intention was for \a widget to be the widget being manipulated,
         though you can also pass pointers to static variables that indicate
         the "class" of widget, for instance if you want a set of checkmarks
         to all act like a single Undo.

         You can call repeatedly with \a widget == NULL and subsequent calls
         are ignored. This makes it convienent to stuff this call into any
         method that is going to change the value.

         \see extra_undo
       */
      void new_undo(const void* controller);
      bool doingUndoRedo();

      virtual Knob* undoer()
      {
        return _undoer ? _undoer : this;
      }

      void setUndoer(Knob* undoer)
      {
        _undoer = undoer;
      }

      static int nudgeController;

      /*! \fn void Knob::new_nudge_undo()
         Call new_undo(widget) with a pointer to a constant that indicates
         a "nudge" modification, such as the numeric keypad keys. All
         adjacent nudge modifications for a knob are merged into a single undo.
       */
      void new_nudge_undo() { new_undo(&nudgeController); }

      /*!
         If possible this backs up to the last time new_undo() was called.
         However this may back up many other knobs to the same state, so
         only call this on one knob per panel!

         \see redo, new_undo, new_nudge_undo
       */
      void undo();

      /*!
         If possible this goes forward and undoes an undo.
         However this may back up many other knobs to the same state, so
         only call this on one knob per panel!

         \see undo, new_undo, new_nudge_undo
       */
      void redo();

      /*!
         This virtual function is called when the value of the knob
         changes.  Subclasses can override this, they should not do anything
         very expensive (such as read a file) or Nuke will lock up! They must
         call the base class implementation, it calls UpdateWidgets(), sets
         the modified flag, increments \a _editVersion, and triggers the viewer
         to check if it needs to update, among other things.
       */
      virtual void changed();

      /*! Global edit version of the Knob, incremented when the contents of the
          Knob are modified and acknowledged by calling Knob::changed(). This
          number is useful for tracking individual Knob edits separately from
          the global Nuke version numbers returned by version().

          Note that this number will only change if the Knob subclass stores data
          and calls Knob::changed() when that data changes. Some that don't
          are called 'compound' knobs, like AxisKnob, which consist of child
          knobs where the actual data is stored. These knobs should implement
          editVersionHash() and return the edit versions of the children
          combined together.
      */
      int editVersion() const;

      /*! Calculate the edit state hash for the Knob's current edit version. This
          can be implemented by compound-knob type subclasses like AxisKnob to
          combine the edit versions of its children to provide a compound edit
          version.

          Base class returns the hash of the _editVersion integer.
      */
      virtual Hash editVersionHash() const;

      /*!
         Sometimes (especially in subclasses) you know that only the
         widget has to redraw and that the knob's value did not change.
         In this case you can call this function instead of changed(), to avoid
         triggering unnecessary recalculation.
       */
      void updateWidgets();

      /*!
         See Op::asapUpdate(). Indicate the output of the Ops controlled by this
         node are dynamically changing. This causes the viewer to update right
         away and disables disk caching of viewer images.

         You must make the hash() of the Op change. If you are altering the
         knob's value, then this is probably enough to make the hash change.
       */
      void asapUpdate();

      /*!
         See Op::asapUpdate(). Indicate the output of the Ops controlled by this
         node are dynamically changing and that the indicated area of the viewer
         should be drawn first. This causes the viewer to update right
         away and disables disk caching of viewer images. The direction indicates
         which way to draw the box, zero indicates you don't care. This is used
         by paint strokes, setting the direction to the same way the paint brush
         is moving makes the best feedback.
       */
      void asapUpdate(const Box& box, int direction = 0);

      /*!
         Make any handles in the viewer redraw, such as to show highlighting
         or previews, without the overhead or setting modified flags that
         changed() does.
       */
      void redraw();
      void undoless(bool undoless);

      ////////////////////////////////////////////////////////////////
      // Storage of the current value into an Op:

      /*!
         Return the "user interface context" from the node this knob belongs
         to. This is best thought of as "what frame/view the user thinks the
         Node is at". In reality, a node may produce several ops at different
         frames/views/proxy settings, \e one of these settings is returned
         by this.
       */
      const OutputContext& uiContext() const;

      /*!
         Same as Op::input_format(). A store() method can use this to scale uv
         coordinates to pixels or to take into account pixel aspect when storing
         width/height pairs.
       */
      const Format& input_format() const;

      /*!
         Write over the passed location (which is in an Op) with the
         new data, and append that data to the Hash somehow. This is
         how data is stored into the pointed-to locations referred to
         by the knobs() function.

         The context is used to figure out the current value from an animated
         knob, and to scale the stored values to the current proxy
         resolution. The hash should depend on these values.

         If there is any problem storing the value you should call error()
         with a message to show the user. You should still put a legal
         value there and return the correct hash.

         The base class does nothing.
       */
      virtual void store(StoreType, void* p, Hash& hash, const OutputContext& context);

      /*!
         Append a representation of the current value() at the context to
         the hash. Nuke will use this to calculate the hash of any knob that
         does not have a pointer for the store() function but also does not
         have the NO_RERENDER flag on.

         If the \a context is null, then you should instead append the entire
         animation of the knob to the hash. This is used to identify if \e
         any frame in a clip has changed.

         The base class does nothing.
       */
      virtual void append(Hash& hash, const OutputContext* context);

      ////////////////////////////////////////////////////////////////

      /*! Return the corresponding compound Value::Type this knob stores
          to, if there is one.
          Note this method returns an int to avoid including the ndk headers,
          so casting to usg::Value::Type may be required.

          Also note that this is not the type of each individual data element
          but the -overall- logical type for the entire Knob. Many Knob types
          may not have a logical correspondence to a Value::Type.
          For example Color_knob returns Value::Color3d, XY_knob returns
          Value::Double2, while AColor_knob returns Value::Double4 since
          there is no corresponding RGBA Value::Type.

          Knobs that manage arbitrarily-sized arrays of data elements can
          return the generic Value::Type array variants like FloatArray,
          DoubleArray, IntArray, etc. Consuming code will need to know how
          to convert those generic arrays.

          Defaults to -1 which should be interpreted as Value::InvalidType.
      */
      virtual int    valueType() const { return -1; }

      /*! Return the number of data elements that this knob stores into, which
          is most useful for numeric type knobs.
          For example a Color_knob stores into 3 elements (rgb floats or doubles)
          while AColor_knob stores into 4 elements (rgba floats or doubles,) and
          Axis_knob stores into 16 elements (Mat4f or Mat4d.)

          Some knob ctors such as Array_knob and MultiFloat_knob have dimension
          arguments so these subclasses should return the knob's currently sized
          dimension.

          Defaults to 1.
      */
      virtual size_t numElements() const { return 1; }


      /*! Set the value on the Knob, returning true on success.

          Currently this only supports Knob types that accept the fdk::math base types
          like Vec2,Vec3,Vec4,Box2,Box3,Mat4, or the common POD types like float, double,
          uint, int, and bool, or some basic stl types like std::string.

          \a channel_start_offset is only valid for Knobs storing compound numeric data types
          like Vec2,Vec3,Vec4. It's most useful for setting a specific element/channel within the
          type. For example to set an AColor_Knob's Vec4.w component (alpha) while leaving the
          Vec4.xyz (color) portion untouched you would do knob->set(0.5, time, 3, view).

          All the components of the source type are copied into the Knob, and any overflow
          components get clipped, or ignored. So copying a Vec3 into a Vec4 Knob with a
          \a channel_start_offset of 2 means only Vec3.xy will get copied into the Knob's Vec4.zw
          components, and Vec3.z will get clipped. If the Vec3 is copied into the Vec4 Knob
          with \a channel_start_offset of 0 then Vec3.xyz will be copied in and Vec4.w will be
          left untouched.

          Requires a keyframe time to set the value at. Does not clear any existing animation
          data.
          If view is <= 0 then the 'main' view index (1) is used.
      */
      template <typename T>
      bool  set(const T& value,
                double   time,
                int32_t  channel_start_offset = 0,
                int32_t  view = -1);

      /*! Same as set(T, time, view) but with arrays of T and time. All values will be
          set on the same view.
          Destination channels unaffected by the source data type size (for example a
          Vec2 being copied into a Vec4) will be unaffected - ie not cleared or set
          animated.

          If \a clear_keys is true then any existing animation data is cleared before
          any keyframes are set, otherwise the keyframes are added to any existing
          animation.

          Both \a values and \a times arrays should be the same size! The smaller array
          size of the two is used for the total value/time count so any values or times
          past the smaller count will be ignored.
      */
      template <typename T>
      bool  set(bool                       clear_keys,
                const std::vector<T>&      values,
                const std::vector<double>& times,
                int32_t                    channel_start_offset = 0,
                int32_t                    view = -1);


      /*! Return the value of the Knob converted to type T, if possible. Nothing is
          done if the type and Knob are incompatible, and the returned value is
          undefined.

          Currently the base fdk::math types are supported like Vec2,Vec3,Vec4,Box2,
          Box3,Mat4, plus the common POD types like float, double, uint, int, bool,
          and std::string.

          Examples of how to return a specific type:
            double  value = knob->get<double>(context);  // FloatKnob, ArrayKnob
            int32_t value = knob->get<int32_t>(context); // IntKnob, ArrayKnob
            Vec2i   value = knob->get<Vec2i>(context);   // XYKnob, UVKnob, ArrayKnob
            Vec3d   value = knob->get<Vec3d>(context);   // XYZKnob, ColorKnob, ArrayKnob
            Vec4f   value = knob->get<Vec4f>(context);   // ColorAKnob, ArrayKnob
            Mat4d   value = knob->get<Mat4d>(context);   // AxisKnob, ArrayKnob
            std::string value = knob->get<std::string>(context);  // StringKnob, EnumerationKnob

          If OutputContext is null then Knob::uiContext() will be used.
      */
      template <typename T>
      T      get(const OutputContext* context = nullptr) const;

      /*! Fill the passed-in data object if its type is compatible with the Knob.
          Nothing is done if the type and Knob are incompatible, and \a val is left
          untouched.

          Currently the base fdk::math types are supported like Vec2,Vec3,Vec4,Box2,
          Box3,Mat4, plus the common POD types like float, double, uint, int, and bool.

          If OutputContext is null then Knob::uiContext() will be used.
      */
      template <typename T>
      void   get(T&                   val,
                 const OutputContext* context = nullptr) const;

      /*! Copy all the time sample values from the Knob. If the Knob is not animated
          there will be only single value and a single defaultTimeValue() time entry.

          If view is not specified the default 'main' (view 1) will be used.

          See the other get() methods for more usage info.
      */
      template <typename T>
      size_t get(std::vector<T>&      values,
                 std::vector<double>& times,
                 int32_t              view = -1) const;


      /*! Does the knob have animation affecting its value over time?

          This method indicates that the knob value -may- vary in time, but not that the
          value -actually- varies in time. For example the knob's animation curve could
          contain a constant value at all keyframes but isAnimating() will still return
          true.

          This is a virtual method so that compound Knobs which contain subknobs, like
          Axis_Knob, can return the global animation state for all of them.

          Default implementation returns false.
      */
      virtual bool  isAnimated() const { return false; }

      /*! Does the knob have an expression affecting its value?

          This is a virtual method so that compound Knobs which contain subknobs, like
          Axis_Knob, can return the global expression state for all of them.

          Default implementation returns false.
      */
      virtual bool  hasExpression() const { return false; }


      /*! Inserts all keyframe times for all Knob channels into the set.
          Primarily useful for ArrayKnob-based knobs which support time-animating values.

          Does not initialize/clear the set! This is so you can call this method on multiple
          knobs and union them all together.
          If view is provided then only that view is retrieved, otherwise the main (view 1)
          will be retrieved.

          Default implementation does nothing.
      */
      virtual void   animationTimes(std::set<double>& times,
                                    int32_t           view = -1) const {}

      /*! Same as animationTimes() but for a single channel.
          If \a channel_index is out of bounds nothing is done.
          Default implementation does nothing.
      */
      virtual void   animationChannelTimes(std::set<double>& times,
                                           int32_t           channel_index,
                                           int32_t           view = -1) const {}

      ////////////////////////////////////////////////////////////////

      static Knob& showPanel; //!< passed to Op::knob_changed() when panel opened
      static Knob& hidePanel; //!< passed to Op::knob_changed() when panel closed
      static Knob& inputChange; //!< passed to Op::knob_changed() when inputs change
      static Knob& nameChange; //!< passed to Op::knob_changed() when Node name is changed
      static bool undo_state; //!< true in Op::knob_changed() when called due to undo/redo

      ////////////////////////////////////////////////////////////////

      // Add tick marks to viewer:

      /*! Turn on a flag in the \a bl array for each frame number between \a
          first and \a last that contains a key frame, and return the total
          number of keyframes. This is used to update the tick mark display
          in the Viewer frame slider.  The base class returns zero. */
      virtual int get_key_list(std::set<int>& keylist) const;
      virtual int get_key_list(std::set<int>& keylist, std::set<int>& predictedKeylist) const;
      virtual int get_key_list(std::set<int>& keylist, int index) const;
      virtual int get_key_list(std::set<int>& keylist, std::set<int>& predictedKeylist, int index) const;

      virtual bool splitForViews() const { return false; }
      virtual ViewSet storedViews() const { return ViewSet(0); }

      /// Whether the viewer should display the tickmarks for me at the moment
      /// subclasses can override this to display tickmarks for invisible knobs
      virtual bool keyframesDisplayed() const
      {
        return isEnabled() && isVisible();
      }

      ////////////////////////////////////////////////////////////////
      // Errors and warnings
      
      //! Use these to report once-off warning or error messages produced by a knob.
      //! They'll only be displayed as message boxes, and won't be stored.
      //! The versions that take a knob parameter use parameters from that knob
      //! they're called on, such as the pop-up scheme from any associated op.
      //\{
      static void error(const char* lpFormat, ...);
      static void warning(const char* lpFormat, ...);
      static void critical(const char* lpFormat, ...);
      static void debug(const char* lpFormat, ...);

      static void error(Knob* lpKnob, const char* lpFormat, ...);
      static void warning(Knob* lpKnob, const char* lpFormat, ...);
      static void critical(Knob* lpKnob, const char* lpFormat, ...);
      static void debug(Knob* lpKnob, const char* lpFormat, ...);
      //\}
      
      //! Callback function for internal use
      static void (*pMessageCallback)(const Knob* lpKnob, const OpMessage& lrMessage);    // lpKnob may be NULL
      
      
      ////////////////////////////////////////////////////////////////
      // From-script:

      /*!
         Parse a string read from a saved script. The result should be a knob
         that is in exactly the same state as it was when to_script() was
         called to produce the string.

         The return value is whether or not the value changed. If it did
         this must also call changed(). Note it should do this even if
         an error() is produced.

         If any errors are encountered you should call error() to print
         a message.

         Nuke provides a helper class Knob::String_List to split nested
         lists of words at spaces (this actually uses tcl to do the work),
         and the to_double() and to_bool() methods to turn words into
         values.

         The default version produces an error. You don't need to implement
         this as long as not_default() is always false.
       */
      virtual bool from_script(const char* v);
      bool set_text(const char* v) { return from_script(v); }

      /*! from_script() can call this to convert a word into a double. This
          will run the Nuke expression evaluator so that math expressions
          typed in by the user will be handled. If there is any problem
          a message is printed with error() and false is returned. If
          true is returned then the second argument is set to the resulting
          double value.

          If you wish to write doubles in to_script(), using << works acceptably.
       */
      bool to_double(const char*, double& result);

      /*! from_script() can call this to convert a word into an integer
          value. The argument can be any legal expression. If it is not
          an integer then int(floor(x)) is returned.
       */
      bool to_int(const char*, int& result);

      /*! from_script() can call this to convert a word into a boolean
          value. The argument can be any legal expression that evaluates
          to 0, non-zero, or the words "false" or "true".
       */
      bool to_bool(const char*, bool& result);

      //-----------------------------------------------------------------

      //! Assigns the authoring handler to the knob. If the handler is non-null
      //! and has at least one authoring mode enabled then authoring is
      //! considered enabled on this knob.
      //! Setting this to null effectively disables the authoring controls for
      //! this knob.
      void setAuthoring(Authoring* handler);

      //! Gets the attribute authoring controls - these modify the way
      //! attributes are written and overridden in the 3D system.
      //! @return Authoring functionality, or nullptr if authoring isn't set
      const Authoring* getAuthoring() const;
            Authoring* getAuthoring();

      //! Return the default authoring mode enum, if enabled. Cast to Authoring::Mode.
      int getDefaultAuthoringMode() const;

      //! Return the current authoring mode enum, if enabled. Cast to Authoring::Mode.
      int getAuthoringMode() const;

      //! Change the currently-assigned authoring handler's mode, if there is one.
      //! Cast Authoring::Mode to int.
      //! Returns true if the mode was changed.
      bool setAuthoringMode(int authoringModeEnum);

      //! Reset the mode to the default, returning true if it changed.
      bool resetAuthoringModeToDefault();

      //! True if authoring set AND we have any enabled modes.
      bool isAuthoringEnabled() const;

      //! Return true if all authoring modes in the mask are enabled.
      //! Optionally check if the current mode is also the one specified (if anything but Invalid).
      //! Cast Authoring::ModeMask and Authoring::Mode to int.
      bool hasEnabledAuthoringModes(int enabledMask,
                                    int currentMode = 3 /* Authoring::Mode::Invalid */) const;

      //-----------------------------------------------------------------

      struct TclSharedAlloc;

      /*! \class DD::Image::Knob::Script_List
          Splits an argument passed to Knob::from_script() into an array of
          const char* words, allowing a parser to use the words as individual
          parts of the value of the knob.  This encapsulates a common tcl
          function so that the scripting language can be replaced.

          The inverse Knob::to_script() function should print the same words
          with spaces between them. If the words may have punctuation or
          spaces in them then you should use a cstring object to "quote"
          them. If the \a quote argument to to_script() is on you should
          also add a '{' before and a '}' after the list.

          This structure may be copied and assigned to extend the lifetime but
          copy/assign/delete operations are not safe to use across threads.

     */
      class DDImage_API Script_List
      {
        friend class _Script_List;
        int n;
        TclSharedAlloc* pAlloc;
      public:

        /*! The constructor allocates any temporary storage, and splits the
            argument according to the scripting language's rules (at spaces
            for tcl).  You can then get at the individual words as C string
            values. These values may be used as an argument to to_double(),
            or as an argument to a constructor to an internal Script_List
            to make a nested list.
         */
        Script_List(const char* arg);
        Script_List(const Script_List& other);
        ~Script_List();

        Script_List& operator=(const Script_List& other);

        /*! \fn char* Knob::Script_List::operator[](int n) const
            Returns word \a n of the split list.  \a n must between 0 and size()-1. */
        const char* operator[](size_t n) const;
        const char** argv() const;

        /*! \fn int Knob::Script_List::size() const
            Returns the size of the array. */
        size_t size() const { return size_t(n >= 0 ? n : 0); }

        /*! \fn bool Knob::Script_List::error() const
            Returns true if the constructor encountered an error.  If so the only
            legal operation on the list is to delete it. Knob::error() will have been
            called to print an appropriate message.
         */
        bool error() const { return n < 0; }
      };

      ////////////////////////////////////////////////////////////////
      // To-script:

      /*!
         Return the null-terminated text that to_script(o, context, false) would
         write to the stream, or return null to indicate that this is not
         implemented.

         The main purpose of this function is so a caller can avoid the overhead
         of creating a stringstream and extracting the result from there if
         they just want to copy the to_script value somewhere.

         A caller can assume this is implemented for String_knob(). It is
         also implemented for Enumeration_knob and Bool_knob and most others
         that use keywords for their value, if a context is given or the
         knob is not animated.

         The returned value is temporary and is only good until the next time
         this or any other knob is changed. However you are allowed to pass
         it \e once to a from_script() method on this or any knob.

         The base class returns null. Notice that this means the base class
         to_script() always writes an empty string.
       */
      virtual const char* get_text(const OutputContext* = nullptr) const;

      /*!
         Write the text that describes the state of the knob to a stream. The
         resulting text must be able to be passed to from_script() to restore
         the current value.

         If \a context is null, then you should write a description of all
         the animation of the knob. If \a context is not null, you should
         write the a description of the value that is stored for the given
         context (this string, if passed to from_script(), would then produce
         a non-animated value for the knob).

         For speed this writes directly to the output file, so you must do
         the quoting if necessary. \a quote indicates that you should modify
         the output with the necessary quoting characters and delimiters so
         that the text results in exactly one word in a space-separated tcl
         list (generally this means you should add curly braces around it).
         If \a quote is false then you should output the text in the form
         it would be after it is extracted from the list.

         The helper class Knob::cstring will correctly quote a string or null
         so that it is returned unchanged to from_script() or so it can be
         put into a space-separated Knob::Script_List.

         The default version prints the result of get_text(), using Knob::cstring
         if necessary to quote it.
       */
      virtual void to_script(std::ostream&, const OutputContext*, bool quote) const;

      /*!
         Write commands that must be executed before the to_script() value
         can be parsed. This is used to write commands to declare Layers
         and Formats and other objects that are shared by knobs. Be sure to
         put a newline after the command. The default version writes
         nothing.
       */
      virtual void to_script_prefix(std::ostream&) const;

      /*!
         Write the arguments to an addUserKnob command that will create
         this knob. Nuke will already have written the ClassID() and the
         name() (the way these are written may change in future versions).
         This should write keyword+value pairs separated by spaces with
         a leading space, and use cstring to quote values.

         The base class writes " l \a label t \a tooltip ".
         \a label is not written if it is the same as name().
         \a tooltip is not written if it is blank.
       */
      virtual void to_script_userknob(std::ostream&) const;

      /*!
         Return true if the knob is not at it's default value. This is
         used to avoid writing knobs which are set to the default to saved
         scripts. The base class returns false always.
       */
      virtual bool not_default() const;

      /*! \class DD::Image::Knob::cstring

          This "class" will print a string in script notation to an ostream.
          Writing a series of these separated by spaces will produce output
          that can later be split apart with Split_List. This can be used
          by a Knob::to_script() method.

          You use it by doing "o << cstring(s)" where s is a const char*.

          You can also output an arbitrary length of characters with "o <<
          cstring(s,length)". This is useful for truncating the string.  It
          should be useful for writing arbitrary binary data, but in the
          current version if you write a null the script parser will turn it
          into the illegal UTF-8 2-byte sequence 0xc0,0x80.

          The current version puts double quotes around it if is contains
          any characters that tcl does not handle and adds backslash quoting.
          Therefore "foo123" prints unchanged, but "foo bar \baz" prints as
          "foo bar \\baz" with quotes around it.
       */
      class DDImage_API cstring
      {
      public:
        const char* contents;
        size_t length;
        cstring(const char* s, int n = -1) : contents(s), length(n) {}
        cstring(const std::string & s) : contents(s.c_str()), length(s.length()) {}

        /** Returns true if <<cstring(w) will put quotes around the string.
            This always returns true if the string has a '{' in it.
         */
        static bool needQuotes(const char* s);
        static bool needQuotes(const char* s, size_t n);
        static bool needQuotes(const std::string& s) { return needQuotes(s.c_str(), s.length()); }
      };

      class DDImage_API Indent
      {
      public:
        static int amount;
      };

      /*! DD::Image::Knob::Indent* DD::Image::Knob::indent()
          Writes the current indentation to the output stream.  You use it
          by doing "o << indent()". This is used by Knob::to_script() to
          indent lines of text to match the surrounding braces. If you want
          more indentation, just print spaces after the indent, or feel free
          to mess with the "Indent::amount" variable, as long as you restore
          the value after you are done.
       */
      static Indent* indent() { return nullptr; }

      void* pyObject() { return _pyobj; }
      void setPyObject(void* s) { _pyobj = s; }

      KnobImpl* pImpl();

    private:

      //!< Private implementation of \a error(), \a warning(), etc
      static void ReportMessageImpl(OpMessage::Type t, Knob* lpKnob, const char* lpMessage, bool critical = false);
      
      Node* node_;

      std::string name_;
      std::string label_;
      FlagMask flags_;
      std::string tooltip_;
      bool label_set;
      int _editVersion;       //!< Incremented when changed() is called
      bool _pushed;
      Knob* _undoer;
      void* _pyobj;

      Authoring* _authoring;
      std::unique_ptr<AssetInfo> _assetInfo;

      int doLegacyCallbacks(LegacyCallbackReason reason) const; // DEPRECATED
      int doNotificationCallbacks(NotificationCallbackReason reason) const;
      int doVisibilityCallbacks() const;

      typedef struct
      {
        LegacyCallback _callback;
        void* _userData;
      } LegacyCallbackPackage;

      void addNotificationCallbackInternal(const NotificationCallbackFunctor* cs);
      void addVisibilityCallbackInternal(const VisibilityCallbackFunctor* cs);

      mutable std::vector<LegacyCallbackPackage>*             _legacyCallbacks;
      mutable std::list<const NotificationCallbackFunctor*>*  _notificationCallbacks;
      mutable std::list<const VisibilityCallbackFunctor*>*    _visibilityCallbacks;

      mutable std::list<const NotificationCallbackFunctor*>   _activeNotificationCallbacks;
      mutable std::list<const VisibilityCallbackFunctor*>     _activeVisibilityCallbacks;

      mutable std::list<const NotificationCallbackFunctor*>   _pendingRemoveNotificationCallbacks;
      mutable std::list<const VisibilityCallbackFunctor*>     _pendingRemoveVisibilityCallbacks;

      friend class KnobHelpers;
    };

    DDImage_API std::ostream& operator<<(std::ostream& o, const Knob::cstring& c);
    DDImage_API std::ostream& operator<<(std::ostream& o, const Knob::Indent*);

    DDImage_API std::string escape(const char* s);

    /**
     * a KnobChangeGroup will batch up all Knob::changed() calls
     * made whilst it exists.  The effects of the Knob::changed()
     * calls will be executed when the KnobChangeGroup is destroyed.
     * Note this does not batch the effects of subclasses' virtual
     * changed() methods.
     *
     * KnobChangeGroup is used, for example, when a large number of
     * keyframes on a knob or group knobs are being set, which would
     * otherwise cause the curve editor to update too often and be
     * too slow.
     */
    class DDImage_API KnobChangeGroup
    {
      friend class Knob;
      std::set<Knob*> _knobs;
      bool _running;
    public:
      //! KnobChangeGroup effectively calls DD::Image::Knob::changed directly,
      //! bypassing any virtual overrides of it.  If you want to have some
      //! custom stuff run, then implement listener, and then
      //! it will call knobChangeFinished() after calling knob_changed.
      class DDImage_API Listener
      {
      public:
        virtual ~Listener();
        virtual void knobChangeFinished() = 0;
      };

      KnobChangeGroup();
      void flush();

      //! whether a KnobChangeGroup exists at present
      static bool delaying();
      ~KnobChangeGroup();
    };

    class DDImage_API KnobUndoGroup
    {
      Knob* _knob;

    public:
      KnobUndoGroup(Knob* knob)
      :_knob(knob)
      {
        _knob->add_to_undo();
      }

      ~KnobUndoGroup()
      {
        _knob->stop_add_to_undo();
      }
    };

    // deprecated version
    void Knob::make_handle(HandleContext command, ViewerContext* ctx, Handle* cb, int index, float x, float y, float z /*=0*/, const ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      make_handle(command, ctx, cb, index, DD::Image::Vector3(x, y, z), cursor);
    }

    // same as make_handle(POSITION, ...
    void Knob::make_handle(ViewerContext* ctx, Handle* cb, const int index, const DD::Image::Vector3& pos, const ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      make_handle(POSITION, ctx, cb, index, pos, cursor);
    }

    // deprecated version
    void Knob::make_handle(ViewerContext* ctx, Handle* cb, const int index, const float x, const float y, const float z /*= 0.0f*/, const ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      make_handle(POSITION, ctx, cb, index, DD::Image::Vector3(x, y, z), cursor);
    }

    // deprecated version
    void Knob::begin_handle(HandleContext command, ViewerContext* ctx, Handle* cb, int index, float x /*= 0*/, float y /*= 0*/, float z /*= 0*/, ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      begin_handle(command, ctx, cb, index, DD::Image::Vector3(x, y, z), cursor);
    }

    // same as begin_handle(POSITION, ...
    void Knob::begin_handle(ViewerContext* ctx, Handle* cb, int index, const DD::Image::Vector3& pos, ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      begin_handle(POSITION, ctx, cb, index, pos, cursor);
    }

    // deprecated version
    void Knob::begin_handle(ViewerContext* ctx, Handle* cb, int index, float x /*= 0*/, float y /*= 0*/, float z /*= 0*/, ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      begin_handle(POSITION, ctx, cb, index, DD::Image::Vector3(x, y, z), cursor);
    }

#ifdef DD_IMAGE_EXPERIMENTAL_HANDLES
    // same as make_handle(POSITION, ...
    void Knob::make_handle(ViewerContext* ctx, HandleBatch* cb, const int index, const DD::Image::Vector3& pos, const ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      make_handle(POSITION, ctx, cb, index, pos, cursor);
    }

    // deprecated version
    void Knob::make_handle(HandleContext command, ViewerContext* ctx, HandleBatch* cb, const int index, const float x, const float y, const float z /*= 0.0f*/, const ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      make_handle(command, ctx, cb, index, DD::Image::Vector3(x, y, z), cursor);
    }

    // same as begin_handle(POSITION, ...
    void Knob::begin_handle(ViewerContext* ctx, HandleBatch* cb, int index, const DD::Image::Vector3& pos, ViewerContext::Cursor cursor /*= ViewerContext::kNoCursor*/ )
    {
      begin_handle(POSITION, ctx, cb, index, pos, cursor);
    }
#endif //def DD_IMAGE_EXPERIMENTAL_HANDLES

  } // namespace Image
} // namespace DD

template <typename CallbackPackageType>
void DD::Image::Knob::addNotificationCallback(const CallbackPackageType& cs)
{
  // Create a callback package on the heap
  CallbackPackageType* pPayload = new CallbackPackageType(cs);
  addNotificationCallbackInternal(pPayload);
}

template <typename CallbackPackageType>
void DD::Image::Knob::addVisibilityCallback(const CallbackPackageType& cs)
{
  // Create a callback package on the heap
  CallbackPackageType* pPayload = new CallbackPackageType(cs);
  addVisibilityCallbackInternal(pPayload);
}

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)
#  pragma warning(pop) // reenable 4251: needs to have dll-interface to be used by clients of <X>s to have dll-interface to be used by clients of <X>
#endif // FN_OS_WINDOWS &&  FN_COMPILER_INTEL


#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
