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

#pragma once

#include <AssetBuilderSDK/AssetBuilderBusses.h>
#include <AssetBuilderSDK/AssetBuilderSDK.h>
#include <AzCore/Outcome/Outcome.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/Asset/AssetCommon.h>
#include <ScriptCanvas/Translation/Translation.h>
#include <ScriptCanvas/Asset/RuntimeAsset.h>
#include <ScriptCanvas/Asset/RuntimeAssetHandler.h>

namespace AZ
{
    class ScriptAsset;

    namespace Data
    {
        class AssetHandler;
    }
}

namespace ScriptCanvas
{
    class RuntimeAsset;
    class SubgraphInterfaceAsset;
}

namespace ScriptCanvasEditor
{
    class Graph;
    class ScriptCanvasAsset;
    class ScriptCanvasFunctionAsset;
}
namespace ScriptCanvasBuilder
{
    constexpr const char* s_scriptCanvasBuilder = "ScriptCanvasBuilder";
    constexpr const char* s_scriptCanvasCopyJobKey = "Script Canvas Copy Job";
    constexpr const char* s_scriptCanvasProcessJobKey = "Script Canvas Process Job";
    constexpr const char* s_unitTestParseErrorPrefix = "LY_SC_UnitTest";

    enum class BuilderVersion : int
    {
        SplitCopyFromCompileJobs = 9,
        ChangeScriptRequirementToAsset,
        RemoveDebugVariablesFromRelease,
        FailJobsOnMissingLKG,
        // add new entries above
        Current,
    };

    using HandlerOwnership = AZStd::pair<AZ::Data::AssetHandler*, bool>;

    struct SharedHandlers;

    struct AssetHandlers
    {
        AZ::Data::AssetHandler* m_editorAssetHandler = nullptr;
        AZ::Data::AssetHandler* m_editorFunctionAssetHandler = nullptr;
        AZ::Data::AssetHandler* m_runtimeAssetHandler = nullptr;
        AZ::Data::AssetHandler* m_subgraphInterfaceHandler = nullptr;

        AssetHandlers() = default;
        explicit AssetHandlers(SharedHandlers& source);
    };

    struct SharedHandlers
    {
        HandlerOwnership m_editorAssetHandler{};
        HandlerOwnership m_editorFunctionAssetHandler{};
        HandlerOwnership m_runtimeAssetHandler{};
        HandlerOwnership m_subgraphInterfaceHandler{};

        void DeleteOwnedHandlers();

    private:
        void DeleteIfOwned(HandlerOwnership& handler);
    };

    struct ProcessTranslationJobInput
    {
        AZ::Data::AssetId assetID;
        const AssetBuilderSDK::ProcessJobRequest* request = nullptr;
        AssetBuilderSDK::ProcessJobResponse* response = nullptr;
        AZStd::string runtimeScriptCanvasOutputPath;
        AZ::Data::AssetHandler* assetHandler = nullptr;
        AZ::Entity* buildEntity = nullptr;
        AZStd::string fullPath;
        AZStd::string fileNameOnly;
        AZStd::string namespacePath;
        bool saveRawLua = false;
        ScriptCanvas::RuntimeData runtimeDataOut;
        ScriptCanvas::Grammar::SubgraphInterface interfaceOut;
    };

    class JobDependencyVerificationHandler : public ScriptCanvas::RuntimeAssetHandler
    {
    public:
        AZ_CLASS_ALLOCATOR(JobDependencyVerificationHandler, AZ::SystemAllocator, 0);
        AZ_RTTI(JobDependencyVerificationHandler, "{3997EF50-350A-46F0-9D84-7FA403855CC5}", ScriptCanvas::RuntimeAssetHandler);

        void InitAsset(const AZ::Data::Asset<AZ::Data::AssetData>& asset, bool loadStageSucceeded, bool isReload) override
        {
            AZ_UNUSED(asset);
            AZ_UNUSED(loadStageSucceeded);
            AZ_UNUSED(isReload);
            // do nothing, this just verifies the asset existed
        }
    };

    AZ::Outcome<AZ::Data::Asset<ScriptCanvas::RuntimeAsset>, AZStd::string> CreateRuntimeAsset(const AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasAsset>& asset);

    AZ::Outcome<AZ::Data::Asset<ScriptCanvas::SubgraphInterfaceAsset>, AZStd::string> CreateRuntimeFunctionAsset(const AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasFunctionAsset>& asset);

    AZ::Outcome<ScriptCanvas::GraphData, AZStd::string> CompileGraphData(AZ::Entity* scriptCanvasEntity);

    AZ::Outcome<ScriptCanvas::VariableData, AZStd::string> CompileVariableData(AZ::Entity* scriptCanvasEntity);

    AZ::Outcome<ScriptCanvas::Translation::LuaAssetResult, AZStd::string> CreateLuaAsset(AZ::Entity* buildEntity, AZ::Data::AssetId scriptAssetId, AZStd::string_view rawLuaFilePath);

    int GetBuilderVersion();

    AZ::Outcome<AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasAsset>, AZStd::string> LoadEditorAsset(AZStd::string_view graphPath);

    AZ::Outcome<AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasFunctionAsset>, AZStd::string> LoadEditorFunctionAsset(AZStd::string_view graphPath);

    AZ::Outcome<void, AZStd::string> ProcessTranslationJob(ProcessTranslationJobInput& input);

    ScriptCanvasEditor::Graph* PrepareSourceGraph(AZ::Entity* const buildEntity);

    AZ::Outcome<void, AZStd::string> SaveSubgraphInterface(ProcessTranslationJobInput& input, ScriptCanvas::SubgraphInterfaceData& subgraphInterface);

    AZ::Outcome<void, AZStd::string> SaveRuntimeAsset(ProcessTranslationJobInput& input, ScriptCanvas::RuntimeData& runtimeData);

    ScriptCanvas::Translation::Result TranslateToLua(ScriptCanvas::Grammar::Request& request);

    class Worker
        : public AssetBuilderSDK::AssetBuilderCommandBus::Handler
    {
    public:
        static AZ::Uuid GetUUID();
               
        Worker() = default;
        Worker(const Worker&) = delete;

        void Activate(const AssetHandlers& handlers);

        //! Asset Builder Callback Functions
        void CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const;

        const char* GetFingerprintString() const;

        int GetVersionNumber() const;

        void ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const;

        void ShutDown() override {};

    private:
        AZ::Data::AssetHandler* m_editorAssetHandler = nullptr;
        AZ::Data::AssetHandler* m_runtimeAssetHandler = nullptr;
        AZ::Data::AssetHandler* m_subgraphInterfaceHandler = nullptr;

        mutable AZStd::vector<AZ::Data::Asset<AZ::Data::AssetData>> m_sourceDependencies;
        // cached on first time query
        mutable AZStd::string m_fingerprintString;
    };
    
    class FunctionWorker
        : public AssetBuilderSDK::AssetBuilderCommandBus::Handler
    {
    public:
        static AZ::Uuid GetUUID();

        FunctionWorker() = default;
        FunctionWorker(const FunctionWorker&) = delete;

        void Activate(const AssetHandlers& handlers);

        //! Asset Builder Callback Functions
        void CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const;

        const char* GetFingerprintString() const;

        int GetVersionNumber() const;

        void ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const;
                
        void ShutDown() override {};

    private:
        AZ::Data::AssetHandler* m_editorAssetHandler = nullptr;
        AZ::Data::AssetHandler* m_runtimeAssetHandler = nullptr;
        AZ::Data::AssetHandler* m_subgraphInterfaceHandler = nullptr;

        mutable AZStd::vector<AZ::Data::Asset<AZ::Data::AssetData>> m_sourceDependencies;

        // cached on first time query
        mutable AZStd::string m_fingerprintString;
    };
}