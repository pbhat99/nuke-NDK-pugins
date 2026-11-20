// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file Blink.h
 * \brief The main header file for the Blink API.
*/

#ifndef Blink_Library_Blink_h
#define Blink_Library_Blink_h

#include "BlinkBuild.h"
#include <string>
#include <vector>

#include "BlinkTypes.h"
#include "BlinkConstants.h"
#include "BlinkVersion.h"

namespace Blink
{

  typedef BlinkComputeDeviceID ComputeDeviceID;
  typedef BlinkCodegenFlags CodegenFlags;
  typedef BlinkDataType DataType;
  typedef BlinkRef Ref;
  typedef BlinkErrorCode ErrorCode;
  typedef BlinkErrorRef ErrorRef;
  typedef BlinkMemoryType MemoryType;

  typedef BlinkKernelType KernelType;
  typedef BlinkKernelGranularity KernelGranularity;
  typedef BlinkImageAccessType ImageAccessType;
  typedef BlinkImagePatternType ImagePatternType;
  typedef BlinkImageEdgeType ImageEdgeType;

  /// \brief Kernel Iteration Controller
  ///
  /// Describes the iteration controller used when executing the kernel.
  struct DeviceMemory: BlinkDeviceMemory
  {
    DeviceMemory() {}
    DeviceMemory(const DeviceMemory& memory);
    DeviceMemory(MemoryType type, void* memoryBasePtr, unsigned long long memoryBaseSize, unsigned long long offset);
  };

  class RefHolder
  {
  public:
    RefHolder();

    RefHolder(Ref ref, bool retain);
    RefHolder(const RefHolder& holder);

    RefHolder& operator=(const RefHolder& holder);
    void take(Ref ref);

    RefHolder& operator=(Ref ref);

    Ref ref() const;

    ~RefHolder();

  protected:
    Ref _ref;
  };


  /// Reference to a device used for Blink computation.
  class ComputeDevice: protected RefHolder
  {
  public:
    ComputeDevice(const ComputeDevice& dev);

    /// Fetch the currently selected CPU
    static ComputeDevice CurrentCPUDevice();

    /// Fetch the currently selected GPU
    static ComputeDevice CurrentGPUDevice();

    /// This function may change before release. Do not use.
    static ComputeDevice DeviceWithID(ComputeDeviceID id);

    /// Returns true if this device can be used
    bool available() const;

    /// Return the name of the device
    std::string name() const;

  private:
    ComputeDevice(Ref deviceRef);

    friend class ComputeDeviceBinder;
    friend class Image;
    friend class Kernel;
  };

  /// \brief Scoped lock for Blink device
  ///
  /// One of these should be constructed for a given device before attempting to perform any actions with it.
  /// The lock is not guarenteed to be re-entrant.
  class ComputeDeviceBinder: protected RefHolder
  {
  public:
    ComputeDeviceBinder(ComputeDevice& device);
  };

  /// \brief Device independent class holding source to a program.
  ///
  /// These should be constructed once to allow fast retrieval of kernels from the cache.
  class ProgramSource: protected RefHolder
  {
  public:
    ProgramSource(const char* source);
    ProgramSource(const std::string& source);

  private:
    void setSource(const char* source);
    friend class IKernel;
    friend class Kernel;
  };

  /// \brief Rectangle
  ///
  /// Rectangle covering range  (x1,y1) to (x2,y2)
  /// x range is x1 to x2-1
  /// y range is y1 to y2-1
  struct Rect: BlinkRect
  {

    Rect();

    Rect(const Rect& r);

    /// Constructor for creating a Rect
    Rect(int x1, int y1, int x2, int y2);

    int width() const;
    int height() const;
  };

  /// \brief Kernel Iteration Controller
  ///
  /// Describes the iteration controller used when executing the kernel.
  struct KernelIterationController: BlinkKernelIterationController
  {

    KernelIterationController(const KernelIterationController& iterationController);
    /// Constructor for creating a ImageInfo
    KernelIterationController(const Rect& bounds);
  };

  /// \brief Buffer layout description
  ///
  /// Describes the pixel, row and component stride in bytes of a buffer
  /// used with \ref Image::copyToBuffer() and \ref Image::copyFromBuffer()
  struct BufferDesc: BlinkBufferDesc
  {

    BufferDesc();

    /// Constructor for creating a BufferDesc
    BufferDesc(unsigned int pixelStepBytes,
               unsigned int rowStepBytes,
               unsigned int componentStepBytes);
  };

  /// \brief Pixel information
  ///
  /// Describes the format of a pixel inside an image.
  struct PixelInfo: BlinkPixelInfo
  {

    PixelInfo();
    /// Constructor for creating a PixelInfo
    PixelInfo(int nComponents, DataType dataType);
  };

  /// \brief Image information
  ///
  /// Describes the bounds and pixel format of an image.
  struct ImageInfo: BlinkImageInfo
  {

    ImageInfo();

    ImageInfo(const ImageInfo& info);

    /// Constructor for creating a ImageInfo
    ImageInfo(const Rect& bounds, const PixelInfo& pixelInfo);

    Rect bounds() const;
  };

  struct ImageAccessDesc: BlinkImageAccessDesc
  {
    ImageAccessDesc(void) {}
    ImageAccessDesc(BlinkImagePatternType type);
    ImageAccessDesc(BlinkAxisType axis, int min, int max);
    ImageAccessDesc(Rect Range);
  };

  /// Reference to a Blink image
  class Image: protected RefHolder
  {
  public:
    /// Image ref not pointing to any data
    Image();

    /// Copy reference to source image. Both images will point to the same data.
    Image(const Image& src);


    Image(const ImageInfo& info, ComputeDevice& device);


    Image(const DeviceMemory& memory, const ImageInfo& info, const BufferDesc& desc, ComputeDevice& device);

    Image& operator=(const Image& src);

    /// \brief Return a reference to the image with the same data on the specified device performing a copy if necessary
    ///
    /// If the image is already on the specified device then return a reference to the image.
    /// Otherwise copy the image to the device and return a reference to the copy.
    Image distributeTo(const ComputeDevice& dev);

    /// \brief Return a reference to a new image with the same layout on the specified device
    ///
    /// An Image returned from this call will be compatible for a copyFrom.
    Image makeLike(const ComputeDevice& dev);

    /// Copies the data from src on any device into this image. The formats of the images must match.
    void copyFrom(const Image& src);

    /// Fetch info for image
    ImageInfo info() const;

    /// Clear reference to image.
    void clear();

    /// Returns true if there is no underlying data
    bool isEmpty() const;

    /// Copy the contents of the image to the provided buffer
    void copyToBuffer(void* buffer, const BufferDesc& bufferDesc) const;

    /// Copy provided buffer to the image
    void copyFromBuffer(const void* buffer, const BufferDesc& bufferDesc) const;

    Image(Ref ref);

  private:
    friend class Kernel;
    //friend Image BlinkImageForRef(Ref);
  };

  /// \brief Kernel information
  ///
  /// Describes characteristics of a kernel.
  struct KernelInfo: BlinkKernelInfo
  {

    KernelInfo();

    KernelInfo(const KernelInfo& info);

    KernelInfo(const std::string& name, KernelType type,
               KernelGranularity granularity);

    std::string getName() const;
    KernelType getType() const;
    KernelGranularity getGranularity() const;
  };

  /// \brief Static image information
  ///
  /// Describes the characteristics of an image as declared in a Blink Kernel.
  struct KernelImageInfo: BlinkKernelImageInfo
  {

    KernelImageInfo();

    KernelImageInfo(const KernelImageInfo& info);

    KernelImageInfo(const std::string& name, ImageAccessType accessType,
                    ImagePatternType patternType, ImageEdgeType edgeType);

    std::string getName() const;
    ImageAccessType getAccessType() const;
    ImagePatternType getPatternType() const;
    ImageEdgeType getEdgeType() const;
  };

  /// \brief Parameter information
  ///
  ///
  struct KernelParameterInfo: BlinkKernelParameterInfo
  {

    KernelParameterInfo();

    KernelParameterInfo(const KernelParameterInfo& info);

    KernelParameterInfo(const std::string& name, DataType type, unsigned nComponents,
                        unsigned nElements);

    unsigned getNumParameters();

    std::string getName() const;
    DataType getDataType() const;
    unsigned getNComponents() const;
    unsigned getNElements() const;
  };

  /// Base class for all Kernels.
  class IKernel: protected RefHolder
  {

  public:
    /// \brief Introspect a Kernel
    IKernel(const ProgramSource& programSource);

    unsigned getNumImages() const;
    unsigned getNumParameters() const;

    KernelInfo getKernelInfo() const;

    std::vector<KernelImageInfo> getImagesInfo() const;

    std::vector<KernelParameterInfo> getParametersInfo() const;

  protected:
    IKernel();
  };

  /// Kernel for a specific device.
  class Kernel: public IKernel
  {

  public:
    /// \brief Fetch a kernel
    /// Fetch a kernel for the program to execute on the specified device with images with the same layouts as those provided.
    /// Generally it is best to call this every time a kernel is used rather than storing the result. Blink will cache the results of any compilation performed
    /// and can quickly retrieve the result using information stored in the program source.
    Kernel(const ProgramSource& programSource, ComputeDevice& dev, const std::vector<Image>& images, CodegenFlags flags = kBlinkCodegenDefault);

    void setParamValue(const std::string& name, bool value);
    void setParamValue(const std::string& name, float value);
    void setParamValue(const std::string& name, int value);

    /// For setting multiple booleans.
    void setParamValue(const std::string& name, const char* value, unsigned int count);
    void setParamValue(const std::string& name, const float* value, unsigned int count);
    void setParamValue(const std::string& name, const int* value, unsigned int count);

    void getParamValue(const std::string& name, char* value, unsigned int count);
    void getParamValue(const std::string& name, float* value, unsigned int count);
    void getParamValue(const std::string& name, int* value, unsigned int count);

    std::vector<Blink::ImageAccessDesc> getInputRanges(void);

    //Iterate the kernel using the last image as the iteration bounds
    void iterate();

    //Iterate over the provided bounds
    void iterate(const KernelIterationController& controller);
  };


  /// \brief Base exception for Blink API
  ///
  /// Exception all other Blink exceptions inherit from. Can be thrown when an unknown error occurs.
  class Exception: protected RefHolder
  {
  public:
    /// \brief Get description of error
    std::string userMessage() const;

  protected:
    Exception(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };

  /// \brief Exception thrown when device runs out of memory
  ///
  /// Exception thrown when a device runs out of memory. This may be thrown from a number of places since devices may be using deferred memory allocation.
  class OutOfMemoryException: public Exception
  {
  protected:
    OutOfMemoryException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };


  /// \brief Exception thrown when kernel fails to run
  class ComputeException: public Exception
  {
  protected:
    ComputeException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };

  /// \brief Exception thrown when parameters for image construction are invalid
  class ImageSpecException: public Exception
  {
  protected:
    ImageSpecException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };


  /// \brief Exception thrown when attempt to build kernel fails
  class CompilationException: public Exception
  {
  protected:
    CompilationException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };

  /// \brief Exception thrown when attempt to parse kernel fails.
  ///
  /// Exception when kernel parsing fails. This usually results in the ability to identify the location and cause of the exception.
  class ParseException: public CompilationException
  {
  public:
    /// \brief Line number of first error
    ///
    /// Line number of first parse error. -1 if not known.
    int lineNumber() const;

    /// \brief Error generated if line number is valid
    std::string parseError() const;

  protected:
    ParseException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };

  /// \brief Exception thrown when there is an error on a device
  class DeviceException: public Exception
  {
  protected:
    DeviceException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };

  /// \brief Exception throw when parameter is set/get with incorrect parameters
  ///
  /// Exception when attempt to set/get parameter fails. Common causes are invalid name, incorrect type, or incorrect number of components.
  class ParameterException: public Exception
  {
  protected:
    ParameterException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };

  /// \brief Exception thrown when images are incompatible such as in copy operation.
  class IncompatibleImagesException: public Exception
  {
  protected:
    IncompatibleImagesException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };

  class InvalidAccessDescException: public Exception
  {
  protected:
    InvalidAccessDescException(Ref ref);
    friend void TranslateBlinkError(ErrorCode errorCode, ErrorRef errorRef);
  };

} // namespace Blink
#include "BlinkImpl.h"
#endif
