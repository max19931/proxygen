/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

namespace proxygen {

// Case-insensitive string comparison
inline bool caseInsensitiveEqual(folly::StringPiece s, folly::StringPiece t) {
  if (s.size() != t.size()) {
    return false;
  }
  return std::equal(
      s.begin(), s.end(), t.begin(), folly::AsciiCaseInsensitive());
}

struct AsciiCaseUnderscoreInsensitive {
  bool operator()(char lhs, char rhs) const {
    if (lhs == '_') {
      lhs = '-';
    }
    if (rhs == '_') {
      rhs = '-';
    }
    return folly::AsciiCaseInsensitive()(lhs, rhs);
  }
};

// Case-insensitive string comparison
inline bool caseUnderscoreInsensitiveEqual(
    folly::StringPiece s,
    folly::StringPiece t) {
  if (s.size() != t.size()) {
    return false;
  }
  bool result = std::equal(
      s.begin(), s.end(), t.begin(), AsciiCaseUnderscoreInsensitive());
  return result;
}

inline bool validateURL(folly::ByteRange url) {
  for (auto p: url) {
    if (p <= 0x20 || p == 0x7f) {
      // no controls or unescaped spaces
      return false;
      }
  }
  return true;
}

inline size_t findLastOf(folly::StringPiece sp, char c) {
  size_t pos = sp.size();
  while (--pos != std::string::npos && sp[pos] != c) {
    // pass
  }
  return pos;
}

}
