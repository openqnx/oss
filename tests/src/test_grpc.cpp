#include "common.h"
#include <grpcpp/grpcpp.h>
#include <grpc/grpc.h>
/* Drives gRPC core init, credentials and a channel object. No server needed:
 * a channel starts IDLE and only connects on first RPC. */
int main() {
  QT_BEGIN("grpc");
  printf("  info grpc core %s\n", grpc_version_string());
  auto creds = grpc::InsecureChannelCredentials();
  QT_CHECK(creds != nullptr, "InsecureChannelCredentials");
  auto ch = grpc::CreateChannel("127.0.0.1:50051", creds);
  QT_CHECK(ch != nullptr, "CreateChannel");
  QT_CHECK(ch && ch->GetState(false) == GRPC_CHANNEL_IDLE, "channel starts IDLE");
  grpc::ChannelArguments args;
  args.SetMaxReceiveMessageSize(4 * 1024 * 1024);
  auto ch2 = grpc::CreateCustomChannel("127.0.0.1:50052", creds, args);
  QT_CHECK(ch2 != nullptr, "CreateCustomChannel with args");
  auto sslc = grpc::SslCredentials(grpc::SslCredentialsOptions());
  QT_CHECK(sslc != nullptr, "SslCredentials (TLS stack linked)");
  QT_END();
}
