// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DD_IMAGE_AUTHORING_H
#define DD_IMAGE_AUTHORING_H

#include "DDImage/DDImage_API.h"

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

namespace DD { namespace Image {
  class Authoring;
  class AuthoringSettings;

  //! Default authoring settings: Set & NoChange, default mode NoChange.
  DDImage_API const AuthoringSettings& AuthoringSetNoChangeDefaults();

  //! Default authoring settings: all modes, default mode NoChange.
  DDImage_API const AuthoringSettings& AuthoringAllModesDefaults();

  /// name of the authoring command
  static const char* kAuthoringModeCmd = "setAuthorMode";

  //-------------------------------------------------------------------------------

  //! \class DD::Image::Authoring
  //!
  //! Provides the interface for authoring. Authoring is a USD concept, which
  //! means writing to USD layers.
  //! So, this is designed for interacting with the USD stage of the 3D
  //! system.
  class DDImage_API Authoring
  {
  public:
    using ModeMask = uint8_t;

    // Note: When extending this enum, make sure to update mask values
    // and check Knob interface.
    //! Possible authoring modes.
    enum Mode : uint8_t {
      SetValue,       //!< Set or create the value in the USD layer.
      NoChange,       //!< Do not author the value in the USD layer.
      DefaultValue,   //!< Restore the default value in the USD layer.
      //
      Invalid,        //!< Special value for disable/failure cases
    };

    static constexpr ModeMask  SetValueBit() { return 0x01; }
    static constexpr ModeMask  NoChangeBit() { return 0x02; }
    static constexpr ModeMask  DefaultValueBit() { return 0x04; }

    static constexpr ModeMask  NoModes() { return 0x00; }
    static constexpr ModeMask  AllModes() { return 0x07; }

    //------------------------------------------------------

    //! Default ctor makes a default mode authoring control, setting the mode
    //! the same as default mode.
    Authoring(Mode mode = Mode::NoChange, Mode defaultMode = Mode::NoChange);

    //! Create an authoring control for a knob using specific settings and modes.
    Authoring(const AuthoringSettings& settings, Mode mode, Mode defaultMode);

    //! Create an authoring control for a knob using specific settings
    Authoring(const AuthoringSettings& settings);

    virtual ~Authoring() = default;

    //! Copy a set of settings
    Authoring& operator=(const AuthoringSettings& settings);

    //! Is authoring control enabled?
    bool enabled() const { return (_enabledModes != NoModes()); }

    //! Disable the authoring control (remove all enabled modes.)
    //! This leaves the current and default modes as is.
    void disable() { _enabledModes = NoModes(); }

    //! Enable the authoring control by setting the enabled mode mask.
    void enable(ModeMask modes = AllModes()) { _enabledModes = modes; }

    //! Return the mask of enabled modes.
    ModeMask enabledModes() const { return _enabledModes; }

    //------------------------------------------------------

    //! Returns the available authoring mode names.
    static const std::vector<std::string>& GetModeNames();

    //! Returns the name of the authoring mode from the type.
    static const std::string& GetModeName(Mode mode);

    //! Returns the corresponding mask bit for the mode.
    static ModeMask GetModeBit(Mode mode);

    //! Returns the authoring mode type from the mode name string.
    static Mode GetModeEnumFromName(const char* name);
    static Mode GetModeEnumFromName(const std::string& name) { return GetModeEnumFromName(name.c_str()); }

    //------------------------------------------------------

    //! Returns a list of enabled authoring modes, otherwise an empty list.
    std::vector<Mode> getModes() const;

    //! Return a list of enabled authoring mode names, otherwise an empty list.
    std::vector<const char*> getModeNames() const;

    //! Sets the available authoring modes. Without setting this, authoring
    //! is disabled. This will update the set mode and the default mode if the
    //! currently set mode isn't in the list of modes.
    void setModes(const std::vector<Mode>& modes);

    //! Return true if mode is in the enabled set.
    bool modeEnabled(Mode mode) const;

    //! Enable a mode.
    void enableMode(Mode mode) { _enabledModes |= GetModeBit(mode); }

    //! Disable a mode.
    void disableMode(Mode mode) { _enabledModes &= ~GetModeBit(mode); }

    //! Get the choice index of the mode (as in a dropdown,) or -1 if not in the
    //! enabled list.
    int getModeIndex(Mode mode) const;

    //! Get the choice index from a named mode, or -1 if not in the enabled list.
    int getModeIndex(const char* name) const { return getModeIndex(GetModeEnumFromName(name)); }

    //! Get the enabled mode from the choice index (as in a dropdown,) or Mode::Invalid
    //! if not in the enabled list.
    Mode getModeEnumFromIndex(int index) const;

    //------------------------------------------------------

    //! Sets the mode that will be used by default - won't set values if
    //! the default doesn't exist in the enabled modes.
    void setDefaultMode(Mode defaultMode);

    //! Gets the authoring mode that will be used by default.
    Mode getDefaultMode() const { return _defaultMode; }

    //! Returns the current authoring mode.
    Mode getMode() const { return _mode; }

    //! Gets the choice index of the current authoring mode (as in a dropdown.)
    int getModeIndex() const { return getModeIndex(_mode); }

    //! Returns the current authoring mode's name.
    const std::string& getModeName() const { return GetModeName(_mode); }

    //! Sets the authoring mode, returning false if mode is not in enabled list.
    bool setMode(Mode mode);

    //! Sets the authoring mode from a choice index, returning false if not in enabled list.
    bool setModeFromIndex(int index) { return setMode(getModeEnumFromIndex(index)); }

    //! Sets the mode from a mode name, returning false if not in enabled list.
    bool setMode(const std::string& name) { return setMode(GetModeEnumFromName(name)); }

    //------------------------------------------------------

    friend DDImage_API std::ostream& operator << (std::ostream& o, const Authoring&);


  protected:

    Mode     _mode;           //!< Currently selected mode
    Mode     _defaultMode;    //!< Initial mode (the default)
    ModeMask _enabledModes;   //!< Which modes to show as authoring options (in menus, etc)
  };

#if 0
  //! Print out to a stream.
  std::ostream& operator << (std::ostream& o, const Authoring&);
#endif


  //-------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------


  //! Class for handling authoring settings, such as defaults,
  //! currently set mode, and list of possible modes
  class DDImage_API AuthoringSettings {
  public:
    //! Default ctor sets mode to NoChange.
    AuthoringSettings(Authoring::Mode mode = Authoring::Mode::NoChange);

    //! Copy settings from another Authoring instance.
    AuthoringSettings(const Authoring* other);

    AuthoringSettings(
      Authoring::Mode mode,
      Authoring::Mode defaultMode,
      const std::vector<Authoring::Mode>& modes
    );

    //! Returns true if the settings are valid
    bool isValid() const;

    Authoring::Mode mode{Authoring::Mode::NoChange};
    Authoring::Mode defaultMode{Authoring::Mode::NoChange};
    std::vector<Authoring::Mode> modes{};
  };


}}

#endif
