/****************************************************************************
** Copyright 2022 The Open Group
** Copyright 2022 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#ifndef LAYERMETADATACONTAINER_H
#define LAYERMETADATACONTAINER_H

namespace OpenVDS
{
class LayerMetadataContainer
{
public:
  virtual bool GetMetadataStatus(std::string const &layerName, MetadataStatus &metadataStatus) const = 0;
  virtual void SetMetadataStatus(std::string const &layerName, std::string const &channelName, MetadataStatus &metadataStatus, int pageLimit) = 0;
};
}
#endif
