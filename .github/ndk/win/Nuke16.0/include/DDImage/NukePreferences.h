#ifndef DD_Image_NukePreferences_h
#define DD_Image_NukePreferences_h

#include "DDImage_API.h"

#include <string>

// Provides functions to access Nuke's preferences. This functionality is only
// available when using DD::Image from inside the Nuke GUI; outside of it, the
// functions will return hardcoded values.
namespace DD {
  namespace Image {

    //
    // Enumeration preferences types.
    //

    enum ViewerWorldCoordinate { 
      eViewerWorldCoordinateYUp = 0,  
      eViewerWorldCoordinateZUp,

      eViewerWorldCoordinateCount
    };

    //
    // UInt32 preference names.
    //

    // The viewer's background colour in 2D mode.
    extern DDImage_API const char* kViewerBGColor;
    // The viewer's foreground colour in 2D mode.
    extern DDImage_API const char* kViewerFGColor;
    // The viewer's background colour in 3D mode.
    extern DDImage_API const char* kViewerBGColor3D;
    // The viewer's foreground colour in 3D mode.
    extern DDImage_API const char* kViewerFGColor3D;
    // The colour used for 3D selections in the viewer.
    extern DDImage_API const char* kViewerSelColor3D;
    // The colour used for marginally selected selections in the viewer (part of soft selection).
    extern DDImage_API const char* kViewerMinSelColor3D;

    // The colour used to highlight things in the UI.
    extern DDImage_API const char* kUIHighlightColor;
    // The colour used for text inside a highlighted item.
    extern DDImage_API const char* kUIHighlightedTextColor;
    // The base font size for text in the UI.
    extern DDImage_API const char* kUIFontSize;


    //
    // Enumeration preference names.
    //

    // The viewer world coordinate system up direction
    extern DDImage_API const char* kViewerWorldCoordinateSystem;

    //
    // Float preference names.
    //

    // The diameter in pixels of point handles in the 2D viewer.
    extern DDImage_API const char* kViewerHandleSize;
    // The diameter in pixels of the pickable area for point handles in the 2D viewer.
    extern DDImage_API const char* kViewerHandlePickSize;
    // The diameter in pixels of point handles in the 3D viewer.
    extern DDImage_API const char* kViewerHandleSize3D;
    // The size in pixels of transform handles in the 3D viewer.
    extern DDImage_API const char* kViewerTransformHandleSize3D;
    // The diameter in pixels of the pickable area for point handles in the 3D viewer.
    extern DDImage_API const char* kViewerHandlePickSize3D;

    // the amount of channels we can create before a warning occurs
    extern DDImage_API const char* kChannelWarningThreshold;

    //
    // Functions
    //

    // Get the current value of a bool-valued preference. If the name you
    // provide is for a preference which has a different type of value, the
    // result may be garbage.
    //
    // This function will only return real values when running from the Nuke
    // GUI. At other times it will always return false.
    // This function is not thread safe and should only be called from the 
    // main thread
    extern DDImage_API bool (*GetBoolPreference)  (const char* name);

    // Get the current value of an unsigned int-valued preference. This includes
    // most colours. If the name you provide is for a preference which has a
    // different type of value, the result may be garbage.
    //
    // This function will only return real values when running from the Nuke
    // GUI. At other times it will always return 0.
    // This function is not thread safe and should only be called from the 
    // main thread
    extern DDImage_API unsigned int (*GetUIntPreference)(const char* name);

    // Get the current value of an int-valued preference. If the name you
    // provide is for a preference which has a different type of value, the
    // result may be garbage.
    //
    // This function will only return real values when running from the Nuke
    // GUI. At other times it will always return 0.
    // This function is not thread safe and should only be called from the 
    // main thread
    extern DDImage_API int (*GetIntPreference) (const char* name);

    // Get the current value of a float-valued preference. If the name you
    // provide is for a preference which has a different type of value, the
    // result may be garbage.
    //
    // This function will only return real values when running from the Nuke
    // GUI. At other times it will always return 0.0.
    // This function is not thread safe and should only be called from the 
    // main thread
    extern DDImage_API float (*GetFloatPreference) (const char* name);

    // Get the current value of a double-valued preference. If the name you
    // provide is for a preference which has a different type of value, the
    // result may be garbage.
    //
    // This function will only return real values when running from the Nuke
    // GUI. At other times it will always return 0.0.
    // This function is not thread safe and should only be called from the 
    // main thread
    extern DDImage_API double (*GetDoublePreference)(const char* name);

    // Get the current value of a string-valued preference. If the name you
    // provide is for a preference which has a different type of value, the
    // result will be an empty string.
    //
    // This function will only return real values when running from the Nuke
    // GUI. At other times it will always return an empty string.
    // This function is not thread safe and should only be called from the 
    // main thread
    extern DDImage_API std::string (*GetStringPreference)(const char* name);

    // Get the current localisation path specified in the preferences.
    extern DDImage_API std::string (*GetLocalisationPath)();

  } // namespace Image
} // namespace DD

#endif // DD_Image_NukePreferences_h
