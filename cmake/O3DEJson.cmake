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

include_guard()

set(LY_EXTERNAL_SUBDIRS "" CACHE STRING "List of subdirectories to recurse into when running cmake against the engine's CMakeLists.txt")

#! read_json_external_subdirs
#  Read the "external_subdirectories" array from a *.json file
#  External subdirectories are any folders with CMakeLists.txt in them
#  This could be regular subdirectories, Gems(contains an additional gem.json),
#  Restricted folders(contains an additional restricted.json), etc...
#  
#  \arg:output_external_subdirs name of output variable to store external subdirectories into
#  \arg:input_json_path path to the *.json file to load and read the external subdirectories from
#  \return: external subdirectories as is from the json file.
function(read_json_external_subdirs output_external_subdirs input_json_path)
    o3de_read_json_array(json_array ${input_json_path} "external_subdirectories")
    set(${output_external_subdirs} ${json_array} PARENT_SCOPE)
endfunction()

#! read_json_array
#  Reads the a json array field into a cmake list variable
function(o3de_read_json_array read_output_array input_json_path array_key)
    file(READ ${input_json_path} manifest_json_data)
    string(JSON array_count ERROR_VARIABLE manifest_json_error
        LENGTH ${manifest_json_data} ${array_key})
    if(manifest_json_error)
        # There is no key, return
        return()
    endif()

    if(array_count GREATER 0)
        math(EXPR array_range "${array_count}-1")
        foreach(array_index RANGE ${array_range})
            string(JSON array_element ERROR_VARIABLE manifest_json_error
                GET ${manifest_json_data} ${array_key} "${array_index}")
            if(manifest_json_error)
                message(FATAL_ERROR "Error reading field at index ${array_index} in \"${array_key}\" JSON array: ${manifest_json_error}")
            endif()
            list(APPEND array_elements ${array_element})
        endforeach()
    endif()
    set(${read_output_array} ${array_elements} PARENT_SCOPE)
endfunction()

function(o3de_read_json_key output_value input_json_path key)
    file(READ ${input_json_path} manifest_json_data)
    string(JSON value ERROR_VARIABLE manifest_json_error GET ${manifest_json_data} ${key})
    if(manifest_json_error)
        message(FATAL_ERROR "Error reading field at key ${key} in file \"${input_json_path}\" : ${manifest_json_error}")
    endif()
    set(${output_value} ${value} PARENT_SCOPE)
endfunction()