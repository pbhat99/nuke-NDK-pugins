// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DD_IMAGE_AUTHORING_H
#define DD_IMAGE_AUTHORING_H

#include "DDImage/DDImage_API.h"

#include <string>
#include <vector>

namespace DD { namespace Image {

  class Knob;
  class OutputContext;
  class Authoring;
  class AuthoringSettings;

  //! Get the default authoring settings - all modes, default mode nothing
  DDImage_API const AuthoringSettings& AuthoringDefaults();

  /// name of the authoring command
  static const char* kAuthoringModeCmd = "setAuthorMode";

  //! \class DD::Image::Authoring
  //!
  //! Provides the interface for authoring. Authoring is a USD concept, which
  //! means writing to USD layers.
  //! So, this is designed for interacting with the USD stage of the 3D
  //! system.
  class DDImage_API Authoring {
  public:
    // Note: When extending this enum, make sure to update the private lookup
    // table and the container holding all the values.
    //! Possible authoring modes.
    enum class Mode {
      Set,            //!< Set or create the value in the USD layer.
      RestoreDefault, //!< Restore the default value in the USD layer.
      NoChange,       //!< Do not author the value in the USD layer.
    };
    //! Create an authoring control for a knob using specific settings
    //! @param knob
    //! @param settings
    Authoring(Knob* knob, const AuthoringSettings& settings);
    //! Don't allow copy constructor however, knob pointer shouldn't be copied
    Authoring(const Authoring&) = delete;
    ~Authoring() = default;

    //! Copy a set of settings
    Authoring& operator=(const AuthoringSettings& settings);

    //! Returns the authoring mode type from the name.
    static Mode getMode(const std::string& name);

    //! Returns the name of the authoring mode from the type.
    static std::string getModeName(Mode mode);

    //! Gets the choice index of the mode (as in a dropdown)
    //!
    //! @param mode to get the choice index for
    //! @return index of the mode or -1 if there isn't one
    int getIndex(Mode mode) const;

    //! Gets the mode from the choice index (as in a dropdown)
    //! @param index the choice number to get the mode for
    //! @return the mode it corresponds to for this knob
    Mode getMode(int index) const;

    //! Returns the available authoring mode types if authoring is supported,
    //! otherwise an empty list.
    std::vector<Mode> getModes() const;

    //! Sets the available authoring modes. Without setting this, authoring
    //! is disabled. This will update the set mode and the default mode if the
    //! currently set mode isn't in the list of modes.
    void setModes(const std::vector<Mode>& modes);

    //! Sets the mode that will be selected by default - won't set values if
    //! the default doesn't exist in the modes
    void setDefaultMode(Mode defaultMode);

    //! Gets the mode that will be selected by default
    Mode getDefaultMode() const;

    //! Returns the available authoring mode names if authoring is supported,
    //! otherwise an empty list.
    std::vector<std::string> getModeNames() const;

    //! Returns the authoring mode name.
    std::string getModeName() const;

    //! Returns the authoring mode type.
    Mode getMode() const;

    //! Sets the authoring mode from the type.
    //! Returns true if successfully set, otherwise false, e.g. if the
    //! mode is not a supported authoring mode by the Node (Op).
    bool setMode(Mode mode);

    //! Sets the authoring mode from the integer.
    //! Returns true if successfully set, otherwise false, e.g. if the
    //! integer cannot be converted to a valid authoring mode.
    bool setMode(int mode);

    //! Sets the authoring mode from the name.
    //! Returns true if successfully set, otherwise false, e.g. if the
    //! string is not a valid authoring mode name.
    bool setMode(const std::string& name);

  private:
    // The currently selected authoring mode. This can be any of the
    // supported authoring modes for the Op.
    Mode _mode = Mode::NoChange;
    Mode _defaultMode = Mode::NoChange;

    // The list of supported authoring modes. Different Ops can decide to
    // support a different set of authoring modes. Some may support none,
    // some all, and some may support a subset of all the possible authoring
    // modes. The possible authoring modes that an OP can select from is not
    // standardized by USD, but a choice Nuke has made. They are the options
    // listed in the corresponding enum.
    std::vector<Mode> _modes;

    // The list of supported authoring mode names. This is offered for
    // convenience so that it can be used through Python, etc.
    std::vector<std::string> _modeNames;

    // Pointer back to the knob that owns it
    Knob* _knob;

    // Lookup table to convert between enum values and string identifiers back
    // and forth.
    static const std::vector<std::pair<Mode, std::string>> kModeNameLookupTable;
  };

  //! Class for handling authoring settings, such as defaults,
  //! currently set mode, and list of possible modes
  class DDImage_API AuthoringSettings {
  public:
    AuthoringSettings(const Authoring* other);
    AuthoringSettings(
      Authoring::Mode mode,
      const std::vector<Authoring::Mode>& modes,
      Authoring::Mode defaultMode
    );

    //! Returns true if the settings are valid
    bool isValid() const;

    Authoring::Mode mode{Authoring::Mode::NoChange};
    Authoring::Mode defaultMode{Authoring::Mode::NoChange};
    std::vector<Authoring::Mode> modes{};
  };
}}

#endif
