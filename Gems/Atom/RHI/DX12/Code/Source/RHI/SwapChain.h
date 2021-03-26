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

#include <Atom/RHI/SwapChain.h>

namespace AZ
{
    namespace DX12
    {
        class Device;
        class Image;
        class CommandQueue;

        class SwapChain
            : public RHI::SwapChain
        {
            using Base = RHI::SwapChain;
        public:
            AZ_RTTI(SwapChain, "{974AC6A9-5009-47BE-BD7E-61348BF623F0}", Base);
            AZ_CLASS_ALLOCATOR(SwapChain, AZ::SystemAllocator, 0);

            static RHI::Ptr<SwapChain> Create();

            Device& GetDevice() const;

        private:
            SwapChain() = default;

            //////////////////////////////////////////////////////////////////////////
            // RHI::SwapChain
            RHI::ResultCode InitInternal(RHI::Device& deviceBase, const RHI::SwapChainDescriptor& descriptor, RHI::SwapChainDimensions* nativeDimensions) override;
            void ShutdownInternal() override;
            uint32_t PresentInternal() override;
            RHI::ResultCode InitImageInternal(const InitImageRequest& request) override;
            void ShutdownResourceInternal(RHI::Resource& resourceBase) override;
            RHI::ResultCode ResizeInternal(const RHI::SwapChainDimensions& dimensions, RHI::SwapChainDimensions* nativeDimensions) override;
            bool IsExclusiveFullScreenPreferred() const override;
            bool GetExclusiveFullScreenState() const override;
            bool SetExclusiveFullScreenState(bool fullScreenState) override;
            //////////////////////////////////////////////////////////////////////////

            void ConfigureDisplayMode(const RHI::SwapChainDimensions& dimensions);
            void EnsureColorSpace(const DXGI_COLOR_SPACE_TYPE& colorSpace);
            void DisableHdr();
            void SetHDRMetaData(float maxOutputNits, float minOutputNits, float maxContentLightLevel, float maxFrameAverageLightLevel);

            static const uint32_t InvalidColorSpace = 0xFFFFFFFE;
            DXGI_COLOR_SPACE_TYPE m_colorSpace = static_cast<DXGI_COLOR_SPACE_TYPE>(InvalidColorSpace);

            RHI::Ptr<IDXGISwapChainX> m_swapChain;
            bool m_isInFullScreenExclusiveState = false; //!< Was SetFullscreenState used to enter full screen exclusive state?
            bool m_isTearingSupported = false; //!< Is tearing support available for full screen borderless windowed mode?
        };
    }
}