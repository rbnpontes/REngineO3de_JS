"""
All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
its licensors.

For complete copyright and license terms please see the LICENSE at the root of this
distribution (the "License"). All use of this software is governed by the License,
or, if provided, by the license below or the license accompanying this file. Do not
remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
"""


# Test case ID : C15308217
# Test Case Title : Verify that the Terrain texture layer doesn't crash when changing
# from on a level with a terrain component to another level without a terrain component
# URL of the test case : https://testrail.agscollab.com/index.php?/cases/view/15308217


# fmt: off
class Tests():
    enter_game_mode_1 = ("Entered game mode for level1", "Failed to enter game mode for level1")
    enter_game_mode_2 = ("Entered game mode for level2", "Failed to enter game mode for level2")
    find_terrain      = ("Terrain found",                "Terrain not found")
    exit_game_mode_1  = ("Exited game mode for level1",  "Couldn't exit game mode for level1")
    exit_game_mode_2  = ("Exited game mode for level2",  "Couldn't exit game mode for level2")
# fmt: on


def C15308217_NoCrash_LevelSwitch():

    """
    Summary:
    Verify that the Terrain texture layer doesn't crash when changing from on a level with a
    terrain component to another level without a terrain component

    Level Description:
        Level C15308217_NoCrash_LevelSwitchWithOutTerrain:
            No entities
        Level C15308217_NoCrash_LevelSwitchWithTerrain:
            Terrain (entity) - PhysX Terrain entity is created in the level

    Expected Behavior:
    Editior should not crash.
    We are switching the levels and validating the entities and checking any crash is happening while 
    switching between levels

    Test Steps:
     1) Open level with PhysX Terrain component
     2) Enter game mode
     3) Retrieve and validate entities
     4) Exit game mode
     5) Open level which doesn't have a PhysX Terrain component
     6) Enter game mode
     7) Wait for WAIT_TIME to check if any crash happens
     8) Exit game mode
     9) Close the editor


    Note:
    - This test file must be called from the Lumberyard Editor command terminal
    - Any passed and failed tests are written to the Editor.log file.
            Parsing the file or running a log_monitor are required to observe the test results.

    :return: None
    """

    import os
    import sys

    import ImportPathHelper as imports

    imports.init()


    from utils import Report
    from utils import TestHelper as helper

    import azlmbr.legacy.general as general
    import azlmbr.bus

    # Constants
    WAIT_TIME = 1.0

    helper.init_idle()

    # 1) Open level with PhysX Terrain component
    helper.open_level("Physics", "C15308217_NoCrash_LevelSwitchWithTerrain")

    # 2) Enter game mode
    helper.enter_game_mode(Tests.enter_game_mode_1)

    # 3) Retrieve and validate entities
    terrain_id = general.find_game_entity("Terrain")
    Report.critical_result(Tests.find_terrain, terrain_id.IsValid())

    # 4) Exit game mode
    helper.exit_game_mode(Tests.exit_game_mode_1)

    # 5) Open level which doesn't have a PhysX Terrain component
    helper.open_level("Physics", "C15308217_NoCrash_LevelSwitchWithOutTerrain")

    # 6) Enter game mode
    helper.enter_game_mode(Tests.enter_game_mode_2)

    # 7) Wait for WAIT_TIME to check if any crash happens
    general.idle_wait(WAIT_TIME)

    # 8) Exit game mode
    helper.exit_game_mode(Tests.exit_game_mode_2)



if __name__ == "__main__":
    import ImportPathHelper as imports
    imports.init()

    from utils import Report
    Report.start_test(C15308217_NoCrash_LevelSwitch)