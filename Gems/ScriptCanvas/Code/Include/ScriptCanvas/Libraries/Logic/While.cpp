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

#include "While.h"

#include <ScriptCanvas/Execution/ErrorBus.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Logic
        {
            AZ::Outcome<DependencyReport, void> While::GetDependencies() const
            {
                return AZ::Success(DependencyReport{});
            }
            
            SlotId While::GetLoopFinishSlotId() const
            {
                return WhileProperty::GetOutSlotId(const_cast<While*>(this));
            }

            SlotId While::GetLoopSlotId() const
            {
                return WhileProperty::GetLoopSlotId(const_cast<While*>(this));
            }

            bool While::IsFormalLoop() const
            { 
                return true; 
            }
        }
    }
}