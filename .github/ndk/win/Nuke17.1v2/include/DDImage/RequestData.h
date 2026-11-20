#ifndef RequestData_H
#define RequestData_H

#include "DDImage/Box.h"
#include "DDImage/ChannelSet.h"

namespace DD {
  namespace Image {

    class Op;
    class DeepOp;
    class Iop;

    class DDImage_API RequestData
    {
      DD::Image::Op* _op;
      
      enum RequestType {
        eReqIop,
        eReqDeepOp,
      } _reqType;
      
      DD::Image::Box _box;
      ChannelSet _channels;
      int _count;
      
    public:
      RequestData(Iop* op, Box box, ChannelSet channels, int count);
      
      RequestData(DeepOp* op, Box box, ChannelSet channels, int count);

      void doRequest();

      const ChannelSet& channels() const;
    };

  }
}

#endif
