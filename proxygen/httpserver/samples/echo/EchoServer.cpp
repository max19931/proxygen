/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Memory.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/Unistd.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>

#include "EchoHandler.h"
#include "EchoStats.h"

using namespace EchoService;
using namespace proxygen;

using folly::SocketAddress;

using Protocol = HTTPServer::Protocol;

DEFINE_int32(http_port, 11000, "Port to listen on with HTTP protocol");
DEFINE_int32(spdy_port, 11001, "Port to listen on with SPDY protocol");
DEFINE_int32(h2_port, 11002, "Port to listen on with HTTP/2 protocol");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");
DEFINE_int32(threads, 0, "Number of threads to listen on. Numbers <= 0 "
             "will use the number of cores on this machine.");

class EchoHandlerFactory : public RequestHandlerFactory {
 public:
  void onServerStart(folly::EventBase* /*evb*/) noexcept override {
    stats_.reset(new EchoStats);
  }

  void onServerStop() noexcept override {
    stats_.reset();
  }

  RequestHandler* onRequest(RequestHandler*, HTTPMessage*) noexcept override {
    return new EchoHandler(stats_.get());
  }

 private:
  folly::ThreadLocalPtr<EchoStats> stats_;
};

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::vector<HTTPServer::IPConfig> IPs = {
    {SocketAddress(FLAGS_ip, FLAGS_http_port, true), Protocol::HTTP},
    {SocketAddress(FLAGS_ip, FLAGS_spdy_port, true), Protocol::SPDY},
    {SocketAddress(FLAGS_ip, FLAGS_h2_port, true), Protocol::HTTP2},
  };

  if (FLAGS_threads <= 0) {
    FLAGS_threads = sysconf(_SC_NPROCESSORS_ONLN);
    CHECK(FLAGS_threads > 0);
  }

  HTTPServerOptions options;
  options.threads = static_cast<size_t>(FLAGS_threads);
  options.idleTimeout = std::chrono::milliseconds(60000);
  options.shutdownOn = {SIGINT, SIGTERM};
  options.enableContentCompression = false;
  options.handlerFactories = RequestHandlerChain()
      .addThen<EchoHandlerFactory>()
      .build();
  // Increase the default flow control to 1MB/10MB
  options.initialReceiveWindow = uint32_t(1 << 20);
  options.receiveStreamWindowSize = uint32_t(1 << 20);
  options.receiveSessionWindowSize = 10 * (1 << 20);
  options.h2cEnabled = true;

  HTTPServer server(std::move(options));
  server.bind(IPs);

  // Start HTTPServer mainloop in a separate thread
  std::thread t([&] () {
    server.start();
  });

  t.join();
  return 0;
}
