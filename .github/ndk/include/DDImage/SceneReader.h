#ifndef DDImage_SceneReader_h
#define DDImage_SceneReader_h

#include <tuple>
#include <unordered_set>

#include "DDImage/Knobs.h"

namespace DD {
  namespace Image {

    class SceneReaderPlugin;
    static const std::string kFrameRateKnobName = "frame_rate";
    static const std::string kUseFrameRateKnobName = "use_frame_rate";
    static const std::string kReadFromFileKnobName = "read_from_file";
    static const std::string kLoadHintKnobName = "load_hint";
    static const std::string kFileKnobName = "file";
    static const std::string kReloadKnobName = "reload";


    class DDImage_API SceneReader
    {
    public:
      double getFrameRate() const { return _frameRate; }
      void setFrameRate(double f) { _frameRate = f; }

      float getStartFrame() const { return _startFrame; }
      void setStartFrame(float f) { _startFrame = f; }

      float getEndFrame() const { return _endFrame; }
      void setEndFrame(float f) { _endFrame = f; }

      bool getUseFrameRate() const { return _useFrameRate; }
      void setUseFrameRate(bool u) { _useFrameRate = u; }

      bool getFileData() const{ return _fileData; }

      bool getReadFromFile() const{ return _readFromFile; }

      bool getLoadHint() const{ return _pLoadHintKnob ? _pLoadHintKnob->get_value() : _loadHint; }

      /// the file that is being read
      std::string getFilename() const;

      Knob* getFrameRateKnob() { return _pFrameRateKnob; }
      Knob* getFilenameKnob() { return _pFilenameKnob; }
      Knob* getReloadKnob() { return _pReloadKnob; }

      /// knobs that are enabled only if read_from_file is checked
      std::unordered_set<Knob*> getFileDependentKnobs();

      //! This will validate the scene reader
      void _validate(bool for_real);

      //! This will add the filename to the error message
      virtual void error(const char* fmt, ...);

    protected:
      static const std::string kNoneName;
      static const std::string kSuppressConfirmationDialogKnobName;

      //! This is the class constructor.
      SceneReader();
      //! This is the class destructor.
      virtual ~SceneReader();

      /*! This will extract the filename extension
       */
      static const char* getFileExtension(const char* pFilename);

      //! This specifies the knobs.
      void knobs(Knob_Callback f);

      /*! Set knobs to only be enabled if read_from_file is checked
       * \param knobs     The knobs to set as file dependent
       */
      void addFileDependentKnobs(const std::unordered_set<DD::Image::Knob*>& knobs);

      /*! Remove knobs from the list of knobs that are disabled/enabled if read_from_file is checked or unchecked
       * \param knobs     The knobs to remove from the file dependent knobs list
       */
      void removeFileDependentKnobs(const std::unordered_set<DD::Image::Knob*>& knobs);

      //! This will enable/disable all registered file dependent knobs
      void enableFileDependentKnobs(bool on);

      /*! Create link knobs for the file-related knobs "read_from_file", "file" and "reload"

       This could be called e.g. at the start of the knobs() function of the op so the user
       can open a file without needing to switch to another tab. The knobs are created
       with the names "read_from_file_link", "file_link" and "reload_link".
       The resulting pointers to the latter two are stored in _pFilenameLinkKnob and _pReloadLinkKnob.
       The original knobs are created in SceneReader::knobs().
       */
      void linkFileKnobs(Knob_Callback f);

      /*! Create additional link knobs for the file-related knobs

       This creates further link knobs, e.g. if the op has additional tabs
       to allow the user to choose a file to load from there.
       The knobs will be created using the names "read_from_file", "file"
       and "reload" with the suffix argument appended.
       The created knobs are returned in the tuple in the order "read_from_file", "file" and "reload".
       */
      std::tuple<Knob*, Knob*, Knob*> additionalLinkFileKnobs(const std::string& suffix, Knob_Callback f);

      /*! Enable/check file-dependent knobs, trigger reload (asking for confirmation if required)

        If the respective plugin for the file type is created successfully its
        SceneReaderPlugin::setAttributes() method is called.
       */
      int knob_changed(Knob* k);

      virtual bool allowFileReload() = 0;
      /// check if the file is readable.
      /// \return true if the file can be read
      virtual bool isReadableFile(const char* pFilename);

      Knob* _pReloadKnob = nullptr;
      Knob* _pFrameRateKnob = nullptr;
      Knob* _pUseFrameRateKnob = nullptr;
      Knob* _pFilenameKnob = nullptr;
      ///    this is set to false when data is loaded in through a browser
      Knob* _pLoadHintKnob = nullptr;
      Knob* _pVersionKnob = nullptr;
      Knob* _pReadFromFileKnob = nullptr;
      Knob* _pSuppressConfirmationDialogKnob = nullptr;
      Knob* _pFilenameLinkKnob = nullptr;
      Knob* _pReloadLinkKnob = nullptr;

      float _startFrame = 1.0f;
      float _endFrame = 1.0f;
      bool _fileData = false;
      bool _readFromFile = false;
      bool _invalidFileError = false;
      bool  _loadHint{true};

      int _version = 0;
      double _frameRate;
      bool _useFrameRate;
      bool _suppressConfirmationDialog = false;

    private:
      void initReaderPlugin(const std::string& fileName);

      const char* _pFilename = nullptr;
      bool _validatePlugin = false;

      virtual const char* Class() const = 0;

      int _knobsCreated{0};
      std::unordered_set<DD::Image::Knob*> _fileDependentKnobs;
      std::unique_ptr<SceneReaderPlugin> _reader;
    };
  }
}

#endif
