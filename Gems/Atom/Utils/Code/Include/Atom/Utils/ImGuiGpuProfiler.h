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

#include <Atom/RPI.Public/GpuQuery/GpuQueryTypes.h>

#include <AzCore/std/containers/array.h>
#include <AzCore/std/string/string.h>

namespace AZ
{
    namespace Render
    {
        //! Intermediate resource that represents the structure of a Pass within the FrameGraph.
        //! A tree structure will be created from these entries that mimics the pass' structure.
        //! By default, all entries have a parent<-child reference, but only entries that pass the filter will also hold a parent->child reference.
        struct PassEntry
        {
            //! Total number of attribute columns to draw the PipelineStatistics.
            static const uint32_t PipelineStatisticsAttributeCount = 7u;

            using PipelineStatisticsArray = AZStd::array<uint64_t, PipelineStatisticsAttributeCount>;

            PassEntry() = default;
            PassEntry(const class AZ::RPI::Pass* pass, PassEntry* parent);

            ~PassEntry() = default;

            //! Links the child TimestampEntry to the parent's, and sets the dirty flag for both the parent and child entry.
            //! Calling this method will effectively add a parent->child reference for this instance, and all parent entries leading up to this
            //! entry from the root entry.
            void LinkChild(PassEntry* childEntry);

            //! Checks if timestamp queries are enabled for this PassEntry.
            bool IsTimestampEnabled() const;
            //! Checks if PipelineStatistics queries are enabled for this PassEntry.
            bool IsPipelineStatisticsEnabled() const;

            //! The name of the pass.
            Name m_name;
            //! Cache the path name of the Pass as a unique identifier.
            Name m_path;

            RPI::TimestampResult m_timestampResult;
            uint64_t m_interpolatedTimestampInNanoseconds = 0u;

            //! Convert the PipelineStatistics result to an array for easier access.
            PipelineStatisticsArray m_pipelineStatistics;

            //! Used as a double linked structure to reference the parent <-> child relationship.
            PassEntry* m_parent = nullptr;
            AZStd::vector<PassEntry*> m_children;

            //! Mirrors the enabled queries state of the pass.
            bool m_timestampEnabled = false;
            bool m_pipelineStatisticsEnabled = false;

            //! Mirrors the enabled/disabled state of the pass.
            bool m_enabled = false;

            //! Dirty flag to determine if this entry is linked to an parent entry.
            bool m_linked = false;

            //! Cache if the pass is a parent.
            bool m_isParent = false;
        };

        class ImGuiPipelineStatisticsView
        {
            // Sorting types for the PipelineStatistics.
            enum class StatisticsSortType : uint32_t
            {
                Alphabetical,
                Numerical,

                Count
            };

            // Sort options per variant.
            const uint32_t SortVariantPerColumn = 2u;

        public:
            ImGuiPipelineStatisticsView();

            //! Draw the PipelineStatistics window.
            void DrawPipelineStatisticsWindow(bool& draw, const PassEntry* rootPassEntry, AZStd::unordered_map<AZ::Name, PassEntry>& m_timestampEntryDatabase);

            //! Total number of columns (Attribute columns + PassName column).
            static const uint32_t HeaderAttributeCount = PassEntry::PipelineStatisticsAttributeCount + 1u;

        private:
            // Creates a row entry within the attribute matrix.
            void CreateAttributeRow(const PassEntry* passEntry, const PassEntry* rootEntry);

            // Sort the view depending on the selected attribute.
            void SortView();

            // Returns the column index to sort on (i.e 0 = PasName, 1 = Attribute0, etc).
            uint32_t GetSortIndex() const;
            // Returns whether it should be sorting alphabetically or numerically.
            StatisticsSortType GetSortType() const;
            // Returns whether the sorting is normal or inverted.
            bool IsSortStateInverted() const;

            // SortIndex is used to store the state in what order to list the PassEntries.
            // The matrix is only able to draw one sorting state at a time. Each column has two
            // states: normal and inverted. The column's sorting index is interleaved 
            // (e.g ColumnIndex 0: 0 = normal, 1 = inverted; ColumnIndex 1: 2 = normal, 3 = inverted, etc).
            uint32_t m_sortIndex = 0u;

            // Array of PassEntries that will be sorted depending on the sorting index.
            AZStd::vector<const PassEntry*> m_passEntryReferences;

            // Width of the columns.
            const float m_headerColumnWidth[HeaderAttributeCount];

            // Whether the color-coding is enabled.
            bool m_enableColorCoding = true;
            // Whether the exclusion filter is enabled.
            bool m_excludeFilterEnabled = false;
            // Show the contribution of a PassEntry attribute to the total in percentages.
            bool m_showAttributeContribution = false;
            // Show pass' tree state.
            bool m_showPassTreeState = false;
            // Show the disabled passes for the PipelineStatistics window.
            bool m_showDisabledPasses = true;
            // Show parent passes.
            bool m_showParentPasses = true;

            // ImGui filter used to filter passes by the user's input.
            ImGuiTextFilter m_passFilter;
        };

        class ImGuiTimestampView
        {
            // Metric unit in which the timestamp entries are represented.
            enum class TimestampMetricUnit : int32_t
            {
                Milliseconds = 0,
                Nanoseconds,
                Count
            };

            // Workload views.
            enum class FrameWorkloadView : int32_t
            {
                FpsView30 = 0,
                FpsView60,
                Count
            };

            // Sorting types for the flat view.
            enum class ProfilerSortType : uint32_t
            {
                Alphabetical = 0u,
                AlphabeticalInverse,
                AlphabeticalCount = AlphabeticalInverse - Alphabetical + 1u,
                Timestamp,
                TimestampInverse,
                TimestampCount = TimestampInverse - Timestamp + 1u,

                Count = AlphabeticalCount + TimestampCount,
            };

            // Timestamp structure views.
            enum class ProfilerViewType : int32_t
            {
                Hierarchical = 0,
                Flat,
                Count
            };

        public:
            //! Draw the Timestamp window.
            void DrawTimestampWindow(bool& draw, const PassEntry* rootPassEntry, AZStd::unordered_map<Name, PassEntry>& m_timestampEntryDatabase);

        private:
            // Draw option for the hierarchical view of the passes.
            // Recursively iterates through the timestamp entries, and creates an hierarchical structure.
            void DrawHierarchicalView(const PassEntry* entry) const;
            // Draw option for the flat view of the passes.
            void DrawFlatView() const;

            // Sorts the entries array depending on the sorting type.
            void SortFlatView();

            // Draws a single instance of the frame workload bar.
            void DrawFrameWorkloadBar(double value) const;
            // Converts nano- to milliseconds.
            double NanoToMilliseconds(uint64_t nanoseconds) const;
            // When the user clicks a sorting category, depending on the state of the profiler,
            // two things can happen:
            // - If the state changes, the sorting type will change.
            // - If the state stays the same, the order of sorting will change.
            void ToggleOrSwitchSortType(ProfilerSortType start, ProfilerSortType count);
            // Normalizes the timestamp parameter to a 30FPS (33 ms) or 60 FPS (16 ms) metric.
            double NormalizeFrameWorkload(uint64_t timestamp) const;
            // Formats the timestamp label depending on the time metric.
            AZStd::string FormatTimestampLabel(uint64_t timestamp) const;

            // Used to set the timestamp metric unit option.
            TimestampMetricUnit m_timestampMetricUnit = TimestampMetricUnit::Milliseconds;
            // Used to set the frame load option.
            FrameWorkloadView m_frameWorkloadView = FrameWorkloadView::FpsView30;
            // Used to set the sorting type option.
            ProfilerSortType m_sortType = ProfilerSortType::Timestamp;
            // Used to set the view option.
            ProfilerViewType m_viewType = ProfilerViewType::Hierarchical;

            // Array that will be sorted for the flat view.
            static const uint32_t TimestampEntryCount = 1024u;
            AZStd::fixed_vector<PassEntry*, TimestampEntryCount> m_passEntryReferences;

            // ImGui filter used to filter passes.
            ImGuiTextFilter m_passFilter;
        };

        class ImGuiGpuProfiler
        {
        public:
            ImGuiGpuProfiler() = default;
            ~ImGuiGpuProfiler() = default;

            ImGuiGpuProfiler(ImGuiGpuProfiler&& other) = delete;
            ImGuiGpuProfiler(const ImGuiGpuProfiler& other) = delete;
            ImGuiGpuProfiler& operator=(const ImGuiGpuProfiler& other) = delete;

            //! Draws the ImGuiProfiler window.
            void Draw(bool& draw, RHI::Ptr<RPI::ParentPass> rootPass);

        private:
            // Interpolates the values of the PassEntries from the previous frame.
            void InterpolatePassEntries(AZStd::unordered_map<Name, PassEntry>& passEntryDatabase, float weight) const;

            // Create the PassEntries, and returns the root entry.
            PassEntry* CreatePassEntries(RHI::Ptr<RPI::ParentPass> rootPass);

            // Holds a PathName -> PassEntry reference for the PassEntries. 
            AZStd::unordered_map<Name, PassEntry> m_passEntryDatabase;

            bool m_drawTimestampView = false;
            bool m_drawPipelineStatisticsView = false;

            ImGuiTimestampView m_timestampView;
            ImGuiPipelineStatisticsView m_pipelineStatisticsView;
        };
    } //namespace Render
} // namespace AZ

#include "ImGuiGpuProfiler.inl"