//=============================================================================
// <copyright>
// Copyright (c) 2022 Bluware Inc. All rights reserved.
//
// All rights are reserved. Reproduction or transmission in whole or in part,
// in any form or by any means, electronic, mechanical or otherwise,
// is prohibited without the prior written permission of the copyright owner.
// </copyright>
//=============================================================================

#ifndef CPPJNI_H_INCLUDED
#define CPPJNI_H_INCLUDED

#define JAVA_WRAPPER_GENERATOR // Needed for some otherwise invisible accessors

#include <climits>

#include "OpenVDS/Exceptions.h"
#include "OpenVDS/VolumeDataLayout.h"
#include "OpenVDS/VolumeDataAccess.h"
#include "OpenVDS/GlobalState.h"
#include "OpenVDS/OpenVDS.h"

using CPPJNILibraryException = OpenVDS::Exception;

#include "CPPJNICommon.h"

void CPPJNI_onVDSError(OpenVDS::VDSError const& error);

CPPJNI_SPECIALIZE_SHAREDPTR_NODELETE(OpenVDS::VDS)
CPPJNI_SPECIALIZE_SHAREDPTR_NODELETE(OpenVDS::VolumeDataPageAccessor)

template<>
struct Cleaner<OpenVDS::VDS>
{
  static void 
  cleanup(struct CPPJNIObjectContext& context, std::shared_ptr<OpenVDS::VDS> instancePtr, bool is_disposing) 
  { 
    OpenVDS::VDSError error;
    OpenVDS::Close(instancePtr.get(), error); 
    if (error.code) 
    {
      CPPJNI_onVDSError(error);
    }
  }
};

template<>
struct Cleaner<OpenVDS::VolumeDataPage>
{
  static void 
  cleanup(struct CPPJNIObjectContext& context, std::shared_ptr<OpenVDS::VolumeDataPage> instancePtr, bool is_disposing) 
  { 
    if (is_disposing)
    {
      instancePtr.get()->Release();
    }
    //else
    //{
    //  int debug = 0;
    //}
  }
};

template<>
struct Cleaner<OpenVDS::VolumeDataPageAccessor>
{
  static void 
  cleanup(struct CPPJNIObjectContext& context, std::shared_ptr<OpenVDS::VolumeDataPageAccessor> instancePtr, bool is_disposing) 
  { 
    if (is_disposing)
    {
      instancePtr.get()->Commit();
      auto creator = context.getCreator<OpenVDS::VolumeDataAccessManager>(); // May throw
      creator->DestroyVolumeDataPageAccessor(instancePtr.get());
    }
    //else
    //{
    //  int debug = 0;
    //}
  }
};

#endif
