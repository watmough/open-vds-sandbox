#ifndef OPENVDS_NO_AWS_IOMANAGER
#include <aws/transfer/TransferManager.h>
#include <aws/core/auth/AWSCredentials.h>

void addSomeSymbolsToTheExecutableSoTheLinkerChainWorkAws()
{
  //We do this to use a symbol from the transfermanager so we get the linker chain working on linux
  Aws::Auth::AWSCredentials credentials;
  Aws::S3::S3Client client(credentials);
  Aws::Utils::Threading::DefaultExecutor threadExecutor;
  Aws::Transfer::TransferManagerConfiguration transferConfig(&threadExecutor);
  std::shared_ptr<Aws::S3::S3Client> s3ClientSharedPtr(&client, [](Aws::S3::S3Client*) {});
  transferConfig.s3Client = s3ClientSharedPtr;
  auto manager = Aws::Transfer::TransferManager::Create(transferConfig);
}
#endif

