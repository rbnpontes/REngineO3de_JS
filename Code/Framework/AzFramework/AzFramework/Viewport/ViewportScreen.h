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

#include <AzCore/Math/Vector2.h>

namespace AZ
{
    class Frustum;
    class Matrix4x4;
    class Vector3;
    struct ViewFrustumAttributes;
} // namespace AZ

namespace AzFramework
{
    struct CameraState;
    struct ScreenPoint;
    struct ViewportInfo;

    //! Projects a position in world space to screen space for the given camera.
    ScreenPoint WorldToScreen(const AZ::Vector3& worldPosition, const CameraState& cameraState);

    //! Overload of WorldToScreen that accepts camera values that can be precomputed if this function
    //! is called many times in a loop.
    ScreenPoint WorldToScreen(
        const AZ::Vector3& worldPosition, const AZ::Matrix4x4& cameraView, const AZ::Matrix4x4& cameraProjection,
        const AZ::Vector2& viewportSize);

    //! Unprojects a position in screen space to world space.
    //! Note: The position returned will be on the near clip plane of the camera in world space.
    AZ::Vector3 ScreenToWorld(const ScreenPoint& screenPosition, const CameraState& cameraState);

    //! Overload of ScreenToWorld that accepts camera values that can be precomputed if this function
    //! is called many times in a loop.
    AZ::Vector3 ScreenToWorld(
        const ScreenPoint& screenPosition, const AZ::Matrix4x4& inverseCameraView,
        const AZ::Matrix4x4& inverseCameraProjection, const AZ::Vector2& viewportSize);

    //! Returns the camera projection for the current camera state.
    AZ::Matrix4x4 CameraProjection(const CameraState& cameraState);

    //! Returns the inverse of the camera projection for the current camera state.
    AZ::Matrix4x4 InverseCameraProjection(const CameraState& cameraState);

    //! Returns the camera view for the current camera state.
    //! @note This is the 'v' in the MVP transform going from world space to view space (viewFromWorld).
    AZ::Matrix4x4 CameraView(const CameraState& cameraState);

    //! Returns the inverse of the camera view for the current camera state.
    //! @note This is the same as the CameraTransform but corrected for Z up.
    AZ::Matrix4x4 InverseCameraView(const CameraState& cameraState);

    //! Returns the camera transform for the current camera state.
    //! @note This is the inverse of 'v' in the MVP transform going from view space to world space (worldFromView).
    AZ::Matrix4x4 CameraTransform(const CameraState& cameraState);

    //! Takes a camera view (the world to camera space transform) and returns the
    //! corresponding camera transform (the world position and orientation of the camera).
    //! @note The parameter is the viewFromWorld transform (the 'v' in MVP) going from world space
    //! to view space. The return value is worldFromView transform going from view space to world space.
    AZ::Matrix4x4 CameraTransformFromCameraView(const AZ::Matrix4x4& cameraView);

    //! Takes a camera transform (the world position and orientation of the camera) and
    //! returns the corresponding camera view (to be used to transform from world to camera space).
    //! @note The parameter is the worldFromView transform going from view space to world space. The
    //! return value is viewFromWorld transform (the 'v' in MVP) going from view space to world space.
    AZ::Matrix4x4 CameraViewFromCameraTransform(const AZ::Matrix4x4& cameraTransform);

    //! Returns a frustum representing the camera transform and view volume in world space.
    AZ::Frustum FrustumFromCameraState(const CameraState& cameraState);

    //! Returns a structure representing frustum attributes for the current camera state.
    AZ::ViewFrustumAttributes ViewFrustumAttributesFromCameraState(const CameraState& cameraState);

    //! Returns the aspect ratio of the viewport dimensions.
    //! @note Ensure a valid viewport size is provided to this function.
    inline float AspectRatio(const AZ::Vector2& viewportSize)
    {
        AZ_Assert(viewportSize.GetY() > 0.0f, "AspectRatio called with invalid viewport size");
        return viewportSize.GetX() / viewportSize.GetY();
    }
} // namespace AzFramework