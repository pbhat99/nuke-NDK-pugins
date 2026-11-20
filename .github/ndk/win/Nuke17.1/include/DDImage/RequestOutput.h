#ifndef RequestOutput_H
#define RequestOutput_H

#include "DDImage/RequestData.h"

namespace DD {
  namespace Image {

    /**
     * Helper class which is a wrapper for a vector of RequestDatas, providing syntactic sugar for adding requests
     * to it
     */
    class DDImage_API RequestOutput
    {
      std::vector<RequestData> _requests;

    public:
      RequestOutput() { }

      /**
       * add a request.
       */
      template<class OpType> void request(OpType* opType, const DD::Image::Box& box, const DD::Image::ChannelSet& channels, int count)
      {
        _requests.push_back(RequestData(opType, box, channels, count));
      }

      /**
       * process all the requests
       */
      void doRequest()
      {
        for (size_t i = 0; i < _requests.size(); i++) {
          _requests[i].doRequest();
        }
      }

      size_t size() const
      {
        return _requests.size();
      }

      const RequestData& operator[](size_t i) const {
        return _requests[i];
      }
    };

  }
}

#endif
