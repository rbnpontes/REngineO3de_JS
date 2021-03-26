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

#include <AzCore/Script/ScriptSystemBus.h>
#include <Editor/Framework/ScriptCanvasTraceUtilities.h>
#include <Editor/Framework/ScriptCanvasReporter.h>

namespace ScriptCanvas
{
    class RuntimeAsset;
    class RuntimeComponent;
}

namespace ScriptCanvasEditor
{
    using LoadedInterpretedDependencies = AZStd::vector<AZStd::pair<AZStd::string, ScriptCanvas::Translation::LuaAssetResult>>;
    AZ_INLINE LoadedInterpretedDependencies LoadInterpretedDepencies(const ScriptCanvas::DependencySet& dependencySet);

    AZ_INLINE LoadTestFunctionResult LoadTestFunction(AZStd::string_view path);

    AZ_INLINE LoadTestGraphResult LoadTestGraph(AZStd::string_view path);

    struct RunSpec
    {
        DurationSpec duration;
        ScriptCanvas::ExecutionMode execution = ScriptCanvas::ExecutionMode::Interpreted;
        bool expectRuntimeFailure = false;;
        bool processOnly = false;
        bool release = true;
        bool debug = true;
        bool traced = true;
        AZStd::function<void()> m_onPostSimulate;
    };

    struct RunGraphSpec
    {
        AZStd::string_view graphPath;
        AZStd::string_view dirPath;
        RunSpec runSpec;
    };

    AZ_INLINE Reporters RunGraph(const RunGraphSpec& runGraphSpec);
    AZ_INLINE void RunEditorAsset(AZ::Data::Asset<AZ::Data::AssetData> asset, Reporter& reporter, ScriptCanvas::ExecutionMode mode);

    AZ_INLINE void RunGraphImplementation(const RunGraphSpec& runGraphSpec, Reporter& reporter);
    AZ_INLINE void RunGraphImplementation(const RunGraphSpec& runGraphSpec, LoadTestGraphResult& loadGraphResult, Reporter& reporter);
    AZ_INLINE void RunGraphImplementation(const RunGraphSpec& runGraphSpec, Reporters& reporters);
    
    AZ_INLINE void Simulate(const DurationSpec& duration);
    AZ_INLINE void SimulateDuration(const DurationSpec& duration);
    AZ_INLINE void SimulateSeconds(const DurationSpec& duration);
    AZ_INLINE void SimulateTicks(const DurationSpec& duration);
} // ScriptCanvasEditor
#include <Editor/Framework/ScriptCanvasGraphUtilities.inl>