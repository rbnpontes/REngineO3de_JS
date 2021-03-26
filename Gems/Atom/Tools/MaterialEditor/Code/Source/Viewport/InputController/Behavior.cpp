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

#include <AzCore/Math/Vector3.h>
#include <AzCore/Component/TransformBus.h>

#include <Source/Viewport/InputController/Behavior.h>
#include <Viewport/InputController/MaterialEditorViewportInputController.h>

namespace MaterialEditor
{
    Behavior::Behavior()
    {
        AZ::TickBus::Handler::BusConnect();
    }

    Behavior::~Behavior()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void Behavior::Start()
    {
        m_x = 0;
        m_y = 0;
        m_z = 0;

        MaterialEditorViewportInputControllerRequestBus::BroadcastResult(
            m_cameraEntityId,
            &MaterialEditorViewportInputControllerRequestBus::Handler::GetCameraEntityId);
        AZ_Assert(m_cameraEntityId.IsValid(), "Failed to find m_cameraEntityId");
        MaterialEditorViewportInputControllerRequestBus::BroadcastResult(
            m_distanceToTarget,
            &MaterialEditorViewportInputControllerRequestBus::Handler::GetDistanceToTarget);
        MaterialEditorViewportInputControllerRequestBus::BroadcastResult(
            m_targetPosition,
            &MaterialEditorViewportInputControllerRequestBus::Handler::GetTargetPosition);
    }

    void Behavior::End()
    {
    }

    void Behavior::MoveX(float value)
    {
        m_x += value * GetSensitivityX();
    }

    void Behavior::MoveY(float value)
    {
        m_y += value * GetSensitivityY();
    }

    void Behavior::MoveZ(float value)
    {
        m_z += value * GetSensitivityZ();
    }

    bool Behavior::HasDelta() const
    {
        return
            AZ::GetAbs(m_x) > std::numeric_limits<float>::min() ||
            AZ::GetAbs(m_y) > std::numeric_limits<float>::min() ||
            AZ::GetAbs(m_z) > std::numeric_limits<float>::min();
    }

    void Behavior::TickInternal([[maybe_unused]] float x, [[maybe_unused]] float y, float z)
    {
        m_distanceToTarget = m_distanceToTarget - z;
        AZ::Transform transform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(transform, m_cameraEntityId, &AZ::TransformBus::Events::GetLocalTM);
        AZ::Vector3 position = m_targetPosition -
            transform.GetRotation().TransformVector(AZ::Vector3::CreateAxisY(m_distanceToTarget));
        AZ::TransformBus::Event(m_cameraEntityId, &AZ::TransformBus::Events::SetLocalTranslation, position);
    }

    float Behavior::GetSensitivityX()
    {
        return 0;
    }

    float Behavior::GetSensitivityY()
    {
        return 0;
    }

    float Behavior::GetSensitivityZ()
    {
        return 0.001f;
    }

    AZ::Quaternion Behavior::LookRotation(AZ::Vector3 forward)
    {
        forward.Normalize();
        AZ::Vector3 right = forward.CrossZAxis();
        right.Normalize();
        AZ::Vector3 up = right.Cross(forward);
        up.Normalize();
        AZ::Quaternion rotation = AZ::Quaternion::CreateFromBasis(right, forward, up);
        rotation.Normalize();
        return rotation;
    }

    float Behavior::TakeStep(float& value, float t)
    {
        const float absValue = AZ::GetAbs(value);
        float step;
        if (absValue < SnapInterval)
        {
            step = value;
        }
        else
        {
            step = AZ::Lerp(0, value, t);
        }
        value -= step;
        return step;
    }

    void Behavior::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // delta x and y values are accumulated in MoveX and MoveY functions (by dragging the mouse)
        // in the Tick function we then lerp them down to 0 over short time and apply delta transform to an entity
        if (HasDelta())
        {
            // t is a lerp amount based on time between frames (deltaTime)
            // GetMin restricts how much we can lerp in case of low fps (and very high deltaTime)
            const float t = AZ::GetMin(deltaTime / LerpTime, 0.5f);
            const float x = TakeStep(m_x, t);
            const float y = TakeStep(m_y, t);
            const float z = TakeStep(m_z, t);
            TickInternal(x, y, z);
        }
    }
} // namespace MaterialEditor