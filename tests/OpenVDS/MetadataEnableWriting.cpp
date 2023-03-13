//=============================================================================
// <copyright>
// Copyright (c) 2023 Bluware Inc. All rights reserved.
//
// All rights are reserved. Reproduction or transmission in whole or in part,
// in any form or by any means, electronic, mechanical or otherwise,
// is prohibited without the prior written permission of the copyright owner.
// </copyright>
//=============================================================================

#include <OpenVDS/KnownMetadata.h>
#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/IO/IOManager.h>
#include <OpenVDS/IO/IOManagerInMemory.h>
#include <OpenVDS/GlobalMetadataCommon.h>

#include "../utils/FacadeIOManager.h"
#include "../utils/GenerateVDS.h"
#include <fmt/format.h>

#include <OpenVDS/VDS/VDS.h>

#include "gtest/gtest.h"

class IOManagerFailToEnableWriting : public IOManagerFacadeLight 
{
public:
  IOManagerFailToEnableWriting(OpenVDS::IOManager* backend)
    : IOManagerFacadeLight(backend)
  {}

  bool EnableWriting(OpenVDS::Error& error) override
  {
    error.code = -1;
    error.string = "This is a readonly IOManager";
    return false;
  }

};

TEST(OpenVDS_integration, TestFailToEnableWritingWhenGettingMetadataWriteAccessInterface)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate));
    fill3DVDSWithBitNoise(handle);
  }
  IOManagerFailToEnableWriting *readonlyIOManager = new IOManagerFailToEnableWriting(inMemory.get());
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(readonlyIOManager, error));
  ASSERT_TRUE(handle);

  EXPECT_THROW(OpenVDS::GetMetadataWriteAccessInterface(handle), OpenVDS::InvalidOperation);
}