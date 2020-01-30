/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/TraceEventContext.h>
#include <proxygen/lib/utils/TraceEventObserver.h>

namespace proxygen {

void TraceEventContext::traceEventAvailable(TraceEvent event) {
  for (const auto observer : observers_) {
    observer->traceEventAvailable(event);
  }
}

bool TraceEventContext::isAllTraceEventNeeded() const {
  return allTraceEventNeeded_;
}

}
