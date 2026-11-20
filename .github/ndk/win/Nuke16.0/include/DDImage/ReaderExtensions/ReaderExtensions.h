//---------------------------------------------------------------------------------
//
// This file should be included within the public part of the DD::Image::Reader
// class definition.
//
// It adds the optimised planar read/decode interface to the Reader base class from
// which format specific sub-classes (dpxReader, exrReader, pngReader, etc, and
// customer's own sub-classes built with the NDK) are derived. This file only contains
// the member function prototypes, their implementations are in
// ReaderExtensions/ReaderExtensions.cpp, which is included by Reader.cpp.
//
// NOTE:
// Nuke itself makes no use of these extensions but it needs to compile with them
// so that Reader-derived plugins are compatible with Hiero. Similarly, thirdparty
// plugins built with these extensions included will be compatible with Hiero.
// Note that Reader-derived plugins do not need to provide their own implementation
// of the extensions API, even for use in Hiero, as Reader provides a default
// implementation in terms of its existing virtual engine() interface.
//
//---------------------------------------------------------------------------------

//! Get information about the planar image, including the size of the buffer need to store the encoded image (which
//! can be used for allocating an external buffer for use with planarReadPass()) and the ideal decoded image format
//! for the specified channel.
//! The default implementation cannot know what the genuine ideal channel depth is and will assume floats.
//! Derived classes should override this as approprite - the 'ideal' depth would typically mean the type supported
//! by DataTypeEnum that's closest to the encoded format.
virtual PlanarReadInfo planarReadInfo(const ChannelSet& channels) /* override cannot be used because it is included in the base class as well as in the child class */;

//! Planar read and decode the specified channels of the image in one go. This will happen in a single thread.
//! The default implementation for this function likely to be slow, it's intended to be overridden by derived classes
//! that want and are able to optimise the read and decode.
//! The application may provide a non-zero value to indicate the relative priority (a higher value for higher priority)
//! of different planarReadAndDecode calls, which may be on different threads. Reader sub-classes may make use of this
//! to order their internal work (e.g. if using a thread pool).
virtual void planarReadAndDecode(GenericImagePlane& image, const ChannelSet& channels, int priority) /* override cannot be used because it is included in the base class as well as in the child class */;

//! Planar read the specified channels of the whole file into the buffer in one go, returning the number of bytes read.
//! Make sure the buffer is big enough - that information can be extracted form PlanarReadInfo, which can be got from
//! a call to planarReadInfo().
//! The base class does not provide a working implementation, it's up to derived classes that want to provide this.
//! The function is not pure virtual so that only the relevant derived classes need to be modified.
virtual int planarReadPass(void* buffer, const ChannelSet& channels) /* override cannot be used because it is included in the base class as well as in the child class */;

//! Do a planar decode of the image data in the specified source buffer into the GenericImagePlane, for the
//! specified channels.
//! The source buffer would normally be that previously read into by a call to planarReadPass().
//! The thread parameters indicate which part of the image to decode. It's up to derived classes to decide how
//! to implement this - it might breaks the decode into different regions of the image or different channels, for
//! example.
//! The base class does not provide a working implementation, it's up to derived classes that want to provide this.
//! The function is not pure virtual so that only the relevant derived classes need to be modified.
//! The application may provide a non-zero value to indicate the relative priority (a higher value for higher priority)
//! of different planarDecodePass calls, which may be on different threads. Reader sub-classes may make use of this
//! to order their internal work (e.g. if using a thread pool).
virtual void planarDecodePass(void* srcBuffer, GenericImagePlane& image, const ChannelSet& channels,
                              int threadIndex, int nDecodeThreads, int priority) /* override cannot be used because it is included in the base class as well as in the child class */;
