#include <catch2/catch.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "test.grpc.pb.h"
#include "test.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloRequest;
using helloworld::HelloResponse;

class GreeterServiceImpl final : public Greeter::Service {
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloResponse* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
  }
};

std::string read_keycert(const std::string& filepath) {
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

TEST_CASE("Greeter Server and Client Communication") {
  std::string server_address("localhost:50052");
  GreeterServiceImpl service;
  ServerBuilder builder;

  grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
  pkcp.private_key = read_keycert("test-private-key.pem");
  pkcp.cert_chain = read_keycert("test-certificate.pem");
  grpc::SslServerCredentialsOptions ssl_opts;
  ssl_opts.pem_root_certs="";
  ssl_opts.pem_key_cert_pairs.push_back(pkcp);

  auto channel_creds = grpc::SslServerCredentials(ssl_opts);
  builder.AddListeningPort(server_address, channel_creds);
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  REQUIRE(server != nullptr);

  std::cout << "Server listening on " << server_address << std::endl;
  std::string user("world");
  // Load your SSL certificates
  grpc::SslCredentialsOptions ssl_opts1;
  ssl_opts1.pem_root_certs = read_keycert("test-certificate.pem");
  // Create secure channel credentials
  auto creds = grpc::SslCredentials(ssl_opts1);
  std::shared_ptr<Channel> channel = grpc::CreateChannel("localhost:50052", creds);
  Greeter::Stub stub(channel);

  HelloRequest request;
  request.set_name(user);
  HelloResponse reply;
  ClientContext context;

  Status status = stub.SayHello(&context, request, &reply);
  REQUIRE(status.ok());

  std::cout << "Greeter received: " << reply.message() << std::endl;
}
