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

#include <AzCore/Component/EntityUtils.h>
#include <AzCore/Script/ScriptSystemBus.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <ScriptCanvas/Asset/RuntimeAsset.h>
#include <ScriptCanvas/Core/Nodeable.h>
#include <ScriptCanvas/Execution/Interpreted/ExecutionInterpretedAPI.h>
#include <ScriptCanvas/Execution/RuntimeComponent.h>

#include "ExecutionContext.h"
#include "AzCore/Slice/SliceComponent.h"
#include "AzFramework/Entity/SliceEntityOwnershipServiceBus.h"

namespace ExecutionContextCpp
{
    void TypeCopy(AZ::BehaviorValueParameter& lhs, const AZ::BehaviorValueParameter& rhs)
    {
        lhs.m_typeId = rhs.m_typeId;
        lhs.m_azRtti = rhs.m_azRtti;
    }

    void ValueCopy(AZ::BehaviorValueParameter& lhs, const AZ::BehaviorValueParameter& rhs)
    {
        lhs.m_typeId = rhs.m_typeId;
        lhs.m_azRtti = rhs.m_azRtti;
        lhs.m_value = rhs.m_value;
    }
}

namespace ScriptCanvas
{
    namespace Execution
    {
        ActivationData::ActivationData(const RuntimeComponent& component, ActivationInputArray& storage)
            : entityId(component.GetEntityId())
            , variableOverrides(component.GetVariableOverrides())
            , runtimeData(component.GetAsset()->GetData())
            , storage(storage)
        {}

        ActivationData::ActivationData(const AZ::EntityId entityId, const VariableData& variableOverrides, const RuntimeData& runtimeData, ActivationInputArray& storage)
            : entityId(entityId)
            , variableOverrides(variableOverrides)
            , runtimeData(runtimeData)
            , storage(storage)
        {}

        ActivationInputRange Context::CreateActivateInputRange(ActivationData& activationData)
        {
            const RuntimeData& runtimeData = activationData.runtimeData;

            ActivationInputRange rangeOut;
            rangeOut.inputs = activationData.storage.begin();
            rangeOut.nodeableCount = runtimeData.m_activationInputRange.nodeableCount;
            rangeOut.variableCount = runtimeData.m_activationInputRange.variableCount;
            rangeOut.entityIdCount = runtimeData.m_activationInputRange.entityIdCount;
            rangeOut.totalCount = rangeOut.nodeableCount + rangeOut.variableCount + rangeOut.entityIdCount;

            // nodeables
            {
                auto sourceVariableIter = runtimeData.m_activationInputRange.inputs;
                const auto sourceVariableSentinel = runtimeData.m_activationInputRange.inputs + runtimeData.m_activationInputRange.nodeableCount;
                auto destVariableIter = rangeOut.inputs;
                for (; sourceVariableIter != sourceVariableSentinel; ++sourceVariableIter, ++destVariableIter)
                {
                    ExecutionContextCpp::ValueCopy(*destVariableIter, *sourceVariableIter);
                }
            }

            // (possibly overridden) variables
            {
                auto sourceVariableIter = runtimeData.m_activationInputRange.inputs + runtimeData.m_activationInputRange.nodeableCount;
                auto destVariableIter = rangeOut.inputs + runtimeData.m_activationInputRange.nodeableCount;

                for (auto& idDatumPair : runtimeData.m_input.m_variables)
                {
                    ExecutionContextCpp::TypeCopy(*destVariableIter, *sourceVariableIter);

                    auto variableOverride = activationData.variableOverrides.FindVariable(idDatumPair.first);
                    const Datum* datum = variableOverride ? variableOverride->GetDatum() : &idDatumPair.second;
                    destVariableIter->m_value = const_cast<void*>(datum->GetAsDanger());

                    ++destVariableIter;
                    ++sourceVariableIter;
                }
            }

            // (must always be re-mapped) EntityId
            {
                AZ::SliceComponent::EntityIdToEntityIdMap loadedEntityIdMap;
                AzFramework::EntityContextId owningContextId = AzFramework::EntityContextId::CreateNull();
                AzFramework::EntityIdContextQueryBus::EventResult(owningContextId, activationData.entityId, &AzFramework::EntityIdContextQueries::GetOwningContextId);
                if (!owningContextId.IsNull())
                {
                    AzFramework::SliceEntityOwnershipServiceRequestBus::EventResult(loadedEntityIdMap, owningContextId, &AzFramework::SliceEntityOwnershipServiceRequestBus::Events::GetLoadedEntityIdMap);
                }

                AZ::BehaviorValueParameter* destVariableIter = rangeOut.inputs
                    + runtimeData.m_activationInputRange.nodeableCount
                    + runtimeData.m_activationInputRange.variableCount;

                const auto entityIdTypeId = azrtti_typeid< Data::EntityIDType>();
                for (auto& idEntityPair : runtimeData.m_input.m_entityIds)
                {
                    destVariableIter->m_typeId = entityIdTypeId;
                    destVariableIter->m_value = destVariableIter->m_tempData.allocate(sizeof(Data::EntityIDType), AZStd::alignment_of<Data::EntityIDType>::value, 0);
                    auto entityIdValuePtr = reinterpret_cast<AZStd::decay_t<Data::EntityIDType>*>(destVariableIter->m_value);

                    if (auto variableOverride = activationData.variableOverrides.FindVariable(idEntityPair.first))
                    {
                        *entityIdValuePtr = *variableOverride->GetDatum()->GetAs<Data::EntityIDType>();
                    }
                    else
                    {
                        auto iter = loadedEntityIdMap.find(idEntityPair.second);
                        if (iter != loadedEntityIdMap.end())
                        {
                            *entityIdValuePtr = iter->second;
                        }
                        else
                        {
                            *entityIdValuePtr = Data::EntityIDType();
                        }
                    }

                    ++destVariableIter;
                }
            }

            return rangeOut;
        }

        void Context::IntializeActivationInputs(RuntimeData& runtimeData)
        {
            AZ::BehaviorContext* behaviorContext(nullptr);
            AZ::ComponentApplicationBus::BroadcastResult(behaviorContext, &AZ::ComponentApplicationRequests::GetBehaviorContext);
            AZ_Assert(behaviorContext, "missing behavior context");

            AZStd::vector<AZ::BehaviorValueParameter>& parameters = runtimeData.m_activationInputStorage;
            parameters.reserve(runtimeData.m_input.GetParameterSize());

            for (const Nodeable* nodeable : runtimeData.m_input.m_nodeables)
            {
                AZ::BehaviorValueParameter bvp;
                bvp.m_typeId = azrtti_typeid(nodeable);

                const auto classIter(behaviorContext->m_typeToClassMap.find(bvp.m_typeId));
                AZ_Assert(classIter != behaviorContext->m_typeToClassMap.end(), "No class by typeID of %s in the behavior context!", bvp.m_typeId.ToString<AZStd::string>().c_str());
                bvp.m_azRtti = classIter->second->m_azRtti;
                bvp.m_value = const_cast<Nodeable*>(nodeable);
                parameters.push_back(bvp);
            }

            for (auto& idDatumPair : runtimeData.m_input.m_variables)
            {
                const Datum* datum = &idDatumPair.second; 
                AZ::BehaviorValueParameter bvp;
                bvp.m_typeId = datum->GetType().GetAZType();
                const auto classIter(behaviorContext->m_typeToClassMap.find(bvp.m_typeId));
                bvp.m_azRtti = classIter != behaviorContext->m_typeToClassMap.end() ? classIter->second->m_azRtti : nullptr;
                bvp.m_value = const_cast<void*>(datum->GetAsDanger());
                parameters.push_back(bvp);
            }

            for (const auto& entityId : runtimeData.m_input.m_entityIds)
            {
                AZ::BehaviorValueParameter bvp;
                bvp.m_typeId = azrtti_typeid<Data::EntityIDType>();
                parameters.push_back(bvp);
            }

            runtimeData.m_activationInputRange.inputs = parameters.begin();
            runtimeData.m_activationInputRange.nodeableCount = runtimeData.m_input.m_nodeables.size();
            runtimeData.m_activationInputRange.variableCount = runtimeData.m_input.m_variables.size();
            runtimeData.m_activationInputRange.entityIdCount = runtimeData.m_input.m_entityIds.size();
        }

        void Context::IntializeAssetData(RuntimeData& runtimeData)
        {
            // \todo, the stack push functions could be retrieved here
            IntializeActivationInputs(runtimeData);
            IntializeStaticCloners(runtimeData);
        }

        void Context::IntializeStaticCloners(RuntimeData& runtimeData)
        {
            AZ::BehaviorContext* behaviorContext(nullptr);
            AZ::ComponentApplicationBus::BroadcastResult(behaviorContext, &AZ::ComponentApplicationRequests::GetBehaviorContext);
            if (!behaviorContext)
            {
                AZ_Error("Behavior Context", false, "A behavior context is required!");
                return;
            }

            runtimeData.m_cloneSources.reserve(runtimeData.m_input.m_staticVariables.size());

            for (AZStd::pair<VariableId, AZStd::any>& staticSource : runtimeData.m_input.m_staticVariables)
            {
                AZStd::any& anySource = staticSource.second;
                auto bcClass = AZ::BehaviorContextHelper::GetClass(behaviorContext, anySource.type());
                AZ_Assert(bcClass, "BehaviorContext class for type %s was deleted", anySource.type().ToString<AZStd::string>().c_str());
                runtimeData.m_cloneSources.emplace_back(*bcClass, AZStd::any_cast<void>(&anySource));
            }
        }

        void Context::UnloadData(RuntimeData& runtimeData)
        {
            Execution::InterpretedUnloadData(runtimeData);
        }

    } 

} 