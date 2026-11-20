#ifndef DDImage_FileOp_H
#define DDImage_FileOp_H

#include "DDImage/Knobs.h"

namespace DD {
  namespace Image {

    /*
     * This is a file format handler interface.  Writers implement this directly,
     * but because of the way Readers work (multi-readers per Node), if they want
     * this they have to make a new class.
     *
     * Subclasses should implement knobs() just as an Op would.
     */
    class DDImage_API FileHandler
    {
    public:
      virtual void knobs(Knob_Callback cb)
      {
      }

      virtual int knob_changed(Knob* k)
      {
        return 0;
      }

      virtual int knob_change_finished(Knob* k, bool changeByUser)
      {
        return 0;
      }

      /** Extra knobs can be used to define knobs that get added after all other knobs.
          This can be used, for example, if you want format-specific tab knobs.
      */
      virtual void extraKnobs(Knob_Callback cb)
      {
      }

      /** Reevaluate dynamic knobs, those which react to loaded settings.
          This might include setting default values based on file parameters.
      */
      virtual void initDynamicKnobs() {}

      /** Get a description of the file format */
      virtual const char* help() { return nullptr; }

      virtual ~FileHandler() { }
    };

    /**
     * This class is a mixin class for Read/Write/ReadGeo/WriteGeo etc.
     * It allows file-format-specific knobs to be substituted in to a
     * particular place more easily
     */
    class DDImage_API FileOp
    {
      /// ID for the knob producer. Knobs are assumed to be unchanged if this is unchagned
      const void* _replaced_knobs_id;

      /// ID for the knob producer. Knobs are assumed to be unchanged if this is unchagned
      const void* _extra_knobs_id;

      /// number of replaced knobs
      int _replaced_knobs;

      /// number of extra knobs
      int _extra_knobs;

      /// last knob added in Read/Write pannel, used to replace knobs
      Knob* _placemarker;

      /// last knob added in Read/Write pannel, used to add extra knobs
      Knob* _extraplacemarker;

      /// the version number for the "reload" button
      int _version;

    protected:
      //! whether or not the reader is in error (set by internalError)
      bool _readerError;

      //! the last reader error (set by internalError)
      std::string _readerErrorString;

    public:

      static const char* StripPrefix(const char* filename, std::string* prefix = nullptr);
      static const char* StripSuffix(const char* filename, std::string* suffix = nullptr);

      //! parse fname and figure out what fileformat it should be in, and the remaining
      //! filename to use.  If the filename is prefixed with "format:" it will strip that
      //! out and use it as the format, putting the remainder of the filename in name, otherwise
      //! it will place the extension in format and the full filename in name.
      //!
      //! Returns true if it found both components, false otherwise.
      static bool GetFormatAndName(const char* fname, std::string& format, std::string& name);

      /** constructor */
      FileOp()
      {
        _replaced_knobs_id = nullptr;
        _replaced_knobs = 0;
        _placemarker = nullptr;
        _extra_knobs_id = nullptr;
        _extra_knobs = 0;
        _extraplacemarker = nullptr;
        _readerError = false;
        _version = 0;
      }

      /** descructor */
      virtual ~FileOp() { }

      virtual Op* op() = 0;

      /** add a reload button */
      void Reload_Button(Knob_Callback f);

      int version() const
      {
        return _version;
      }

      void setVersion(int newVersion)
      {
        _version = newVersion;
      }

      /*!
        Normally calls Op::error(), but prefixes "filename: " to the
        message. This also switches it so that it produces a checkerboard
        pattern, so the Reader is not called anymore.
      */
      void internalError(const char* fmt, ...);
      void internalErrorV(const char* fmt,  va_list args );

      /** get the number of replaced knobs */
      int replaced_knobs() const
      {
        return _replaced_knobs;
      }

      /** get the number of extra knobs */
      int extra_knobs() const
      {
        return _extra_knobs;
      }

      /** the placemarker: the knob immediately before the first file-format-specific knobs */
      Knob* placemarker() const
      {
        return _placemarker;
      }

      /** the placemarker: the knob immediately before the extra file-format-specific knobs */
      Knob* extraPlacemarker() const
      {
        return _extraplacemarker;
      }

      virtual const char* getFilename() const = 0;

      /** get the file extension of the file format */
      virtual const char* file_type() const = 0;

      /** return the file format handler for the current knob settings */
      virtual FileHandler* handler() const = 0;

      /** insert in the file-format specific file handle knobs. */
      virtual void filehandler_knobs(Knob_Callback f);

      /** call file-format specific file handle knob callbacks. */
      virtual int filehandler_knob_changed(Knob* k);

      /** call file-format specific file handle knob callbacks. */
      virtual int filehandler_knob_change_finished(Knob* k, bool changeByUser);

      /** insert in the extra file-format specific file handle knobs. */
      virtual void filehandler_extra_knobs(Knob_Callback f);

      /** insert in the file-format specific knobs. subclasses should call this in their knobs() implementation */
      virtual void replaceable_knobs(Knob_Callback f);

      /** insert in the file-format specific knobs. subclasses should call this in their knobs() implementation */
      virtual void extra_knobs(Knob_Callback f);

      /*!
         \brief Change the set of knobs in response to the user changing the filename
         or the file type pulldown.
       */
      /** If the id is different, replace the knobs by calling the above. Subclasses
          should call this in e.g. knob_changed() after the handler has changed */
      void replace_knobs(const void* id, const char* fileExt);

      /** return the id sent to replace_knobs() most recently. 0 is the initial value. **/
      const void* replaced_knobs_id() const { return _replaced_knobs_id; }

      /** If the id is different, replace the knobs by calling the above. Subclasses
          should call this in e.g. knob_changed() after the handler has changed */
      void add_extra_knobs(const void* id, const char* fileExt);

      /** return the id sent to replace_knobs() most recently. 0 is the initial value. **/
      const void* extra_knobs_id() const { return _extra_knobs_id; }

      /*!
         \brief Return an array of names of file types known about.
         This is generated by searching the pluigin directories for any
         plugins called "*Writer.plugin_ext". The first location in the
         array is a blank string for 'unknown' and the array is NULL-terminated.
       */
      /** Helper function for implementation of Write::file_types() and WriteGeo::file_types()
          and similar functions.  Suffix is a user-specific suffix (e.g. "WriterGeo"),
          altsuffix is an (optional) backwards-compatability suffix to look for ("Writer")

          prefixDenyList is a pointer to a NULL-terminated list of strings to
          not include (these are compared to the start of the found string, so "foo" would
          block foobar", for example.) It can be NULL.

          extras is a pointer to a NULL-terminated list of strings to
          force to be included in the list, even if no matching plugins were found. It can be NULL.

          obsoleteIDs is an (optional) pointer to a NULL-terminated list of strings to extend
          with obsoleteIDs. This allows file types to have synonyms.
          The format is:
          id\t\t\tobsoleteID1,obsoleteID2,...
          For example, to make ffmpeg load as mov64:
          mov64\t\t\tffmpeg
      */
      static const char* const* file_types(const char* suffix,
                                           const char* altsuffix,
                                           const char** prefixDenylist,
                                           const char** extras,
                                           const char** obsoleteIDs = nullptr);

      static void AddAlias(const char* from, const char* to);

      /**
       * look up a file extension alias.  If there is no alias, returns the core extension.
       */
      static const char* LookupAlias(const char* from);
    };
 }
}

#endif
