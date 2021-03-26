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

#include "OperatorArithmetic.h"
#include <Include/ScriptCanvas/Libraries/Operators/Math/OperatorAdd.generated.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Operators
        {
            //! Node that provides addition
            class OperatorAdd : public OperatorArithmetic
            {
            public:

                SCRIPTCANVAS_NODE(OperatorAdd);

                OperatorAdd() = default;

                void Operator(Data::eType type, const ArithmeticOperands& operands, Datum& result) override;
                AZStd::unordered_set< Data::Type > GetSupportedNativeDataTypes() const override;

            protected:

                bool IsValidArithmeticSlot(const SlotId& slotId) const override;
            };
        }
    }
}