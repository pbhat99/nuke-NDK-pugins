// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file BlinkImpl.h
 * \brief Implementation of the functions in Blink.h.
*/

#ifndef Blink_Library_BlinkImpl_h
#define Blink_Library_BlinkImpl_h

#include "Blink/Blink.h"
#include <memory>
#include "BlinkConstants.h"
#include "BlinkC.h"

namespace Blink
{

  //Error handling
  inline void
#ifdef BLINK_ERROR_TRANSLATE
  #define TranslateBlinkError BLINK_ERROR_TRANSLATE
  TranslateBlinkErrorSwitch(BlinkErrorCode errorCode)
#else
  TranslateBlinkError(BlinkErrorCode errorCode, BlinkErrorRef errorRef)
#endif
  {
#ifndef BLINK_ERROR_THROW
  #define BLINK_ERROR_THROW(type) throw Blink::type(errorRef);
#endif
    if (errorCode) {
      switch (errorCode) {
        case BLINK_ERROR_ERROR:
          BLINK_ERROR_THROW(Exception)
        case BLINK_ERROR_OUT_OF_MEMORY:
          BLINK_ERROR_THROW(OutOfMemoryException)
        case BLINK_ERROR_RUNNING_KERNEL:
          BLINK_ERROR_THROW(ComputeException)
        case BLINK_ERROR_PARSING_KERNEL:
          BLINK_ERROR_THROW(ParseException)
        case BLINK_ERROR_COMPILING_KERNEL:
          BLINK_ERROR_THROW(CompilationException)
        case BLINK_ERROR_INVALID_IMAGE_SPEC:
          BLINK_ERROR_THROW(ImageSpecException)
        case BLINK_ERROR_DEVICE:
          BLINK_ERROR_THROW(DeviceException)
        case BLINK_ERROR_PARAMETER:
          BLINK_ERROR_THROW(ParameterException)
        case BLINK_ERROR_INCOMPATIBLE_IMAGES:
          BLINK_ERROR_THROW(IncompatibleImagesException)
        default:
          BLINK_ERROR_THROW(Exception)
      }
    }
  }

  inline RefHolder::RefHolder()
    : _ref(nullptr)
  {}

  inline RefHolder::RefHolder(BlinkRef ref, bool retain)
  {
    if (retain) {
      _ref = Blink_Retain(ref);
    }
    else {
      _ref = ref;
    }
  }

  inline RefHolder::RefHolder(const RefHolder& holder)
  {
    _ref = Blink_Retain(holder._ref);
  }

  inline RefHolder& RefHolder::operator=(const RefHolder& holder)
  {
    BlinkRef newRef = Blink_Retain(holder._ref);
    Blink_Release(_ref);
    _ref = newRef;
    return *this;
  }

  inline void RefHolder::take(BlinkRef ref)
  {
    //TODO should probably assert ref!=_ref
    Blink_Release(_ref);
    _ref = ref;
  }

  inline RefHolder& RefHolder::operator=(BlinkRef ref)
  {
    BlinkRef newRef = Blink_Retain(ref);
    Blink_Release(_ref);
    _ref = newRef;
    return *this;
  }

  inline BlinkRef RefHolder::ref() const
  {
    return _ref;
  }

  inline RefHolder::~RefHolder()
  {
    Blink_Release(_ref);
  }

  //ComputeDevice


  inline ComputeDevice ComputeDevice::CurrentCPUDevice()
  {
    BlinkRef cpuRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode error = Blink_Device_CurrentCPUDevice(&cpuRef, &errorDesc);
    TranslateBlinkError(error, errorDesc);
    return ComputeDevice(cpuRef);
  }

  /// Fetch the currently selected GPU
  inline ComputeDevice ComputeDevice::CurrentGPUDevice()
  {
    BlinkRef cpuRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode error = Blink_Device_CurrentGPUDevice(&cpuRef, &errorDesc);
    TranslateBlinkError(error, errorDesc);
    return ComputeDevice(cpuRef);
  }


  inline ComputeDevice ComputeDevice::DeviceWithID(BlinkComputeDeviceID id)
  {
    BlinkRef cpuRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode error = Blink_Device_DeviceWithID(&cpuRef, id, &errorDesc);
    TranslateBlinkError(error, errorDesc);
    return ComputeDevice(cpuRef);
  }

  /// Returns true if this device can be used
  inline bool ComputeDevice::available() const
  {
    char available;
    BlinkErrorRef errorDesc;
    BlinkErrorCode error = Blink_Device_Available(ref(), &available, &errorDesc);
    TranslateBlinkError(error, errorDesc);
    return available ? true : false;
  }

  /// Return the name of the device
  inline std::string ComputeDevice::name() const
  {
    int nameDataSize;
    BlinkErrorRef errorDesc;
    BlinkErrorCode error = Blink_Device_GetProperty(ref(),
                                                    BLINK_PROPERTY_NAME,
                                                    nullptr,
                                                    &nameDataSize,
                                                    &errorDesc);
    TranslateBlinkError(error, errorDesc);

    char* nameStr = new char[nameDataSize];
    error = Blink_Device_GetProperty(ref(),
                                     BLINK_PROPERTY_NAME,
                                     nameStr,
                                     &nameDataSize,
                                     &errorDesc);
    TranslateBlinkError(error, errorDesc);
    std::string result(nameStr);
    delete[] nameStr;
    return result;
  }

  inline ComputeDevice::ComputeDevice(const ComputeDevice& dev)
    : RefHolder(dev)
  {
  }

  inline ComputeDevice::ComputeDevice(BlinkRef deviceRef)
    : RefHolder(deviceRef, false)
  {
  }

  //ComputeDeviceBinder

  inline ComputeDeviceBinder::ComputeDeviceBinder(ComputeDevice& device)
  {
    BlinkRef deviceBinder = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Device_Bind(&deviceBinder, device.ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(deviceBinder);
  }

  //DeviceMemory
  inline DeviceMemory::DeviceMemory(const DeviceMemory& memory)
    : BlinkDeviceMemory(memory) {}

  inline DeviceMemory::DeviceMemory(MemoryType type, void* memoryBasePtr, unsigned long long memoryBaseSize, unsigned long long offset)
  {
    this->type = type;
    this->memoryBasePtr = memoryBasePtr;
    this->memoryBaseSize = memoryBaseSize;
    this->offset = offset;
  }

  //Rect
  inline Rect::Rect()
  {
    x1 = 0;
    y1 = 0;
    x2 = 0;
    y2 = 0;
  }

  inline Rect::Rect(const Rect& r)
    : BlinkRect(r) {}

  inline Rect::Rect(int x1, int y1, int x2, int y2)
  {
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
  }

  inline int Rect::width() const
  {
    return x2 - x1;
  }

  inline int Rect::height() const
  {
    return y2 - y1;
  }

  //KernelIterationController
  inline KernelIterationController::KernelIterationController(const KernelIterationController& iterationController)
    : BlinkKernelIterationController(iterationController) {}

  inline KernelIterationController::KernelIterationController(const Rect& bounds)
  {
    this->bounds = bounds;
  }

  //BufferDesc
  inline BufferDesc::BufferDesc() {}

  /// Constructor for creating a BufferDesc
  inline BufferDesc::BufferDesc(unsigned int pixelStepBytes,
                                unsigned int rowStepBytes,
                                unsigned int componentStepBytes)
  {
    this->pixelStepBytes = pixelStepBytes;
    this->rowStepBytes = rowStepBytes;
    this->componentStepBytes = componentStepBytes;
  }

  //ImageInfo
  inline ImageInfo::ImageInfo() {}

  inline ImageInfo::ImageInfo(const ImageInfo& info)
    : BlinkImageInfo(info) {}

  inline ImageInfo::ImageInfo(const Rect& bounds, const PixelInfo& pixelInfo)
  {
    this->BlinkImageInfo::bounds = bounds;
    this->pixelInfo = pixelInfo;
  }
  inline Rect ImageInfo::bounds() const { return Rect(BlinkImageInfo::bounds.x1, BlinkImageInfo::bounds.y1, BlinkImageInfo::bounds.x2, BlinkImageInfo::bounds.y2); }

  //PixelInfo
  inline PixelInfo::PixelInfo() {}

  inline PixelInfo::PixelInfo(int nComponents, DataType dataType)
  {
    this->nComponents = nComponents;
    this->dataType = dataType;
  }

  //Image
  /// Image ref not pointing to any data
  inline Image::Image()
  {
    BlinkRef imageRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_CreateEmptyImage(&imageRef, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(imageRef);
  }

  inline Image::Image(const Blink::DeviceMemory& memory, const Blink::ImageInfo& info, const Blink::BufferDesc& desc, ComputeDevice& device)
  {
    BlinkRef imageRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_ImageForDeviceMemory(&imageRef, &memory, &info, &desc, device.ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(imageRef);
  }

  /// Copy reference to source image. Both images will point to the same data.
  inline Image::Image(const Image& src)
  {
    BlinkRef imageRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_CopyRef(&imageRef, src.ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(imageRef);
  }

  inline Image::Image(BlinkRef ref)
    : RefHolder(ref, false)
  {
  }

  inline Image::Image(const Blink::ImageInfo& info, ComputeDevice& device)
  {
    BlinkRef imageRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_CreateImage(&imageRef, &info, device.ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(imageRef);
  }

  inline Image& Image::operator=(const Image& src)
  {
    BlinkRef imageRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_CopyRef(&imageRef, src.ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(imageRef);
    return *this;
  }

  /// \brief Return a reference to the image with the same data on the specified device performing a copy if necessary
  ///
  /// If the image is already on the specified device then return a reference to the image.
  /// Otherwise copy the image to the device and return a reference to the copy.
  inline Image Image::distributeTo(const ComputeDevice& dev)
  {
    BlinkRef imageRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_DistributeTo(&imageRef, ref(), dev.ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    return Image(imageRef);
  }

  inline Image Image::makeLike(const ComputeDevice& dev)
  {
    BlinkRef imageRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_MakeLike(&imageRef, ref(), dev.ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    return Image(imageRef);
  }

  /// Copies the data from src on any device into this image. The formats of the images must match.
  inline void Image::copyFrom(const Image& src)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_CopyFrom(ref(), src.ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  /// Fetch info for image
  inline Blink::ImageInfo Image::info() const
  {
    BlinkErrorRef errorDesc;
    BlinkImageInfo imageInfo;
    BlinkErrorCode errorCode = Blink_Image_Info(ref(), &imageInfo, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    Blink::ImageInfo res(Blink::Rect(imageInfo.bounds.x1, imageInfo.bounds.y1, imageInfo.bounds.x2, imageInfo.bounds.y2), Blink::PixelInfo(imageInfo.pixelInfo.nComponents, imageInfo.pixelInfo.dataType));
    return res;
  }

  /// Clear reference to image.
  inline void Image::clear()
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_Clear(ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline bool Image::isEmpty() const
  {
    BlinkErrorRef errorDesc;
    char empty;
    BlinkErrorCode errorCode = Blink_Image_IsEmpty(ref(), &empty, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    return empty ? true : false;
  }

  /// Copy the contents of the image to the provided buffer
  inline void Image::copyToBuffer(void* buffer, const Blink::BufferDesc& bufferDesc) const
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_CopyToBuffer(ref(), buffer, &bufferDesc, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  /// Copy provided buffer to the image
  inline void Image::copyFromBuffer(const void* buffer, const Blink::BufferDesc& bufferDesc) const
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Image_CopyFromBuffer(ref(), buffer, &bufferDesc, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }


  //ProgramSource

  inline ProgramSource::ProgramSource(const char* source)
  {
    setSource(source);
  }

  inline ProgramSource::ProgramSource(const std::string& source)
  {
    setSource(source.c_str());
  }

  inline void ProgramSource::setSource(const char* source)
  {
    BlinkRef programSourceRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_ProgramSource_Make(&programSourceRef, source, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(programSourceRef);
  }

  // KernelInfo
  inline KernelInfo::KernelInfo()
  {
  }

  inline KernelInfo::KernelInfo(const KernelInfo& info)
  {
    this->kernelName = info.kernelName;
    this->kernelType = info.kernelType;
    this->kernelGranularity = info.kernelGranularity;
  }

  inline KernelInfo::KernelInfo(const std::string& name, KernelType type,
                                KernelGranularity granularity)
  {
    this->kernelName = name.c_str();
    this->kernelType = type;
    this->kernelGranularity = granularity;
  }

  inline std::string KernelInfo::getName() const
  {
    return std::string(this->kernelName);
  }

  inline KernelType KernelInfo::getType() const
  {
    return this->kernelType;
  }

  inline BlinkKernelGranularity KernelInfo::getGranularity() const
  {
    return this->kernelGranularity;
  }

  // KernelImageInfo
  inline KernelImageInfo::KernelImageInfo()
  {
  }

  inline KernelImageInfo::KernelImageInfo(const KernelImageInfo& info)
  {
    this->name = info.name;
    this->accessType = info.accessType;
    this->patternType = info.patternType;
    this->edgeType = info.edgeType;
  }

  inline KernelImageInfo::KernelImageInfo(const std::string& name,
                                          ImageAccessType accessType,
                                          ImagePatternType patternType,
                                          ImageEdgeType edgeType)
  {
    this->name = name.c_str();
    this->accessType = accessType;
    this->patternType = patternType;
    this->edgeType = edgeType;
  }

  inline std::string KernelImageInfo::getName() const
  {
    return std::string(this->name);
  }

  inline ImageAccessType KernelImageInfo::getAccessType() const
  {
    return this->accessType;
  }

  inline ImagePatternType KernelImageInfo::getPatternType() const
  {
    return this->patternType;
  }

  inline ImageEdgeType KernelImageInfo::getEdgeType() const
  {
    return this->edgeType;
  }

  inline KernelParameterInfo::KernelParameterInfo()
  {
  }

  inline KernelParameterInfo::KernelParameterInfo(const KernelParameterInfo& info)
  {
    this->name = info.name;
    this->type = info.type;
    this->nComponents = info.nComponents;
    this->nElements = info.nElements;
  }

  inline KernelParameterInfo::KernelParameterInfo(const std::string& name, DataType type,
                                                  unsigned nComponents, unsigned nElements)
  {
    this->name = name.c_str();
    this->type = type;
    this->nComponents = nComponents;
    this->nElements = nElements;
  }

  inline std::string KernelParameterInfo::getName() const
  {
    return std::string(this->name);
  }

  inline DataType KernelParameterInfo::getDataType() const
  {
    return this->type;
  }

  inline unsigned KernelParameterInfo::getNComponents() const
  {
    return this->nComponents;
  }

  inline unsigned KernelParameterInfo::getNElements() const
  {
    return this->nElements;
  }
  // IKernel
  inline IKernel::IKernel()
  {
  }

  inline IKernel::IKernel(const ProgramSource& programSource)
  {
    BlinkRef iKernelRef = nullptr;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_IKernel_Make(&iKernelRef, programSource.ref(),
                                                  &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(iKernelRef);
  }

  inline unsigned IKernel::getNumImages() const
  {
    BlinkErrorRef errorDesc;
    unsigned nImgs = 0;
    BlinkErrorCode errorCode = Blink_IKernel_GetNumImages(ref(), &nImgs, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    return nImgs;
  }

  inline unsigned IKernel::getNumParameters() const
  {
    BlinkErrorRef errorDesc;
    unsigned nParams = 0;
    BlinkErrorCode errorCode = Blink_IKernel_GetNumParameters(ref(), &nParams, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    return nParams;
  }

  inline KernelInfo IKernel::getKernelInfo() const
  {
    BlinkErrorRef errorDesc;
    KernelInfo kInfo;
    BlinkErrorCode errorCode = Blink_IKernel_GetKernelInfo(ref(), &kInfo, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    return kInfo;
  }

  inline std::vector<KernelImageInfo> IKernel::getImagesInfo() const
  {
    BlinkErrorRef errorDesc;
    std::vector<KernelImageInfo> vecImgInfo;
    unsigned numImgs = getNumImages();
    for (unsigned i = 0; i < numImgs; i++) {
      KernelImageInfo imgInfo;
      BlinkErrorCode errorCode = Blink_IKernel_GetImageInfo(ref(), i, &imgInfo, &errorDesc);
      TranslateBlinkError(errorCode, errorDesc);
      vecImgInfo.push_back(imgInfo);
    }
    return vecImgInfo;
  }

  inline std::vector<KernelParameterInfo> IKernel::getParametersInfo() const
  {
    BlinkErrorRef errorDesc;
    std::vector<KernelParameterInfo> vecParameterInfo;
    unsigned numParams = getNumParameters();
    for (unsigned i = 0; i < numParams; i++) {
      KernelParameterInfo parameterInfo;
      BlinkErrorCode errorCode = Blink_IKernel_GetParameterInfo(ref(), i, &parameterInfo,
                                                                &errorDesc);
      TranslateBlinkError(errorCode, errorDesc);
      vecParameterInfo.push_back(parameterInfo);
    }
    return vecParameterInfo;
  }

  //Kernel

  /// \brief Fetch a kernel
  /// Fetch a kernel for the program to execute on the specified device with images with the same layouts as those provided.
  /// Generally it is best to call this every time a kernel is used rather than storing the result. Blink will cache the results of any compilation performed
  /// and can quickly retrieve the result using information stored in the program source.
  inline Kernel::Kernel(const ProgramSource& programSource, ComputeDevice& dev, const std::vector<Image>& images, BlinkCodegenFlags flags)
    : IKernel()
  {
    BlinkRef kernelRef = nullptr;
    BlinkErrorRef errorDesc;
    std::vector<BlinkRef> imageRefs;
    for (const auto& image: images) {
      imageRefs.push_back(image.ref());
    }
    BlinkErrorCode errorCode = Blink_Kernel_Make(&kernelRef, programSource.ref(), dev.ref(), imageRefs.data(), imageRefs.size(), flags, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
    take(kernelRef);
  }

  inline void Kernel::setParamValue(const std::string& name, float value)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_SetFloatParam(ref(), name.c_str(), &value, 1, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline void Kernel::setParamValue(const std::string& name, int value)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_SetIntParam(ref(), name.c_str(), &value, 1, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline void Kernel::setParamValue(const std::string& name, bool value)
  {
    char boolValue = value;
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_SetBoolParam(ref(), name.c_str(), &boolValue, 1, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline void Kernel::setParamValue(const std::string& name, const int* value, unsigned int count)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_SetIntParam(ref(), name.c_str(), value, count, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline void Kernel::setParamValue(const std::string& name, const float* value, unsigned int count)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_SetFloatParam(ref(), name.c_str(), value, count, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline void Kernel::setParamValue(const std::string& name, const char* value, unsigned int count)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_SetBoolParam(ref(), name.c_str(), value, count, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline void Kernel::getParamValue(const std::string& name, int* value, unsigned int count)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_GetIntParam(ref(), name.c_str(), value, count, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline void Kernel::getParamValue(const std::string& name, float* value, unsigned int count)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_GetFloatParam(ref(), name.c_str(), value, count, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline void Kernel::getParamValue(const std::string& name, char* value, unsigned int count)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_GetBoolParam(ref(), name.c_str(), value, count, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline std::vector<Blink::ImageAccessDesc> Kernel::getInputRanges(void)
  {
    BlinkErrorRef errorDesc;

    unsigned int s;
    BlinkErrorCode errorCode = Blink_Kernel_GetNumInputRanges(ref(), &s, &errorDesc);

    TranslateBlinkError(errorCode, errorDesc);

    std::vector<Blink::ImageAccessDesc> ret;
    ret.resize(s);

    errorCode = Blink_Kernel_GetInputRanges(ref(), ret.data(), s, &errorDesc);

    TranslateBlinkError(errorCode, errorDesc);

    return ret;
  }

  //Does it make sense to have iterate without bounds?
  inline void Kernel::iterate()
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_Iterate(ref(), &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  //Maybe have iterate(Device,rect) , so far all iterations are over all components
  inline void Kernel::iterate(const Blink::KernelIterationController& controller)
  {
    BlinkErrorRef errorDesc;
    BlinkErrorCode errorCode = Blink_Kernel_IterateWithController(ref(), &controller, &errorDesc);
    TranslateBlinkError(errorCode, errorDesc);
  }

  inline Exception::Exception(BlinkRef ref)
    : RefHolder(ref, false)
  {
  }

  inline std::string Exception::userMessage() const
  {
    return Blink_Error_GetUserMessage(_ref);
  }

  inline OutOfMemoryException::OutOfMemoryException(BlinkRef ref)
    : Exception(ref) {}
  inline ComputeException::ComputeException(BlinkRef ref)
    : Exception(ref) {}
  inline ImageSpecException::ImageSpecException(BlinkRef ref)
    : Exception(ref) {}
  inline CompilationException::CompilationException(BlinkRef ref)
    : Exception(ref) {}
  inline ParseException::ParseException(BlinkRef ref)
    : CompilationException(ref) {}
  inline DeviceException::DeviceException(BlinkRef ref)
    : Exception(ref) {}
  inline ParameterException::ParameterException(BlinkRef ref)
    : Exception(ref) {}
  inline IncompatibleImagesException::IncompatibleImagesException(BlinkRef ref)
    : Exception(ref) {}

  inline std::string ParseException::parseError() const
  {
    const char* errString;
    std::string result;
    BlinkErrorCode fetchErr = Blink_Error_GetStringProperty(_ref, kBlinkErrorPropertyCompilationErrors, &errString);
    if (!fetchErr) {
      result = errString;
    }
    return result;
  }

  inline int ParseException::lineNumber() const
  {
    int line;
    BlinkErrorCode fetchErr = Blink_Error_GetIntProperty(_ref, kBlinkErrorPropertyLineNumber, &line);
    if (fetchErr) {
      line = -1;
    }
    return line;
  }



} // namespace Blink

#endif
