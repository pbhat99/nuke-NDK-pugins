// OutputContext.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef OutputContext_H
#define OutputContext_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Hash.h"
#include "DDImage/ProxyContext.h"
#include "math.h"
#include "DDImage/Box.h"
#include "DDImage/Deprecation.h"

#include "ndk/graph/GsvTypes.h"

#include <Build/fnBuild.h>
#include <string>
#include <vector>

class Viewer_Window;
class Node;

namespace DD
{
  namespace Image
  {
    using GraphScopeId = ndk::GraphScopeId;
    using GraphScopePtr = ndk::GraphScopePtr;
    class Hash;
    class Op;


    /*! \class DD::Image::OutputContext

       This is a block of data that is passed *up* the Op tree. The context
       modifies the values that are stored by knobs. An Op can further use
       information from it (such as the frame number) to modify it's
       behavior, by implementing Op::setOutputContext(). And an Op can
       modify the context that is used by each input, by implementing
       Op::inputContext().

     */
    class DDImage_API OutputContext : public ProxyContext
    {
    public:

      /*!
        Preferences which can be used to influence how files are read by Readers.
        eScanlineAlways - upstream Reader should always read frames line by line.
        ePreferScanline - prefer line-by-line reads, but allow other nodes upstream to override this preference.
        ePreferAllLines - prefer to read the whole of the requested area at once, but allow other nodes upstream to override this preference.
      */
      enum LineReadPreference
      {
        eScanlineAlways,
        ePreferScanline,
        ePreferAllLines
      };

      enum class ContextType
      {
        eNone,
        eFallbackUi
      };

    public:
      //! Default ctor sets frame=0.0, view=0, lineReadPreference=ePreferScanline and creates an empty GraphScope.
      OutputContext();

      //! Copy ctor with frame/view variant options. Replaces the frame and optionally the view if not -1.
      OutputContext(const OutputContext& src,
                    double               frame,
                    int                  view = -1);

      //! Copy ctor with view variant assignment. Convenient for changing the view without affecting frame or GraphScope.
      OutputContext(const OutputContext& src,
                    int                  view);

      mFnDeprecatedInNuke17(
        "OutputContext(double frame, int view) has been deprecated, use OutputContext(double frame, int view, GraphScopePtr scope) or copy construct from an existing OutputContext instead.")
      OutputContext(double frame,
                    int    view = -1);

      OutputContext(double frame,
                    int    view,
                    const GraphScopePtr graphScope);

      //! Construct a fully defined OutputContext (minus ProxyContext) which must include a Node GraphScope id.
      OutputContext(ndk::GraphScopeId graphScopeCtx, double frame, int view, ContextType type = ContextType::eNone);

      //!
      static const OutputContext& defaultContext();


      double frame() const { return _frame; }
      void   setFrame(double v) { _frame = v; }

      int    view() const { return _view; }
      //! Assigned view values <= 0 are clamped to view index 1 (default).
      void   setView(int view);
      mFnDeprecatedInNuke14("view has been deprecated, use setView instead.")
      void   view(int v);

      //! Sets a GraphScope to the OutputContext.
      void   setGraphScope(const GraphScopePtr scope);

      //! Returns the GraphScope held by the OutputContext. The GraphScopePtr can
      //! be an empty shared pointer, which means that the GraphScope is
      //! nonexisting.
      GraphScopePtr graphScope() const;

      //! Returns the unique ID for the scope held by the OutputContext. Same as
      //! calling graphScope()->id().
      const GraphScopeId& graphScopeId() const;

      //! Retrieves a GraphScope from the repository. This is equivalent to:
      //! OutputContext::graphScopeRepository()->get(graphScopeId).
      static GraphScopePtr graphScope(const GraphScopeId& graphScopeId);

      /*!
        By default readers read frames line by line (ePreferScanline). Some
        formats support reading the entire frame in one go.
        Setting the lineReadPreference can be used to switch between the two
        modes for these formats.
      */
      void setLineReadPreference(LineReadPreference linePreference); //ndk14 deprecate
      LineReadPreference lineReadPreference() const { return _lineReadPreference; } //ndk14 deprecate

      //------------------------------------------------------------------------

      /*!
         Add the contents of this to the hash, thus producing a different hash for any
         different setting of OutputContext.
       */
      void append(Hash&) const;

      bool operator == (const OutputContext& b) const;
      bool operator != (const OutputContext& b) const { return !(*this == b); }
      bool operator <  (const OutputContext& b) const;
      bool operator >  (const OutputContext& b) const { return !(*this < b); }


    public:
      //----------------------------------------------------------
      // Views

      static const std::string& viewname(int v);
      static const std::string& viewshort(int v);

      /**
       * how many views exist.  View 0 is always an extra view called 'default'.  If there
       * is one view, this is will be view 1.
       */
      static int viewcount();

      /**
       * whether or not there are more than one actual view (not counting the hidden 'default' view)
       */
      static bool MultiView();

      const std::string& viewname() const { return viewname(_view); }

      static int lookup_view(const std::string& viewname);

      // s is a newline-separated list of view names.
      static void set_viewnames(const std::string& s);

      static void (* create_view)(const std::string& s);

      //! Returns the GraphScopeRepository in use by Nuke.
      static ndk::GraphScopeRepositoryPtr (* graphScopeRepository)();

      // Merge with lookup_view
      //! If a view exists with name \a viewName on \a op's root node then that
      //! view's index is returned, otherwise -1 is returned.
      static int (*viewIndex)(const std::string& viewName, Op* op);

      // Merge with create_view
      //! Creates a new view with name \a viewName and \a color on \a op's root
      //! node.
      static void (*createView)(const std::string& viewName, const std::string& color, Op* op);

      // Merge with viewname
      //! If a view exists at \a index on \a op's root node then that view's
      //! name is returned, otherwise an empty string is returned.
      static const std::string& (*viewName)(int index, Op* op);

      // Merge with viewshort
      //! If a view exists at \a index on \a op's root node then that view's
      //! short name is returned, otherwise an empty string is returned.
      static std::string (*viewShortName)(int index, Op* op);

      // Merge with set_viewnames
      //! Sets the \a op's root node to have view's named \a viewNames.
      static void (*setViewNames)(const std::vector<std::string>& viewNames, Op* op);

      // Merge with viewcount
      //! Returns the number of views on the \a op's root node.
      static int (*viewCount)(Op* op);

      // Merge with MultiView
      //! Returns true if \a op's root node has more than one view.
      static bool (*hasMultipleViews)(Op* op);


      //----------------------------------------------------------
      // ProxyContext

      // Forwarded proxy methods.
      const Format* to_proxy_rel(const Format& from, Format& to, const OutputContext& rel) const { return ProxyContext::to_proxy_rel(from, to, rel); }
      bool          sameProxy(const DD::Image::OutputContext& b) const { return ProxyContext::isEqualTo(b); }

      //----------------------------------------------------------
      // For internal use only
      ContextType type() const { return _type; }
      void setType( const ContextType value ) { _type = value; }

    private:
      double             _frame;      //!< Frame id for which the current context applies, may not match what is being shown in the Viewer
      int                _view;       //!< View id for which the current context applies.
      GraphScopeId       _graphScopeId; //!< GraphScope hash
      ContextType        _type = ContextType::eNone;
      LineReadPreference _lineReadPreference; //ndk14 deprecate

      friend DDImage_API std::ostream& operator << (std::ostream&, const OutputContext&);

    };
  }
}

#endif // OutputContext_H

