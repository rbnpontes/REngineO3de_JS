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

#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/Component/ComponentApplication.h>
#include <LyShineModule.h>

namespace UnitTest
{
    class LyShineTest
        : public ScopedAllocatorSetupFixture
    {
    protected:
        LyShineTest()
            : m_application()
            , m_systemEntity(nullptr)
        {
        }

        void SetUp() override
        {
            SetupApplication();
            SetupEnvironment();
        }

        virtual void SetupApplication()
        {
            AZ::ComponentApplication::Descriptor appDesc;
            appDesc.m_memoryBlocksByteSize = 10 * 1024 * 1024;
            appDesc.m_recordingMode = AZ::Debug::AllocationRecords::RECORD_FULL;
            appDesc.m_stackRecordLevels = 20;

            m_systemEntity = m_application.Create(appDesc);
            m_systemEntity->Init();
            m_systemEntity->Activate();
        }

        virtual void SetupEnvironment()
        {
            m_env = AZStd::make_unique<StubEnv>();
            memset(&m_env->m_stubEnv, 0, sizeof(SSystemGlobalEnvironment));
            m_priorEnv = gEnv;
            gEnv = &m_env->m_stubEnv;
        }

        void TearDown() override
        {
            m_env.reset();
            gEnv = m_priorEnv;
            m_application.Destroy();
        }

        struct StubEnv
        {
            SSystemGlobalEnvironment m_stubEnv;
        };

        AZ::ComponentApplication m_application;
        AZ::Entity* m_systemEntity;
        AZStd::unique_ptr<StubEnv> m_env;

        SSystemGlobalEnvironment* m_priorEnv = nullptr;
    };
} // namespace UnitTest