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

#ifndef OPENVDS_NO_AZURE_SDK_FOR_CPP_IOMANAGER
#include <azure/storage/blobs.hpp>
#include <azure/storage/common/crypt.hpp>
void addSomeSymbolsToTheExecutableSoTheLinkerChainWorkAzure()
{
  auto this_is_to_work_around_linking_inn_azure_storage_common = Azure::Storage::_internal::UrlEncodePath("hello world");
  (void)this_is_to_work_around_linking_inn_azure_storage_common;
}
#endif
