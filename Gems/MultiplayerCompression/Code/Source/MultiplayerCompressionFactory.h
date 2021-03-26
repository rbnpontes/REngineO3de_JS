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

#include <AzCore/Component/Component.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzNetworking/Framework/ICompressor.h>

namespace MultiplayerCompression
{
    class MultiplayerCompressionFactory
        : public AzNetworking::ICompressorFactory
    {
    public:
        //! Instantiate a new compressor
        //! @return A unique_ptr to a new Compressor
        AZStd::unique_ptr<AzNetworking::ICompressor> Create() override;

        //! Gets the AZ Name of this compressor factory
        //! @return the AZ Name of this compressor factory
        AZ::Name GetFactoryName() const override;

    private:
        const AZ::Name m_name = AZ::Name("MultiplayerCompressor");
    };
}