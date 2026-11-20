// Knobs.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file Knobs.h
 * \brief Knob creation functions
 *
 * Definition of callbacks for the Op::knob() method.
 * These are wrappers around the actual callback, providing type-safe Knob calls.
 */

#ifndef Knobs_H
#define Knobs_H

#include <set>
#include <map>

#include "Op.h"
#include "ChannelSet.h"
#include "LookupCurves.h"
#include "GeneralInfo.h"
#include "DDImage/MetaKeyFrameKnobI.h"
#include "SceneView_KnobI.h"
#include "PathKnobI.h"

// we must fool Doxygen a little
#ifdef DOXYGEN
  #error 'DOXYGEN' must not be defined when compiling!
#endif

namespace ndk
{
  class ChannelManager;
}

namespace DD
{
  namespace Image
  {

    class Matrix4;
    class ReadFileKnobOwner;
    class Histogram_Data;
    class FrameExtentProviderI;
    struct SceneItem;
    struct ConvolveArray;

#ifndef DOXYGEN

    /*! Represents the state of the most recently called Knob::store() method,
        filled in during a Knob_Closure::call().
    */
    struct DDImage_API KnobStoreContext
    {
      Knob*         storedKnob;       //!< Non-null if Knob::store() was performed
      StoreType     storedType;       //!< StoreType passed to Knob::store()
      void*         storedData;       //!< Pointer passed to Knob::store()
      OutputContext storedContext;    //!< OutputContext the Knob stored at
      Hash          storedHash;       //!< Hash value set via Knob::store()

      KnobStoreContext() : storedKnob(nullptr), storedData(nullptr) {}
    };


    /*!
      \internal
      \class DD::Image::Knob_Closure
       This class is used to construct knobs and set values.  This is for
       internal use only.
     */
    class DDImage_API Knob_Closure
    {
    protected:
      /*! Subclass implementations should update _lastKnob and _lastKnobHash
          if they are returning a valid Knob* or storing a hash value.
      */
      virtual Knob* call(int type, StoreType datatype, void* pointer,
                         const char* name, const char* label,
                         const void* extra) = 0;

      bool             _makeKnobs;            //!< Knob_Closure should create Knob allocations
      Knob*            _lastMadeKnob;         //!< Most recent Knob created in call()
      Knob*            _lastMadeCompoundKnob; //!< Most recent compound Knob created in call()
      KnobStoreContext _lastStoredContext;    //!< Filled in during call() when Knob::store() gets called
#if kDDImageVersionMajorNum <= 17
      mFnDeprecatedInNuke14("_queryingKnobs is unused. Implement in subclass if needed.")
      bool             _queryingKnobs;        //!< deprecated, no longer used
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

#if kDDImageVersionMajorNum <= 17
    protected:
      mFnDeprecatedInNuke14("enableKnobQuerying() is unused. Implement in subclass if needed.")
      void enableKnobQuerying();
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

    public:
      Knob_Closure(bool makeKnobs = false);
      virtual ~Knob_Closure() { }


      inline Knob* operator()()
      {
        return call(0, Custom, nullptr, nullptr, nullptr, nullptr);
      }

      inline Knob* operator()(int type, const char* name, const char* label, const void* extra)
      {
        return call(type, Custom, nullptr, name, label, extra);
      }

      inline Knob* operator()(int type, StoreType datatype, void* pointer, const char* name, const char* label = nullptr, const void* extra = nullptr)
      {
        return call(type, datatype, pointer, name, label, extra);
      }


      /*! \fn bool Knob_Closure::makeKnobs() const

         Op::knobs() is called both to construct knobs and to cause store()
         or other actions to be done to each knob for each instance of an
         Op. Due to time warping or cloning, there may be more than one
         instance of an Op for each knob, so we have to use this callback
         mechanism to store into each of them.

         Since constructing the knobs cannot be done with a callback in
         C++, this flag is used to indicate that the constructor should be
         done instead of the callback. I recommend you use the KNOB() macro
         or something similar to avoid passing the wrong type of pointer to
         the callback.
       */
      bool makeKnobs() const { return _makeKnobs; }

      /*! Not making Knob allocations, likely storing their values.

          Note that Op::knobs is called twice, first for EARLY_STORE knobs and again
          for non-EARLY_STORE knobs. Use storeMainKnobs() and storeEarlyKnobs() when
          only a single store cycle is desired to do work in.
      */
      bool storeKnobs() const { return !_makeKnobs; }

      /*! Not making Knob allocations, likely storing their values during the 'main'
          knob store cycle, ie NOT the EARLY_STORE knob store cycle.

          Use storeEarlyKnobs() to test specifically for EARLY_STORE mode.
      */
      bool storeMainKnobs() const { return (!_makeKnobs && !storeEarlyKnobs()); }

      /*! Knobs with EARLY_STORE enabled will be stored when this is true.
          Use storeMainKnobs() to test specifically for non-EARLY_STORE mode.

          Op::knobs() is normally called twice during each store cycle and an Op can
          check this value to avoid doing the same work twice.

          Knobs with EARLY_STORE enabled allows their stored values to be used to control
          things like node input handling or internal Op state management prior to other
          Knobs being evaluated. A good example of this is storing a filepath knob early
          to determine the file type plugin to load, where the type may affect the knob
          UI.

          Implemented in the internal Nuke store callback subclass but likely of little
          use by other subclasses.
      */
      virtual bool storeEarlyKnobs() const { return false; }

      /*! Subclass implementations can return the OutputContext being provided to
          Knobs in the call() method, useful when there is no stored Knob to provide
          this info via lastStoredContext().
          Should only be valid during Knob store passes (when makeKnobs() is false.)
      */
      virtual const OutputContext* currentContext() const { return nullptr; }

      /*! Returns info about the most recent knob stored in the call() method.
          Knob_Closure subclasses which call Knob::store() should update this structure.
          If makeKnobs() is true this value is not valid as Knobs are being created, not stored.
      */
      const KnobStoreContext& lastKnobStoreContext() const { return _lastStoredContext; }
      Knob*                   lastStoredKnob()       const { return _lastStoredContext.storedKnob; }
      const OutputContext&    lastStoredContext()    const { return _lastStoredContext.storedContext; }
      const Hash&             lastStoredHash()       const { return _lastStoredContext.storedHash; }

      //! Do any knobs processed so far have animation enabled? Only valid during knob store.
      virtual bool animatingKnobs() const { return false; }

#if kDDImageVersionMajorNum <= 17
      /** Return whether or not this callback is being used to query/list the knobs an
       * Op has. This can be used to perform introspection on an Op to see the kinds
       * of knobs it uses.
       */
      mFnDeprecatedInNuke14("queryingKnobs() is unused. Implement in subclass if needed.")
      bool queryingKnobs() const;
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

      /** Returns false if the knob will be filtered out by name, for
          CustomKnob macros to avoid creating the knob if not used */
      virtual bool filter(const char* name) const { return true; }

      // extra crap only used by when makeKnobs is true:
      virtual void constructed( Knob* ) {}
      virtual size_t getKnobCount() const { return 0; }
      void setLastMadeKnob(Knob* k) { _lastMadeKnob = k; }
      Knob* getLastMadeKnob() const { return _lastMadeKnob; }
      Knob* getLastMadeCompoundKnob() const { return _lastMadeCompoundKnob; }
    };

    typedef Knob_Closure& Knob_Callback;

    // some typedefs to make this more readable:
    typedef const char* NAME;
    typedef const char* LABEL;
    typedef const char* const* ENUM_LABELS;

    // IMPORTANT:
    // The indices of the enumerations correspond to the ID's in 'addUserKnob' and must not change! 
    // Changing the order of these will break compatability with user knobs generated in previous versions of Nuke.
    // Any new knob types must be added immediately before LAST_KNOB.
    // Entries must never be removed from this list.
    enum {
      OBSOLETE_KNOB = 0,               // Allows knobs to be obseleted and script stored values to be translated to new knobs.
      STRING_KNOB,                     // Stores a char* pointer or std::string, and presents a string box in the panel.
      FILE_KNOB,                       // A STRING_KNOB with a file chooser in the panel interface.
      INT_KNOB,                        // Stores an int & presents a numeric entry box in the panel. No slider by default.
      ENUMERATION_KNOB,                // Popup/dropdown list with single selection. Stores an int.
      BITMASK_KNOB,                    // Stores an int & presents one checkbox for each bit.
      BOOL_KNOB,                       // Stores a bool & presents a single checkbox. Does not set STARTLINE by default.
      DOUBLE_KNOB,                     // Stores a double & presents a numeric entry boxes and slider.
      FLOAT_KNOB,                      // Stores a float & presents a numeric entry boxes and slider.
      ARRAY_KNOB,                      // 2 dimensional array of floats, as used by Convolve. Uses data storage struct ConvolveArray from Convolve.h.
      CHANNEL_MASK_KNOB,               // Stores a ChannelSet and presents a variety of panel interfaces allowing user channelset selection.
      CHANNEL_KNOB,                    // Stores an array of 1 to 4 Channels and presents a panel interface allowing user channel selection.
      XY_KNOB,                         // Stores 2 floats/doubles, presents a panel interface of two numerical entry boxes and a crosshair in the viewer. Proxy scales.
      XYZ_KNOB,                        // Stores 3 floats for an xyz position. Presents three numerical entry boxes and a 3d position widget in the viewer. 
      WH_KNOB,                         // Stores 2 floats/doubles representing width & height. Presents one or two numerical entry boxes plus a slider in the former case. Proxy scales.
      BBOX_KNOB,                       // Stores 4 floats/doubles representing the x,y,r,t of a rectangle. Presents four numerical entry boxes with hardcoded labels & a rectangle widget in the viewer. Proxy scales.
      SIZE_KNOB,                       // Obsolete; same as FLOAT_KNOB.
      FORMAT_KNOB,                     // Stores a FormatPair which contains a variety of format information. Presents a popup of current formats. 
      COLOR_KNOB,                      // Stores 3 floats/doubles representing an rgb color. Presents either a numerical entry box and slider, or 3 numerical entry boxes, both with eyedropper and colour picker buttons.
      ACOLOR_KNOB,                     // Stores 4 floats/doubles representing an rgba color. Presents either a numerical entry box and slider, or 4 numerical entry boxes, both with eyedropper and colour picker buttons.
      TAB_KNOB,                        // Starts a new tab in the param panel. No data storage.
      CUSTOM_KNOB,                     // Create custom instances of knobs (inherited from the knob class). Variety of methods for different knob constructor requirements.
      PYSCRIPT_KNOB,                   // Presents a button, which executes a python script when pressed. STARTLINE not set by default.
      TEXT_EDITOR_KNOB,                // same as MULTILINE_STRING_KNOB
      TRANSFORM2D_KNOB,                // Stores a Matrix4, representing a 4x4 transformation matrix with a 2-d affine xform. Presents a variety of params, along with a viewer transform jack widget. Proxy scales.
      SPACER_KNOB,                     // Presents an empty space. No data storage. STARTLINE not set by default.
      TEXT_KNOB,                       // Presents a user defined piece of text. No data storage. STARTLINE not set by default. 
      HELP_KNOB,                       // Presents a small button with a ? label, which pops up user defined helpful text when pressed.
      MULTILINE_STRING_KNOB,           // Stores a char* pointer, and presents a multiple line text entry widget. 
      AXIS_KNOB,                       // Stores a Matrix4, representing a 3D 6-DOF transformation. Presents a variety of params, along with a viewer 3d transform jack widget.
      UV_KNOB,                         // Stores 2 floats/doubles, presented as two numerical entry boxes with hardcoded 'u' and 'v' labels.
      BOX3_KNOB,                       // Stores 6 floats, representing 2 3D points describing a 3D volume. Presented as 6 numerical entry boxes with hardcoded labels and a 3d box viewer widget.
      SCRIPT_KNOB,                     // Presents a Button that either can use knob_changed to execute c++, or can execute tcl directly. STARTLINE not set by default.
      LOOKUPCURVES_KNOB,               // Stores a LookupCurves pointer as defined in LookupCurves.h and presents a curve editor widget on the param panel. Curve display can be customised on creation
      TOOLTIP_KNOB,                    // Obsolete; set-tooltip call!
      PULLDOWN_KNOB,                   // Presents a pulldown menu, which upon selection executes TCL scripts. No data storage.
      EYEDROPPER_KNOB,                 // Stores 8 floats representing a sampled colour and the area from which it was sampled. Presented as a colour swatch button, pushing which allows colour sampling in the viewer.
      RANGE_KNOB,                      // Stores an arbitrary number of floats/doubles. Presented as numerical entry boxes below a single slider with multiple handles.
      HISTOGRAM_KNOB,                  // Stores a Histogram_Data struct pointer, as defined in HistogramKnob.h. Presented as a histogram graph with in and out drawn at different luminances.
      KEYER_KNOB,                      // Stores 4 doubles, presented as the vertices on a trapezoid graph shape, used in the Keyer node.
      COLORCHIP_KNOB,                  // Stores an unsigned int, representing an 8-bit hex colour code, generally for UI colors. Presented as a button with a color swatch label, popping up a color picker when pushed. 
      LINK_KNOB,                       // Soft links to another knob. No internal data storage. Presentation matches the linked knob.
      SCALE_KNOB,                      // Stores 2 doubles. Presented as one numerical entry box with slider, or two numerical entry boxes.
      MULTILINE_EVAL_STRING_KNOB,      // Same as MULTILINE_STRING_KNOB
      ONEVIEW_KNOB,                    // Stores an int. Presented as a single selection drop down menu, populated with the current script's views.
      MULTIVIEW_KNOB,                  // Stores a std::set of ints. Presented as a multiple selection drop down menu, populated with the current script's views.
      VIEWVIEW_KNOB,                   // Stores a std::map relating ints to ints. Presented as a series of buttons allowing patches to be added, where each patch maps from one view to another.
      PYPULLDOWN_KNOB,                 // Presents a pulldown menu, which upon selection executes python scripts. No data storage.
      OBSOLETE_GPUENGINE_KNOB,         // Obsolete, not for use.
      MULTIARRAY_KNOB,                 // Manages internal storage. Presented as a series of array knobs. Generally recommend use of Table_knob over MultiArray.
      VIEWPAIR_KNOB,                   // Stores a std::pair of ints. Presented as a pair of buttons, pressing which pops up a select list of the current script's views.
      LIST_KNOB,                       // Stores a std::vector of std::vectors of std::strings. Columns numbered, and defined based on vector size. Read only.
      PYTHON_KNOB,                     // Used as part of python knob implementations. Not for use via NDK.
      METADATA_KNOB,                   // No storage. Presented as a read only list of metadata found on input 0.
      PIXELASPECT_KNOB,                // Stores a double. Presented as a numeric entry box and associated slider. Proxy scales.
      CP_KNOB,                         // Obselete; represents roto curve's control points
      TOOLBAR_KNOB,                    // Presents toolbars around the edge of the viewer, containing the wrapped controls. Only suitable for use with a subset of knobs. No data storage.
      TABGROUP_KNOB,                   // Presents tabs within the current page, wrapping the contained knobs. No data storage.
      PLUGIN_PYTHON_KNOB,              // A user-created knob, with user-provided python bindings.
      EXOGROUP_KNOB,                   // Presents wrapped controls on all tabs of the node. No data storage.
      MENU_KNOB,                       // Presents a button which opens a python defined menu. No data storage.
      PASSWORD_KNOB,                   // A STRING_KNOB which presents a bullet character for any character entered.
      TOOLBOX_KNOB,                    // WIP, not for use.
      TABLE_KNOB,                      // Manages internal storage, and manipulated via TableKnobI.h functions. Presented as defined columns and numbered rows of sub-knobs, allowing for read only and editable params.
      GEOSELECT_KNOB,                  // Not for direct use. Implemented on all GeoOp parented ops.
      INPUTONLY_CHANNEL_MASK_KNOB,     // A CHANNELSET_KNOB, which presents channels available on defined input only.
      INPUTONLY_CHANNEL_KNOB,          // A CHANNEL_KNOB, which presents channels available on defined inputs only.
      CONTROL_POINT_COLLECTION_KNOB,   // WIP, not for use.
      CASCADING_ENUMERATION_KNOB,      // An ENUMERATION_KNOB adding cascading menus.
      DYNAMIC_BITMASK_KNOB,            // Stores an unsigned int, and takes a std::map relating ints to strings (ie int to label). Presented as a multi select pop up menu.
      META_KEY_FRAME_KNOB,             // Manages internal storage. Presented as series of numerical entry boxes and buttons. Used to manage keyframes where handled separately.
      POSITIONVECTOR_KNOB,             // Stores 6 floats. Presented as 6 numerical entry boxes and a moveable 3d viewer arrow.
      CACHED_FILE_KNOB,                // A FILE_KNOB which allows localization of data to local drives.
      TRANSFORM_JACK_KNOB,             // WIP, not for use.
      RIPPLE_KNOB,                     // Manages internal storage and manipulated via RippleKnobI.h functions. Presents controls allowing definition of a frame range.
      SCENE_VIEW_KNOB,                 // Allows display of hierarchical scene data
      VSPACER_KNOB,                    // As SPACER_KNOB, but vertical instead of horizontal.
      CANCELEXECUTION_KNOB,            // Simulates a "Cancel" press on the progress dialog. knob_changed() is not called on this knob.
      SIMPLE_ARRAY_KNOB,               // Raw array knob
      RESIZABLE_ARRAY_KNOB,            // Array knob containing resizable storage
      DISABLE_KNOB,                    // Knob managing special logic for disabling nodes
      ICON_KNOB,                       // Non-interactive icon
      FRAME_EXTENT_KNOB,               // A knob providing Ops/Nodes with the ability to have a frame range rendered in the dopesheet
      RADIO_KNOB,                      // Radio knob, only 1 option can be selected at a time
      FREETYPE_KNOB,                   // Allows preview of the installed freetype font
      EDITABLE_ENUMERATION_KNOB,
      COLORSPACE_KNOB,                 // Handles Colorspace and Role Transform selection
      PARTICLE_CHANNELS_KNOB,          // Allows the selection of particle channels
      SCENE_GRAPH_KNOB,                // Improved display of hierarchical scene data
      PATH_EXPRESSION_KNOB,            // For entering scene graph path matching expressions
      PATH_KNOB,                       // For entering scene graph paths
      GSV_KNOB,                        // For Graph Scope Variables (MultiShot)
      BLINKEDITOR_KNOB,                // Extension of python knob with specific data for the BlinkScript editor.
      VEC2_KNOB,                       // Stores 2 floats/doubles, presented as two numerical entry boxes with customisable labels.
      USD_SCENE_GRAPH_KNOB,            // Improved display of hierarchical scene data updated for viewing or manipulating USD stages
      VEC3_KNOB,                       // Stores 3 floats/doubles, presented as three numerical entry boxes with customisable labels.
      VEC4_KNOB,                       // Stores 4 floats/doubles, presented as four numerical entry boxes with customisable labels.
      LAST_KNOB
    };

    struct IDim {
      int a, b;
      bool force;
      IDim(int A, int B, bool F = false) : a(A), b(B), force(F) {}
      IDim() : a( 0 ), b( 0 ), force( false ) {}
    };

    struct ArrayKnobData {
      IDim dim;
      bool defaultData;
    };

    struct ChannelKnobData {
      int count;
      int input;
      ndk::ChannelManager* chanmgr; // for internal use only
    };

    struct Vec2KnobData {
      const char* xLabel;
      const char* yLabel;
    };
    struct Vec3KnobData {
      const char* xLabel;
      const char* yLabel;
      const char* zLabel;
    };
    struct Vec4KnobData {
      const char* xLabel;
      const char* yLabel;
      const char* zLabel;
      const char* wLabel;
    };

#endif // !DOXYGEN

    // use this to label knobs you don't want the user to see:

    /*! In Op::knobs() use this to label knobs you don't want the user to
        see.  This is the string "INVISIBLE" but the pointer is recognized
        and it causes Knob::hidden() to return true.  */
    extern DDImage_API const char INVISIBLE[];


    /*!
       This one avoids the overhead of converting a character pointer to
       a std::string if the value is not actually used.
     */
    DDImage_API void Tooltip(Knob_Callback f, const char*);

    /*!
       In an Op::knobs() function, this sets the tooltip of the previous
       knob.
     */
    DDImage_API void Tooltip(Knob_Callback f, const std::string& text);

    /*!
       In an Op::knobs() function, this turns on some flags on the previous
       knob. The initial setting of the flags is done by the knob-creating
       function, depending on the type of knob and the arguments to that
       function, but you can modify it with this. Warning: not all flag
       combinations are supported!

       Values that work for any knob:
       - Knob::DISABLED Set by disable(), cleared by enable().
       - Knob::NO_ANIMATION Prevent the value from being animated. This
       removes any animation or view buttons, and it stops tcl expressions
       from being evaluated in string knobs, and may make it ignore attempts
       to set expressions or key frames (nyi).
       - Knob::HIDE_ANIMATION_AND_VIEWS Prevent animation and view buttons
       from being displayed. Animation is still possible, unless NO_ANIMATION
       is specified of course.
       - Knob::DO_NOT_WRITE Don't ever save this knob to a script (including
       copy & paste!)
       - Knob::INVISIBLE The knob does not appear in the panels. No widgets
       are created. This is not the same as hide(), and show() will not undo it!
       - Knob::RESIZABLE The knob can stretch in the panel so that it fills
       up all the remaining space in the line. Defaults to true for most of
       the complex knobs, but off for buttons, checkmarks, and pulldown lists.
       - Knob::STARTLINE This knob starts a new row in the panel. The default
       is true unless a zero-length (not NULL) string is passed as the label.
       Currently the default is false for checkmarks and buttons but this
       may change in future versions.
       - Knob::ENDLINE This knob will end a row, acts exactly like STARTLINE
       was set on the next knob. Set true for divider lines.
       - Knob::NO_RERENDER This knob does not contribute to the hash value
       for the op. This should be used on knobs that have no effect on the
       op's output.
       - Knob::NO_HANDLES Don't draw anything in the viewer, this is useful
       if the Op draws it's own indicators.
       - Knob::HANDLES_ANYWAY makes the handles appear in the viewer when
       the panel is open even if a different tab is selected.
       - Knob::HIDDEN Set by hide(), cleared by show().
       - Knob::NO_UNDO Don't undo/redo any changes to this knob. May be
       replaced with "output knob" in the future.
       - Knob::NO_KNOB_CHANGED: Don't bother calling Op::knob_changed() with
       this knob. This is turned on automatically if the knob_changed() returns
       false.
       - Knob::KNOB_CHANGED_ALWAYS will call node()->knob_changed() every
       time the value of the knob changes. Normally it is only called if
       the user changes the value with the panel open. This allows you to
       track all changes to the value. Be careful as knob_changed() will
       be called without storing the new values into your structure.
       - Knob::ALWAYS_SAVE save the knob to a script even if not_default()
       returns false. Deprecated, instead override not_default() and make
       it return true!
       - Knob::NODE_KNOB is used by Nuke internally for controls on the
       DAG appearance such as xpos and ypos.
       - Knob::MODIFIES_GEOMETRY should be set for any knob that modifies 
       geometry, either by affecting the internal geometry directly or by 
       changing its transform

       Values that work for knobs that store numbers:
       - Knob::MAGNITUDE If there are several numbers, this enables a button
       to only show a single number, and all are set equal to this number.
       Default is true for WH_knob() and Color_knob().
       - Knob::SLIDER Turns on the slider. Currently this only works if
       the size is 1 or MAGNITUDE is enabled and it is set to single numbers.
       Defaults to on for most non-integer numerical controls.
       - Knob::LOG_SLIDER Tick marks on the slider (if enabled with SLIDER)
       are spaced logarithmically.  This is turned on for WH_knob() and
       Color_knob(), and if the range has both ends greater than zero. If
       you turn this on and the range passes through zero, the scale is
       actually the cube root of the number, not the logarithim.
       - Knob::TINY_SLIDER Modifies SLIDER. The slider is shown as a thin
       strip underneath the LineEdit/SpinBox.
       - Knob::STORE_INTEGER Only integer values should be displayed or stored.
       - Knob::FORCE_RANGE Clamps the value to the range when storing.
       - Knob::ANGLE Turn on a little widget depicting this number as an angle.
       - Knob::NO_PROXYSCALE disables proxy scaling for XY or WH knobs.
       Useful if you just want two numbers called "x" and "y" that are
       not really a position. You probably also want to do NO_HANDLES.

       Values that work for Enumeration_knob():
       - Knob::SAVE_MENU writes the contents of the menu to the saved script.
       Useful if your plugin modifies the list of items.

       Values that work for BeginGroup():
       - Knob::CLOSED True for a BeginGroup knob that is closed

       Values that work for ChanneSet_knob() and Channel_knob():
       - Knob::NO_CHECKMARKS Get rid of the individual channel checkmarks.
       - Knob::NO_ALPHA_PULLDOWN Get rid of the extra pulldown that lets you
       set the 4th channel to an arbitrary different layer than the first 3.

       Values that work for Format_knob():
       - Knob::PROXY_DEFAULT makes the default value be the root.proxy_format
       rather than the root.format.
     */
    DDImage_API void SetFlags(Knob_Callback f, Knob::FlagMask n);

    /*!
       In an Op::knobs() function, this turns off some flags on the previous
       knob. The initial setting of the flags is done by the knob-creating
       function, depending on the type of knob and the arguments to that
       function, but you can modify it with this. See DD::Image::SetFlags
       for a list of flags.
     */
    DDImage_API void ClearFlags(Knob_Callback f, Knob::FlagMask n);

    /*!
       In an Op::knobs() function, this sets the numeric range on the
       previous knob. The range if you don't do this is determined by the
       knob but in almost all cases it is 0-1.
     */
    DDImage_API void SetRange(Knob_Callback f, double minimum, double maximum);

    /*!
       Obsolete_Knob is used to throw away the values saved in scripts for
       knobs that are no longer in use. This knob will not be listed in lists
       of knobs or saved to files. But if you know it's name, scripting
       language or loading files can still set the knob or query it's value.

       \a linkto, if not null, is the name of a knob to link to. Attempts to
       set or get information from the knob will instead act on this linked
       knob. If this is null only from_script() will work, all other calls
       will produce errors. This is very similar to Link_knob().

       \a script, if this is not null, is a tcl script that is called \e
       instead of calling from_script() on the linked-to knob. This script
       is executed in the context of the node, and the text attempting to
       be stored into the knob is put in $value.

       If both are null then the knob is ignored, attempts to set it are
       thrown away without any error.

       Be careful with the argument order. It is set up to match the
       Link_knob() arguments and there is some back-compatibility problems.
     */
    inline Knob* Obsolete_knob(Knob_Callback f, const char* linkto, NAME name, const char* script)
    {
      return f(OBSOLETE_KNOB, Custom, nullptr, name, script, linkto);
    }

    /*!
       Back-compatibility of Obsolete_knob(), equivalent to passing null for
       the linketo knob.
     */
    inline Knob* Obsolete_knob(Knob_Callback f, NAME name, const char* script)
    {
      return f(OBSOLETE_KNOB, Custom, nullptr, name, script, nullptr);
    }

    /*!
       Produce the knob named \a name for \a op. This can be used to insert
       knobs from a child Op into your control panel. This is done by calling
       knobs() on the child with a special callback that looks for the name
       and if found it calls \a f with it.
     */
    DDImage_API Knob* Op_knob(Knob_Callback f, Op* op, NAME name);

    /*!
       Store a string that is edited by the user. Nuke will manage the
       memory used by the string, do not free it in your destructor.
     */
    inline Knob* String_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr)
    {
      return f(STRING_KNOB, StringPtr, p, n, l, nullptr);
    }

    /*!
       Create a Password input field
    */
    inline Knob* Password_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr)
    {
      return f( PASSWORD_KNOB, StringPtr, p, n, l, nullptr ); 
    }


    /*!
       Store a string that is edited by the user. Uses an STL string
       so no worries about memory ownership.
     */
    inline Knob* String_knob(Knob_Callback f, std::string* p, NAME n, LABEL l = nullptr)
    {
      return f(STRING_KNOB, StlStringPtr, p, n, l, nullptr);
    }

    inline Knob* String_knob(Knob_Callback f, NAME n, LABEL l = nullptr)
    {
      return f(STRING_KNOB, StringPtr, nullptr, n, l, nullptr);
    }

    enum FileKnob_Type {
      Normal_File = 0,
      Read_File_Proxy = 1,
      Write_File_Proxy = 2,
      Geo_File = 3,
      Write_File_Normal = 4,
      Script_File = 5,
      Read_File_Cached = 6,
      Read_File_NoProxy = 7,
      OCIO_File = 8,
      Particle_Cache_File = 9,
      Cube_File = 10, //< 3D LUT, RED specific file format
      Blib_File = 11,
      Directories = 12,
      Splat_Source_File = 13,
    };

    /*!
       Same as String_knob() but Nuke adds a file chooser, and the string
       is passed through the filename_fix scripting function before being
       stored.
     */
    inline Knob* File_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr, FileKnob_Type type = Normal_File)
    {
      return f(FILE_KNOB, StringPtr, p, n, l, (void*)(long long)type);
    }

    /*!
       Special knob for use by the ReadFileKnobOwners.  Ops must implement that
       interface to include this knob.
     */
    inline Knob* Read_File_knob(ReadFileKnobOwner* ptr, Knob_Callback f, const char** p, NAME n, LABEL l = nullptr)
    {
      return f(FILE_KNOB, StringPtr, p, n, l, (void*)(long long)Read_File_Proxy);
    }

    /*!
       Special knob for use by the ReadFileKnobOwners.  Ops must implement that
       interface to include this knob.
     */
    inline Knob* Read_File_knob(ReadFileKnobOwner* ptr, Knob_Callback f, std::string* p, NAME n, LABEL l = nullptr)
    {
      return f(FILE_KNOB, StlStringPtr, p, n, l, (void*)(long long)Read_File_Proxy);
    }

    /*!
      It allows for local file caching to be enabled on the node
    */
    inline Knob* Cached_File_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr, FileKnob_Type type = Normal_File)
    {
      return f(CACHED_FILE_KNOB, StringPtr, p, n, l, (void*)(long long)type);
    }
    
    /*!
       Special knob for use by the Write. This will present the user with
       two strings for full-size and proxy, and will store the string for
       the current setting. This may be usable by ops other than Write.
     */
    inline Knob* Write_File_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr)
    {
      return f(FILE_KNOB, StringPtr, p, n, l, (void*)(long long)Write_File_Proxy);
    }
    
    /*!
       Special knob for use by the Write. This will present the user with
       two strings for full-size and proxy, and will store the string for
       the current setting. This may be usable by ops other than Write.
     */
    inline Knob* Write_File_knob(Knob_Callback f, std::string* p, NAME n, LABEL l = nullptr)
    {
      return f(FILE_KNOB, StlStringPtr, p, n, l, (void*)(long long)Write_File_Proxy);
    }

    /*!
       Special knob for use by the Read Op only. It stores several other
       values in other fields of Read. Do not use except for the Read Op.
     */
    inline Knob* Proxyless_Read_File_knob(ReadFileKnobOwner* ptr, Knob_Callback f, const char** p, NAME n, LABEL l = nullptr)
    {
      return f(FILE_KNOB, StringPtr, p, n, l, (void*)(long long)Read_File_NoProxy);
    }

    /*! Controls a single integer value. */
    inline Knob* Int_knob(Knob_Callback f, int* p, NAME n, LABEL l = nullptr)
    {
      return f(INT_KNOB, IntPtr, p, n, l, nullptr);
    }

    /*!
       Controls an integer enumeration. The array of labels \a e will produce
       a pop-up list the user can choose from. The index into this list
       is stored. The first item will store a zero, etc. You can assume
       that only legal values are stored.
     */
    inline Knob* Enumeration_knob(Knob_Callback f, int* p, ENUM_LABELS e, NAME n, LABEL l = nullptr)
    {
      return f(ENUMERATION_KNOB, IntPtr, p, n, l, e);
    }

    inline Knob* CascadingEnumeration_knob( Knob_Callback f, int* p, ENUM_LABELS e, NAME n, LABEL l = nullptr )
    {
      return f( CASCADING_ENUMERATION_KNOB, IntPtr, p, n, l, e );
    }

    inline Knob* Colorspace_knob( Knob_Callback f, int* p, const char* e, NAME n, LABEL l = nullptr)
    {
      return f( COLORSPACE_KNOB, IntPtr, p,  n, l, e);
    }

    inline Knob* SceneGraph_knob( Knob_Callback f, int* p,  std::vector<SceneItem>* e, NAME n, LABEL l = nullptr )
    {
      return f( SCENE_GRAPH_KNOB, IntPtr, p, n, l, e );
    }

    inline Knob* USDSceneGraph_knob(Knob_Callback f, const NAME n, const LABEL l)
    {
      return f(USD_SCENE_GRAPH_KNOB, Custom, nullptr, n, l, nullptr);
    }

    inline Knob* SceneView_knob( Knob_Callback f, int* p, ENUM_LABELS e, NAME n, LABEL l = nullptr )
    {
      return f( SCENE_VIEW_KNOB, IntPtr, p, n, l, e );
    }

    inline Knob* SceneView_knob( Knob_Callback f, SceneView_KnobI::ItemList* p, NAME n, LABEL l = nullptr )
    {
      return f( SCENE_VIEW_KNOB, StlStringVecPtr, p, n, l, nullptr );
    }

    inline Knob* FreeTypeFont_knob( Knob_Callback f, StlStringIntPair* p, NAME n, LABEL l = nullptr )
    {
      return f( FREETYPE_KNOB, StlStringIntPairPtr, p, n, l );
    }
    
    inline Knob* OneView_knob(Knob_Callback f, int* p, NAME n, LABEL l = nullptr, const char* defcaption = nullptr)
    {
      return f(ONEVIEW_KNOB, IntPtr, p, n, l, (void*)defcaption);
    }

    inline Knob* ViewPair_knob(Knob_Callback f, std::pair<int, int>* p, NAME n, LABEL l = nullptr)
    {
      return f(VIEWPAIR_KNOB, IntPairPtr, p, n, l);
    }

    inline Knob* MetaData_knob(Knob_Callback f, NAME n, LABEL l = nullptr)
    {
      return f(METADATA_KNOB, Custom, nullptr, n, l);
    }

    inline Knob* MultiView_knob(Knob_Callback f, std::set<int>* p, NAME n, LABEL l = nullptr)
    {
      return f(MULTIVIEW_KNOB, IntSetPtr, p, n, l);
    }

    inline Knob* ViewView_knob(Knob_Callback f, std::map<int, int>* p, NAME n, std::pair<std::string, std::string>* headings, LABEL l = nullptr)
    {
      return f(VIEWVIEW_KNOB, IntIntMapPtr, p, n, l, headings);
    }

    //! Makes a particle channels knob. This displays a dropdown menu of particle channels with a checkbox by each to turn it on, and an option to create new channels.
    inline Knob* ParticleChannels_knob( Knob_Callback f, unsigned int* p, NAME n, LABEL l = nullptr)
    {
      return f( PARTICLE_CHANNELS_KNOB, UnsignedIntPtr, p, n, l);
    }

    /*!
       Produces a set of labelled checkmarks. Each checkmark will set one
       bit in the output integer. The first one sets 1, the next 2, the
       next one 4, etc.
     */
    inline Knob* Bitmask_knob(Knob_Callback f, unsigned int* p, ENUM_LABELS e, NAME n, LABEL l = nullptr)
    {
      return f(BITMASK_KNOB, UnsignedIntPtr, p, n, l, e);
    }

    inline Knob* Dynamic_Bitmask_knob(Knob_Callback f, unsigned int* p, std::map<int, std::string>* e, NAME n, LABEL l = nullptr)
    {
     return f(DYNAMIC_BITMASK_KNOB, UnsignedIntPtr, p, n, l, e);
    }

    /*! A single checkmark that controls a bool */
    inline Knob* Bool_knob(Knob_Callback f, bool* p, NAME n, LABEL l = nullptr)
    {
      return f(BOOL_KNOB, BoolPtr, p, n, l, nullptr);
    }

    //! Displays as Bool_knob but has integer arg (ie int 1 = true, int 0 = false.)
    inline Knob* BoolInt_knob(Knob_Callback f, int* p, NAME n, LABEL l = nullptr)
    {
      return f(BOOL_KNOB, IntPtr, p, n, l, nullptr);
    }

    /*! Controls a single number stored as a double */
    inline Knob* Float_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(FLOAT_KNOB, DoublePtr, p, n, l, nullptr);
    }

    /*! Controls a single number stored as a float */
    inline Knob* Float_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      return f(FLOAT_KNOB, FloatPtr, p, n, l, nullptr);
    }

    /*! Controls a single frame number stored as a float */
    inline Knob* Frame_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      // This is an alias for a float knob at the moment, until we add
      // a proper frame knob.
      return f(FLOAT_KNOB, FloatPtr, p, n, l, nullptr);
    }

    /*! Controls a single number stored as a float, representing a Deep value */
    DDImage_API Knob* Deep_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr);

    /*! Controls \a dim doubles in an array */
    inline Knob* MultiFloat_knob(Knob_Callback f, double* p, int dim, NAME n, LABEL l = nullptr)
    {
      return f(FLOAT_KNOB, DoublePtr, p, n, l, (void*)(long long)dim);
    }

    /*! Controls \a dim floats in an array */
    inline Knob* MultiFloat_knob(Knob_Callback f, float* p, int dim, NAME n, LABEL l = nullptr)
    {
      return f(FLOAT_KNOB, FloatPtr, p, n, l, (void*)(long long)dim);
    }

    /*! Controls \a dim integers in an array */
    inline Knob* MultiInt_knob(Knob_Callback f, int* p, int dim, NAME n, LABEL l = nullptr)
    {
      return f(INT_KNOB, IntPtr, p, n, l, (void*)(long long)dim);
    }

    /*! Controls a single float value, which is a pixel aspect ratio. */
    inline Knob* PixelAspect_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(PIXELASPECT_KNOB, DoublePtr, p, n, l, nullptr);
    }

    //! Controls a string containing a prim path matching expression
    DDImage_API Knob* PathExpression_knob(Knob_Callback f, std::string* p, NAME n, LABEL l = nullptr, const char* filter = nullptr
                                         , Path_KnobI::InputStageMode mode = Path_KnobI::InputStageMode::NodeInput
                                         , int inputIndex = 0, const char* fileKnobName = nullptr );

    //! Controls a string containing a prim path
    DDImage_API Knob* Path_knob(Knob_Callback f, std::string* p, NAME n, LABEL l = nullptr, const char* filter = nullptr
                               , Path_KnobI::InputStageMode mode = Path_KnobI::InputStageMode::NodeInput
                               , int inputIndex = 0, const char* fileKnobName = nullptr );


    /*!
       Sets a matrix as used by the Convolve operator. Currently the user
       interface is limited to fixed-size arrays but the tcl script can
       change the array size.
     */
    inline Knob* Array_knob(Knob_Callback f, ConvolveArray* p, int width, int height, NAME n, LABEL l = nullptr, bool defaultData = false)
    {
      ArrayKnobData data;
      data.dim = IDim( width, height );
      data.defaultData = defaultData;

      return f(ARRAY_KNOB, ConvolveArrayPtr, p, n, l, (void*)&data);
    }

    inline Knob* ResizableArray_knob(Knob_Callback f, ConvolveArray* p, int width, int height, NAME n, LABEL l = nullptr, bool defaultData = false)
    {
      ArrayKnobData data;
      data.dim = IDim( width, height );
      data.defaultData = defaultData;

      return f(RESIZABLE_ARRAY_KNOB, ConvolveArrayPtr, p, n, l, (void*)&data);
    }
    /*!
       Store a ChannelSet. The UI looks almost identical to Channel_knob,
       but the user can select many more channels, including "all".
       \a chanmgr is for internal use only.
     */
    inline Knob* ChannelSet_knob(Knob_Callback f, ChannelSet* p, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{0/*count*/, -1/*input*/, chanmgr};
      return f(CHANNEL_MASK_KNOB, ChannelSetPtr, p, n, l, (void*)&data);
    }

    /*!
       Same as ChannelSet_knob, but the UI menu puts all channels not returned by
       Op::node_input(\a input) into a sub menu called "Other Layers". If input < 0 then
       the menu will show channels provided by \a chanmgr, not an input Iop.
       \a chanmgr is currently for internal use only.
     */
    inline Knob* Input_ChannelSet_knob(Knob_Callback f, ChannelSet* p, int input, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{0/*count*/, input, chanmgr};
      return f(CHANNEL_MASK_KNOB, ChannelSetPtr, p, n, l, (void*)&data);
    }

    /*!
       Same as ChannelSet_knob, but the UI menu only shows channels returned by
       Op::node_input(\a input). If input < 0 then the menu will only show channels
       provided by \a chanmgr, not an input Iop.
       \a chanmgr is currently for internal use only.
     */
    inline Knob* InputOnly_ChannelSet_knob(Knob_Callback f, ChannelSet* p, int input, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{0/*count*/, input, chanmgr};
      return f(INPUTONLY_CHANNEL_MASK_KNOB, ChannelSetPtr, p, n, l, (void*)&data);
    }

    /*!
       Store \a count channel numbers for your operator to write to. The user selects
       it from a popup list that has all the \e layers, a "new" item that lets them
       define a new layer, and "none" which stores zero into all the numbers (attempts
       to write to the zero channel will write to a junk buffer that is thrown away. If
       a layer with more than one channel is selected the user gets checkmarks to turn
       the individual layers off (storing zero instead of the channel number).
       \a chanmgr is for internal use only.
     */
    inline Knob* Channel_knob(Knob_Callback f, Channel* p, int count, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{count, -1/*input*/, chanmgr};
      return f(CHANNEL_KNOB, ChannelPtr, p, n, l, (void*)&data);
    }

    /*!
       Same as Channel_knob but the popup menu lacks the "new" option.
       Also only layers that exist in the \a input Iop (and RGB, RGBA and Alpha) are
       listed in the top level UI menu. All other channels are listed in the "other"
       submenu (and if this input is not connected to an Iop then all layers are listed).
       This is used to store channel numbers that your operator will read from. If input < 0
       then the menu will show channels provided by \a chanmgr, not an input Iop.
       \a chanmgr is currently for internal use only.
     */
    inline Knob* Input_Channel_knob(Knob_Callback f, Channel* p, int count, int input, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{count, input, chanmgr};
      return f(CHANNEL_KNOB, ChannelPtr, p, n, l, (void*)&data);
    }

    /*!
       Same as Input_Channel_knob but the popup menu only shows layers that exist in the
       \a input Iop. If this input is not connected to an Iop then all layers are listed. This
       is used to store channel numbers that your operator will read from. If input < 0 then
       the menu will only show channels provided by \a chanmgr, not an input Iop.
       \a chanmgr is currently for internal use only.
     */
    inline Knob* InputOnly_Channel_knob(Knob_Callback f, Channel* p, int count, int input, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{count, input, chanmgr};
      return f(INPUTONLY_CHANNEL_KNOB, ChannelPtr, p, n, l, (void*)&data);
    }

    /*!
       Stores an x and y position.
       Draws a square dot in the viewer that the user can drag around.

       The \a parent is an optional XY_knob() return value. The
       stored values are then relative to the point stored by that
       knob, and this draws a vector from this point to the parent
       point, and moving the parent moves this one with it.
     */
    inline Knob* XY_knob(Knob_Callback f, double* p, NAME n, LABEL l, Knob* parent = nullptr)
    {
      return f(XY_KNOB, DoublePtr, p, n, l, (void*)parent);
    }
    inline Knob* XY_knob(Knob_Callback f, double* p, NAME n, Knob* parent = nullptr)
    {
      return f(XY_KNOB, DoublePtr, p, n, nullptr, (void*)parent);
    }
    inline Knob* XY_knob(Knob_Callback f, float* p, NAME n, LABEL l, Knob* parent = nullptr)
    {
      return f(XY_KNOB, FloatPtr, p, n, l, (void*)parent);
    }
    inline Knob* XY_knob(Knob_Callback f, float* p, NAME n, Knob* parent = nullptr)
    {
      return f(XY_KNOB, FloatPtr, p, n, nullptr, (void*)parent);
    }

    /*!
       Stores an xyz position and draws a square dot on the 3D viewer that
       the user can drag around.

       The \a parent is an optional XYZ_knob() return value. The
       stored values are then relative to the point stored by that
       knob, and this draws a vector from this point to the parent
       point, and moving the parent moves this one with it.
     */
    inline Knob* XYZ_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr, Knob* parent = nullptr)
    {
      return f(XYZ_KNOB, DoublePtr, p, n, l, (const void*)parent);
    }
    inline Knob* XYZ_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr, Knob* parent = nullptr)
    {
      return f(XYZ_KNOB, FloatPtr, p, n, l, (const void*)parent);
    }

    /*!
       Makes two numbers called 'u' and 'v'.
     */
    inline Knob* UV_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(UV_KNOB, DoublePtr, p, n, l);
    }
    inline Knob* UV_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      return f(UV_KNOB, FloatPtr, p, n, l);
    }

    /*!
       It's used to store a collection of control points
     */

    inline Knob* CP_knob(Knob_Callback f, float* p, int dim, NAME n, LABEL l = nullptr)
    {
      return f(CP_KNOB, FloatPtr, p, n, l, (void*)(long long)dim);
    }

    /*!
       Stores a width and height. The user can change the slider to two
       input fields so they can make the width and height different.
       When stored the horizontal value (the first one) is divided
       by the incoming format's pixel aspect ratio, so if the user
       sees equal values the result is square in the viewer. The sizes
       are also scaled to account for the difference between the full
       size and proxy formats.
     */
    inline Knob* WH_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(WH_KNOB, DoublePtr, p, n, l, nullptr);
    }
    inline Knob* WH_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      return f(WH_KNOB, FloatPtr, p, n, l, nullptr);
    }

    /*!
       Stores four numbers representing the x, y, right, and top of a
       rectangle selected by the user. The values are scaled by the
       current proxy settings. Draws this rectangle on the
       viewer that the user can manipulate.

       The \a parent is an optional XY_knob() return value. The
       stored values are then relative to the point stored by that
       knob, and moving the XY_knob will move the box with it.
     */
    inline Knob* BBox_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr, Knob* parent = nullptr)
    {
      return f(BBOX_KNOB, DoublePtr, p, n, l, (void*)parent);
    }
    inline Knob* BBox_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr, Knob* parent = nullptr)
    {
      return f(BBOX_KNOB, FloatPtr, p, n, l, (void*)parent);
    }

    /*!
       User chooses a format, which is stored in p.full_size_format.
       Also the current proxy scaling is used to produce another format
       that is stored in p.format.

       Normally you should use p.format for any calculations. However since
       the bottom and top values are rounded to the nearest integer, you
       may want to use outputContext().to_proxy() on the full_size_format
       values to get exact vertical positions.

       The FormatPair structure also contains space for making
       the p.format if necessary.
     */
    inline Knob* Format_knob(Knob_Callback f, FormatPair* p, NAME n, LABEL l = nullptr)
    {
      return f(FORMAT_KNOB, FormatPairPtr, p, n, l, nullptr);
    }

    /*!
       Stores 3 floating-point numbers that represent a rgb color
       selected by the user.
     */
    inline Knob* Color_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      return f(COLOR_KNOB, FloatPtr, p, n, l, nullptr);
    }
    inline Knob* Color_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(COLOR_KNOB, DoublePtr, p, n, l, nullptr);
    }

    /*!
       Stores 4 floating-point numbers that represent a rgb color
       and an alpha value selected by the user.
     */
    inline Knob* AColor_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      return f(ACOLOR_KNOB, FloatPtr, p, n, l, nullptr);
    }
    inline Knob* AColor_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(ACOLOR_KNOB, FloatPtr, p, n, l, nullptr);
    }


    /*! This knob provides an interface to sample an average color from a
        viewer. It also provides the position and size of the sampled area
     */
    inline Knob* Eyedropper_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      return f(EYEDROPPER_KNOB, FloatPtr, p, n, l, nullptr);
    }

    /*! Store \a np numbers, but show a single slider with multiple moving
       slider handles. Values will always be in sorted because the user
       cannnot move the sliders through each other.
     */
    inline Knob* Range_knob(Knob_Callback f, double* p, int np, NAME n, LABEL l = nullptr)
    {
      return f(RANGE_KNOB, DoublePtr, p, n, l, (void*)(long long)np);
    }
    inline Knob* Range_knob(Knob_Callback f, float* p, int np, NAME n, LABEL l = nullptr)
    {
      return f(RANGE_KNOB, FloatPtr, p, n, l, (void*)(long long)np);
    }

    inline void FrameRange_knob(Knob_Callback f, FrameRange p, const std::string& n, LABEL l = nullptr)
    {
      Int_knob(f, &p[0], (n + "_first").c_str(), l);
      Int_knob(f, &p[1], (n + "_last").c_str(), "");
    }

    enum {
      TRANSFORM = 0, POSITIONER = 1, TO_PROXY = 2, NO_SCALE = 4,
      NO_UNIFORM_SCALE = 8, NO_SKEW = 16, NO_CENTER = 32, T2D_GAUGE = 64,
      T2D_INVERT = 128,
    };
    /*!
       This knob stores into a Mat4f a 2-D transformation (although
       a Mat4f is 4x4, this will only set 6 a00,a01,a03,a10,a11,a13
       to values other than the identity). The user is presented with
       a "transform jack" in the viewer.

       This creates named child knobs.  If the name of the knob
       ends with "_" then these names are instead appended to this 
       overall name, which allows more than one of these to be used in a
       node.

       \a type modifies how the transform knob works. Known types are:

       TRANSFORM (0) = Used by Transform operator. Initializes to the
       identity and initializes the center to put itself in the middle of
       the image. The stored matrix will transform pixel positions.

       TO_PROXY = Same as TRANSFORM but when it calculates the matrix
       when Nuke is in proxy mode, it also stores the conversion from
       full-size to proxy format. This can be used by a knob like the
       Bezier that wants to store a set of x/y positions specified by
       the user.

       POSITIONER = Used to place objects, such as Text, where location
       0,0 is of some importance to the user. Hides the "center" control
       from the user (though the user can still move it with ctrl+drag)
       The passed matrix is used to initialize the transform.  Currently
       only scale and translate is deduced from it.

       POSITIONER|TO_PROXY = Does both.

       T2D_GAUGE adds a little angular handle that outputs a value between
       0 and 1 in bottom right corner of the matrix. Be sure to reset
       this value to 1 before using the matrix! Example use is transparency
       in the 'wipe' UI of the viewer.
     */
    inline Knob* Transform2d_knob(Knob_Callback f, Matrix4* p, NAME n, int type = 0)
    {
      return f(TRANSFORM2D_KNOB, FloatPtr, p, n, nullptr, (void*)(long long)type);
    }

    inline Knob* Transform2d_knob(Knob_Callback f, Matrix4* p, NAME n, LABEL l, int type = 0)
    {
      return f(TRANSFORM2D_KNOB, FloatPtr, p, n, l, (void*)(long long)type);
    }

    /*!
       A set of lookup curves is stored and the user is presented with an
       editor in which they can manipulate them. See the LookupCurves class for
       more information.
     */
    inline Knob* LookupCurves_knob(Knob_Callback f, LookupCurves* p, DD::Image::LookupCurvesType type, NAME n, LABEL l = nullptr)
    {
      return f(LOOKUPCURVES_KNOB, LookupCurvesPtr, p, n, l, &type);
    }

    /*!
       A set of lookup curves is stored and the user is presented with an
       editor in which they can manipulate them. See the LookupCurves class for
       more information.
     */
    inline Knob* LookupCurves_knob(Knob_Callback f, LookupCurves* p, NAME n, LABEL l = nullptr)
    {
      return f(LOOKUPCURVES_KNOB, LookupCurvesPtr, p, n, l, nullptr);
    }

    /*!
       This knob stores into a Mat4d a 3-D transformation. A 3-D
       transform jack is presented to the user in the 3D viewer.

       This creates named child knobs.  If the name of the knob
       ends with "_" then these names are instead appended to this 
       overall name, which allows more than one of these to be used in a
       node.
     */
    inline Knob* Axis_knob(Knob_Callback f, fdk::Mat4d* p, NAME n, LABEL l = nullptr)
    {
      return f(AXIS_KNOB, DoublePtr, p, n, l, nullptr);
    }
    inline Knob* Axis_knob(Knob_Callback f, Matrix4* p, NAME n, LABEL l = nullptr)
    {
      return f(AXIS_KNOB, FloatPtr, p, n, l, nullptr);
    }


    /*!
      A knob which represents a vector as an arrow joining two X,Y,Z positions.

      The parameter p is a pointer to a pair of x, y, z points (6 floats) which are
      used to set the default start and end position of the arrow.
    */
    inline Knob* PositionVector_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      return f(POSITIONVECTOR_KNOB, FloatPtr, p, n, l, nullptr);
    }


    class Box3;
    /*! Stores 2 x,y,z points(6 floats) describing a 3D volume. */
    inline Knob* Box3_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr)
    {
      return f(BOX3_KNOB, FloatPtr, p, n, l, nullptr);
    }

    /*! A histogram knob displays a histogram and lets the user
        choose 5 mapping parameters */
    inline Knob* Histogram_knob(Knob_Callback f, Histogram_Data* p, NAME n, LABEL l = nullptr)
    {
      return f(HISTOGRAM_KNOB, Custom, nullptr, n, l, p);
    }

    /*! obsolete */
    inline Knob* Keyer_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(KEYER_KNOB, DoublePtr, p, n, l, nullptr);
    }

    /*!
       Stores an unsigned value representing a color for use in coloring
       in the GUI. The stored bytes are rrggbb00. If you want the floating
       point equivalents you can use from_sRGB() on these.
     */
    inline Knob* ColorChip_knob(Knob_Callback f, unsigned* p, NAME n, LABEL l = nullptr)
    {
      return f(COLORCHIP_KNOB, UnsignedIntPtr, p, n, l, nullptr);
    }

    ////////////////////////////////////////////////////////////////
    // Layout knobs. Should always pass null as the "name" so the find_knob
    // functions work.

    /*! Starts a new tab on the control panel and resets the BeginGroup()
       hierarchy level to zero. */
    inline Knob* Tab_knob(Knob_Callback f, LABEL label)
    {
      return f(TAB_KNOB, Custom, nullptr, nullptr, label, nullptr);
    }

    inline Knob* MultiArray_knob(Knob_Callback f, NAME n, LABEL l = nullptr)
    {
      char dummy;
      return f(MULTIARRAY_KNOB, Custom, &dummy, n, l, nullptr);
    }

    /*!
       Starts a new hierarchy level. This produces a line with an open/close icon on the
       left, which causes all knobs between this and the matching EndGroup() to show/hide.

       The name can be used to look this up so you can call Knob::open() and Knob::close()
       on it from a knob_changed() method.

       The default-open state must be passed via constructor rather than via SetFlags so
       that the default can be remembered and scripts written with the settings that way.
     */
    inline Knob* BeginOpenGroup(Knob_Callback f, NAME name, LABEL label = nullptr)
    {
      return f(TAB_KNOB, Custom, nullptr, name, label, (void*)(long long)1/*DefaultOpen*/);
    }

    /*!
       Same as BeginOpenGroup() except the group is closed by default.

       The default-closed state must be passed via constructor rather than via SetFlags so
       that the default can be remembered and scripts written with the settings that way.
     */
    inline Knob* BeginClosedGroup(Knob_Callback f, NAME name, LABEL label = nullptr)
    {
      return f(TAB_KNOB, Custom, nullptr, name, label, (void*)(long long)2/*DefaultClosed*/);
    }

    //! Same as BeginOpenGroup() or BeginClosedGroup() depending on \a groupOpenByDefault state.
    inline Knob* BeginGroup(Knob_Callback f, bool groupOpenByDefault, NAME name, LABEL label = nullptr)
    {
      return (groupOpenByDefault) ? BeginOpenGroup(f, name, label) : BeginClosedGroup(f, name, label);
    }

    /*!
       Ends a hierarchy level.
     */
    inline Knob* EndGroup(Knob_Callback f)
    {
      return f(TAB_KNOB, Custom, nullptr, nullptr, nullptr, (void*)(long long)-1);
    }

    //! Same as BeginOpenGroup(), the group is open by default. Use BeginOpenGroup() instead.
    inline Knob* BeginGroup(Knob_Callback f, NAME name, LABEL label = nullptr) { return BeginOpenGroup(f, name, label); }

    /*!
       Starts a new viewer toolbar. This produces a toolbar which appears along a
       edge of each viewer.
       Use EndToolbar to finish the toolbar.
     */
    inline Knob* BeginToolbar(Knob_Callback f, NAME name, LABEL label = nullptr, int position = Knob::TOOLBAR_TOP)
    {
      return f(TOOLBAR_KNOB, Custom, nullptr, name, label, (void*)(long long)position);
    }

    /*!
       Ends a toolbar.
     */
    inline Knob* EndToolbar(Knob_Callback f)
    {
      return f(TOOLBAR_KNOB, Custom, nullptr, nullptr, nullptr, (void*)(long long)-1);
    }

    /*!
       Starts a new tabbed group.
       Use EndTabGroup to finish the group and TabKnob to add new tabs to the group.
     */
    inline Knob* BeginTabGroup(Knob_Callback f, NAME name, LABEL label = nullptr )
    {
      return f(TABGROUP_KNOB, Custom, nullptr, name, label, (void*)(long long)0);
    }

    /*!
       Ends a tabbed group.
     */
    inline Knob* EndTabGroup(Knob_Callback f)
    {
      return f(TABGROUP_KNOB, Custom, nullptr, nullptr, nullptr, (void*)(long long)1);
    }

    /*!
       Starts a new group outside of any tabs
       Use EndExogroup to finish the group
     */
    inline Knob* BeginExoGroup(Knob_Callback f, NAME name, LABEL label = nullptr )
    {
      return f(EXOGROUP_KNOB, Custom, nullptr, name, label, (void*)(long long)0);
    }

    /*!
       Ends a extratabular group.
     */
    inline Knob* EndExoGroup(Knob_Callback f)
    {
      return f(EXOGROUP_KNOB, Custom, nullptr, nullptr, nullptr, (void*)(long long)1);
    }

    /*!
       Displays some text. This can be placed between any knobs if you
       do ClearFlags(f,Knob::STARTLINE) on the next knob.
     */
    inline Knob* Text_knob(Knob_Callback f, LABEL text)
    {
      return f(TEXT_KNOB, Custom, nullptr, nullptr, nullptr, text);
    }

    /*!
       Deprecated, do not use. Same as Newline(f,label) followed by
       Text_knob(f,text).
     */
    inline Knob* Text_knob(Knob_Callback f, LABEL label, LABEL text)
    {
      return f(TEXT_KNOB, Custom, nullptr, nullptr, label, text);
    }

    /*!
       Same as Text_knob() but the knob has a name. This is necessary if
       you want to find the knob again in order to hide/show it.
     */
    inline Knob* Named_Text_knob(Knob_Callback f, LABEL name, LABEL text)
    {
      return f(TEXT_KNOB, Custom, nullptr, name, "", text);
    }

    /*!
       Same as Named_Text_knob() with the addition that this text knob have a
       label in the left column on the property panel to show.
     */
    inline Knob* Named_Text_knob(Knob_Callback f, LABEL name, LABEL label, LABEL text)
    {
      return f(TEXT_KNOB, Custom, nullptr, name, label, text);
    }

    /*!
       Start a new line in the panel. The label, if provided, is put on
       the left.
     */
    inline Knob* Newline(Knob_Callback f, LABEL label = " ")
    {
      return f(TEXT_KNOB, Custom, nullptr, nullptr, label, "");
    }

    /*! Horizontal divider line. The label, if provided, is put to the left
       of it. */
    inline Knob* Divider(Knob_Callback f, LABEL label = nullptr)
    {
      return f(TEXT_KNOB, Custom, nullptr, nullptr, label, nullptr);
    }

    /*! Horizontal gap between knobs. Be sure to
        ClearFlags(f,Knob::STARTLINE) on the next knob. */
    inline Knob* Spacer(Knob_Callback f, int spaceInPixels)
    {
      return f(SPACER_KNOB, Custom, nullptr, nullptr, nullptr, (void*)(long long)spaceInPixels);
    }

    /*! Vertical gap between knobs. */
    inline Knob* VSpacer(Knob_Callback f, int spaceInPixels)
    {
      return f(VSPACER_KNOB, Custom, nullptr, nullptr, nullptr, (void*)(long long)spaceInPixels);
    }

    /*! A non-interactive icon */
    inline Knob* Icon_knob(Knob_Callback f, const char* icon)
    {
      return f(ICON_KNOB, Custom, nullptr, nullptr, nullptr, (void*)icon);
    }

    /*! Displays a questionmark button, when the user clicks it a popup
       window appears displaying a block of text.
     */
    inline Knob* Help_knob(Knob_Callback f, LABEL text)
    {
      return f(HELP_KNOB, Custom, nullptr, nullptr, nullptr, text);
    }

    /*! You can put other symbols on the questionmark button with this. */
    inline Knob* Help_knob(Knob_Callback f, LABEL label, LABEL text)
    {
      return f(HELP_KNOB, Custom, nullptr, nullptr, label, text);
    }

    /*!
       Produces a button that when the user presses it it will call the
       Op::knob_changed() method.
     */
    inline Knob* Button(Knob_Callback f, NAME n, LABEL l = nullptr)
    {
      return f(SCRIPT_KNOB, Custom, nullptr, n, l, nullptr);
    }

    /*!
       Produces a button that when the user presses it it will execute in Nuke
       scripting language the passed command, with this knob as "context" so
       that relative names work. Also does knob_callback().
     */
    inline Knob* Script_knob(Knob_Callback f, NAME command, NAME n, LABEL l = nullptr)
    {
      return f(SCRIPT_KNOB, Custom, nullptr, n, l, (const char* const*)command);
    }

    /*!
       Produces a button that when the user presses it it will execute in Nuke
       Python the passed command, with this knob as "context" so
       that relative names work. Also does knob_callback().
     */
    inline Knob* PyScript_knob(Knob_Callback f, NAME command, NAME n, LABEL l = nullptr)
    {
      return f(PYSCRIPT_KNOB, Custom, nullptr, n, l, (const char* const*)command);
    }

    /*!
     Produces a button that when the user presses it it will cancel and abort the current
     Op tree. It effectively simulates pressing "Cancel" in the progress dialog.
     */
    inline Knob* CancelExecution_knob(Knob_Callback f, NAME n, LABEL l = nullptr)
    {
      return f(CANCELEXECUTION_KNOB, Custom, nullptr, n, l, nullptr);
    }

    inline Knob* Menu_knob(Knob_Callback f, const char* menuName, NAME name, LABEL label = nullptr)
    {
      return f(MENU_KNOB, StringPtr, &menuName, name, label, nullptr);
    }
    
    /*!
       Produces a custom Python knob. This is a knob which will execute Python
       code to produce its UI.
     */
    inline Knob* Python_knob(Knob_Callback f, NAME command, NAME n, LABEL l = nullptr)
    {
      return f(PYTHON_KNOB, Custom, nullptr, n, l, (const char* const*)command);
    }

    /*!
       Makes two numbers with custom labels for the x and y values.
     */
    inline Knob* Vec2d_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr, LABEL xLabel = nullptr, LABEL yLabel = nullptr)
    {
      Vec2KnobData data{xLabel, yLabel};
      return f(VEC2_KNOB, DoublePtr, p, n, l, (void*)&data);
    }
    inline Knob* Vec2f_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr, LABEL xLabel = nullptr, LABEL yLabel = nullptr)
    {
      Vec2KnobData data{xLabel, yLabel};
      return f(VEC2_KNOB, FloatPtr, p, n, l, (void*)&data);
    }

    /*!
       Makes three numbers with custom labels for the x, y and z values.
     */
    inline Knob* Vec3d_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr, 
                            LABEL xLabel = nullptr, LABEL yLabel = nullptr, LABEL zLabel = nullptr)
    {
      Vec3KnobData data{xLabel, yLabel, zLabel};
      return f(VEC3_KNOB, DoublePtr, p, n, l, (void*)&data);
    }
    inline Knob* Vec3f_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr, 
                            LABEL xLabel = nullptr, LABEL yLabel = nullptr, LABEL zLabel = nullptr)
    {
      Vec3KnobData data{xLabel, yLabel, zLabel};
      return f(VEC3_KNOB, FloatPtr, p, n, l, (void*)&data);
    }

    /*!
       Makes four numbers with custom labels for the x, y and z values.
     */
    inline Knob* Vec4d_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr, 
                            LABEL xLabel = nullptr, LABEL yLabel = nullptr, LABEL zLabel = nullptr, LABEL wLabel = nullptr)
    {
      Vec4KnobData data{xLabel, yLabel, zLabel, wLabel};
      return f(VEC4_KNOB, DoublePtr, p, n, l, (void*)&data);
    }
    inline Knob* Vec4f_knob(Knob_Callback f, float* p, NAME n, LABEL l = nullptr, 
                            LABEL xLabel = nullptr, LABEL yLabel = nullptr, LABEL zLabel = nullptr, LABEL wLabel = nullptr)
    {
      Vec4KnobData data{xLabel, yLabel, zLabel, wLabel};
      return f(VEC4_KNOB, FloatPtr, p, n, l, (void*)&data);
    }

    /*!
       Produces an extended custom Python knob. This is a knob which will execute Python
       code to produce its UI and will have extended functionality specific to the 
       BlinkScript editor.
     */
    inline Knob* BlinkEditor_knob(Knob_Callback f, NAME command, NAME n, LABEL l = nullptr)
    {
      return f(BLINKEDITOR_KNOB, Custom, nullptr, n, l, (const char* const*)command);
    }

    /*!
       Produces a pulldown menu with multiple items that will execute in Nuke
       scripting language the passed command, with this knob as "context" so
       that relative names work.
     */
    inline Knob* Pulldown_knob(Knob_Callback f, ENUM_LABELS e, NAME n, LABEL l = nullptr)
    {
      return f(PULLDOWN_KNOB, Custom, nullptr, n, l, e);
    }

    /*!
       Produces a pulldown menu with multiple items that will execute in Nuke
       scripting language the passed command, with this knob as "context" so
       that relative names work.
     */
    inline Knob* PyPulldown_knob(Knob_Callback f, ENUM_LABELS e, NAME n, LABEL l = nullptr)
    {
      return f(PYPULLDOWN_KNOB, Custom, nullptr, n, l, e);
    }

    /*! Multi-line text editor. */
    inline Knob* Multiline_String_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr, int lines = 5)
    {
      return f(MULTILINE_STRING_KNOB, StringPtr, p, n, l, (void*)(long long)lines);
    }

    /*!
       Use this macro in Op::knobs to create your own class of knob.
       Your class must have a constructor that takes (&cb, name)
       as arguemnts, and the store() function will not be called.
     */
#define CustomKnob0(knobclass, cb, name)           \
  (knobclass*)cb( DD::Image::CUSTOM_KNOB, DD::Image::Custom, 0, name, 0, \
                  (cb.makeKnobs() && cb.filter(name)) ? new knobclass(&cb, name) : (cb.setLastMadeKnob(nullptr), (knobclass*)nullptr))

    /*!
       Use this macro in Op::knobs to create your own class of knob.
       A constructor for your class must take (&cb, pointer, name)
       as arguments, and store(pointer,...) will be called if
       pointer is not zero.
     */
#define CustomKnob1(knobclass, cb, pointer, name)        \
  (knobclass*)cb( DD::Image::CUSTOM_KNOB, DD::Image::Custom, pointer, name, 0, \
                  (cb.makeKnobs() && cb.filter(name)) ? new knobclass(&cb, pointer, name) : (cb.setLastMadeKnob(nullptr), (knobclass*)nullptr))

    /*!
       Use this macro in Op::knobs to create your own class of knob.  A
       constructor for your class must take (&cb, pointer, name, label) as
       arguments (note label can be an arbitrary type), and
       store(pointer,...) will be called if pointer is not zero.
     */
#define CustomKnob2(knobclass, cb, pointer, name, label) \
  (knobclass*)cb( DD::Image::CUSTOM_KNOB, DD::Image::Custom, pointer, name, 0, \
                  (cb.makeKnobs() && cb.filter(name)) ? new knobclass(&cb, pointer, name, label) : (cb.setLastMadeKnob(nullptr), (knobclass*)nullptr))

    /*!
       Use this macro in Op::knobs to create your own class of knob.
       The \a args is a parenthesized set of arguments to pass to the
       constructor, and store(pointer...) will be called if pointer
       is not zero.
       This is not type safe as nothing makes sure the
       type in \a pointer is correct because it is not passed to the
       constructor.
     */
#define CustomKnobA(knobclass, cb, pointer, name, args)  \
  (knobclass*)cb( CUSTOM_KNOB, DD::Image::Custom, pointer, name, 0, \
                  (cb.makeKnobs() && cb.filter(name)) ? new knobclass args : (cb.setLastMadeKnob(nullptr), (knobclass*)nullptr))


    ////////////////////////////////////////////////////////////////

    // Rest is for Nuke4 back-compatibility. You should be able to replace
    // all uses of this with the above functions.

    /*!
       Represents an inclusive range from one double value to another.
     */
    struct IRange {
      double a, b;
      bool force;
      /*!
         Construct an inclusive range. 
         \a A refers to the start of the range.
         \a B refers to the end of the range. 
         \a F indicates whether the range should be enforced or not. Setting
         this to true is equivalent to setting the FORCE_RANGE flag on the
         knob.
       */
      IRange(double A, double B, bool F = false) : a(A), b(B), force(F) {}
    };
    inline Knob* RangeStuff(Knob_Callback f, Knob* knob, const IRange& r)
    {
      SetRange(f, r.a, r.b);
      if (r.force)
        SetFlags(f, Knob::FORCE_RANGE);
      if (r.a > 0 || (!r.a && !fmod(sqrt(r.b), 1)))
        SetFlags(f, Knob::LOG_SLIDER);
      return knob;
    }

    inline Knob* Int_knob(Knob_Callback f, int* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      Knob* k = f(INT_KNOB, IntPtr, p, n, l, nullptr);
      SetRange(f, r.a, r.b);
      if (r.force)
        SetFlags(f, Knob::FORCE_RANGE);
      return k;
    }

    inline Knob* Double_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(FLOAT_KNOB, DoublePtr, p, n, l, nullptr);
    }

    inline Knob* Double_knob(Knob_Callback f, double* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      return RangeStuff(f, f(FLOAT_KNOB, DoublePtr, p, n, l, nullptr), r);
    }

    inline Knob* Float_knob(Knob_Callback f, double* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      return RangeStuff(f, f(FLOAT_KNOB, DoublePtr, p, n, l, nullptr), r);
    }

    inline Knob* Float_knob(Knob_Callback f, float* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      return RangeStuff(f, f(FLOAT_KNOB, FloatPtr, p, n, l, nullptr), r);
    }

    inline Knob* ChannelMask_knob(Knob_Callback f, ChannelSet* p, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{0/*count*/, -1/*input*/, chanmgr};
      return f(CHANNEL_MASK_KNOB, ChannelSetPtr, p, n, l, (void*)&data);
    }

    inline Knob* Input_ChannelMask_knob(Knob_Callback f, ChannelSet* p, int input, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{0/*count*/, input, chanmgr};
      return f(CHANNEL_MASK_KNOB, ChannelSetPtr, p, n, l, (void*)&data);
    }

    inline Knob* InputOnly_ChannelMask_knob(Knob_Callback f, ChannelSet* p, int input, NAME n, LABEL l = nullptr, ndk::ChannelManager* chanmgr = nullptr)
    {
      ChannelKnobData data{0/*count*/, input, chanmgr};
      return f(INPUTONLY_CHANNEL_MASK_KNOB, ChannelSetPtr, p, n, l, (void*)&data);
    }

    inline Knob* WH_knob(Knob_Callback f, double* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      return RangeStuff(f, f(WH_KNOB, DoublePtr, p, n, l, nullptr), r);
    }

    inline Knob* Scale_knob(Knob_Callback f, double* p, NAME n, LABEL l = nullptr)
    {
      return f(SCALE_KNOB, DoublePtr, p, n, l, nullptr);
    }

    inline Knob* Scale_knob(Knob_Callback f, double* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      return RangeStuff(f, f(SCALE_KNOB, DoublePtr, p, n, l, nullptr), r);
    }

    inline Knob* Color_knob(Knob_Callback f, float* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      return RangeStuff(f, f(COLOR_KNOB, FloatPtr, p, n, l, nullptr), r);
    }

    inline Knob* Color_knob(Knob_Callback f, double* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      return RangeStuff(f, f(COLOR_KNOB, DoublePtr, p, n, l, nullptr), r);
    }

    inline Knob* AColor_knob(Knob_Callback f, float* p, const IRange& r, NAME n, LABEL l = nullptr)
    {
      return RangeStuff(f, f(ACOLOR_KNOB, FloatPtr, p, n, l, nullptr), r);
    }

    inline Knob* List_knob(Knob_Callback f, std::vector<std::vector<std::string> >* p, NAME n, LABEL l = nullptr)
    {
      return f(LIST_KNOB, StringVecVecPtr, p, n, l, nullptr);
    }

    inline Knob* Table_knob(Knob_Callback f, NAME n, LABEL l = nullptr)
    {
      return f(TABLE_KNOB, Custom, (void*)1, n, l, nullptr);
    }

    inline Knob* Range_knob(Knob_Callback f, double* p, int np, int ixp, NAME n, LABEL l = nullptr,
                            double min = 0.0, double max = 1.0)
    {
      Knob* knob = f(RANGE_KNOB, DoublePtr, p, n, l, (void*)(long long)np);
      if (ixp)
        SetFlags(f, Knob::LOG_SLIDER);
      SetRange(f, min, max);
      return knob;
    }

    inline Knob* Tab_knob(Knob_Callback f, int level, LABEL label)
    {
      return f(TAB_KNOB, Custom, nullptr, nullptr, label, nullptr);
    }

    inline Knob* Link_knob(Knob_Callback f, const char* linkto, NAME name, LABEL label = nullptr)
    {
      return f(LINK_KNOB, Custom, nullptr, name, label, linkto);
    }

    inline Knob* Multiline_Eval_String_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr, int lines = 5)
    {
      return Multiline_String_knob(f, p, n, l, lines);
    }

    /*!
     * Creates a new String knob that may contain an expression.
     */
    inline Knob* Eval_String_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr)
    {
      return Multiline_String_knob(f, p, n, l, 0);
    }


    inline Knob* TextEditor_knob(Knob_Callback f, const char** p, NAME n, LABEL l = nullptr, int lines = 5)
    {
      return Multiline_String_knob(f, p, n, l, lines);
    }

    /*!
       Controls an integer enumeration. The array of labels \a e will produce
       a pop-up list the user can choose from. The index into this list
       is stored. The first item will store a zero, etc. You can assume
       that only legal values are stored.
     */
    inline Knob* Toolbox_knob(Knob_Callback f, int* p, bool expanded, ENUM_LABELS e, NAME n, LABEL l = nullptr, bool changeOnClick = false)
    {
      std::pair< ENUM_LABELS, std::pair<bool, bool> > value(e, std::make_pair(expanded, changeOnClick));
      
      Knob* newKnob = f(TOOLBOX_KNOB, IntPtr, p, n, l, &value);
      
      return newKnob;
    }

    inline Knob* GeoSelect_knob(DD::Image::Knob_Callback f, const char* name)
    {
      return f(GEOSELECT_KNOB, Custom, nullptr, name, nullptr, nullptr);
    }
    
    inline Knob* ControlPointCollection_knob(Knob_Callback f, NAME n, LABEL l = nullptr)
    {
      return f(CONTROL_POINT_COLLECTION_KNOB, Custom, nullptr, n, l, nullptr);
    }

    inline Knob* MetaKeyFrame_knob(Knob_Callback f, NAME n, LABEL l = nullptr,
                                   const DD::Image::MetaKeyFrameOptions& options = DD::Image::MetaKeyFrameOptions())
    {
      return f(META_KEY_FRAME_KNOB, Custom, nullptr, n, l, (const char*)&options);
    }

    inline Knob* TransformJack_knob( Knob_Callback f, NAME n )
    {
      return f( TRANSFORM_JACK_KNOB, Custom, nullptr, n, nullptr );
    }
    
    inline Knob* Ripple_knob(Knob_Callback f, NAME n, LABEL l = nullptr)
    {
      return f(RIPPLE_KNOB, Custom, nullptr, n, l, nullptr);
    }

    /*! A knob managing the logic for disabling nodes */
    inline Knob* Disable_knob(Knob_Callback f, bool* p, NAME n, LABEL l = nullptr)
    {
      return f(DISABLE_KNOB, BoolPtr, p, n, l, nullptr);
    }

    inline Knob* FrameExtent_knob(Knob_Callback f, DD::Image::FrameExtentProviderI* prov, NAME n, LABEL l = nullptr)
    {
      return f(FRAME_EXTENT_KNOB, Custom, prov, n, l, nullptr);
    }

    inline Knob* Radio_knob( Knob_Callback f, int* p, ENUM_LABELS e, NAME n, LABEL l = nullptr )
    {
      return f( RADIO_KNOB, IntPtr, p, n, l, e );
    }

    inline Knob* Gsv_knob( Knob_Callback f, NAME n, LABEL l = nullptr )
    {
      return f(GSV_KNOB, Custom, nullptr, n, l, nullptr);
    }
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
