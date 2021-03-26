"""
All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
its licensors.

For complete copyright and license terms please see the LICENSE at the root of this
distribution (the "License"). All use of this software is governed by the License,
or, if provided, by the license below or the license accompanying this file. Do not
remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

Test case ID : C14861501
Test Case Title : Verify PxMesh is auto-assigned when Collider component is added after Rendering Mesh component
URL of the test case : https://testrail.agscollab.com/index.php?/cases/view/14861501
"""


# fmt: off
class Tests():
    create_entity          = ("Created test entity",                   "Failed to create test entity")
    mesh_added             = ("Added Mesh component",                  "Failed to add Mesh component")
    physx_collider_added   = ("Added PhysX Collider component",        "Failed to add PhysX Collider component")
    assign_mesh_asset      = ("Assigned Mesh asset to Mesh component", "Failed to assign mesh asset to Mesh component")
    automatic_shape_change = ("Shape was changed automatically",       "Shape failed to change automatically")
# fmt: on


def run():
    """
    Summary:
     Create entity with Mesh component and assign a render mesh to the Mesh component. Add Physics Collider component
     and Verify that the physics mesh asset is auto-assigned.

    Expected Behavior:
     The physics asset in PhysX Collider component is auto-assigned

    Test Steps:
     1) Load the empty level
     2) Create an entity
     3) Add Mesh component
     4) Assign a render mesh asset to Mesh component (the fbx mesh having both Static mesh and PhysX collision Mesh)
     5) Add PhysX Collider component
     6) The physics asset in PhysX Collider component is auto-assigned.

    Note:
     - This test file must be called from the Lumberyard Editor command terminal
     - Any passed and failed tests are written to the Editor.log file.
            Parsing the file or running a log_monitor are required to observe the test results.

    :return: None
    """
    # Builtins
    import os

    # Helper Files
    import ImportPathHelper as imports

    imports.init()
    from utils import Report
    from utils import TestHelper as helper
    from editor_entity_utils import EditorEntity as Entity
    from asset_utils import Asset

    # Asset paths
    STATIC_MESH = os.path.join("assets", "c14861501_physxcollider_rendermeshautoassigned", "spherebot", "r0-b_body.cgf")
    PHYSX_MESH = os.path.join(
        "assets", "c14861501_physxcollider_rendermeshautoassigned", "spherebot", "r0-b_body.pxmesh"
    )

    helper.init_idle()
    # 1) Load the empty level
    helper.open_level("Physics", "Base")

    # 2) Create an entity
    test_entity = Entity.create_editor_entity("test_entity")
    Report.result(Tests.create_entity, test_entity.id.IsValid())

    # 3) Add Mesh component
    mesh_component = test_entity.add_component("Mesh")
    Report.result(Tests.mesh_added, test_entity.has_component("Mesh"))

    # 4) Assign a render mesh asset to Mesh component (the fbx mesh having both Static mesh and PhysX collision Mesh)
    mesh_asset = Asset.find_asset_by_path(STATIC_MESH)
    mesh_component.set_component_property_value("MeshComponentRenderNode|Mesh asset", mesh_asset.id)
    mesh_asset.id = mesh_component.get_component_property_value("MeshComponentRenderNode|Mesh asset")
    Report.result(Tests.assign_mesh_asset, mesh_asset.get_path() == STATIC_MESH.replace(os.sep, "/"))

    # 5) Add PhysX Collider component
    test_component = test_entity.add_component("PhysX Collider")
    Report.result(Tests.physx_collider_added, test_entity.has_component("PhysX Collider"))

    # 6) The physics asset in PhysX Collider component is auto-assigned.
    asset_id = test_component.get_component_property_value("Shape Configuration|Asset|PhysX Mesh")
    test_asset = Asset(asset_id)
    Report.result(Tests.automatic_shape_change, test_asset.get_path() == PHYSX_MESH.replace(os.sep, "/"))


if __name__ == "__main__":
    run()