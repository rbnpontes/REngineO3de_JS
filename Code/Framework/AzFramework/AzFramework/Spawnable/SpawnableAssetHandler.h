/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#pragma once

#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Asset/AssetTypeInfoBus.h>
#include <AzCore/Memory/SystemAllocator.h>

namespace AzFramework
{
    class SpawnableAssetHandler final
        : public AZ::Data::AssetHandler
        , public AZ::AssetTypeInfoBus::MultiHandler
    {
    public:
        AZ_CLASS_ALLOCATOR(SpawnableAssetHandler, AZ::SystemAllocator, 0);
        AZ_RTTI(AZ::SpawnableAssetHandler, "{BF6E2D17-87C9-4BB1-A205-3656CF6D551D}", AZ::Data::AssetHandler);

        SpawnableAssetHandler();
        ~SpawnableAssetHandler() override;

        //
        // AssetHandler
        //

        AZ::Data::AssetPtr CreateAsset(const AZ::Data::AssetId& id, const AZ::Data::AssetType& type) override;
        void DestroyAsset(AZ::Data::AssetPtr ptr) override;
        void GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes) override;


        //
        // AssetTypeInfoBus
        //

        AZ::Data::AssetType GetAssetType() const override;
        const char* GetAssetTypeDisplayName() const override;
        const char* GetGroup() const override;
        const char* GetBrowserIcon() const override;
        void GetAssetTypeExtensions(AZStd::vector<AZStd::string>& extensions) override;
        
    protected:
        LoadResult LoadAssetData(
            const AZ::Data::Asset<AZ::Data::AssetData>& asset,
            AZStd::shared_ptr<AZ::Data::AssetDataStream> stream,
            const AZ::Data::AssetFilterCB& assetLoadFilterCB) override;
    };
} // namespace AzFramework