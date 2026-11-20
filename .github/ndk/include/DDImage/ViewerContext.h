// ViewerContext.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef ViewerContext_H
#define ViewerContext_H

#include "DDImage/Box.h"
#include "DDImage/Box3.h"
#include "DDImage/Matrix4.h"
#ifndef FN_NO_LEGACY_3D
#include "DDImage/GeoInfo.h"
#endif
#include "DDImage/Hash.h"
#include "DDImage/Format.h"
#include "DDImage/Menu.h" // TODO : forward declare instead

#include "DDImage/GlCacheApi.h"

#include "fdk/math/Ray.h"
#include "ndk/geo/utils/Intersect.h"

#include "usg/geom/MeshPrim.h"
 
#include <memory>
#include <vector>

class Viewer_Window;
class Node;
class SelectionManager;
struct KnobSelectionRecord;

#ifdef FN_OS_WINDOWS
# ifndef __INTEL_COMPILER
  #pragma warning(push)
  #pragma warning(disable:4251)
# endif // __INTEL_COMPILER
#endif // FN_OS_WINDOWS

namespace usg {
  class Prim;
}

namespace ndk {
  struct MeshTopology;
}

namespace DD {
  namespace Image {

    class AxisOp;
    class CameraOp;
    class LightOp;
#ifndef FN_NO_LEGACY_3D
    class RegularGrid;
    class GeoInfo;
#endif
    class GeoSelection;

    /*!
       A pointer to this structure is passed to the Op::build_handles() methods
       which then add callbacks to the draw_handle_list member that will
       draw things in the viewer and handle mouse clicks or other events.

       The build_handles() methods can modify this structure. Besides adding
       to the draw_handle_list, the can modify the bbox or camera to output
       information needed by the viewer as to what is being looked at. And
       they can modify the projections and flags before recursively calling
       their inputs, but these changes must be restored afterwards.

       The same structure is passed to the callbacks added to draw_handle_list.
       These callbacks cannot modify the structure, but they can look at
       it. Many of the fields, such as the projection, will be set to match
       their state when build_handles() added the callback.
     */
    class ViewerContext;

    typedef void (* DrawHandleCallbackFunc)(void*, ViewerContext*);

    /*!
       The types of drawing handle that Nuke recognises. Geometry handles are
       always drawn, regardless of the selection mode; all other types are only
       drawn when the Nuke viewer is in the appropriate selection mode. The
       default mode is node selection, which corresponds to the original Nuke
       behaviour.
     */
    enum DrawHandleType {
      eDrawHandleAlways	         = 0xFFFFFFFF,   // Always draw this handle, regardless of which selection mode we're in.
      eDrawHandleVertexSelection = 0x1,          // Draw this handle when in vertex selection mode.
      eDrawHandleEdgeSelection	 = 0x2,          // Draw this handle when in edge selection mode. Note that edge selection mode is not implemented yet.
      eDrawHandleFaceSelection	 = 0x4,          // Draw this handle when in face selection mode. Note that face selection mode is not implemented yet
      eDrawHandleObjectSelection = 0x8,          // Draw this handle when in object selection mode. Note that object selection mode is not implemented yet
      eDrawHandleNodeSelection   = 0x10          // Draw this handle when in node selection mode. This is the default selection mode in Nuke.
    };

    typedef unsigned int    DrawHandleTypeMask;   // A logical or of the DrawHandleTypes

    /*! \relates DD::Image::ViewerContext
       Values for ViewerContext::connected() */
    enum ViewerConnected {
      NOT_CONNECTED = 0, //!< There is no connection.
      CONNECTED = 1,     //!< There is a connection from here to viewer input
      SHOW_OBJECT,       //!< Connected, draw an object
      SHOW_PUSHED_OBJECT //!< Connected, draw object if node is pushed()
    };

    /*! \relates DD::Image::ViewerContext
       Member of Viewer_Context::draw_handle_list. */
    struct Draw_Handle_Callback
    {
      DrawHandleCallbackFunc callback; //!< Function to call
      void* object; //!< argument for the callback
      Node* node; //!< node to hit-detect if no other begin_handle done
      Matrix4 matrix; //!< saved context modelmatrix
      ViewerConnected connected; //!< saved value for connected
      bool reallyConnected; //!< saved value for reallyConnected
      DrawHandleTypeMask drawHandleType; //!< the handle type. A logical or or DrawHandleTypes
      bool glCached; //!< flag used to know if the handles drawing opengl function calls are cached
      const void* data; //!< Internal use only.
      Matrix4 parentTransform; //!< saved context parent transformation
    };

#ifndef FN_NO_LEGACY_3D
    struct DrawableGeo
    {
      GeoInfo* geo;
      unsigned int offset;  ///< The starting index for subselectable items on this drawable
      unsigned int length;  ///< The number of subselectable items on this drawable.
      Node* node;
      Matrix4 matrix;
      ViewerConnected connected;
      Hash hash;

      /// So we can use the STL's binary search functions.
      bool operator < (const DrawableGeo& other) const { return offset < other.offset; }
    };
#endif

    /*! \relates DD::Image::ViewerContext
       Value for ViewerContext::transformSpace() */
    enum class TransformSpace : uint8_t {
      Object, //!< Object Space
      Screen, //!< Screen Space
      World   //!< World Space
    };

    /*! \relates DD::Image::ViewerContext
       Value for ViewerContext::viewer_mode() */
    enum ViewerMode {
      VIEWER_2D = 0, //!< orthographic 2D
      VIEWER_PERSP, //!< perspective 3D
      VIEWER_RSIDE, //!< ortho 3D down -x axis
      VIEWER_LSIDE, //!< ortho 3D down +x axis
      VIEWER_TOP, //!< ortho 3D down -y axis
      VIEWER_BOTTOM, //!< ortho 3D down +y axis
      VIEWER_FRONT, //!< ortho 3D down -z axis
      VIEWER_BACK, //!< ortho 3D down +z axis
      VIEWER_MODE_COUNT
    };

    /*! \relates DD::Image::ViewerContext
       Value for ViewerContext::event(). */
    enum ViewerEvent {
      NO_EVENT = 0, //!< only call menu(), otherwise ignore
      DRAW_OPAQUE, //!< solid objects/texture maps
      DRAW_TRANSPARENT, //!< transparent (below alpha threshold) 3D pass
      DRAW_STIPPLED, //!< draw things hidden behind solid objects
      DRAW_SHADOW, //!< shadows under lines in the 2D pass
      DRAW_LINES, //!< lines in 3D, and the entire 2D pass
      DRAW_OVERLAY, //!< overlays in 3D, which should be on top of everything else.
      MOVE,       //!< draw_handles() is being called to find tooltip
      PUSH,       //!< user just pushed the mouse down, hit detection. Any handle callback which returns true for this event should also return true for DRAG and RELEASE
      DRAG,       //!< user is dragging mouse, region selection hit detection. Any handle callback which returns true for this event should also return true for PUSH and RELEASE
      RELEASE,    //!< user released the mouse. Any handle callback which returns true for this event should also return true for PUSH and DRAG
      KEY,        //!< user hit a key
      KEYUP,      //!< user let go of key
      //   WHEEL,          //!< user moved mouse wheel or other tracking device
      DROP,       //!< user dropped some data in a drag'n'drop operation
      DROP_CHECK, //!< user is dragging data over a handle
      ENTER_VIEWER, //!< the mouse has entered the viewer
      EXIT_VIEWER, //!< the mouse has exited the viewer
      CURSOR, //!< hit-detecting for a handle cursor
      HOVER_ENTER, //!< the hover was entered
      HOVER_MOVE, //!< user is hovering
      HOVER_LEAVE, //!< user has left hovering
      DRAG_SELECT,  //!< user is drag-selecting handles
      DRAG_SELECT_FINISHED, //!< drag selection of handles has finished
      FIRST_MENU = 40 //!< recommended start of Menu events
    };

    /*! \relates DD::Image::ViewerContext
    Value for camera lock mode */
    enum CameraLockMode {
      CAMERA_LOCKMODE_UNLOCKED = 0,
      CAMERA_LOCKMODE_LOCKED, 
      CAMERA_LOCKMODE_INTERACTIVE,
      CAMERA_LOCKMODE_COUNT
    };

    /*! \relates DD::Image::ViewerContext
       Value for GeoOp::display3d() and ViewerContext::display3d() */
    enum Display3DMode : int {
      DISPLAY_OFF = 0,
      DISPLAY_WIREFRAME,
      DISPLAY_SOLID,
      DISPLAY_SOLID_LINES,
      DISPLAY_TEXTURED,
      DISPLAY_TEXTURED_LINES,
      DISPLAY_UNCHANGED // Must be last/highest number
    };

    /*! Per-object render modes. Deprecated.
    */
    enum RenderMode : int {
      RENDER_OFF = 0,
      RENDER_WIREFRAME,
      RENDER_SOLID,
      RENDER_SOLID_LINES,
      RENDER_TEXTURED,
      RENDER_TEXTURED_LINES,
      RENDER_UNCHANGED // Must be last/highest number
    };

    //! Name text array for render_mode enumeration knob
    extern DDImage_API const char* const render_modes[];
    //! Same as render_modes, except missing the "unchanged" entry
    extern DDImage_API const char* const render_modes_source[];


    /// The selection modes for the 3d viewer. The default is eSelect3DNode, in
    /// which an entire node will be selected if you click on any part of it
    /// in the 3d viewer (the old Nuke behaviour). eSelect3DVertex will select
    /// individual vertices;
    enum SelectionMode3D {
      eSelect3DNode,    // Clicking on geometry selects the associated node; the Nuke default.
      eSelect3DVertex,  // Select individual vertices on objects.
      eSelect3DFace,    // Select individual faces on objects.
      eSelect3DObject,  // Select individual objects.

      eNumberOfSelectionModes3D
    };

    /// Selection options for the 3D viewer. These options appear in the same menu as
    /// the Selection modes above, so it's important that the first option's ID is
    // outside the range of the viewer mode IDs so that options can be distinguished.
    enum SelectionOptions3D
    {
      eOptionDisableOcclusionTesting = eNumberOfSelectionModes3D
    };

    enum Select3DAction {
      eSelectReplace,   // For when you're making a new selection.
      eSelectAdd,       // For when you're adding things to the existing selection.
      eSelectRemove,    // For when you're removing things from the existing selection.
      eSelectDrag       // For when you clicked on an already selected item and are now dragging it.
    };

    /*! Test to see if you should draw lines */
    inline bool style_needs_wireframe(int x) { return (x & 1) != 0; }

    /*! Name text array for ViewerContext::display3d() enumeration knob*/
    extern DDImage_API const char* const display3d_names[];

    /*! Same as display3d_names, except missing the "unchanged" entry */
    extern DDImage_API const char* const display3d_names_source[];

    /*! \relates DD::Image::ViewerContext
       Value for ViewerContext::what_to_draw().
       This is a bunch of checkmarks on the viewer.
       The viewer will implement some of these (such as transparency or lighting)
       by initializing the OpenGL state, but it may still be useful to check
       if they are off.
     */
    enum {
      SHOW_POINT_HANDLES    = 0x00000100,
      SHOW_POINT_NUM    = 0x00000200,
      SHOW_POINT_INFO    = 0x00000400,
      SHOW_POINT_UVS    = 0x00000800,
      SHOW_POINT_NORMALS    = 0x00001000, //!< draw point normals
      SHOW_VERTEX_NUM    = 0x00002000, //!< draw vertex normals
      SHOW_VERTEX_UVS    = 0x00004000,
      SHOW_VERTEX_NORMALS   = 0x00008000, //!< draw vertex normals
      SHOW_PRIMITIVE_NUM    = 0x00010000, //!< draw primitive number
      SHOW_PRIMITIVE_NORMAL = 0x00020000, //!< draw primitive normal
      SHOW_BBOX         = 0x00080000, //!< draw bounding box around things
      SHOW_TRANSPARENCY    = 0x00100000, //!< transparency checkmark from viewer
      SHOW_LIGHTING        = 0x00200000, //!< lighting checkmark from viewer
      SHOW_WIREFRAME_IF_NOT_PANEL_VISIBLE = 0x00400000,

      SHOW_NO_3D = 0x00800000,  //!< turn off Nuke's native 3D viewer, for use by plugin viewers

      SHOW_OBJECT_SELECTION_DEPTH_TESTED = 0x01000000 ///!< depth tested selection from viewer
    };

    /*! \relates DD::Image::ViewerContext
       Values for ViewerContext::key() and ViewerContext::is_down().
       Same values as Qt. Use lowercase 'x' or the unshifted symbol for the
       printing character keys. */
    enum {
      LeftButton    = 1,
      MiddleButton    = 2,
      RightButton    = 3,
      SpaceKey    = 32,  // same as ' '
      // 'a'-'z', '`', '-', '=', '[', ']', '\\', ',', '.', '/', ';', '\''
      PlusKey         = 43,
      MinusKey        = 45,
      BackSpaceKey    = 0x01000003,
      TabKey    = 0x01000001,
      ClearKey    = 0x0100000b, // '5' keypad key with NumLock off on some systems
      ReturnKey    = 0x01000004,
      PauseKey    = 0x01000008,
      ScrollLockKey    = 0x01000026,
      EscapeKey    = 0x01000000,
      HomeKey    = 0x01000010,
      LeftKey    = 0x01000012,
      UpKey        = 0x01000013,
      RightKey    = 0x01000014,
      DownKey    = 0x01000015,
      PageUpKey    = 0x01000016,
      PageDownKey    = 0x01000017,
      EndKey    = 0x01000011,
      PrintKey    = 0x01000009,
      InsertKey    = 0x01000006,
      MenuKey    = 0x01000055,
      HelpKey    = 0x01000058, // the 'help' key on Mac keyboards
      NumLockKey    = 0x01000025,
      KeypadEnter    = 0x01000005,
      F0Key        = 0x01000030,
      F1Key        = F0Key + 1,
      F2Key        = F0Key + 2,
      F3Key        = F0Key + 3,
      F4Key        = F0Key + 4,
      F5Key        = F0Key + 5,
      F6Key        = F0Key + 6,
      F7Key        = F0Key + 7,
      F8Key        = F0Key + 8,
      F9Key        = F0Key + 9,
      F10Key    = F0Key + 10,
      F11Key    = F0Key + 11,
      F12Key    = F0Key + 12,
      // use F0Key+n to get function key n
      LastFunctionKey = F0Key + 35,
      LeftShiftKey    = 0x01000020,
      RightShiftKey    = 0x01000020,
      LeftControlKey = 0x01000021,
      RightControlKey = 0x01000021,
      CapsLockKey    = 0x01000024,
      LeftMetaKey    = 0x01000022, // the left "Windows" key or Ctrl on OSX
      RightMetaKey    = 0x01000022, // the right "Windows" key or Ctrl on OSX
      LeftAltKey    = 0x01000023,
      RightAltKey    = 0x01000023,
      DeleteKey    = 0x01000007
    };

    /*! \relates DD::Image::ViewerContext
       Values for ViewerContext::state().
       On the Macintosh, the Command key gives CTRL and the Ctrl key gives META. */
    enum {
      SHIFT         = 0x02000000,
      CAPSLOCK      = 0x40000000,
      CTRL          = 0x04000000,
      ALT           = 0x08000000,
      KEYPAD        = 0x20000000,
      META          = 0x10000000,
      COMMAND       = CTRL,
      OPTION        = ALT | META,
      SCROLLLOCK    = 0x80000000,
      
      ALL_MODIFIERS = 0xfe000000,
      KEY_MASK      = 0x01ffffff,
    };

    /*! \relates DD::Image::ViewerContext
       Values for ViewerContext::pointer_type().
       Valid for pen types when used with a tablet
     */
    enum {
      eUnknown = 0,
      ePen,
      eCursor,
      eEraser
    };

    /*! \relates DD::Image::ViewerContext
       Values for the selection of the USD kind
     */
    enum class SelectionKind {
        Assembly,
        Group,
        Model,
        Component,
        Subcomponent,
        Scope,
        Xform,
        Mesh,
        Material,
        NumSelectionKinds
  };

    struct SelectionFilters {
      SelectionKind _kind;
      bool _selectGeometry;
      bool _selectCameras;
      bool _selectLights;
    };

    class ViewerContextImpl;

    //--------------------------------------------------------------------

    /*! \class DD::Image::ViewerContext

        This structure is passed to the Op::build_handles() functions,
        and also to any callbacks that the build_handles() creates with
        add_draw_handle().

        The data inside of it describes the state of the Nuke viewer
        and any modifications done by the build_handles() calls by
        operators between the one being called and the viewer.
     */
    class DDImage_API ViewerContext
    {
      std::unique_ptr<ViewerContextImpl> _pImpl;

      // These friend classes can access the private variables:
      friend class ::Viewer_Window;
      friend class ::SelectionManager;
      friend struct ::KnobSelectionRecord;

    protected:

      // This portion is for both draw + build:
      ViewerMode viewer_mode_;
      TransformSpace _transformSpace;
      ViewerConnected connected_;

      // During add_draw_handles this is set to false if the Op is not connected
      // to a viewer and is only being displayed because the panel is open.
      bool _reallyConnected;
      // indicates whether a render is currently in progress
      bool _renderInProgress;
      // indicates whether the viewer is currently playing
      bool _playbackInProgress;

      Display3DMode display3d_;
      int what_to_draw_;

      // indicates whether the 3D viewer grid should be drawn
      bool _showViewerGrid;
      bool _showCameras = true;
      bool _showLighting = true;
      bool _showAxis = true;

      // This portion is for draw & add_handle callbacks:
      DD::Image::ViewerEvent event_;
      static int* mouse_x_;
      static int* mouse_y_;
      static int* wheel_dx_;
      static int* wheel_dy_;
      static float* pressure_;
      static unsigned* state_;
      static int* clicks_;
      static unsigned* is_click_;
      static unsigned* key_;
      static unsigned* length_;
      static const char** text_;
      static bool* auto_repeat_;
      static int* pointer_type_;
      Vector3 pos_;                   //< see const Vector3& ViewerContext::pos() for details
      Vector3 posDeltaSinceLast_;     //< see const Vector3& ViewerContext::dPos() for details
      Vector3 mousePosInHandleSpace_; //< see const Vector3& ViewerContext::mousePosInHandleSpace() for details

      // This portion is only for draw:
      unsigned node_color_;
      unsigned fg_color_;
      unsigned bg_color_;
      unsigned selected_color_;
      unsigned focused_color_;
      unsigned soft_select_min_color_;
      unsigned soft_select_max_color_;
      unsigned selected_3D_color_;
      float icon_size_base;
      float icon_size_power;
      
      float   transformHandleSize3D_;
      float   handleSize_;
      float   handlePickSize_;
      float   lineWidth_;
      float   _devicePixelRatio;

      Box viewport_; //!< see viewport() for more information
      float center_x, center_y;
      float reserve2, reserve3;
      Box visibleViewportArea_;
      Box _clipArea; //!< see clipArea() for more information

      int pixmap_size_hint_w_;
      int pixmap_size_hint_h_;
      bool ndtm;
      bool center_called;

      // This portion is only for build:
      std::vector<Draw_Handle_Callback> draw_handle_list;
#ifndef FN_NO_LEGACY_3D
      std::vector<DrawableGeo> drawableGeoList;
      RegularGrid* _grid;
#endif
      Hash _gridHash;

      Box3 bbox_; //!< build_handles sets this to bounding box of everything
      Box3 active_bbox_; //!< build_handles sets this to bounding box of active things
      std::vector<AxisOp*> _cameras;
      std::vector<LightOp*> _lights;
      AxisOp* camera_;
      CameraLockMode _camera_lockmode;
      int transform_mode_;

      // Used by zoom():
      Matrix4 projmatrix;
      Matrix4 cameramatrix;
      Vector4 _cameraPos; //!< The current camera location in world coordinates.

      // Used by XY_Knob, XYZ_Knob, and UV_Knob
      bool my_draw_animation_path;
      bool my_draw_animation_path_done;
      bool _accept_drop;
      Menu* _menu;

      Hash _geometryListHash;
      bool _glHandlesCached;

      void saveZBuffer();

#ifndef FN_NO_LEGACY_3D
      unsigned int getGeometryComponentLength(GeoInfo* geo);
      void recalculateGeometryOffsets();
#endif

      void setRenderInProgress(bool inProgress);

    protected:
      ViewerContext();

      /// noncopy and nonassign
      ViewerContext& operator=(const ViewerContext &);
      ViewerContext(const ViewerContext &);

      virtual ~ViewerContext();

    public:

      /*! Context information for the current viewer window's format
       */
      struct ViewerWindowFormatContext
      {
        DD::Image::Format   format;
        DD::Image::Vector2  formatCenter;
        float               formatWidth;
        bool                ignoreFormatPixelApsect;
      };

      /*! \relates DD::Image::ViewerContext
         Values for ViewerContext::setCursor().
       */
      enum Cursor {
        kNoCursor,
        kBlankCursor,
        kArrowCursor,
        kCrossCursor,
        kIBeamCursor,
        kOpenHandCursor,
        kClosedHandCursor,
        kTranslateCursor,
        kSizeNSCursor,
        kSizeEWCursor,
        kSizeNWSECursor,
        kSizeSWNECursor,
        kSizeAllCursor,
        kRotateNECursor,
        kRotateNWCursor,
        kRotateSWCursor,
        kRotateSECursor,
        kSkewXCursor,
        kSkewYCursor,
        kCornerPinCursor,
        kForbiddenCursor,
        kMovePointCursor,
        kAddPointCursor,
        kRemovePointCursor,
        kFeatherPointCursor,
        kRemoveFeatherPointCursor,
        kSmoothPointCursor,
        kCuspPointCursor,
        kClosePathCursor,
        kZoomInCursor,
        kZoomOutCursor,
        kMarqueeCursor,

        kNumCursors
      };

      /*! \fn const Vector3& ViewerContext::pos() const NB : also see ViewerContext::dpos()
          Returns the coordinate associated with the event (e.g. the
          mouse pos when clicking on a handle or the pos of the selected
          handle when a key is pressed) translated into whatever coordinate
          system was in effect when the current handle was created.

          When the event type is PUSH, CURSOR or HOVER_MOVE and the
          handle type is POSITION this is equal to the handle's location
          in handle-space. This call only ever operates on one handle at a
          time.

          Similarly when the event type is PUSH, CURSOR or HOVER_MOVE
          and the handle type is SELECTABLE this is equal to the handle's
          location for non-batched handles and the absolute (adjusted) mouse
          location (in handle space) for batched handles.

          Otherwise this position, for any other event and for any handle
          type (with two exceptions; details to follow) is set to be the
          current (adjusted) mouse location.
       */
      const Vector3& pos() const { return pos_; }

      /*! \fn const Vector3& ViewerContext::dPos() const
          Returns the change in coordinate of the mouse since the last
          event, translated into whatever coordinate system was in effect
          when the current handle was created.

          This will (probably) be (0.0f, 0.0f, 0.0f) on push, and the change
          is movement since the last move otherwise.
       */
      const Vector3& dPos() const { return posDeltaSinceLast_; }

      /*! \fn const Vector3& ViewerContext::mousePosInHandleSpace() const
          Returns the coordinate associated with the event (e.g. the
          mouse pos when clicking on a handle or the pos of the selected
          handle when a key is pressed) translated into handle space
       */
      const Vector3& mousePosInHandleSpace() const { return mousePosInHandleSpace_; }

      /*! \fn double ViewerContext::x() const
          Returns the x coordinate of the mouse, translated into whatever
          coordinate system was in effect when the current handle was
          created.
       */
      float x() const { return pos_.x; } //!< Position of mouse in OpenGL

      /*! \fn double ViewerContext::y() const
          Returns the y coordinate of the mouse, translated into whatever
          coordinate system was in effect when the current handle was
          created.
       */
      float y() const { return pos_.y; } //!< Position of mouse in OpenGL

      /*! \fn double ViewerContext::z() const 
          Returns the z coordinate of the mouse, translated into whatever
          coordinate system was in effect when the current handle was
          created.
       */
      float z() const { return pos_.z; } //!< Position of mouse in OpenGL

      /*! \fn int ViewerContext::mouse_x()
          Returns the position of the mouse in pixels
       */
      static int mouse_x()        { return *mouse_x_; }

      /*! \fn int ViewerContext::mouse_y()
          Returns the position of the mouse in pixels, 0 is the top of the viewer.
       */
      static int mouse_y()        { return *mouse_y_; }

      /*! \fn int ViewerContext::wheel_dx()
          Returns the number of clicks the wheel moved horizontally.
       */
      static int wheel_dx()        { return *wheel_dx_; }

      /*! \fn int ViewerContext::wheel_dy()
          Returns the number of clicks the wheel moved vertically.
       */
      static int wheel_dy()        { return *wheel_dy_; }
      static float pressure()        { return *pressure_; }

      /*! \fn int ViewerContext::clicks()
          Returns 0 on a PUSH event for a single click, 1 for a double click,
          2 for triple, etc.
       */
      static int clicks()        { return *clicks_; }

      /*! \fn void ViewerContext::clicks(int i)
          Change the number of clicks, for fooling other knobs into acting
          different.
       */
      static void clicks(int i)    { *clicks_ = i; }

      /*! \fn bool ViewerContext::is_click()
          True if a short enough time has passed since the last PUSH that this
          should be considered a "click" rather than a "hold". Clicks pop up
          menus, holds select menu items.
       */
      static bool is_click()    { return *is_click_ != 0; }

      /*! \fn void ViewerContext::clear_is_click()
          Turn a click into a hold. If you do this before setting a menu then
          the click will pick the default item rather than making the menu
          appear.
       */
      static void clear_is_click()    { *is_click_ = 0; }

      /*! \fn int ViewerContext::button()
          For PUSH and RELEASE events, returns one of these values:
          - DD::Image::LeftButton (1)
          - DD::Image::MiddleButton (2)
          - DD::Image::RightButton (3)
          May return higher numbers for mice with more buttons. Returns garbage
          if the event is not PUSH or RELEASE.

          Also see pointer_type() and key().
       */
      static unsigned button()    { return *key_; }

      /*! \fn bool ViewerContext::state(int flags)
          True if any of the shift keys or buttons indicated by flags is
          held down. The symbols DD::Image::SHIFT, DD::Image::CTRL, and many
          others are defined in ViewerContext.h. Be careful not to confuse the symbols
          returned by key() for the shift keys with the bitmasks.
       */
      static bool state(unsigned v)    { return (*state_ & v) != 0; }
      static unsigned state()    { return *state_; }

      /*! \fn int ViewerContext::key()
          Returns which key the user hit for a KEY event. Letters return
          lower-case characters like 'x'. Other keys return symbols that
          are listed in ViewerContext.h.
       */
      static unsigned key()        { return *key_; }

      /*! \fn bool (*ViewerContext::is_down)(int key)
          Returns true if the key identified by \a key is currently held down.
       */
      static bool (* is_down)(unsigned key);

      /*! \fn const char* ViewerContext::key_text()
          String produced by this key. Mostly this lets you see the actual
          letter typed, for instance the X key produces 'x' for key() but
          "x", "X", or "\x18" depending on the shift and control keys.
       */
      static const char* key_text() { return *text_; }

      /*! \fn int ViewerContext::key_text_length()
          Length of string returned by key_text(), so that a key that produces
          "\0" can be distinguished from a key that produces nothing.
       */
      static unsigned key_text_length() { return *length_; }

      /*! \fn int ViewerContext::pointer_type()
          For PUSH and RELEASE events, returns one of these values:
          - DD::Image::eUnknown
          - DD::Image::ePen
          - DD::Image::eCursor (indicating the mouse)
          - DD::Image::eEraser
          See also button().
       */
      static int pointer_type() { return *pointer_type_; }
      static bool auto_repeat() { return *auto_repeat_; }

      /*! \fn int ViewerContext::event() const
          Return the event that caused Knob::draw_handles() or a handle
          callback to be done. This can be DRAW, DRAW_SHADOW, MOVE, PUSH,
          DRAG, RELEASE, KEY, KEYUP, WHEEL, or items from menus. See
          Knob::begin_handle() for more information. It may also be NO_EVENT
          when after menu() has been called.
       */
      DD::Image::ViewerEvent event() const { return event_; }
      void event(const DD::Image::ViewerEvent newEvent) { event_ = newEvent; }

      /*! True if you should draw solid opaque objects */
      bool draw_solid();

      /*! True if you should draw objects that may be transparent */
      bool draw_transparent();

      /*! True if you should draw solid objects that are can be seen behind objects with a stippled pattern. */
      bool draw_hidden_solid();

      /*! True if you should draw wireframes and points. */
      bool draw_lines();

      /*! True if you should draw wireframes and points that can be seen behind objects with a stippled pattern. */
      bool draw_hidden_lines();

      /*! True if you should draw overlays. */
      bool draw_overlay();

      /*! For back compatibility, same as draw_hidden_lines(). */
      bool draw_knobs();

      /*! True if you should draw lines that are not hit-detected. */
      bool draw_unpickable_lines();

      /*! True if you should draw lines that are not hit-detected. */
      bool draw_unpickable_hidden_lines();

      /*! True if draw is being called for hit detection. */
      bool hit_detect() { return event_ >= MOVE; }

      /*! True if OpenGL lights have been enabled */
      bool lighting() { return _lights.size() || what_to_draw_ & SHOW_LIGHTING; }

      /*! True if the 3D viewer grid should be drawn. */
      bool showViewerGrid() const { return _showViewerGrid; }
      /*! Sets whether the 3D viewer grid should be drawn. */
      void showViewerGrid(bool show) { _showViewerGrid = show; }
      /*! True if cameras should be shown in the 3D viewer. */
      bool showCameras() const { return _showCameras; }
      /*! Sets whether cameras should be shown in the 3D viewer. */
      void showCameras(const bool show) { _showCameras = show; }
      /*! True if lights should be shown in the 3D viewer. */
      bool showLighting() const { return _showLighting; }
      /*! Sets whether lights should be shown in the 3D viewer. */
      void showLighting(const bool show) { _showLighting = show; }
      /*! True if axis should be shown in the 3D viewer. */
      bool showAxis() const { return _showAxis; }
      /*! Sets whether axis should be shown in the 3D viewer. */
      void showAxis(const bool show) { _showAxis = show; }

      /*! A Knob handle callback can call this to make a popup menu. This
          is a little complex in order to allow a single menu item to modify
          all the selected handles, even if they belong to different knobs.

          If event() is PUSH then the menu is popped up and it waits for the
          user to select an item. You may want to call this only if button()
          or state() is correct, or change which menu is used depending on
          the buttons. When this returns event() will have either been changed
          to NO_EVENT or to one of the event numbers from an item in the menu.

          If event() is KEY then if this matches a menu item then the event
          number is changed to the number from that item. If there is no
          match then event() is not changed (so you could use the KEY for
          something else).

          If event() is NO_EVENT then an item was picked already from some
          menu. In this case, if this is the \e same menu, then the event()
          is changed to the same value. If this is a different menu then
          the event() is left at NO_EVENT. This allows different knobs and
          handles to share the effects of a menu.
       */
      void menu(Menu* menu);
      bool nudgeXY(double& x, double& y, double amount = 1.0) {
        return nudgeTransform(x, y, amount);
      }

      /** Apply the current key state to nudge X and Y values */
      bool nudgeTransform(double& x, double& y, double amount = 1.0, double *rotate = nullptr, double *scale = nullptr);

      /** Apply the current key state to nudge X, Y and Z values */
      bool nudgeXYZ(double&, double&, double&, double amount = 1.0);

      int what_to_draw() const { return what_to_draw_; }
      void what_to_draw(int n) { what_to_draw_ = n; }

      /*! Add a callback that will be used to draw something in the viewer.
          A build_handles() function calls this, the list is then called
          repeatedly to draw the viewer, and also for hit-detection.

          \a object is an arbitrary pointer that is passed to the callback.

          \a node is a pointer to a Node. This is used to set the colors
          and also as the node to be selected if the user clicks on any of
          the graphics drawn by the callback.

          \a type is an indicator of the type of handle we are adding. A
          geometry handle will always be drawn, regardless of whether which
          selection mode we're in; a selection handle will only be drawn
          when we're in the appropriate selection mode; and any other handle
          will be drawn when we're in node select mode (the Nuke default).

          This callback will be called with this exact same ViewerContext.
          The modelmatrix and connected() settings will be restored to
          how they are when this was called (actually the modelmatrix is
          also multiplied by the camera and zoom/pan currently being used
          by the viewer). The event() will be set to say why it is being
          called.
       */
      void add_draw_handle(DrawHandleCallbackFunc cb, void*, Node*, DrawHandleTypeMask drawHandleType = eDrawHandleNodeSelection);
#ifndef FN_NO_LEGACY_3D
      void addDrawableGeo(GeoInfo* geo, Node* node);

      const std::vector<DrawableGeo>& getDrawableGeoList() const;
#endif

      //! The current time for the viewer
      double time() const;

      //! The mode of the viewer. 0 for 2D, non-zero for various 3D modes
      /*! \fn ViewerMode ViewerContext::viewer_mode() const
          Return which type of viewer this is for.
       */
      ViewerMode viewer_mode() const { return viewer_mode_; }

      /*! \fn ViewerMode ViewerContext::transformSpace() const
          Return which type of transform space this is for.
          The default is object space.
       */
      TransformSpace transformSpace() const { return _transformSpace; }

      /*! The mode of the current transform, 0 means 2D, with 1unit = 1pixel. */
      int transform_mode() const { return transform_mode_; }

      /*! Some clever nodes can change the mode of their input. */
      void transform_mode(int v) { transform_mode_ = v; }

      //! How to draw objects:
      /*! Return how to draw 3d objects. \a object_style3d is the setting on this
         object's control panel. Current version returns the minimum of this
         and the viewer's style3d setting, and returns wireframe if this node
         is not connected to the viewer or if the panel is closed and the
         viewer control to make closed things wireframe is set.
       */
      Display3DMode display3d(Display3DMode mydisplay3d) const;

      /*! \fn unsigned ViewerContext::node_color() const
          Return the color of the current node. This can be used to color
          things in the viewer to match. The color is 8 bits per channel
          in the form 0xrrggbbaa. Currently the alpha is ignored.

          If event() is DRAW_SHADOW this returns a color to draw shadows.
       */
      unsigned node_color() const { return node_color_; }

      /*! \fn unsigned ViewerContext::fg_color() const
          Return the user-selected foreground color for the viewer. This
          can be used to draw line segments atop your object. We recommend
          you use node_color() if your object only consists of lines, however.

          If event() is DRAW_SHADOW this returns a color to draw shadows.
       */
      unsigned fg_color() const { return fg_color_; }

      /*! \fn unsigned ViewerContext::bg_color() const
          Return the color of the background of the viewer. This can be
          used to make sure you contrast with it.
       */
      unsigned bg_color() const { return bg_color_; }

      /*! \fn unsigned ViewerContext::selected_color() const
          Return the color of selected elements (e.g. handles) in the viewer.
       */
      unsigned selected_color() const { return selected_color_; }

      /*! \fn unsigned ViewerContext::focused_color() const
          Return the color of focused elements (e.g. handles) in the viewer.
       */
      unsigned focused_color() const { return focused_color_; }

      /*! \fn unsigned ViewerContext::soft_select_min_color() const
          Return the lowest value in the soft selection color interpolation
          of the selected geometry in the viewer.

          \see ViewerContext::soft_select_max_color()
       */
      unsigned soft_select_min_color() const { return soft_select_min_color_; }

      /*! \fn unsigned ViewerContext::soft_select_max_color() const
          Return the highest value in the soft selection color interpolation
          of the selected geometry in the viewer.

          \see ViewerContext::soft_select_min_color()
       */
      unsigned soft_select_max_color() const { return soft_select_max_color_; }

      /*! \fn unsigned ViewerContext::selected_3D_color() const
          Return the color of selected elements (e.g. handles) in the 3D viewer.
       */
      unsigned selected_3D_color() const { return selected_3D_color_; }

      void set_node_color(unsigned col) { node_color_ = col; }
      void set_selected_color(unsigned col) { selected_color_ = col; }
      void set_focused_color(unsigned col) { focused_color_ = col; }
      void set_soft_select_min_color(unsigned col) { soft_select_min_color_ = col; }
      void set_soft_select_max_color(unsigned col) { soft_select_max_color_ = col; }
      void set_selected_3D_color(unsigned col) { selected_3D_color_ = col; }

      /*! \fn float ViewerContext::transformHandleSize3D() const
          Return the size of 3D transform handles in the viewer.
       */
      float transformHandleSize3D() const { return transformHandleSize3D_; }

      float handleSize() const { return handleSize_; }
      float handlePickSize() const { return handlePickSize_; }
      float lineWidth() const { return lineWidth_; }

      /*! \fn float ViewerContext::devicePixelRatio() const
          Return the device pixel ratio for the viewer. This is
          the ration of screen pixels to UI pixels when on a scaled
          high-DPI display.
       */
      float devicePixelRatio() const { return _devicePixelRatio; }

      /*!
         In draw_handles this is the current OpenGl model matrix. If you change
         the OpenGL matrix you should also change this one so that add_handles
         remembers the correct values. You must restore the OpenGL matrix but
         you can leave this one changed and the calling code will restore it.

         In build_handles whatever value you put in here is remembered for
         each add_draw_handle(). You can multiply this to change the transform
         before calling other functions so they get different transforms.
         You must put the original value back before returning.
       */
      Matrix4 modelmatrix;

      const Matrix4& model_matrix() const { return modelmatrix; }
      const Matrix4& proj_matrix() const { return projmatrix; }
      const Matrix4& cam_matrix() const { return cameramatrix; }
      const Vector4& camera_pos() const { return _cameraPos; }

      // Figure out how big to draw things:

      /*! Same as zoom(0,0,0). If the viewer is not showing perspective
          the position does not matter, so it is safe to call this.
       */
      float zoom() const;

      /*! Return how big a 1-unit line segment starting at the given
          xyz will draw in pixels. This can be used for hit detection
          or to scale things so they draw a given size on the screen.

          This depends on the camera position and 2D zoom factor. These
          are not decided when build_handles() is called on objects. So
          you cannot call this at that time, instead you must call and
          use this in draw_handle(). As this function is somewhat slow
          (it does 3 matrix multiplications) you should call it once and
          put the result in a local variable.

          Warning: returns INFINITY for things at or behind the camera,
          or nearer than the near plane. This indicates that things are
          invisible.

          Note that as zoom is mostly intended for drawing handles at
          a useful size for the user, it works in UI pixels, not device
          pixels so you may need to scale by devicePixelRatio if using
          it for other purposes.
       */
      float zoom(float x, float y, float z = 0) const;

      /*! Same as icon_size(0,0,0) */
      float icon_size() const;

      /*! Return a scale factor to draw things that should be fixed-size
          on the screen. This takes into account zoom() and the user's
          preferences for icon size and power factor. The returned value
          is used as the length of the 3D axis handles and the diameter
          of the circle in the 2D transform manipulator, scale your
          icons appropriately to match.

          Warning: returns zero (0) for things at or behind the camera,
          or nearer than the near plane. This indicates that things are
          invisible.
       */
      float icon_size(float x, float y, float z = 0) const;

      /*! Returns the BBox xywh of the viewer in pixels, in screen space (i.e.
          not render/format space).
          To convert the return values from screen-space to render/format-space
          call viewareaFromScreenToRender()
       */
      const Box& viewport() const { return viewport_; }

      /*! Returns the BBox xywh of the visible viewer in pixels, this value is
          the same as viewport() but smaller as it removes the width/height of .
          any toolbars.
          As with viewport() the return value is in screen space (i.e. not
          render/format space). To convert from screen-space to render/format-space
          call viewareaFromScreenToRender()
       */
      const Box& visibleViewportArea() const { return visibleViewportArea_; }

      /*! Similar to viewport() except this returns the BBox xywh of the area of
          interest, where interest denotes an area outside of which rendered items
          can be ignored rather than the full visible area. i.e. will not be
          considered during some arbritrary calculation.
          For example during non-PUSH events the return value will be equal to the
          return value from visibleViewportArea() and during PUSH
          events it will be whatever region the hit-detect area is interested in
          (i.e. much smaller).
          The return value is the screen-space (i.e. not render/format space) BBox
          xywh of the area of interest. To convert from screen-space to
          render/format-space call viewAreaFromScreenToRender().
       */
      const Box& clipArea() const { return _clipArea; }

      /*! Same as the above but converts the clip area into format space
       */
      Box clipAreaFormatSpace() const;

      /*! Get the screen space bounds of a vector of points
       */
      void getScreenBoundingBox(const std::vector<DD::Image::Vector3>& points, DD::Image::Box& bboxOut) const;

      /*! Converts a render or viewport area from screen-space for render/format-space,
          e.g. those returned from viewport(),  visibleViewportArea() or renderArea().
          Note that this is comparitivly expensive operation.
       */
      Box viewAreaFromScreenToRender(const Box& area) const;

      AxisOp* camera() const { return camera_; }
      bool locked() const { return _camera_lockmode == CAMERA_LOCKMODE_LOCKED; }
      bool unlocked() const {return _camera_lockmode == CAMERA_LOCKMODE_UNLOCKED;}
      bool interactive() const {return _camera_lockmode == CAMERA_LOCKMODE_INTERACTIVE;}

      int pixmap_size_hint_w() const { return pixmap_size_hint_w_; }
      int pixmap_size_hint_h() const { return pixmap_size_hint_h_; }
      void pixmap_size_hint(int w, int h)
      {
        pixmap_size_hint_w_ = w;
        pixmap_size_hint_h_ = h;
      }
      bool non_default_texture_matrix() const { return ndtm; }
      bool non_default_texture_matrix(bool v) { return ndtm = v; }

      void center_viewer(float x, float y)
      {
        center_x = x;
        center_y = y;
        center_called = true;
      }

      bool draw_is_expensive() const
      {
        return _draw_is_expensive;
      }

      void set_draw_is_expensive(bool expensive)
      {
        _draw_is_expensive = expensive;
      }

      // build_handles functions:
      ViewerConnected connected() const { return connected_; } //!< Flags about what is between this & viewer
      bool reallyConnected() const { return _reallyConnected; }
      void connected(ViewerConnected v, bool reallyConnected = true) { connected_ = v; _reallyConnected = reallyConnected; }

      /*! build_handles() can call this to indicate the volume that surrounds
         anything that is being drawn. These are all merged together to
         produce auto-frame and near/far plane settings in the viewer.

         If \a active is true this indicates that the volume is selected
         and the "frame" shortcut in the Viewer should show it. If this is
         not true the volume is only used for setting the near/far planes
         of the Viewer's OpenGL camera.
       */
      void expand_bbox(bool active, const Box3&);

      /*! Add a flat rectangle in the z=0 plane to the volume that surrounds
         anything that is being drawn. This is most useful for 2D controls.
       */
      void expand_bbox(bool active, float x, float y, float r, float t);

      /*! Add a point to the volume of things being drawn. Enough slop
         is added so that the icon_size() surrounding this is visible.
       */
      void expand_bbox(bool active, float x, float y, float z = 0);
      Box3& bbox() { return bbox_; }
      Box3& active_bbox() { return active_bbox_; }

      /*! build_handles() can call this to set a camera that the user is
          likely to want to view through. The 'h' key in the viewer will
          then lock to one of these cameras. AxisOp is a base class
          common to both Camera and Light, you can put any AxisOp on this
          list, objects without lens information will use the normal
          zoom factor of the Viewer.

          If there are no other rules, the \e first camera added takes
          precedence. Thus a renderer should add the camera before it adds
          it's input operators.

          Returns true if this really is a new camera. Returns false if
          the camera already exists in the list, possibly from a different
          operator.
       */
      bool addCamera( AxisOp* newCamera );

      /*! build_handles() can call this to add a light to the OpenGL scene.
          Returns false if already in scene.
       */
      bool addLight( LightOp* newLight );
      const std::vector<AxisOp*>& cameras() const { return _cameras; }
      const std::vector<LightOp*>& lights() const { return _lights; }
      
      bool renderInProgress() const { return _renderInProgress; }
      
      bool playbackInProgress() const { return _playbackInProgress; }
      void setPlaybackInProgress(bool inProgress);

      // Used by XY_Knob, XYZ_Knob, and UV_Knob
      bool draw_animation_path() { return my_draw_animation_path; }
      bool draw_animation_path_done() { return my_draw_animation_path_done; }
      void draw_animation_path_done( bool done ) { my_draw_animation_path_done = done; }

      // Drag and drop handling: Call this in the DROP_CHECK handler to indicate you will accept a drop. The default is not to accept the drop.
      // For backward compatibility, retuning false form the handler for the DROP_CHECK event will cause the drop to be accepted.
      void accept_drop( bool accept ) { _accept_drop = accept; }
      bool accept_drop() { return _accept_drop; }

			// The zbuffer is stored as a viewport.w() * viewport.h() array of z depths.
      const float* getZBuffer() const;
      size_t getZBufferSize() const;

      RegularGrid* buildGrid();

      // Returns a hash of the last built geometry list
      Hash getDrawableGeometryHash() const;

      Box selection() const;
      SelectionMode3D getSelectionMode3D() const;
      Select3DAction getSelectionAction() const;
      bool getOcclusionTest() const;
      const SelectionFilters& getSelectionFilters3D() const;

      /// The width and height of the box you pass in here are expanded by 1
      /// pixel, so that it represents an inclusive range.
      void setSelection(const Box& newSelection);
      void setSelectionMode3D(SelectionMode3D mode);
      void setSelectionAction(Select3DAction action);
      void setOcclusionTest(bool enabled);
      void setSelectionFilters3D(const SelectionFilters& filters);

      // Returns a reference to the cached mesh topology for the current selection
      // and time. The topology is built and cached on demand.
      const ndk::MeshTopology& selectionMeshTopologyCache(const usg::MeshPrim& meshPrim, double time);

      // Clear the cached mesh topology if stage has changed or if viewer context is destroyed.
      void clearSelectionMeshTopologyCache();

      virtual float gamma() const;
      virtual float gain() const;

      void setMenuFirstCall(bool menuFirstCall)
      {
        _menuFirstCall = menuFirstCall;
      }

      bool menuFirstCall() const
      {
        return _menuFirstCall;
      }

      /*!
       * Return the context menu. May not be valid in all circumstances.
       */
      Menu* menu() const
      {
        return _menu;
      }

      inline bool glHandlesCached() const { return _glHandlesCached; }
      inline void glHandlesCached(const bool glHandlesCached) { _glHandlesCached = glHandlesCached; }

      static void getViewerWindowPanZoom(const ViewerWindowFormatContext& formatCtx, float& panx, float& pany, float& zoomx, float& zoomy);
      static DD::Image::Vector2 convertFromViewerToFormat(const ViewerWindowFormatContext& formatCtx, const DD::Image::Vector2& pos);
      
      // Viewer window format setting/getting
      ViewerWindowFormatContext& viewerWindowFormatContext()
      {
        return _viewerWindowFormatCtx;
      }

      // Are we currently doing a 3D render (either scanline render or OpenGl preview)
      bool isPerspectiveRender() const;

      //! Force a redraw of the viewer. This is for use by plugins which are doing progressive rendering and don't want to frezze the UI.
      void requestRedraw(bool request);
      bool redrawRequested() const;

      //! Returns a ray from the given screen position into the world
      fdk::Rayf getScreenToWorldRay(int screenX, int screenY) const;

      //! Find the closest intersections of a number of rays with the objects in the context
      std::vector<ndk::CollisionResult> intersectRays(const ndk::IntersectionContext& context, const std::vector<fdk::Rayf>& rays) const;

      //! Get the prim that was intersected from the collisionPrimitiveIdx member of a ndk::CollisionResult
      const usg::Prim& getIntersectionPrim(unsigned int collisionPrimitiveIdx) const;

      //! Internal use only.
      ViewerContextImpl* pImpl() const;

    private:
      // Viewer window format context information
      ViewerWindowFormatContext _viewerWindowFormatCtx;

      bool _draw_is_expensive;
      bool _menuFirstCall;
    };


    extern DDImage_API const ViewerContext* (*activeViewerContext)();
    extern DDImage_API void (*setSelectionMode3D)(SelectionMode3D newMode);
    extern DDImage_API void (*setSelection3D)(const DD::Image::GeoSelection& newSelection);
    extern DDImage_API const DD::Image::GeoSelection* (*getSelection3D)();
    extern DDImage_API void (*viewThroughCamera)(const CameraOp* camOp, CameraLockMode lockMode);

  } // namespace Image
} // namespace DD


#ifdef FN_OS_WINDOWS
# ifndef __INTEL_COMPILER
  #pragma warning(pop)
# endif // __INTEL_COMPILER
#endif // FN_OS_WINDOWS

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
