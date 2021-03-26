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

#include <AtomActor.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzFramework/Visibility/BoundsBus.h>

#include <Integration/Rendering/RenderActorInstance.h>

#include <LmbrCentral/Rendering/MeshComponentBus.h>

#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <Atom/Feature/SkinnedMesh/SkinnedMeshFeatureProcessorBus.h>
#include <Atom/Feature/SkinnedMesh/SkinnedMeshRenderProxyInterface.h>
#include <Atom/Feature/SkinnedMesh/SkinnedMeshFeatureProcessorInterface.h>
#include <Atom/Feature/SkinnedMesh/SkinnedMeshInputBuffers.h>
#include <Atom/Feature/SkinnedMesh/SkinnedMeshOutputStreamManagerInterface.h>
#include <Atom/Feature/SkinnedMesh/SkinnedMeshShaderOptions.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/std/smart_ptr/intrusive_base.h>

namespace EMotionFX
{
    class Actor;
    class ActorInstance;
}
namespace AZ::RPI
{
    class Model;
    class Buffer;
}

namespace AZ
{
    namespace Render
    {
        class SkinnedMeshFeatureProcessorInterface;
        class SkinnedMeshInputBuffers;
        class MeshFeatureProcessorInterface;
        class AtomActor;

        //! Render node for managing and rendering actor instances. Each Actor Component
        //! creates an ActorRenderNode. The render node is responsible for drawing meshes and
        //! passing skinning transforms to the skinning pipeline.
        class AtomActorInstance
            : public EMotionFX::Integration::RenderActorInstance
            , public AZ::TransformNotificationBus::Handler
            , public AZ::Render::MaterialReceiverRequestBus::Handler
            , public AzFramework::BoundsRequestBus::Handler
            , public AZ::Render::MaterialComponentNotificationBus::Handler
            , public AZ::Render::MeshComponentRequestBus::Handler
            , public LmbrCentral::MeshComponentRequestBus::Handler
            , private AZ::Render::SkinnedMeshFeatureProcessorNotificationBus::Handler
            , private AZ::Render::SkinnedMeshOutputStreamNotificationBus::Handler
            , private LmbrCentral::SkeletalHierarchyRequestBus::Handler
            , private Data::AssetBus::MultiHandler
        {
        public:
            AZ_CLASS_ALLOCATOR_DECL;
            AZ_RTTI(AZ::Render::AtomActorInstance, "{6C933B44-8D4A-43B0-9F0F-C1932A257ABC}", EMotionFX::Integration::RenderActorInstance)

            AZ_DISABLE_COPY_MOVE(AtomActorInstance);

            AtomActorInstance() = delete;
            AtomActorInstance(AZ::EntityId entityId,
                const EMotionFX::Integration::EMotionFXPtr<EMotionFX::ActorInstance>& actorInstance,
                const AZ::Data::Asset<EMotionFX::Integration::ActorAsset>& asset,
                const AZ::Transform& worldTransform,
                EMotionFX::Integration::SkinningMethod skinningMethod);
            ~AtomActorInstance() override;

            // AtomActorInstanceRequestBusTEMP::Handler interface implementation
            // RenderActorInstance overrides ...
            void OnTick(float timeDelta) override;
            void UpdateBounds() override;
            void DebugDraw(const DebugOptions& debugOptions) override { AZ_UNUSED(debugOptions) };
            void SetMaterials(const EMotionFX::Integration::ActorAsset::MaterialList& materialPerLOD) override { AZ_UNUSED(materialPerLOD); };
            void SetSkinningMethod(EMotionFX::Integration::SkinningMethod emfxSkinningMethod);
            SkinningMethod GetAtomSkinningMethod() const;

            // BoundsRequestBus overrides ...
            AZ::Aabb GetWorldBounds() override;
            AZ::Aabb GetLocalBounds() override;

            AtomActor* GetRenderActor() const;

            /////////////////////////////////////////////


            void Activate();
            void Deactivate();

            // Create all the buffers necessary for a skinned mesh render proxy and mesh render proxy, then register it with Atom
            void Create();
            // Release render proxies and destroy all the buffers
            void Destroy();

            // Acquire render proxies from Atom. Assumes the input buffers already exist.
            void RegisterActor();
            // Release render proxies, but don't destroy the buffers used to create them. Can be used to hide the actor while keeping its resources around for later.
            void UnregisterActor();

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // TransformNotificationBus::Handler overrides...
            void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // MaterialReceiverRequestBus::Handler overrides...
            MaterialAssignmentMap GetMaterialAssignments() const override;
            AZStd::unordered_set<AZ::Name> GetModelUvNames() const override;

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // MaterialComponentNotificationBus::Handler overrides...
            void OnMaterialsUpdated(const MaterialAssignmentMap& materials) override;

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // MeshComponentRequestBus::Handler overrides...
            void SetModelAsset(Data::Asset<RPI::ModelAsset> modelAsset) override;
            const Data::Asset<RPI::ModelAsset>& GetModelAsset() const override;
            void SetModelAssetId(Data::AssetId modelAssetId) override;
            Data::AssetId GetModelAssetId() const override;
            void SetModelAssetPath(const AZStd::string& modelAssetPath) override;
            AZStd::string GetModelAssetPath() const override;
            const AZ::Data::Instance<RPI::Model> GetModel() const override;
            void SetSortKey(RHI::DrawItemSortKey sortKey) override;
            RHI::DrawItemSortKey GetSortKey() const override;
            void SetLodOverride(RPI::Cullable::LodOverride lodOverride) override;
            RPI::Cullable::LodOverride GetLodOverride() const override;
            void SetVisibility(bool visible) override;
            bool GetVisibility() const override;
            // GetWorldBounds/GetLocalBounds already overridden by BoundsRequestBus::Handler

            //////////////////////////////////////////////////////////////////////////
            // LmbrCentral::MeshComponentRequestBus::Handler
            void SetMeshAsset(const AZ::Data::AssetId& id) override;
            AZ::Data::Asset<AZ::Data::AssetData> GetMeshAsset() override;
            bool GetVisibility() override;
            // SetVisibility already overridden by MeshComponentRequestBus::Handler
            // GetWorldBounds/GetLocalBounds already overridden by BoundsRequestBus::Handler

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // SkeletalHierarchyRequestBus::Handler overrides...
            AZ::u32 GetJointCount() override;
            const char* GetJointNameByIndex(AZ::u32 jointIndex) override;
            AZ::s32 GetJointIndexByName(const char* jointName) override;
            AZ::Transform GetJointTransformCharacterRelative(AZ::u32 jointIndex) override;

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Data::AssetBus::MultiHandler::Handler overrides...
            void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // SkinnedMeshFeatureProcessorNotificationBus
            void OnUpdateSkinningMatrices() override;

            void CreateRenderProxy(const MaterialAssignmentMap& materials);

        private:
            void CreateSkinnedMeshInstance();

            // Copies input buffers to output skinned buffers when the skinned mesh instance is created.
            void FillSkinnedMeshInstanceBuffers();

            // SkinnedMeshOutputStreamNotificationBus
            void OnSkinnedMeshOutputStreamMemoryAvailable() override;

            AZStd::intrusive_ptr<AZ::Render::SkinnedMeshInputBuffers> m_skinnedMeshInputBuffers = nullptr;
            AZStd::intrusive_ptr<SkinnedMeshInstance> m_skinnedMeshInstance;
            AZ::Data::Instance<AZ::RPI::Buffer> m_boneTransforms = nullptr;
            AZ::Render::SkinnedMeshRenderProxyInterfaceHandle m_skinnedMeshRenderProxy;
            AZ::Render::SkinnedMeshFeatureProcessorInterface* m_skinnedMeshFeatureProcessor = nullptr;
            AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor = nullptr;
            //m_meshHandle is wrapped in a shared pointer so that it can be shared between this and the SkinnedMeshRenderProxy (the handle itself cannot be copied)
            AZStd::shared_ptr<MeshFeatureProcessorInterface::MeshHandle> m_meshHandle;
            AZ::TransformInterface* m_transformInterface = nullptr;
            AZStd::set<Data::AssetId> m_waitForMaterialLoadIds;
        };

    } // namespace Render
} // namespace AZ