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

#include "Sequencer.h"
#include <ScriptCanvas/Libraries/Logic/OrderedSequencer.h>
#include <ScriptCanvas/Libraries/Logic/TargetedSequencer.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Logic
        {
            static const int NUMBER_OF_OUTPUTS = 8;

            Sequencer::Sequencer()
                : Node()
                , m_selectedIndex(0)
                , m_currentIndex(0)
                , m_order(0)
                , m_outputIsValid(true)
            {}

            NodeConfiguration Sequencer::GetReplacementNodeConfiguration() const
            {
                auto inSlot = SequencerProperty::GetInSlot(this);
                auto nextSlot = SequencerProperty::GetNextSlot(this);
                
                if (inSlot && inSlot->IsConnected())
                {
                    // Replace by TargetedSequencer
                    return { AZ::Uuid("E1B5F3F8-AFEE-42C9-A22C-CB93F8281CC4") };
                }
                else if (nextSlot && nextSlot->IsConnected())
                {
                    // Replace by OrderedSequencer
                    return { AZ::Uuid("BAFDA139-49A8-453B-A556-D4F4BA213B5C") };
                }

                return {};
            }

            void Sequencer::CustomizeReplacementNode(Node* replacementNode, AZStd::unordered_map<SlotId, AZStd::vector<SlotId>>& outSlotIdMap) const
            {
                auto outSlots = this->GetSlotsByType(CombinedSlotType::ExecutionOut);
                AZ::Crc32 dummyId;

                if (auto newNode = azrtti_cast<TargetedSequencer*>(replacementNode))
                {
                    for (size_t index = 0; index < outSlots.size() - 1; index++)
                    {
                        newNode->HandleExtension(dummyId);
                    }

                    auto inSlot = SequencerProperty::GetInSlot(this);
                    auto newExecutionInSlots = replacementNode->GetSlotsByType(ScriptCanvas::CombinedSlotType::ExecutionIn);
                    if (newExecutionInSlots.size() == 1)
                    {
                        outSlotIdMap.emplace(inSlot->GetId(), AZStd::vector<SlotId>{ newExecutionInSlots[0]->GetId() });
                    }
                    auto newDataInSlots = replacementNode->GetSlotsByType(ScriptCanvas::CombinedSlotType::DataIn);
                    if (newDataInSlots.size() == 1)
                    {
                        outSlotIdMap.emplace(SequencerProperty::GetIndexSlotId(this), AZStd::vector<SlotId>{ newDataInSlots[0]->GetId() });
                    }


                    outSlotIdMap.emplace(SequencerProperty::GetNextSlotId(this), AZStd::vector<SlotId>());
                    outSlotIdMap.emplace(SequencerProperty::GetOrderSlotId(this), AZStd::vector<SlotId>());
                }
                else if (auto newNode2 = azrtti_cast<OrderedSequencer*>(replacementNode))
                {
                    for (size_t index = 0; index < outSlots.size() - 1; index++)
                    {
                        newNode2->HandleExtension(dummyId);
                    }

                    auto nextSlot = SequencerProperty::GetNextSlot(this);
                    auto newExecutionInSlots = replacementNode->GetSlotsByType(ScriptCanvas::CombinedSlotType::ExecutionIn);
                    if (newExecutionInSlots.size() == 1)
                    {
                        outSlotIdMap.emplace(nextSlot->GetId(), AZStd::vector<SlotId>{ newExecutionInSlots[0]->GetId() });
                    }

                    outSlotIdMap.emplace(SequencerProperty::GetInSlotId(this), AZStd::vector<SlotId>());
                    outSlotIdMap.emplace(SequencerProperty::GetIndexSlotId(this), AZStd::vector<SlotId>());
                    outSlotIdMap.emplace(SequencerProperty::GetOrderSlotId(this), AZStd::vector<SlotId>());
                }
                else
                {
                    // This case should not happen
                    return;
                }

                // Map rest execution out slots
                auto newExecutionOutSlots = replacementNode->GetSlotsByType(ScriptCanvas::CombinedSlotType::ExecutionOut);
                if (newExecutionOutSlots.size() == outSlots.size())
                {
                    for (size_t index = 0; index < newExecutionOutSlots.size(); index++)
                    {
                        outSlotIdMap.emplace(outSlots[index]->GetId(), AZStd::vector<SlotId>{ newExecutionOutSlots[index]->GetId() });
                    }
                }
            }

            void  Sequencer::OnInputSignal(const SlotId& slot)
            {
                m_selectedIndex = SequencerProperty::GetIndex(this);
                m_order = SequencerProperty::GetOrder(this);

                const SlotId inSlot = SequencerProperty::GetInSlotId(this);
                const SlotId nextSlot = SequencerProperty::GetNextSlotId(this);
                
                if (slot == inSlot)
                {
                    m_currentIndex = m_selectedIndex;
                } 
                else if (slot == nextSlot)
                {
                    int step = m_order == Order::Forward ? 1 : -1;

                    m_outputIsValid = false;
                    int startIndex = m_currentIndex;
                    while (!m_outputIsValid)
                    {
                        m_currentIndex = (m_currentIndex + step + NUMBER_OF_OUTPUTS) % NUMBER_OF_OUTPUTS;
                        SlotId outSlotId = GetCurrentSlotId();
                        Slot* outSlot = GetSlot(outSlotId);
                        if (outSlot)
                        {
                            m_outputIsValid = !GetConnectedNodes(*outSlot).empty();
                        }

                        //Avoid infinite loop when none of the outputs or only the current output connects to other nodes.
                        if (m_currentIndex == startIndex)
                        {
                            m_outputIsValid = false;
                            break;
                        }
                    }
                }

                if (m_outputIsValid)
                {
                    SlotId outSlotId = GetCurrentSlotId();
                    SignalOutput(outSlotId);
                }
            }

            SlotId Sequencer::GetCurrentSlotId() const
            {
                AZStd::string slotName = "Out" + AZStd::to_string(m_currentIndex);
                SlotId outSlotId = GetSlotId(slotName.c_str());
                return outSlotId;
            }
        }
    }
}