"""
All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
its licensors.

For complete copyright and license terms please see the LICENSE at the root of this
distribution (the "License"). All use of this software is governed by the License,
or, if provided, by the license below or the license accompanying this file. Do not
remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
"""
"""
C2561342: Exclusive Surface Masks tags function
"""
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import azlmbr.areasystem as areasystem
import azlmbr.bus as bus
import azlmbr.editor as editor
import azlmbr.legacy.general as general
import azlmbr.math as math
import azlmbr.shape as shape
import azlmbr.surface_data as surface_data


sys.path.append(os.path.join(azlmbr.paths.devroot, 'AutomatedTesting', 'Gem', 'PythonTests'))
import automatedtesting_shared.hydra_editor_utils as hydra
from automatedtesting_shared.editor_test_helper import EditorTestHelper
from largeworlds.large_worlds_utils import editor_dynveg_test_helper as dynveg


class TestExclusiveSurfaceMasksTag(EditorTestHelper):
    def __init__(self):
        EditorTestHelper.__init__(self, log_prefix="SurfaceMaskFilter_ExclusionList", args=["level"])

    def run_test(self):
        """
        Summary:
        New level is created and set up with surface shapes with varying surface tags. A simple vegetation area has been
        created and Vegetation Surface Mask Filter component is added to entity with terrain hole exclusion tag.

        Expected Behavior:
        With default Exclusion settings, vegetation does not plant over the terrain holes.
        With Exclusion Weight Max below 1.0, vegetation plants over the terrain holes.

        Test Steps:
         1) Create a new level.
         2) Create entity with components "Vegetation Layer Spawner", "Vegetation Asset List", "Box Shape"
         3) Add a Vegetation Surface Mask Filter component to the entity.
         4) Create 2 surface entities to represent terrain and terrain hole surfaces
         5) Add an Exclusion List tag to the component, and set it to terrainHole.
         6) Check spawn count with default Exclusion Weights
         7) Check spawn count with Exclusion Weight Max set below 1.0
         
        Note:
        - Any passed and failed tests are written to the Editor.log file.
                Parsing the file or running a log_monitor are required to observe the test results.

        :return: None
        """

        def update_surface_tag_exclusion_list(Entity, component_index, surface_tag):
            tag_list = [surface_data.SurfaceTag()]

            # assign list with one surface tag to exclusion list
            hydra.get_set_test(Entity, component_index, "Configuration|Exclusion|Surface Tags", tag_list)

            # set that one surface tag element to required surface tag
            component = Entity.components[component_index]
            path = "Configuration|Exclusion|Surface Tags|[0]|Surface Tag"
            editor.EditorComponentAPIBus(bus.Broadcast, "SetComponentProperty", component, path, surface_tag)
            new_value = hydra.get_component_property_value(component, path)

            if new_value == surface_tag:
                self.log(f"Exclusive surface mask filter of {surface_tag} is added successfully")
            else:
                self.log(f"Failed to add an Exclusive surface mask filter of {surface_tag}")

        def update_generated_surface_tag(Entity, component_index, surface_tag):
            tag_list = [surface_data.SurfaceTag()]

            # assign list with one surface tag to Generated Tags list
            hydra.get_set_test(Entity, component_index, "Configuration|Generated Tags", tag_list)

            # set that one surface tag element to required surface tag
            component = Entity.components[component_index]
            path = "Configuration|Generated Tags|[0]|Surface Tag"
            editor.EditorComponentAPIBus(bus.Broadcast, "SetComponentProperty", component, path, surface_tag)
            new_value = hydra.get_component_property_value(component, path)

            if new_value == surface_tag:
                self.log(f"Generated surface tag of {surface_tag} is added successfully")
            else:
                self.log(f"Failed to add Generated surface tag of {surface_tag}")

        # 1) Create a new level
        self.test_success = self.create_level(
            self.args["level"],
            heightmap_resolution=1024,
            heightmap_meters_per_pixel=1,
            terrain_texture_resolution=4096,
            use_terrain=False,
        )

        # 2) Create entity with components "Vegetation Layer Spawner", "Vegetation Asset List", "Box Shape"
        entity_position = math.Vector3(512.0, 512.0, 32.0)
        asset_path = os.path.join("Slices", "PurpleFlower.dynamicslice")
        spawner_entity = dynveg.create_vegetation_area("Instance Spawner",
                                                       entity_position,
                                                       10.0, 10.0, 10.0,
                                                       asset_path)

        # 3) Add a Vegetation Surface Mask Filter component to the entity.
        spawner_entity.add_component("Vegetation Surface Mask Filter")

        # 4) Create 2 surface entities to represent terrain and terrain hole surfaces
        surface_tags: dict = {"terrainHole": 1327698037, "terrain": 3363197873}
        entity_position = math.Vector3(510.0, 512.0, 32.0)
        surface_entity_1 = dynveg.create_surface_entity("Surface Entity 1",
                                                        entity_position,
                                                        10.0, 10.0, 1.0)
        update_generated_surface_tag(surface_entity_1, 1, surface_tags["terrainHole"])

        entity_position = math.Vector3(520.0, 512.0, 32.0)
        surface_entity_2 = dynveg.create_surface_entity("Surface Entity 2",
                                                        entity_position,
                                                        10.0, 10.0, 1.0)
        update_generated_surface_tag(surface_entity_2, 1, surface_tags["terrain"])

        # 5) Add an Exclusion List tag to the component, and set it to "terrainHole".
        update_surface_tag_exclusion_list(spawner_entity, 3, surface_tags["terrainHole"])

        # 6) Check spawn count with default Exclusion Weights
        general.idle_wait(2.0)  # Allow a few seconds for instances to spawn
        num_expected_instances = 39
        box = shape.ShapeComponentRequestsBus(bus.Event, 'GetEncompassingAabb', spawner_entity.id)
        num_found = areasystem.AreaSystemRequestBus(bus.Broadcast, 'GetInstanceCountInAabb', box)
        self.log(f"Expected {num_expected_instances} instances - Found {num_found} instances")
        self.test_success = self.test_success and num_found == num_expected_instances

        # 7) Check spawn count with Exclusion Weight Max set below 1.0
        hydra.get_set_test(spawner_entity, 3, "Configuration|Exclusion|Weight Max", 0.9)
        general.idle_wait(2.0)  # Allow a few seconds for instances to spawn
        num_expected_instances = 169
        num_found = areasystem.AreaSystemRequestBus(bus.Broadcast, 'GetInstanceCountInAabb', box)
        self.log(f"Expected {num_expected_instances} instances - Found {num_found} instances")
        self.test_success = self.test_success and num_found == num_expected_instances


test = TestExclusiveSurfaceMasksTag()
test.run()