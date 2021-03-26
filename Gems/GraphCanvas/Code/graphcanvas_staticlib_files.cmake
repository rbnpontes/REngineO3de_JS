#
# All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
# its licensors.
#
# For complete copyright and license terms please see the LICENSE at the root of this
# distribution (the "License"). All use of this software is governed by the License,
# or, if provided, by the license below or the license accompanying this file. Do not
# remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#

set(FILES
    StaticLib/GraphCanvas/Widgets/Resources/GraphCanvasEditorResources.qrc
    StaticLib/GraphCanvas/Widgets/Resources/Resources.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasMimeContainer.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasMimeContainer.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasMimeEvent.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasMimeEvent.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasTreeCategorizer.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasTreeCategorizer.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasTreeItem.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasTreeItem.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasTreeModel.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasTreeModel.h
    StaticLib/GraphCanvas/Widgets/NodePropertyBus.h
    StaticLib/GraphCanvas/GraphCanvasBus.h
    StaticLib/GraphCanvas/Components/EntitySaveDataBus.h
    StaticLib/GraphCanvas/Components/GeometryBus.h
    StaticLib/GraphCanvas/Components/GraphCanvasPropertyBus.h
    StaticLib/GraphCanvas/Components/GridBus.h
    StaticLib/GraphCanvas/Components/LayerBus.h
    StaticLib/GraphCanvas/Components/MimeDataHandlerBus.h
    StaticLib/GraphCanvas/Components/PersistentIdBus.h
    StaticLib/GraphCanvas/Components/SceneBus.h
    StaticLib/GraphCanvas/Components/StyleBus.h
    StaticLib/GraphCanvas/Components/ToastBus.h
    StaticLib/GraphCanvas/Components/ViewBus.h
    StaticLib/GraphCanvas/Components/VisualBus.h
    StaticLib/GraphCanvas/GraphicsItems/AnimatedPulse.cpp
    StaticLib/GraphCanvas/GraphicsItems/AnimatedPulse.h
    StaticLib/GraphCanvas/GraphicsItems/GlowOutlineGraphicsItem.cpp
    StaticLib/GraphCanvas/GraphicsItems/GlowOutlineGraphicsItem.h
    StaticLib/GraphCanvas/GraphicsItems/GraphicsEffect.h
    StaticLib/GraphCanvas/GraphicsItems/Occluder.cpp
    StaticLib/GraphCanvas/GraphicsItems/Occluder.h
    StaticLib/GraphCanvas/GraphicsItems/ParticleGraphicsItem.cpp
    StaticLib/GraphCanvas/GraphicsItems/ParticleGraphicsItem.h
    StaticLib/GraphCanvas/GraphicsItems/PulseBus.h
    StaticLib/GraphCanvas/Components/Bookmarks/BookmarkBus.h
    StaticLib/GraphCanvas/Components/ColorPaletteManager/ColorPaletteManagerComponent.cpp
    StaticLib/GraphCanvas/Components/ColorPaletteManager/ColorPaletteManagerComponent.h
    StaticLib/GraphCanvas/Components/Connections/ConnectionBus.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/AssetIdDataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/BooleanDataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/ComboBoxDataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/DataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/DoubleDataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/EntityIdDataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/NodePropertyDisplay.cpp
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/NodePropertyDisplay.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/NumericDataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/ReadOnlyDataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/StringDataInterface.h
    StaticLib/GraphCanvas/Components/NodePropertyDisplay/VectorDataInterface.h
    StaticLib/GraphCanvas/Components/Nodes/NodeBus.h
    StaticLib/GraphCanvas/Components/Nodes/NodeUIBus.h
    StaticLib/GraphCanvas/Components/Nodes/NodeConfiguration.h
    StaticLib/GraphCanvas/Components/Nodes/NodeLayoutBus.h
    StaticLib/GraphCanvas/Components/Nodes/NodeTitleBus.h
    StaticLib/GraphCanvas/Components/Nodes/NodeUIBus.h
    StaticLib/GraphCanvas/Components/Nodes/Comment/CommentBus.h
    StaticLib/GraphCanvas/Components/Nodes/Group/NodeGroupBus.h
    StaticLib/GraphCanvas/Components/Nodes/Wrapper/WrapperNodeBus.h
    StaticLib/GraphCanvas/Components/Slots/SlotBus.h
    StaticLib/GraphCanvas/Components/Slots/Data/DataSlotBus.h
    StaticLib/GraphCanvas/Components/Slots/Property/PropertySlotBus.h
    StaticLib/GraphCanvas/Editor/AssetEditorBus.h
    StaticLib/GraphCanvas/Editor/Automation/AutomationIds.h
    StaticLib/GraphCanvas/Editor/Automation/AutomationUtils.h
    StaticLib/GraphCanvas/Editor/EditorDockWidgetBus.h
    StaticLib/GraphCanvas/Editor/EditorTypes.h
    StaticLib/GraphCanvas/Editor/GraphCanvasProfiler.h
    StaticLib/GraphCanvas/Editor/GraphModelBus.h
    StaticLib/GraphCanvas/Styling/definitions.cpp
    StaticLib/GraphCanvas/Styling/definitions.h
    StaticLib/GraphCanvas/Styling/PseudoElement.cpp
    StaticLib/GraphCanvas/Styling/PseudoElement.h
    StaticLib/GraphCanvas/Styling/Selector.cpp
    StaticLib/GraphCanvas/Styling/SelectorImplementations.cpp
    StaticLib/GraphCanvas/Styling/SelectorImplementations.h
    StaticLib/GraphCanvas/Styling/Parser.cpp
    StaticLib/GraphCanvas/Styling/Parser.h
    StaticLib/GraphCanvas/Styling/Selector.h
    StaticLib/GraphCanvas/Styling/Style.cpp
    StaticLib/GraphCanvas/Styling/Style.h
    StaticLib/GraphCanvas/Styling/StyleHelper.h
    StaticLib/GraphCanvas/Styling/StyleManager.cpp
    StaticLib/GraphCanvas/Styling/StyleManager.h
    StaticLib/GraphCanvas/Types/ComponentSaveDataInterface.h
    StaticLib/GraphCanvas/Types/ConstructPresets.cpp
    StaticLib/GraphCanvas/Types/ConstructPresets.h
    StaticLib/GraphCanvas/Types/EntitySaveData.h
    StaticLib/GraphCanvas/Types/Endpoint.h
    StaticLib/GraphCanvas/Types/GraphCanvasGraphData.cpp
    StaticLib/GraphCanvas/Types/GraphCanvasGraphData.h
    StaticLib/GraphCanvas/Types/GraphCanvasGraphSerialization.cpp
    StaticLib/GraphCanvas/Types/GraphCanvasGraphSerialization.h
    StaticLib/GraphCanvas/Types/SceneMemberComponentSaveData.h
    StaticLib/GraphCanvas/Types/TranslationTypes.h
    StaticLib/GraphCanvas/Types/Types.h
    StaticLib/GraphCanvas/Types/QtMetaTypes.h
    StaticLib/GraphCanvas/Widgets/Resources/default_style.json
    StaticLib/GraphCanvas/Widgets/AssetEditorToolbar/AssetEditorToolbar.cpp
    StaticLib/GraphCanvas/Widgets/AssetEditorToolbar/AssetEditorToolbar.h
    StaticLib/GraphCanvas/Widgets/AssetEditorToolbar/AssetEditorToolbar.ui
    StaticLib/GraphCanvas/Widgets/Bookmarks/BookmarkDockWidget.cpp
    StaticLib/GraphCanvas/Widgets/Bookmarks/BookmarkDockWidget.h
    StaticLib/GraphCanvas/Widgets/Bookmarks/BookmarkDockWidget.ui
    StaticLib/GraphCanvas/Widgets/Bookmarks/BookmarkTableModel.cpp
    StaticLib/GraphCanvas/Widgets/Bookmarks/BookmarkTableModel.h
    StaticLib/GraphCanvas/Widgets/ComboBox/ComboBoxItemModels.h
    StaticLib/GraphCanvas/Widgets/ComboBox/ComboBoxItemModelInterface.h
    StaticLib/GraphCanvas/Widgets/ConstructPresetDialog/ConstructPresetDialog.cpp
    StaticLib/GraphCanvas/Widgets/ConstructPresetDialog/ConstructPresetDialog.h
    StaticLib/GraphCanvas/Widgets/ConstructPresetDialog/ConstructPresetDialog.ui
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/EditorContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/EditorContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/BookmarkContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/BookmarkContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/CollapsedNodeGroupContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/CollapsedNodeGroupContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/CommentContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/CommentContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/ConnectionContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/ConnectionContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/NodeContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/NodeContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/NodeGroupContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/NodeGroupContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/SceneContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/SceneContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/SlotContextMenu.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenus/SlotContextMenu.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ContextMenuAction.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/AlignmentMenuActions/AlignmentActionsMenuGroup.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/AlignmentMenuActions/AlignmentActionsMenuGroup.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/AlignmentMenuActions/AlignmentContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/AlignmentMenuActions/AlignmentContextMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/AlignmentMenuActions/AlignmentContextMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/BookmarkConstructMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/BookmarkConstructMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/CommentConstructMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/CommentConstructMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/ConstructContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/ConstructPresetMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/ConstructPresetMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/GraphCanvasConstructActionsMenuGroup.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/GraphCanvasConstructActionsMenuGroup.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/DisableMenuActions/DisableActionsMenuGroup.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/DisableMenuActions/DisableActionsMenuGroup.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/DisableMenuActions/DisableMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/DisableMenuActions/DisableMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/DisableMenuActions/DisableMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/CommentMenuActions/CommentActionsMenuGroup.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/CommentMenuActions/CommentActionsMenuGroup.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/CommentMenuActions/CommentContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/CommentMenuActions/CommentContextMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/CommentMenuActions/CommentContextMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/EditMenuActions/EditActionsMenuGroup.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/EditMenuActions/EditActionsMenuGroup.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/EditMenuActions/EditContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/EditMenuActions/EditContextMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/EditMenuActions/EditContextMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/GeneralMenuActions/GeneralMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/GeneralMenuActions/GeneralMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeGroupMenuActions/NodeGroupActionsMenuGroup.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeGroupMenuActions/NodeGroupActionsMenuGroup.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeGroupMenuActions/NodeGroupContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeGroupMenuActions/NodeGroupContextMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeGroupMenuActions/NodeGroupContextMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeMenuActions/NodeContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeMenuActions/NodeContextMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeMenuActions/NodeContextMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/SceneMenuActions/SceneActionsMenuGroup.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/SceneMenuActions/SceneActionsMenuGroup.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/SceneMenuActions/SceneContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/SceneMenuActions/SceneContextMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/SceneMenuActions/SceneContextMenuActions.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/SlotMenuActions/SlotContextMenuAction.h
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/SlotMenuActions/SlotContextMenuActions.cpp
    StaticLib/GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/SlotMenuActions/SlotContextMenuActions.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasAssetEditorMainWindow.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasAssetEditorMainWindow.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasEditorCentralWidget.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasEditorCentralWidget.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasEditorCentralWidget.ui
    StaticLib/GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasEditorDockWidget.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasEditorDockWidget.h
    StaticLib/GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasEditorDockWidget.ui
    StaticLib/GraphCanvas/Widgets/GraphCanvasGraphicsView/GraphCanvasGraphicsView.cpp
    StaticLib/GraphCanvas/Widgets/GraphCanvasGraphicsView/GraphCanvasGraphicsView.h
    StaticLib/GraphCanvas/Widgets/MimeEvents/CreateSplicingNodeMimeEvent.cpp
    StaticLib/GraphCanvas/Widgets/MimeEvents/CreateSplicingNodeMimeEvent.h
    StaticLib/GraphCanvas/Widgets/MiniMapGraphicsView/MiniMapGraphicsView.cpp
    StaticLib/GraphCanvas/Widgets/MiniMapGraphicsView/MiniMapGraphicsView.h
    StaticLib/GraphCanvas/Widgets/NodePalette/NodePaletteDockWidget.cpp
    StaticLib/GraphCanvas/Widgets/NodePalette/NodePaletteDockWidget.h
    StaticLib/GraphCanvas/Widgets/NodePalette/NodePaletteDockWidget.ui
    StaticLib/GraphCanvas/Widgets/NodePalette/NodePaletteTreeView.cpp
    StaticLib/GraphCanvas/Widgets/NodePalette/NodePaletteTreeView.h
    StaticLib/GraphCanvas/Widgets/NodePalette/NodePaletteWidget.cpp
    StaticLib/GraphCanvas/Widgets/NodePalette/NodePaletteWidget.h
    StaticLib/GraphCanvas/Widgets/NodePalette/NodePaletteWidget.ui
    StaticLib/GraphCanvas/Widgets/NodePalette/Model/NodePaletteSortFilterProxyModel.cpp
    StaticLib/GraphCanvas/Widgets/NodePalette/Model/NodePaletteSortFilterProxyModel.h
    StaticLib/GraphCanvas/Widgets/NodePalette/TreeItems/DraggableNodePaletteTreeItem.cpp
    StaticLib/GraphCanvas/Widgets/NodePalette/TreeItems/DraggableNodePaletteTreeItem.h
    StaticLib/GraphCanvas/Widgets/NodePalette/TreeItems/IconDecoratedNodePaletteTreeItem.cpp
    StaticLib/GraphCanvas/Widgets/NodePalette/TreeItems/IconDecoratedNodePaletteTreeItem.h
    StaticLib/GraphCanvas/Widgets/NodePalette/TreeItems/NodePaletteTreeItem.cpp
    StaticLib/GraphCanvas/Widgets/NodePalette/TreeItems/NodePaletteTreeItem.h
    StaticLib/GraphCanvas/Widgets/StyledItemDelegates/GenericComboBoxDelegate.cpp
    StaticLib/GraphCanvas/Widgets/StyledItemDelegates/GenericComboBoxDelegate.h
    StaticLib/GraphCanvas/Widgets/StyledItemDelegates/IconDecoratedNameDelegate.cpp
    StaticLib/GraphCanvas/Widgets/StyledItemDelegates/IconDecoratedNameDelegate.h
    StaticLib/GraphCanvas/Widgets/ToastNotification/ToastNotification.cpp
    StaticLib/GraphCanvas/Widgets/ToastNotification/ToastNotification.h
    StaticLib/GraphCanvas/Widgets/ToastNotification/ToastNotification.ui
    StaticLib/GraphCanvas/Utils/ColorUtils.h
    StaticLib/GraphCanvas/Utils/ConversionUtils.h
    StaticLib/GraphCanvas/Utils/GraphUtils.cpp
    StaticLib/GraphCanvas/Utils/GraphUtils.h
    StaticLib/GraphCanvas/Utils/NodeNudgingController.cpp
    StaticLib/GraphCanvas/Utils/NodeNudgingController.h
    StaticLib/GraphCanvas/Utils/QtDrawingUtils.cpp
    StaticLib/GraphCanvas/Utils/QtDrawingUtils.h
    StaticLib/GraphCanvas/Utils/QtMimeUtils.h
    StaticLib/GraphCanvas/Utils/QtVectorMath.h
    StaticLib/GraphCanvas/Utils/StateControllers/StateController.h
    StaticLib/GraphCanvas/Utils/StateControllers/PrioritizedStateController.h
    StaticLib/GraphCanvas/Utils/StateControllers/StackStateController.h
)