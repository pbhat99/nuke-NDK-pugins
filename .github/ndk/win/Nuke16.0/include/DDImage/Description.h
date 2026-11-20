// Description.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Description_h
#define DDImage_Description_h

#include <vector>

#include "ddImageVersion.h"

extern "C" {
struct SecurityEnvelope;
}

class Node;

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::License

        Object describing licensing restrictions for a Description
        object. If the licensing fails, the Description (and thus the
        plugin) will not work.

        The \a system_id field must be set to
        DD::Image::License::this_system_id or you will get a license
        failure error when loading the plugin, and the program will act as
        though the Description containing this license does not exist. If
        you use any of the more advanced features then failure of them
        will also cause the same result. For better security, you should
        also check your License instance and Description at various points
        in the code, to make sure some cracker has not altered them. Also
        check any local data you have showing that your copy protection
        test really worked.

        There are three types of security features available to
        plugins from Nuke:

        1. The first is simply the unique ID in this_system_id that any
        plugin or script has access to. It provides no security other than
        giving the developer something unique to key their own licensing
        code to. Your test can indicate success by copying this_system_id
        into the \a system_id field, or zero or any other value to
        indicate failure.

        2. The second is a check for a "feature" line added to Nuke's
        FLEXlm license file. The \a name and \a version, if not null,
        indicate a "feature" line to be looked up in the FLEXlm license
        file. If the feature is not there with at least the given version
        number, you will get a license failure error. The Foundry can
        generate such feature lines for you.

        3. The third type is the \a security structure. Currently this is
        unsupported, but is reserved for future use.

        If DDImage is not being used by Nuke, than any Description with a
        License where either the \a system_id or \a name are not zero will
        fail as though the Description does not exist.

     */
    struct License
    {

      /*! This is a 32-bit value that is unique for each customer
          site. In the case of a node-locked license, each workstation has a
          different ID. In the case of a license server, the license server
          has a unique ID. Any plugin can examine this value, and tcl code
          can also get it with $this_system_id. Attempts to alter it are
          caught by Nuke and cause all plugins to stop working.

          Simple protection is achieved by checking this_system_id against a
          list of known systems and setting \a system_id in a License to the
          first matching one, or zero if none. To populate this list you can
          ask your client to report the result of the Nuke tcl variable
          "$this_system_id" and then compile it in. Or you can do some trick
          whereby an unknown system_id does some web query or asks the user
          a question and if the results are ok it adds it to some internal
          encrypted list. If your plugin is supposed to work at exactly one
          site you can simply set license->system_id to the one for that
          site, and the plugin will not work anywhere else.

          This value is zero if DDImage is being used by some program other
          than Nuke. If you want your plugin to work in such cases you must
          set the license pointer to zero if this value is zero.  
       */
      DDImage_API static unsigned this_system_id;
      unsigned system_id; /*!< Will not work unless this == this_system_id */
      const char* name; /*!< If not null, run license check for this */
      const char* version; /*!< Version number for license check */
      SecurityEnvelope* security; /*!< If not null, this structure is filled with secret stuff. */
    };

    /*!
       Describes an object provided by a plugin. Each different class of
       object has it's own subclass of Description. The base class mostly provides
       licensing check information. Writers of plugins can take advantage of
       this to prevent their plugin from working on unauthorized systems.
     */
    class DDImage_API Description
    {
    public:
      typedef Node* (*NodeBuilder)(void*);

      typedef std::vector<Description*> DescriptionList;

    private:
      /*! Non-inline portion of ctor() */
      void ctor2(void (* f)(Description*));
      void ctor2(void (* f)(Description*), NodeBuilder nodeBuilder);
    protected:
      /*! Subclasses should call this as part of their constructor. These
         constructors should be written in-line so that the compile-time
         information is inserted into the description.

         The function \a f will be called if license is null or if the
         license check works.
       */
      void ctor(void (* f)(Description*))
      {
        compiled = __DATE__ " for " kDDImageVersion;
        ctor2(f);
      }
      void ctor(void (* f)(Description*), NodeBuilder nodeBuilder)
      {
        compiled = __DATE__ " for " kDDImageVersion;
        ctor2(f, nodeBuilder);
      }
    public:
      const char* compiled; /*!< Date and DD_IMAGE_VERSION_LONG this was compiled for */
      const char* plugin; /*!< Set to the plugin filename */
      License* license; /*!< If non-null, license check is run */
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of Description.h
