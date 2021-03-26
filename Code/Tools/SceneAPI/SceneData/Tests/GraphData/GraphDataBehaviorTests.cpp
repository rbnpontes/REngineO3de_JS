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

#include <AzCore/Component/ComponentApplication.h>
#include <AzCore/Math/MathReflection.h>
#include <AzCore/Name/NameDictionary.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/RTTI/ReflectionManager.h>
#include <AzCore/Serialization/Json/RegistrationContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <AzTest/AzTest.h>

#include <SceneAPI/SceneCore/DataTypes/IGraphObject.h>
#include <SceneAPI/SceneCore/DataTypes/GraphData/IMeshData.h>
#include <SceneAPI/SceneData/ReflectionRegistrar.h>
#include <SceneAPI/SceneData/GraphData/MeshData.h>
#include <SceneAPI/SceneData/GraphData/MeshVertexBitangentData.h>
#include <SceneAPI/SceneData/GraphData/MeshVertexColorData.h>
#include <SceneAPI/SceneData/GraphData/MeshVertexTangentData.h>
#include <SceneAPI/SceneData/GraphData/MeshVertexUVData.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace Containers
        {
            struct MockGraphData final
            {
                AZ_TYPE_INFO(MockGraphData, "{06996B36-E204-4ECC-9F3C-3D644B8CAE07}");

                MockGraphData() = default;
                ~MockGraphData() = default;

                static bool FillData(AZStd::any& data)
                {
                    if (data.get_type_info().m_id == azrtti_typeid<AZ::SceneData::GraphData::MeshData>())
                    {
                        auto* meshData = AZStd::any_cast<AZ::SceneData::GraphData::MeshData>(&data);
                        meshData->AddPosition(Vector3{1.0f, 1.1f, 2.2f});
                        meshData->AddPosition(Vector3{2.0f, 2.1f, 3.2f});
                        meshData->AddPosition(Vector3{3.0f, 3.1f, 4.2f});
                        meshData->AddPosition(Vector3{4.0f, 4.1f, 5.2f});
                        meshData->AddNormal(Vector3{0.1f, 0.2f, 0.3f});
                        meshData->AddNormal(Vector3{0.4f, 0.5f, 0.6f});
                        meshData->SetOriginalUnitSizeInMeters(10.0f);
                        meshData->SetSdkMeshIndex(1337);
                        meshData->SetUnitSizeInMeters(0.5f);
                        meshData->SetVertexIndexToControlPointIndexMap(0, 10);
                        meshData->SetVertexIndexToControlPointIndexMap(1, 11);
                        meshData->SetVertexIndexToControlPointIndexMap(2, 12);
                        meshData->SetVertexIndexToControlPointIndexMap(3, 13);
                        meshData->AddFace({0, 1, 2}, 1);
                        meshData->AddFace({3, 4, 5}, 2);
                        meshData->AddFace({6, 7, 8}, 3);
                        return true;
                    }
                    else if (data.get_type_info().m_id == azrtti_typeid<AZ::SceneData::GraphData::MeshVertexColorData>())
                    {
                        auto* colorData = AZStd::any_cast<AZ::SceneData::GraphData::MeshVertexColorData>(&data);
                        colorData->SetCustomName("mesh_vertex_color_data");
                        colorData->AppendColor(AZ::SceneAPI::DataTypes::Color{0.1f, 0.2f, 0.3f, 0.4f});
                        colorData->AppendColor(AZ::SceneAPI::DataTypes::Color{0.5f, 0.6f, 0.7f, 0.8f});
                        return true;
                    }
                    else if (data.get_type_info().m_id == azrtti_typeid<AZ::SceneData::GraphData::MeshVertexUVData>())
                    {
                        auto* uvData = AZStd::any_cast<AZ::SceneData::GraphData::MeshVertexUVData>(&data);
                        uvData->SetCustomName("mesh_vertex_uv_data");
                        uvData->AppendUV(AZ::Vector2{0.1f, 0.2f});
                        uvData->AppendUV(AZ::Vector2{0.3f, 0.4f});
                        return true;
                    }
                    else if (data.get_type_info().m_id == azrtti_typeid<AZ::SceneData::GraphData::MeshVertexBitangentData>())
                    {
                        auto* bitangentData = AZStd::any_cast<AZ::SceneData::GraphData::MeshVertexBitangentData>(&data);
                        bitangentData->AppendBitangent(AZ::Vector3{0.12f, 0.34f, 0.56f});
                        bitangentData->AppendBitangent(AZ::Vector3{0.77f, 0.88f, 0.99f});
                        bitangentData->SetTangentSpace(AZ::SceneAPI::DataTypes::TangentSpace::FromFbx);
                        bitangentData->SetBitangentSetIndex(1);
                        return true;
                    }
                    else if (data.get_type_info().m_id == azrtti_typeid<AZ::SceneData::GraphData::MeshVertexTangentData>())
                    {
                        auto* tangentData = AZStd::any_cast<AZ::SceneData::GraphData::MeshVertexTangentData>(&data);
                        tangentData->AppendTangent(AZ::Vector4{0.12f, 0.34f, 0.56f, 0.78f});
                        tangentData->AppendTangent(AZ::Vector4{0.18f, 0.28f, 0.19f, 0.29f});
                        tangentData->AppendTangent(AZ::Vector4{0.21f, 0.43f, 0.65f, 0.87f});
                        tangentData->SetTangentSpace(AZ::SceneAPI::DataTypes::TangentSpace::EMotionFX);
                        tangentData->SetTangentSetIndex(2);
                        return true;
                    }
                    return false;
                }

                static void Reflect(ReflectContext* context)
                {
                    BehaviorContext* behaviorContext = azrtti_cast<BehaviorContext*>(context);
                    if (behaviorContext)
                    {
                        behaviorContext->Class<MockGraphData>()
                            ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                            ->Attribute(AZ::Script::Attributes::Module, "scene")
                            ->Method("FillData", &MockGraphData::FillData);
                    }
                }
            };

            class GrapDatahBehaviorScriptTest
                : public UnitTest::AllocatorsFixture
            {
            public:
                AZStd::unique_ptr<AZ::ScriptContext> m_scriptContext;
                AZStd::unique_ptr<AZ::BehaviorContext> m_behaviorContext;
                AZStd::unique_ptr<AZ::SerializeContext> m_serializeContext;

                static void TestExpectTrue(bool value)
                {
                    EXPECT_TRUE(value);
                }

                static void TestExpectIntegerEquals(AZ::s64 lhs, AZ::s64 rhs)
                {
                    EXPECT_EQ(lhs, rhs);
                }

                static void TestExpectFloatEquals(float lhs, float rhs)
                {
                    EXPECT_EQ(lhs, rhs);
                }

                void SetUp() override
                {
                    UnitTest::AllocatorsFixture::SetUp();
                    AZ::NameDictionary::Create();

                    m_serializeContext = AZStd::make_unique<AZ::SerializeContext>();
                    AZ::SceneAPI::RegisterDataTypeReflection(m_serializeContext.get());

                    m_behaviorContext = AZStd::make_unique<AZ::BehaviorContext>();
                    m_behaviorContext->Method("TestExpectTrue", &TestExpectTrue);
                    m_behaviorContext->Method("TestExpectIntegerEquals", &TestExpectIntegerEquals);
                    m_behaviorContext->Method("TestExpectFloatEquals", &TestExpectFloatEquals);
                    MockGraphData::Reflect(m_behaviorContext.get());
                    AZ::MathReflect(m_behaviorContext.get());
                    AZ::SceneAPI::RegisterDataTypeBehaviorReflection(m_behaviorContext.get());

                    m_scriptContext = AZStd::make_unique<AZ::ScriptContext>();
                    m_scriptContext->BindTo(m_behaviorContext.get());
                }

                void TearDown() override
                {
                    m_scriptContext.reset();
                    m_serializeContext.reset();
                    m_behaviorContext.reset();

                    AZ::NameDictionary::Destroy();
                    UnitTest::AllocatorsFixture::TearDown();
                }

                void ExpectExecute(AZStd::string_view script)
                {
                    EXPECT_TRUE(m_scriptContext->Execute(script.data()));
                }
            };

            TEST_F(GrapDatahBehaviorScriptTest, SceneGraph_MeshData_AccessWorks)
            {
                ExpectExecute("meshData = MeshData()");
                ExpectExecute("TestExpectTrue(meshData ~= nil)");
                ExpectExecute("MockGraphData.FillData(meshData)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetVertexCount(), 4)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(0).x, 1.0)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(0).y, 1.1)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(0).z, 2.2)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(1).x, 2.0)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(1).y, 2.1)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(1).z, 3.2)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(2).x, 3.0)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(2).y, 3.1)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(2).z, 4.2)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(3).x, 4.0)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(3).y, 4.1)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetPosition(3).z, 5.2)");
                ExpectExecute("TestExpectTrue(meshData:HasNormalData())");
                ExpectExecute("TestExpectFloatEquals(meshData:GetNormal(0).x, 0.1)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetNormal(0).y, 0.2)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetNormal(0).z, 0.3)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetNormal(1).x, 0.4)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetNormal(1).y, 0.5)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetNormal(1).z, 0.6)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetOriginalUnitSizeInMeters(), 10.0)");
                ExpectExecute("TestExpectFloatEquals(meshData:GetUnitSizeInMeters(), 0.5)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetSdkMeshIndex(), 1337)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetUsedControlPointCount(), 4)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetControlPointIndex(0), 10)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetControlPointIndex(1), 11)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetControlPointIndex(2), 12)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetControlPointIndex(3), 13)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetUsedPointIndexForControlPoint(10), 0)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetUsedPointIndexForControlPoint(11), 1)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetUsedPointIndexForControlPoint(12), 2)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetUsedPointIndexForControlPoint(13), 3)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetUsedPointIndexForControlPoint(0), -1)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetFaceCount(), 3)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetVertexIndex(0, 0), 0)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetVertexIndex(0, 1), 1)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetVertexIndex(0, 2), 2)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetVertexIndex(2, 0), 6)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetVertexIndex(2, 1), 7)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetVertexIndex(2, 2), 8)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetFaceMaterialId(2), 3)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetFaceMaterialId(1), 2)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetFaceMaterialId(0), 1)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetFaceInfo(0):GetVertexIndex(0), 0)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetFaceInfo(0):GetVertexIndex(1), 1)");
                ExpectExecute("TestExpectIntegerEquals(meshData:GetFaceInfo(0):GetVertexIndex(2), 2)");
            }

            TEST_F(GrapDatahBehaviorScriptTest, SceneGraph_MeshVertexColorData_AccessWorks)
            {
                ExpectExecute("meshVertexColorData = MeshVertexColorData()");
                ExpectExecute("TestExpectTrue(meshVertexColorData ~= nil)");
                ExpectExecute("MockGraphData.FillData(meshVertexColorData)");
                ExpectExecute("TestExpectTrue(meshVertexColorData:GetCustomName() == 'mesh_vertex_color_data')");
                ExpectExecute("TestExpectIntegerEquals(meshVertexColorData:GetCount(), 2)");
                ExpectExecute("TestExpectFloatEquals(meshVertexColorData:GetColor(0).red, 0.1)");
                ExpectExecute("TestExpectFloatEquals(meshVertexColorData:GetColor(0).green, 0.2)");
                ExpectExecute("TestExpectFloatEquals(meshVertexColorData:GetColor(0).blue, 0.3)");
                ExpectExecute("TestExpectFloatEquals(meshVertexColorData:GetColor(0).alpha, 0.4)");
                ExpectExecute("colorOne = meshVertexColorData:GetColor(1)");
                ExpectExecute("TestExpectFloatEquals(colorOne.red, 0.5)");
                ExpectExecute("TestExpectFloatEquals(colorOne.green, 0.6)");
                ExpectExecute("TestExpectFloatEquals(colorOne.blue, 0.7)");
                ExpectExecute("TestExpectFloatEquals(colorOne.alpha, 0.8)");
            }

            TEST_F(GrapDatahBehaviorScriptTest, SceneGraph_MeshVertexUVData_AccessWorks)
            {
                ExpectExecute("meshVertexUVData = MeshVertexUVData()");
                ExpectExecute("TestExpectTrue(meshVertexUVData ~= nil)");
                ExpectExecute("MockGraphData.FillData(meshVertexUVData)");
                ExpectExecute("TestExpectTrue(meshVertexUVData:GetCustomName() == 'mesh_vertex_uv_data')");
                ExpectExecute("TestExpectIntegerEquals(meshVertexUVData:GetCount(), 2)");
                ExpectExecute("TestExpectFloatEquals(meshVertexUVData:GetUV(0).x, 0.1)");
                ExpectExecute("TestExpectFloatEquals(meshVertexUVData:GetUV(0).y, 0.2)");
                ExpectExecute("uvOne = meshVertexUVData:GetUV(1)");
                ExpectExecute("TestExpectFloatEquals(uvOne.x, 0.3)");
                ExpectExecute("TestExpectFloatEquals(uvOne.y, 0.4)");
            }

            TEST_F(GrapDatahBehaviorScriptTest, SceneGraph_MeshVertexBitangentData_AccessWorks)
            {
                ExpectExecute("meshVertexBitangentData = MeshVertexBitangentData()");
                ExpectExecute("TestExpectTrue(meshVertexBitangentData ~= nil)");
                ExpectExecute("MockGraphData.FillData(meshVertexBitangentData)");
                ExpectExecute("TestExpectIntegerEquals(meshVertexBitangentData:GetCount(), 2)");
                ExpectExecute("TestExpectFloatEquals(meshVertexBitangentData:GetBitangent(0).x, 0.12)");
                ExpectExecute("TestExpectFloatEquals(meshVertexBitangentData:GetBitangent(0).y, 0.34)");
                ExpectExecute("TestExpectFloatEquals(meshVertexBitangentData:GetBitangent(0).z, 0.56)");
                ExpectExecute("bitangentData = meshVertexBitangentData:GetBitangent(1)");
                ExpectExecute("TestExpectFloatEquals(bitangentData.x, 0.77)");
                ExpectExecute("TestExpectFloatEquals(bitangentData.y, 0.88)");
                ExpectExecute("TestExpectFloatEquals(bitangentData.z, 0.99)");
                ExpectExecute("TestExpectIntegerEquals(meshVertexBitangentData:GetBitangentSetIndex(), 1)");
                ExpectExecute("TestExpectTrue(meshVertexBitangentData:GetTangentSpace(), MeshVertexBitangentData.FromFbx)");
            }

            TEST_F(GrapDatahBehaviorScriptTest, SceneGraph_MeshVertexTangentData_AccessWorks)
            {
                ExpectExecute("meshVertexTangentData = MeshVertexTangentData()");
                ExpectExecute("TestExpectTrue(meshVertexTangentData ~= nil)");
                ExpectExecute("MockGraphData.FillData(meshVertexTangentData)");
                ExpectExecute("TestExpectIntegerEquals(meshVertexTangentData:GetCount(), 3)");
                ExpectExecute("TestExpectFloatEquals(meshVertexTangentData:GetTangent(0).x, 0.12)");
                ExpectExecute("TestExpectFloatEquals(meshVertexTangentData:GetTangent(0).y, 0.34)");
                ExpectExecute("TestExpectFloatEquals(meshVertexTangentData:GetTangent(0).z, 0.56)");
                ExpectExecute("TestExpectFloatEquals(meshVertexTangentData:GetTangent(0).w, 0.78)");
                ExpectExecute("tangentData = meshVertexTangentData:GetTangent(1)");
                ExpectExecute("TestExpectFloatEquals(tangentData.x, 0.18)");
                ExpectExecute("TestExpectFloatEquals(tangentData.y, 0.28)");
                ExpectExecute("TestExpectFloatEquals(tangentData.z, 0.19)");
                ExpectExecute("TestExpectFloatEquals(tangentData.w, 0.29)");
                ExpectExecute("TestExpectIntegerEquals(meshVertexTangentData:GetTangentSetIndex(), 2)");
                ExpectExecute("TestExpectTrue(meshVertexTangentData:GetTangentSpace(), MeshVertexTangentData.EMotionFX)");
            }
        }
    }
}