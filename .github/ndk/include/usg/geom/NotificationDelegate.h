// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_NOTIFICATIONDELEGATE_H
#define USG_GEOM_NOTIFICATIONDELEGATE_H

#include "usg/api.h"

#include <string>

namespace usg {

  class USG_API NotificationDelegate {
  public:
    virtual void error(const std::string& msg) = 0;
    virtual void fatalError(const std::string& msg) = 0;
    virtual void status(const std::string& msg) = 0;
    virtual void warning(const std::string& msg) = 0;
  };

  USG_API void setNotificationDelegate(NotificationDelegate* notificationDelegate);
}

#endif
