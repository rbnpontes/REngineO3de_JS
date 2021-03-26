/*
* All or portions of this file Copyright(c) Amazon.com, Inc.or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
*or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
*WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "precompiled.h"

#include <Asset/AssetDescription.h>
#include <Asset/Functions/ScriptCanvasFunctionAsset.h>
#include <AssetBuilderSDK/SerializationDependencies.h>
#include <AzCore/Asset/AssetManager.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/IOUtils.h>
#include <AzCore/Math/Uuid.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Script/ScriptComponent.h>
#include <AzFramework/StringFunc/StringFunc.h>

#include <ScriptCanvas/Asset/Functions/RuntimeFunctionAssetHandler.h>
#include <ScriptCanvas/Asset/RuntimeAssetHandler.h>
#include <ScriptCanvas/Assets/Functions/ScriptCanvasFunctionAssetHandler.h>
#include <ScriptCanvas/Assets/ScriptCanvasAssetHandler.h>
#include <ScriptCanvas/Components/EditorGraph.h>
#include <ScriptCanvas/Components/EditorGraphVariableManagerComponent.h>
#include <ScriptCanvas/Core/Connection.h>
#include <ScriptCanvas/Core/Node.h>
#include <ScriptCanvas/Grammar/AbstractCodeModel.h>
#include <ScriptCanvas/Results/ErrorText.h>
#include <ScriptCanvas/Utils/BehaviorContextUtils.h>

#include <Builder/ScriptCanvasBuilderWorker.h>

namespace ScriptCanvasBuilder
{
    void Worker::Activate(const AssetHandlers& handlers)
    {
        m_editorAssetHandler = handlers.m_editorAssetHandler;
        m_runtimeAssetHandler = handlers.m_runtimeAssetHandler;
        m_subgraphInterfaceHandler = handlers.m_subgraphInterfaceHandler;
    }

    void Worker::CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const
    {
        AZ_TracePrintf(s_scriptCanvasBuilder, "Start Creating Job");
        AZStd::string fullPath;
        AzFramework::StringFunc::Path::ConstructFull(request.m_watchFolder.data(), request.m_sourceFile.data(), fullPath, false);
        AzFramework::StringFunc::Path::Normalize(fullPath);

        if (!m_editorAssetHandler)
        {
            AZ_Error(s_scriptCanvasBuilder, false, R"(CreateJobs for %s failed because the ScriptCanvas Editor Asset handler is missing.)", fullPath.data());
        }

        AZStd::shared_ptr<AZ::Data::AssetDataStream> assetDataStream = AZStd::make_shared<AZ::Data::AssetDataStream>();

        AZ::IO::FileIOStream stream(fullPath.c_str(), AZ::IO::OpenMode::ModeRead);
        if (!AZ::IO::RetryOpenStream(stream))
        {
            AZ_Warning(s_scriptCanvasBuilder, false, "CreateJobs for \"%s\" failed because the source file could not be opened.", fullPath.data());
            return;
        }

        // Read the asset into a memory buffer, then hand ownership of the buffer to assetDataStream
        {
            AZ::IO::FileIOStream ioStream;
            if (!ioStream.Open(fullPath.data(), AZ::IO::OpenMode::ModeRead))
            {
                AZ_Warning(s_scriptCanvasBuilder, false, "CreateJobs for \"%s\" failed because the source file could not be opened.", fullPath.data());
                return;
            }
            AZStd::vector<AZ::u8> fileBuffer(ioStream.GetLength());
            size_t bytesRead = ioStream.Read(fileBuffer.size(), fileBuffer.data());
            if (bytesRead != ioStream.GetLength())
            {
                AZ_Warning(s_scriptCanvasBuilder, false, AZStd::string::format("File failed to read completely: %s", fullPath.data()).c_str());
                return;
            }

            assetDataStream->Open(AZStd::move(fileBuffer));
        }

        m_sourceDependencies.clear();
        AZStd::unordered_multimap<AZStd::string, AssetBuilderSDK::SourceFileDependency> sourceFileDependencies;

        auto assetFilter = [&response](const AZ::Data::AssetFilterInfo& filterInfo)
        {
            if (filterInfo.m_assetType == azrtti_typeid<ScriptCanvasEditor::ScriptCanvasAsset>() ||
                filterInfo.m_assetType == azrtti_typeid<ScriptCanvasEditor::ScriptCanvasFunctionAsset>() ||
                filterInfo.m_assetType == azrtti_typeid<ScriptEvents::ScriptEventsAsset>() ||
                filterInfo.m_assetType == azrtti_typeid<ScriptCanvas::SubgraphInterfaceAsset>()) // this required, since nodes reference this, rather than the editor asset
            {
                AssetBuilderSDK::SourceFileDependency dependency;
                dependency.m_sourceFileDependencyUUID = filterInfo.m_assetId.m_guid;

                response.m_sourceFileDependencyList.push_back(dependency);

                /*AssetBuilderSDK::SourceFileDependency dependency;
                dependency.m_sourceFileDependencyUUID = asset.GetId().m_guid;

                if (asset.GetType() == azrtti_typeid<ScriptEvents::ScriptEventsAsset>())
                {
                    sourceFileDependencies.emplace("Script Events", dependency);
                }
                else
                {
                    sourceFileDependencies.emplace(s_scriptCanvasProcessJobKey, dependency);
                }

                this->m_sourceDependencies.push_back(asset);*/
            }

            // Asset filter always returns false to prevent parsing dependencies, but makes note of the script canvas dependencies
            return false;
        };


        AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasAsset> asset;
        asset.Create(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
        if (m_editorAssetHandler->LoadAssetDataFromStream(asset, assetDataStream, assetFilter) != AZ::Data::AssetHandler::LoadResult::LoadComplete)
        {
            AZ_Warning(s_scriptCanvasBuilder, false, "CreateJobs for \"%s\" failed because the asset data could not be loaded from the file", fullPath.data());
            return;
        }

        // Flush asset database events to ensure no asset references are held by closures queued on Ebuses.
        AZ::Data::AssetManager::Instance().DispatchEvents();

        auto* scriptCanvasEntity = asset.Get()->GetScriptCanvasEntity();
        auto* sourceGraph = AZ::EntityUtils::FindFirstDerivedComponent<ScriptCanvasEditor::Graph>(scriptCanvasEntity);
        AZ_Assert(sourceGraph, "Graph component is missing from entity.");
        size_t fingerprint = 0;
        const AZStd::set<AZ::Entity*> sortedEntities(sourceGraph->GetGraphData()->m_nodes.begin(), sourceGraph->GetGraphData()->m_nodes.end());
        for (auto& nodeEntity : sortedEntities)
        {
            if (auto nodeComponent = AZ::EntityUtils::FindFirstDerivedComponent<ScriptCanvas::Node>(nodeEntity))
            {
                AZStd::hash_combine(fingerprint, nodeComponent->GenerateFingerprint());
            }
        }

        {
            AssetBuilderSDK::JobDescriptor copyDescriptor;
            copyDescriptor.m_priority = 2;
            copyDescriptor.m_critical = true;
            copyDescriptor.m_jobKey = s_scriptCanvasCopyJobKey;
            copyDescriptor.SetPlatformIdentifier("pc");
            copyDescriptor.m_additionalFingerprintInfo = AZStd::string(GetFingerprintString())
                .append(AZStd::string::format("|%zu", fingerprint));
            response.m_createJobOutputs.push_back(copyDescriptor);
        }

        for (const AssetBuilderSDK::PlatformInfo& info : request.m_enabledPlatforms)
        {
            AssetBuilderSDK::JobDescriptor jobDescriptor;
            jobDescriptor.m_priority = 2;
            jobDescriptor.m_critical = true;
            jobDescriptor.m_jobKey = s_scriptCanvasProcessJobKey;
            jobDescriptor.SetPlatformIdentifier(info.m_identifier.c_str());
            jobDescriptor.m_additionalFingerprintInfo = AZStd::string(GetFingerprintString())
                .append(AZStd::string::format("|%zu", fingerprint));
            // Graph process job needs to wait until its dependency asset job finished
            for (const auto& sourceDependency : sourceFileDependencies)
            {
                jobDescriptor.m_jobDependencyList.emplace_back(sourceDependency.first, info.m_identifier.c_str(), AssetBuilderSDK::JobDependencyType::Order, sourceDependency.second);
            }
            response.m_createJobOutputs.push_back(jobDescriptor);
        }

        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
        AZ_TracePrintf(s_scriptCanvasBuilder, "Finish Creating Job");
    }

    const char* Worker::GetFingerprintString() const
    {
        if (m_fingerprintString.empty())
        {
            // compute it the first time
            const AZStd::string runtimeAssetTypeId = azrtti_typeid<ScriptCanvas::RuntimeAsset>().ToString<AZStd::string>();
            m_fingerprintString = AZStd::string::format("%i%s", GetVersionNumber(), runtimeAssetTypeId.c_str());
        }
        return m_fingerprintString.c_str();
    }

    int Worker::GetVersionNumber() const
    {
        return GetBuilderVersion();
    }

    AZ::Uuid Worker::GetUUID()
    {
        return AZ::Uuid::CreateString("{6E86272B-7C06-4A65-9C25-9FA4AE21F993}");
    }

    void Worker::ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const
    {
        AZ_TracePrintf(s_scriptCanvasBuilder, "Start Processing Job");
        // A runtime script canvas component is generated, which creates a .scriptcanvas_compiled file
        AZStd::string fullPath;
        AZStd::string fileNameOnly;
        AzFramework::StringFunc::Path::GetFullFileName(request.m_sourceFile.c_str(), fileNameOnly);
        fullPath = request.m_fullPath.c_str();
        AzFramework::StringFunc::Path::Normalize(fullPath);

        bool pathFound = false;
        AZStd::string relativePath;
        AzToolsFramework::AssetSystemRequestBus::BroadcastResult
            ( pathFound
            , &AzToolsFramework::AssetSystem::AssetSystemRequest::GetRelativeProductPathFromFullSourceOrProductPath
            , request.m_fullPath.c_str(), relativePath);

        if (!pathFound)
        {
            AZ_Error(s_scriptCanvasBuilder, false, "Failed to get engine relative path from %s", request.m_fullPath.c_str());
            return;
        }

        if (!m_editorAssetHandler)
        {
            AZ_Error(s_scriptCanvasBuilder, false, R"(Exporting of .scriptcanvas for "%s" file failed as no editor asset handler was registered for script canvas. The ScriptCanvas Gem might not be enabled.)", fullPath.data());
            return;
        }

        if (!m_runtimeAssetHandler)
        {
            AZ_Error(s_scriptCanvasBuilder, false, R"(Exporting of .scriptcanvas for "%s" file failed as no runtime asset handler was registered for script canvas.)", fullPath.data());
            return;
        }

        AZStd::shared_ptr<AZ::Data::AssetDataStream> assetDataStream = AZStd::make_shared<AZ::Data::AssetDataStream>();

        AZ::IO::FileIOStream stream(fullPath.c_str(), AZ::IO::OpenMode::ModeRead);
        if (!AZ::IO::RetryOpenStream(stream))
        {
            AZ_Warning(s_scriptCanvasBuilder, false, "CreateJobs for \"%s\" failed because the source file could not be opened.", fullPath.data());
            return;
        }

        // Read the asset into a memory buffer, then hand ownership of the buffer to assetDataStream
        {
            AZ::IO::FileIOStream ioStream;
            if (!ioStream.Open(fullPath.data(), AZ::IO::OpenMode::ModeRead))
            {
                AZ_Warning(s_scriptCanvasBuilder, false, "CreateJobs for \"%s\" failed because the source file could not be opened.", fullPath.data());
                return;
            }
            AZStd::vector<AZ::u8> fileBuffer(ioStream.GetLength());
            size_t bytesRead = ioStream.Read(fileBuffer.size(), fileBuffer.data());
            if (bytesRead != ioStream.GetLength())
            {
                AZ_Warning(s_scriptCanvasBuilder, false, AZStd::string::format("File failed to read completely: %s", fullPath.data()).c_str());
                return;
            }

            assetDataStream->Open(AZStd::move(fileBuffer));
        }


        AZ::SerializeContext* context{};
        AZ::ComponentApplicationBus::BroadcastResult(context, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasAsset> asset;
        asset.Create(request.m_sourceFileUUID);
        if (m_editorAssetHandler->LoadAssetDataFromStream(asset, assetDataStream, nullptr) != AZ::Data::AssetHandler::LoadResult::LoadComplete)
        {
            AZ_Error(s_scriptCanvasBuilder, false, R"(Loading of ScriptCanvas asset for source file "%s" has failed)", fullPath.data());
            return;
        }

        // Flush asset manager events to ensure no asset references are held by closures queued on Ebuses.
        AZ::Data::AssetManager::Instance().DispatchEvents();

        AZStd::string runtimeScriptCanvasOutputPath;
        AzFramework::StringFunc::Path::Join(request.m_tempDirPath.c_str(), fileNameOnly.c_str(), runtimeScriptCanvasOutputPath, true, true);
        AzFramework::StringFunc::Path::ReplaceExtension(runtimeScriptCanvasOutputPath, ScriptCanvas::RuntimeAsset::GetFileExtension());

        if (request.m_jobDescription.m_jobKey == s_scriptCanvasCopyJobKey)
        {
            // ScriptCanvas Editor Asset Copy job
            // The SubID is zero as this represents the main asset
            AssetBuilderSDK::JobProduct jobProduct;
            jobProduct.m_productFileName = fullPath;
            jobProduct.m_productAssetType = azrtti_typeid<ScriptCanvasEditor::ScriptCanvasAsset>();
            jobProduct.m_productSubID = 0;
            jobProduct.m_dependenciesHandled = true;
            response.m_outputProducts.push_back(AZStd::move(jobProduct));
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
        }
        else
        {
            // force load all dependencies into memory
            for (auto& dependency : m_sourceDependencies)
            {
                auto depAsset = AZ::Data::AssetManager::Instance().GetAsset(dependency.GetId(), dependency.GetType(), AZ::Data::AssetLoadBehavior::PreLoad);
                depAsset.BlockUntilLoadComplete();
            }

            AZ::Entity* buildEntity = asset.Get()->GetScriptCanvasEntity();

            ProcessTranslationJobInput input;
            input.assetID = AZ::Data::AssetId(request.m_sourceFileUUID, AZ_CRC("RuntimeData", 0x163310ae));
            input.request = &request;
            input.response = &response;
            input.runtimeScriptCanvasOutputPath = runtimeScriptCanvasOutputPath;
            input.assetHandler = m_runtimeAssetHandler;
            input.buildEntity = buildEntity;
            input.fullPath = fullPath;
            input.fileNameOnly = fileNameOnly;
            input.namespacePath = relativePath;
            input.saveRawLua = true;

            auto translationOutcome = ProcessTranslationJob(input);
            if (translationOutcome.IsSuccess())
            {
                auto saveOutcome = SaveRuntimeAsset(input, input.runtimeDataOut);
                if (saveOutcome.IsSuccess())
                {
                    response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
                }
                else
                {
                    AZ_Error(s_scriptCanvasBuilder, false, saveOutcome.GetError().c_str());
                }
            }
            else
            {
                if (AzFramework::StringFunc::Find(translationOutcome.GetError().c_str(), ScriptCanvas::ParseErrors::SourceUpdateRequired) != AZStd::string::npos)
                {
                    AZ_Warning(s_scriptCanvasBuilder, false, ScriptCanvas::ParseErrors::SourceUpdateRequired);
                    response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
                }
                else if (AzFramework::StringFunc::Find(translationOutcome.GetError().c_str(), ScriptCanvas::ParseErrors::EmptyGraph) != AZStd::string::npos)
                {
                    response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
                }
                else
                {
                    if (AzFramework::StringFunc::Find(fileNameOnly, s_unitTestParseErrorPrefix) != AZStd::string::npos)
                    {
                        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
                    }
                    AZ_Error(s_scriptCanvasBuilder, false, translationOutcome.GetError().c_str());
                }
            }
        }

        AZ_TracePrintf(s_scriptCanvasBuilder, "Finish Processing Job");
    }
}