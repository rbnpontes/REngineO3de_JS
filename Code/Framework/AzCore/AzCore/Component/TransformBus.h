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

/** @file
 * Header file for buses that dispatch and receive events related to positioning,
 * rotating, scaling, and parenting an entity.
 */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/InterpolationSample.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/EBus/Event.h>

namespace AZ
{
    class Transform;

    using TransformChangedEvent = Event<Transform, Transform>;

    using ParentChangedEvent = Event<EntityId, EntityId>;

    enum class ChildChangeType { Added, Removed };
    using ChildChangedEvent = Event<ChildChangeType, EntityId>;

    //! Interface for AZ::TransformBus, which is an EBus that receives requests
    //! to translate (position), rotate, and scale an entity in 3D space. It
    //! also receives requests to get and set the parent of an entity and get
    //! the descendants of an entity.
    //! 
    //! An entity's local transform is the entity's position relative to its
    //! parent entity. An entity's world transform is the entity's position
    //! within the entire game space.
    class TransformInterface
        : public ComponentBus
    {
    public:
        AZ_RTTI(TransformInterface, "{8DD8A4E2-7F61-4A36-9169-A31F03E25FEB}");

        //! Overrides the default AZ::EBusTraits handler policy to allow one listener only.
        static const EBusHandlerPolicy HandlerPolicy = EBusHandlerPolicy::Single;

        //! Destroys the instance of the class.
        virtual ~TransformInterface() = default;

        //! Event handlers
        //! @{
        //! Binds the provided TransformChangedEvent handler to the TransformComponent.
        //! @param handler the handler to invoke when the entities transform changes
        virtual void BindTransformChangedEventHandler(TransformChangedEvent::Handler& handler) = 0;

        //! Binds the provided ParentChangedEvent to the TransformComponent.
        //! @param handler the handler to invoke when the entities parent changes
        virtual void BindParentChangedEventHandler(ParentChangedEvent::Handler& handler) = 0;

        //! Binds the provided ChildChangedEvent handler to the TransformComponent.
        //! @param handler the handler to invoke when the entity has a child entity added or removed
        virtual void BindChildChangedEventHandler(ChildChangedEvent::Handler& handler) = 0;

        //! Notifies a child change event.
        //! @param changeType the type of child event, adding or removing
        //! @param entityId   the entityId of the child being added or removed
        virtual void NotifyChildChangedEvent(ChildChangeType changeType, EntityId entityId) = 0;
        //! @}

        //! Transform modifiers
        //! @{
        //! Returns the entity's local transform, not including the parent transform.
        //! @return A reference to a transform that represents the entity's position relative to its parent entity.
        virtual const Transform& GetLocalTM() = 0;

        //! Sets the entity's local transform and notifies all listeners.
        //! @param tm A reference to a transform for positioning the entity relative to its parent entity.
        virtual void SetLocalTM([[maybe_unused]] const Transform& tm) {}

        //! Returns the entity's world transform, including the parent transform.
        //! @return A reference to a transform that represents the entity's position within the world.
        virtual const Transform& GetWorldTM() = 0;

        //! Sets the world transform and notifies all listeners.
        //! @param tm A reference to a transform for positioning the entity within the world.
        virtual void SetWorldTM([[maybe_unused]] const Transform& tm) {}

        //! Retrieves the entity's local and world transforms.
        //! @param[out] localTM A reference to a transform that represents the entity's
        //! position relative to its parent entity.
        //! @param[out] worldTM A reference to a transform that represents the entity's
        //! position within the world.
        virtual void GetLocalAndWorld([[maybe_unused]] Transform& localTM, [[maybe_unused]] Transform& worldTM) {}
        //! @}

        //! Translation modifiers
        //! @{
        //! Sets the entity's world space translation, which represents how to move the entity to a new position within the world.
        //! @param newPosition A three-dimensional translation vector.
        virtual void SetWorldTranslation([[maybe_unused]] const AZ::Vector3& newPosition) {}

        //! Sets the entity's local space translation, which represents how to move the entity to a new position relative to its parent.
        //! @param newPosition A three-dimensional translation vector.
        virtual void SetLocalTranslation([[maybe_unused]] const AZ::Vector3& newPosition) {}

        //! Gets the entity's world space translation.
        //! @return A three-dimensional translation vector.
        virtual AZ::Vector3 GetWorldTranslation() { return AZ::Vector3(FLT_MAX); }

        //! Gets the entity's local space translation.
        //! @return A three-dimensional translation vector.
        virtual AZ::Vector3 GetLocalTranslation() { return AZ::Vector3(FLT_MAX); }

        //! Moves the entity within world space.
        //! @param offset A three-dimensional vector that contains the offset to apply to the entity.
        virtual void MoveEntity([[maybe_unused]] const AZ::Vector3& offset) {}

        //! Sets the entity's X coordinate in world space.
        //! @param x A new value for the entity's X coordinate in world space.
        virtual void SetWorldX([[maybe_unused]] float x) {}

        //! Sets the entity's Y coordinate in world space.
        //! @param y A new value for the entity's Y coordinate in world space.
        virtual void SetWorldY([[maybe_unused]] float y) {}

        //! Sets the entity's Z coordinate in world space.
        //! @param z A new value for the entity's Z coordinate in world space.
        virtual void SetWorldZ([[maybe_unused]] float z) {}

        //! Gets the entity's X coordinate in world space.
        //! @return The entity's X coordinate in world space.
        virtual float GetWorldX() { return FLT_MAX; }

        //! Gets the entity's Y coordinate in world space.
        //! @return The entity's Y coordinate in world space.
        virtual float GetWorldY() { return FLT_MAX; }

        //! Gets the entity's Z coordinate in world space.
        //! @return The entity's Z coordinate in world space.
        virtual float GetWorldZ() { return FLT_MAX; }

        //! Sets the entity's X coordinate in local space.
        //! @param x A new value for the entity's X coordinate in local space.
        virtual void SetLocalX([[maybe_unused]] float x) {}

        //! Sets the entity's Y coordinate in local space.
        //! @param y A new value for the entity's Y coordinate in local space.
        virtual void SetLocalY([[maybe_unused]] float y) {}

        //! Sets the entity's Z coordinate in local space.
        //! @param z A new value for the entity's Z coordinate in local space.
        virtual void SetLocalZ([[maybe_unused]] float z) {}

        //! Gets the entity's X coordinate in local space.
        //! @return The entity's X coordinate in local space.
        virtual float GetLocalX() { return FLT_MAX; }

        //! Gets the entity's Y coordinate in local space.
        //! @return The entity's Y coordinate in local space.
        virtual float GetLocalY() { return FLT_MAX; }

        //! Gets the entity's Z coordinate in local space.
        //! @return The entity's Z coordinate in local space.
        virtual float GetLocalZ() { return FLT_MAX; }
        //! @}

        //! Rotation modifiers
        //! @{
        //! @deprecated Use SetLocalRotation()
        //! Sets the entity's rotation in the world.
        //! The origin of the axes is the entity's position in world space.
        //! @param eulerAnglesRadians A three-dimensional vector, containing Euler angles in radians, to rotate the entity by.
        virtual void SetRotation([[maybe_unused]] const AZ::Vector3& eulerAnglesRadians) {}

        //! @deprecated Use SetLocalRotation()
        //! Sets the entity's rotation around the world's X axis.
        //! The origin of the axis is the entity's position in world space.
        //! @param eulerAngleRadians The X coordinate Euler angle in radians to use for the entity's rotation.
        virtual void SetRotationX([[maybe_unused]] float eulerAngleRadian) {}

        //! @deprecated Use SetLocalRotation()
        //! Sets the entity's rotation around the world's Y axis.
        //! The origin of the axis is the entity's position in world space.
        //! @param eulerAngleRadians The Y coordinate Euler angle in radians to use for the entity's rotation.
        virtual void SetRotationY([[maybe_unused]] float eulerAngleRadian) {}

        //! @deprecated Use SetLocalRotation()
        //! Sets the entity's rotation around the world's Z axis.
        //! The origin of the axis is the entity's position in world space.
        //! @param eulerAngleRadians The Z coordinate Euler angle in radians to use for the entity's rotation.
        virtual void SetRotationZ([[maybe_unused]] float eulerAngleRadian) {}

        //! @deprecated Use SetLocalRotationQuaternion()
        //! Sets the entity's rotation in the world in quaternion notation.
        //! The origin of the axes is the entity's position in world space.
        //! @param quaternion A quaternion that represents the rotation to use for the entity.
        virtual void SetRotationQuaternion([[maybe_unused]] const AZ::Quaternion& quaternion) {}

        //! @deprecated Use RotateAroundLocalX()
        //! Rotates the entity around the world's X axis.
        //! The origin of the axis is the entity's position in world space.
        //! @param eulerAngleRadians The Euler angle in radians by which to rotate the entity around the X axis.
        virtual void RotateByX([[maybe_unused]] float eulerAngleRadian) {}

        //! @deprecated Use RotateAroundLocalY()
        //! Rotates the entity around the world's Y axis.
        //! The origin of the axis is the entity's position in world space.
        //! @param eulerAngleRadians The Euler angle in radians by which to rotate the entity around the Y axis.
        virtual void RotateByY([[maybe_unused]] float eulerAngleRadian) {}

        //! @deprecated Use RotateAroundLocalZ()
        //! Rotates the entity around the world's Z axis.
        //! The origin of the axis is the entity's position in world space.
        //! @param eulerAngleRadians The Euler angle in radians by which to rotate the entity around the Z axis.
        virtual void RotateByZ([[maybe_unused]] float eulerAngleRadian) {}

        //! @deprecated Use GetLocalRotation()
        //! Gets the entity's rotation in the world in Euler angles rotation in radians.
        //! @return A three-dimensional vector, containing Euler angles in radians, that represents the entity's rotation.
        virtual AZ::Vector3 GetRotationEulerRadians() { return AZ::Vector3(FLT_MAX); }

        //! @deprecated Use GetLocalRotationQuaternion()
        //! Gets the entity's rotation in the world in quaternion format.
        //! @return A quaternion that represents the entity's rotation in world space.
        virtual AZ::Quaternion GetRotationQuaternion() { return AZ::Quaternion::CreateZero(); }

        //! @deprecated Use GetLocalRotation()
        //! Gets the entity's rotation around the world's X axis.
        //! @return The Euler angle in radians by which the the entity is rotated around the X axis in world space.
        virtual float GetRotationX() { return FLT_MAX; }

        //! @deprecated Use GetLocalRotation()
        //! Gets the entity's rotation around the world's Y axis.
        //! @return The Euler angle in radians by which the the entity is rotated around the Y axis in world space.
        virtual float GetRotationY() { return FLT_MAX; }

        //! @deprecated Use GetLocalRotation()
        //! Gets the entity's rotation around the world's Z axis.
        //! @return The Euler angle in radians by which the the entity is rotated around the Z axis in world space.
        virtual float GetRotationZ() { return FLT_MAX; }

        //! Get angles in radian for each principle axis around which the world transform is
        //! rotated in the order of z-axis and y-axis and then x-axis.
        //! @return The Euler angles in radian indicating how much is rotated around each principle axis.
        virtual AZ::Vector3 GetWorldRotation() { return AZ::Vector3(FLT_MAX); }

        //! Get the quaternion representing the world rotation.
        //! @return The Rotation quaternion in world space.
        virtual AZ::Quaternion GetWorldRotationQuaternion() { return AZ::Quaternion::CreateZero(); }

        //! Set the local rotation matrix using the composition of rotations around
        //! the principle axes in the order of z-axis first and y-axis and then x-axis.
        //! @param eulerRadianAngles A Vector3 denoting radian angles of the rotations around each principle axis.
        virtual void SetLocalRotation([[maybe_unused]] const AZ::Vector3& eulerRadianAngles) {}

        //! Set the local rotation matrix using a quaternion.
        //! @param quaternion A quaternion representing the rotation to set.
        virtual void SetLocalRotationQuaternion([[maybe_unused]] const AZ::Quaternion& quaternion) {}

        //! Rotate around the local x-axis for a radian angle.
        //! @param eulerRadianAngle The angle to rotate around the local x-axis.
        virtual void RotateAroundLocalX([[maybe_unused]] float eulerAngleRadian) {}

        //! Rotate around the local y-axis for a radian angle.
        //! @param eulerRadianAngle The angle to rotate around the local y-axis.
        virtual void RotateAroundLocalY([[maybe_unused]] float eulerAngleRadian) {}

        //! Rotate around the local z-axis for a radian angle.
        //! @param eulerRadianAngle The angle to rotate around the local z-axis.
        virtual void RotateAroundLocalZ([[maybe_unused]] float eulerAngleRadian) {}

        //! Get angles in radian for each principle axis around which the local transform is
        //! rotated in the order of x-axis and y-axis and then z-axis.
        //! @return A value of type Vector3 indicating how much in radian is rotated around each principle axis.
        virtual AZ::Vector3 GetLocalRotation() { return AZ::Vector3(FLT_MAX); }

        //! Get the quaternion representing the local rotation.
        //! @return The rotation quaternion in local space.
        virtual AZ::Quaternion GetLocalRotationQuaternion() { return AZ::Quaternion::CreateZero(); }
        //! @}

        //! Scale modifiers
        //! @{
        //! @deprecated Use SetLocalScale()
        //! Scales the entity along the world's axes. The origin of the axes is the entity's position in the world.
        //! @param scale A three-dimensional vector that represents the multipliers with which to scale the entity in world space.
        virtual void SetScale([[maybe_unused]] const AZ::Vector3& scale) {}

        //! @deprecated Use SetLocalScaleX()
        //! Scales the entity along the world's X axis. The origin of the axis is the entity's position in the world.
        //! @param scaleX The multiplier by which to scale the entity along the X axis in world space.
        virtual void SetScaleX([[maybe_unused]] float scaleX) {}

        //! @deprecated Use SetLocalScaleY()
        //! Scales the entity along the world's Y axis. The origin of the axis is the entity's position in the world.
        //! @param scaleY The multiplier by which to scale the entity along the Y axis in world space.
        virtual void SetScaleY([[maybe_unused]] float scaleY) {}

        //! @deprecated Use SetLocalScaleZ()
        //! Scales the entity along the world's Z axis. The origin of the axis is the entity's position in the world.
        //! @param scaleZ The multiplier by which to scale the entity along the Z axis in world space.
        virtual void SetScaleZ([[maybe_unused]] float scaleZ) {}

        //! @deprecated Use GetLocalScale()
        //! Gets the scale of the entity in world space.
        //! @return A three-dimensional vector that represents the scale of the entity in world space.
        virtual AZ::Vector3 GetScale() { return AZ::Vector3(FLT_MAX); }

        //! @deprecated Use GetLocalScale()
        //! Gets the amount by which an entity is scaled along the world's X axis.
        //! @return The amount by which an entity is scaled along the X axis in world space.
        virtual float GetScaleX() { return FLT_MAX; }

        //! @deprecated Use GetLocalScale()
        //! Gets the amount by which an entity is scaled along the world's Y axis.
        //! @return The amount by which an entity is scaled along the Y axis in world space.
        virtual float GetScaleY() { return FLT_MAX; }

        //! @deprecated Use GetLocalScale()
        //! Gets the amount by which an entity is scaled along the world's Z axis.
        //! @return The amount by which an entity is scaled along the Z axis in world space.
        virtual float GetScaleZ() { return FLT_MAX; }

        //! Set local scale of the transform.
        //! @param scale The new scale to set along three local axes.
        virtual void SetLocalScale([[maybe_unused]] const AZ::Vector3& scale) {}

        //! Set local scale of the transform on x-axis.
        //! @param scaleX The new x-axis scale to set.
        virtual void SetLocalScaleX([[maybe_unused]] float scaleX) {}

        //! Set local scale of the transform on y-axis.
        //! @param scaleY The new y-axis scale to set.
        virtual void SetLocalScaleY([[maybe_unused]] float scaleY) {}

        //! Set local scale of the transform on z-axis.
        //! @param scaleZ The new z-axis scale to set.
        virtual void SetLocalScaleZ([[maybe_unused]] float scaleZ) {}

        //! Get the scale value on each axis in local space
        //! @return The scale value of type Vector3 along each axis in local space.
        virtual AZ::Vector3 GetLocalScale() { return AZ::Vector3(FLT_MAX); }

        //! Get the scale value on each axis in world space.
        //! Note the transform will be skewed when it is rotated and has a parent transform scaled, in which
        //! case the returned world-scale from this function will be inaccurate.
        //! @return The scale value of type Vector3 along each axis in world space.
        virtual AZ::Vector3 GetWorldScale() { return AZ::Vector3(FLT_MAX); }
        //! @}

        //! Transform hierarchy
        //! @{
        //! Returns the entityId of the entity's parent.
        //! @return The entityId of the parent. The entityId is invalid if the entity does not have a parent with a valid entityId.
        virtual EntityId GetParentId() { return EntityId(); }

        //! Returns the transform interface of the parent entity.
        //! @return A pointer to the transform interface of the parent.
        //! Returns a null pointer if no parent is set or the parent entity is not currently activated.
        virtual TransformInterface* GetParent() { return nullptr; }

        //! Sets the entity's parent entity and notifies all listeners.
        //! The entity's local transform is moved into the parent entity's space to preserve the entity's world transform.
        //! @param id The ID of the entity to set as the parent.
        virtual void SetParent([[maybe_unused]] EntityId id) {}

        //! Sets the entity's parent entity, moves the transform relative to the parent entity, and notifies all listeners.
        //! This function uses the world transform as a local transform and moves the transform relative to the parent entity.
        //! @param id The ID of the entity to set as the parent.
        virtual void SetParentRelative([[maybe_unused]] EntityId id) {}

        //! Returns the entityIds of the entity's immediate children.
        //! @return A vector that contains the entityIds of the entity's immediate children.
        virtual AZStd::vector<AZ::EntityId> GetChildren() { return AZStd::vector<AZ::EntityId>(); };

        //! Returns the entityIds of all descendants of the entity.
        //! The descendants are the entity's children, the children's children, and so on.
        //! The entityIds are ordered breadth-first.
        //! @return A vector that contains the entityIds of the descendants.
        virtual AZStd::vector<AZ::EntityId> GetAllDescendants() { return AZStd::vector<AZ::EntityId>(); };

        //! Returns the entityId of the entity and all its descendants. The descendants
        //! are the entity's children, the children's children, and so on. The entityIds
        //! are ordered breadth-first and this entity's ID is the first in the list.
        //! @return A vector that contains the entityId of the entity followed by the entity IDs of its descendants.
        virtual AZStd::vector<AZ::EntityId> GetEntityAndAllDescendants() { return AZStd::vector<AZ::EntityId>(); }
        //! @}

        //! Static transforms
        //! @{
        //! Returns whether the transform is static.
        //! A static transform is unmovable and does not respond to requests that would move it.
        //! @return True if the transform is static, false if the transform is movable.
        virtual bool IsStaticTransform() = 0;

        //! Set the transform to isStatic. This is needed to set a layer as static.
        //! A static transform is unmovable and does not respond to requests that would move it.
        virtual void SetIsStaticTransform([[maybe_unused]] bool isStatic) {}
        //! @}
    };

    //! The EBus for requests to position and parent an entity.
    //! The events are defined in the AZ::TransformInterface class.
    using TransformBus = AZ::EBus<TransformInterface>;

    //! @deprecated Use AZ::Event notifications on the main transform interface.
    //! Interface for AZ::TransformNotificationBus, which is the EBus that dispatches transform changes to listeners.
    class TransformNotification
        : public ComponentBus
    {
    public:

        //! Destroys the instance of the class.
        virtual ~TransformNotification() {}

        //! Signals that the local or world transform of the entity changed.
        //! @param local A reference to the new local transform of the entity.
        //! @param world A reference to the new world transform of the entity.
        virtual void OnTransformChanged(const Transform& /*local*/, const Transform& /*world*/) { }

        //! Signals that the static flag on the transform has changed. This should only be needed during editing.
        //! @param isStatic A boolean that indicates whether the transform is static or not.
        virtual void OnStaticChanged( bool /*isStatic*/) { }

        //! Called right before a parent change, to allow listeners to prevent the entity's parent from changing.
        //! @param parentCanChange A reference used to track if the parent can change. A result parameter is used
        //!                        instead of a return value because this is a multi-handler.
        //! @param oldParent The entityId of the old parent. The entityId is invalid if there was no old parent.
        //! @param newParent The entityId of the new parent. The entityId is invalid if there is no new parent.
        virtual void CanParentChange(bool &parentCanChange, EntityId oldParent, EntityId newParent) { (void)parentCanChange; (void)oldParent; (void)newParent; }

        //! Signals that the parent of the entity changed.
        //! To find if an entityId is valid, use AZ::EntityId::IsValid().
        //! @param oldParent The entityId of the old parent. The entityId is invalid if there was no old parent.
        //! @param newParent The entityId of the new parent. The entityId is invalid if there is no new parent.
        virtual void OnParentChanged(EntityId oldParent, EntityId newParent)    { (void)oldParent; (void)newParent; }

        //! Signals that the transform of the parent of the entity is about to change. Some components will need adjusting before this happens.
        //! To find if an entityId is valid, use AZ::EntityId::IsValid().
        //! @param oldTransform The transform of the old parent. 
        //! @param newTransform The transform of the new parent. 
        virtual void OnParentTransformWillChange(AZ::Transform oldTransform, AZ::Transform newTransform) { (void)oldTransform; (void)newTransform; }

        //! Signals that a child was added to the entity.
        //! @param child The entityId of the added child.
        virtual void OnChildAdded(EntityId child) { (void)child; }

        //! Signals that a child was removed from the entity.
        //! @param child The entityId of the removed child.
        virtual void OnChildRemoved(EntityId child) { (void)child; }
    };

    //! The EBus for transform notification events.
    //! The events are defined in the AZ::TransformNotification class.
    using TransformNotificationBus = AZ::EBus<TransformNotification>;

    //! The typeId of game component AzFramework::TransformComponent.
    static const TypeId TransformComponentTypeId = "{22B10178-39B6-4C12-BB37-77DB45FDD3B6}";

    //! The typeId of editor component AzToolsFramework::Components::TransformComponent.
    static const TypeId EditorTransformComponentTypeId = "{27F1E1A1-8D9D-4C3B-BD3A-AFB9762449C0}";

    //! Component configuration for the transform component.
    class TransformConfig
        : public ComponentConfig
    {
    public:
        AZ_RTTI(TransformConfig, "{B3AAB26D-D075-4E2B-9653-9527EE363DF8}", ComponentConfig);
        AZ_CLASS_ALLOCATOR(TransformConfig, SystemAllocator, 0);

        //! Behavior when a parent entity activates.
        //! A parent may activate before or after its children have activated.
        enum class ParentActivationTransformMode : u32
        {
            MaintainOriginalRelativeTransform,  ///< Child will snap to originally-configured parent-relative transform when parent is activated.
            MaintainCurrentWorldTransform,      ///< Child will still follow parent, but will maintain its current world transform when parent is activated.
        };

        //! Constructor with all default values.
        //! Transform is positioned at (0,0,0) with no rotation and scale of 1.
        TransformConfig() = default;

        //! Constructor which sets a 3D transform.
        //! Sets both the local and world transform to the same value.
        //! @param transform The entity's position, rotation, and scale in 3D.
        explicit TransformConfig(const Transform& transform)
            : m_localTransform(transform)
            , m_worldTransform(transform)
        {}

        //! World 3D transform.
        //! This property is used if no parent is assigned, or if the assigned parent entity cannot be found.
        //! This property is ignored if the assigned parent is present.
        Transform m_worldTransform = Transform::Identity();

        //! Local 3D transform, as an offset from the parent entity.
        //! This property is used to offset the entity from its parent's world transform.
        //! Local transform is ignored if no parent is assigned.
        Transform m_localTransform = Transform::Identity();

        //! ID of parent entity.
        //! When the parent entity moves, this transform will follow.
        EntityId m_parentId;

        //! Behavior when the parent entity activates.
        //! A parent entity is not guaranteed to activate before its children.
        //! If a parent entity activates after its child, this property
        //! determines whether the entity maintains its current world transform
        //! or snaps to maintain the local transform as an offset from the parent.
        ParentActivationTransformMode m_parentActivationTransformMode = ParentActivationTransformMode::MaintainOriginalRelativeTransform;

        //! @deprecated
        //! Whether the transform can be synced over the network.
        bool m_netSyncEnabled = true;

        //! @deprecated
        //! Behavior for smoothing of position between network updates.
        InterpolationMode m_interpolatePosition = InterpolationMode::NoInterpolation;

        //! @deprecated
        //! Behavior for smoothing of rotation between network updates.
        InterpolationMode m_interpolateRotation = InterpolationMode::NoInterpolation;

        //! Whether the transform is static.
        //! A static transform will never move.
        bool m_isStatic = false;

        /// @cond EXCLUDE_DOCS

        /// @deprecated Deprecated, access properties directly.
        void SetTransform(const Transform& transform)
        {
            m_localTransform = transform;
            m_worldTransform = transform;
        }

        /// @deprecated Deprecated, access properties directly.
        void SetLocalAndWorldTransform(const Transform& localTransform, const Transform& worldTransform)
        {
            m_localTransform = localTransform;
            m_worldTransform = worldTransform;
        }

        /// @deprecated Deprecated, access property directly.
        const Transform& GetLocalTransform() const { return m_localTransform; }

        /// @deprecated Deprecated, access property directly.
        const Transform& GetWorldTransform() const { return m_worldTransform; }

        /// @endcond
    };

    AZ_TYPE_INFO_SPECIALIZE(TransformConfig::ParentActivationTransformMode, "{03FD8A24-CE8F-4651-A3CC-09F40D36BC2C}");

    /// @cond EXCLUDE_DOCS

    //! Interface for AZ::TransformHierarchyInformationBus, which the transform components
    //! of parent entities use to get their children's entityIds.
    //! Only children of a particular entity connect to this bus because they use the
    //! parent's entityId to connect to the bus.
    class TransformHierarchyInformation
        : public AZ::EBusTraits
    {
    public:

        //! Destroys the instance of the class.
        virtual ~TransformHierarchyInformation() = default;

        //! Overrides the default AZ::EBusTraits address policy so that the bus has 
        //! multiple addresses at which to receive messages. This bus is identified by EntityId.
        //! Messages addressed to an ID are received by handlers connected to that ID.
        static const EBusAddressPolicy AddressPolicy = EBusAddressPolicy::ById;

        //! Overrides the default AZ::EBusTraits ID type so that entityIds are used to access the addresses of the bus.
        typedef EntityId BusIdType;

        //! Gets the entityIds of the parent entity's children.
        //! @param children A vector that contains the entityIds of the children.
        virtual void GatherChildren(AZStd::vector<AZ::EntityId>& /*children*/) {};
    };

    using TransformHierarchyInformationBus = AZ::EBus<TransformHierarchyInformation>;
    /// @endcond
}