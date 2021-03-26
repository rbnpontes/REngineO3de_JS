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

#include <AzCore/RTTI/BehaviorContext.h>

#include <Atom/RPI.Public/Scene.h>

#include <PostProcess/ExposureControl/ExposureControlComponentController.h>

namespace AZ
{
    namespace Render
    {
        void ExposureControlComponentController::Reflect(ReflectContext* context)
        {
            ExposureControlComponentConfig::Reflect(context);

            if (auto* serializeContext = azrtti_cast<SerializeContext*>(context))
            {
                serializeContext->Class<ExposureControlComponentController>()
                    ->Version(0)
                    ->Field("Configuration", &ExposureControlComponentController::m_configuration);
            }

            if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
            {
                behaviorContext->EBus<ExposureControlRequestBus>("ExposureControlRequestBus")
                    ->Attribute(AZ::Script::Attributes::Module, "render")
                    ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                    // Auto-gen behavior context...
#define PARAM_EVENT_BUS ExposureControlRequestBus::Events
#include <Atom/Feature/ParamMacros/StartParamBehaviorContext.inl>
#include <Atom/Feature/PostProcess/ExposureControl/ExposureControlParams.inl>
#include <Atom/Feature/ParamMacros/EndParams.inl>
#undef PARAM_EVENT_BUS

                    ;
            }
        }

        void ExposureControlComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC_CE("ExposureControlService"));
        }

        void ExposureControlComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC_CE("ExposureControlService"));
        }

        void ExposureControlComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("PostFXLayerService"));
        }

        ExposureControlComponentController::ExposureControlComponentController(const ExposureControlComponentConfig& config)
            : m_configuration(config)
        {
        }

        void ExposureControlComponentController::Activate(EntityId entityId)
        {
            m_entityId = entityId;

            PostProcessFeatureProcessorInterface* fp = RPI::Scene::GetFeatureProcessorForEntity<PostProcessFeatureProcessorInterface>(m_entityId);
            if (fp)
            {
                m_postProcessInterface = fp->GetOrCreateSettingsInterface(m_entityId);
                if (m_postProcessInterface)
                {
                    m_settingsInterface = m_postProcessInterface->GetOrCreateExposureControlSettingsInterface();
                    OnConfigChanged();
                }
            }
            ExposureControlRequestBus::Handler::BusConnect(m_entityId);
        }

        void ExposureControlComponentController::Deactivate()
        {
            ExposureControlRequestBus::Handler::BusDisconnect(m_entityId);

            if (m_postProcessInterface)
            {
                m_postProcessInterface->RemoveExposureControlSettingsInterface();
            }

            m_postProcessInterface = nullptr;
            m_settingsInterface = nullptr;
            m_entityId.SetInvalid();
        }

        // Getters & Setters...

        void ExposureControlComponentController::SetConfiguration(const ExposureControlComponentConfig& config)
        {
            m_configuration = config;
            OnConfigChanged();
        }

        const ExposureControlComponentConfig& ExposureControlComponentController::GetConfiguration() const
        {
            return m_configuration;
        }

        void ExposureControlComponentController::OnConfigChanged()
        {
            if (m_settingsInterface)
            {
                m_configuration.CopySettingsTo(m_settingsInterface);
                m_settingsInterface->OnConfigChanged();
            }
        }

        // Auto-gen getter/setter function definitions...
        // The setter functions will set the values on the Atom settings class, then get the value back
        // from the settings class to set the local configuration. This is in case the settings class
        // applies some custom logic that results in the set value being different from the input
#define AZ_GFX_COMMON_PARAM(ValueType, Name, MemberName, DefaultValue)                                  \
        ValueType ExposureControlComponentController::Get##Name() const                                 \
        {                                                                                               \
            return m_configuration.MemberName;                                                          \
        }                                                                                               \
        void ExposureControlComponentController::Set##Name(ValueType val)                               \
        {                                                                                               \
            if(m_settingsInterface)                                                                     \
            {                                                                                           \
                m_settingsInterface->Set##Name(val);                                                    \
                m_settingsInterface->OnConfigChanged();                                                 \
                m_configuration.MemberName = m_settingsInterface->Get##Name();                          \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                m_configuration.MemberName = val;                                                       \
            }                                                                                           \
        }                                                                                               \

#define AZ_GFX_COMMON_OVERRIDE(ValueType, Name, MemberName, OverrideValueType)                          \
        OverrideValueType ExposureControlComponentController::Get##Name##Override() const               \
        {                                                                                               \
            return m_configuration.MemberName##Override;                                                \
        }                                                                                               \
        void ExposureControlComponentController::Set##Name##Override(OverrideValueType val)             \
        {                                                                                               \
            m_configuration.MemberName##Override = val;                                                 \
            if(m_settingsInterface)                                                                     \
            {                                                                                           \
                m_settingsInterface->Set##Name##Override(val);                                          \
                m_settingsInterface->OnConfigChanged();                                                 \
            }                                                                                           \
        }                                                                                               \

#include <Atom/Feature/ParamMacros/MapAllCommon.inl>
#include <Atom/Feature/PostProcess/ExposureControl/ExposureControlParams.inl>
#include <Atom/Feature/ParamMacros/EndParams.inl>


    } // namespace Render
} // namespace AZ