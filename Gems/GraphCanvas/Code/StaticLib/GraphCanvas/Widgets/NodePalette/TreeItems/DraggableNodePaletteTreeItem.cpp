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
#include <qpixmap.h>

#include <GraphCanvas/Widgets/NodePalette/TreeItems/DraggableNodePaletteTreeItem.h>

namespace GraphCanvas
{
    /////////////////////////////////
    // DraggableNodePaletteTreeItem
    /////////////////////////////////
    DraggableNodePaletteTreeItem::DraggableNodePaletteTreeItem(AZStd::string_view name, EditorId editorId)
        : IconDecoratedNodePaletteTreeItem(name, editorId)
    {
    }

    Qt::ItemFlags DraggableNodePaletteTreeItem::OnFlags() const
    {
        if (IsEnabled() && !HasError())
        {
            return Qt::ItemIsDragEnabled;
        }

        return Qt::ItemFlags();
    }
}