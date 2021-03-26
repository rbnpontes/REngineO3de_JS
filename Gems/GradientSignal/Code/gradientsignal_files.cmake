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
    Source/GradientSignal_precompiled.cpp
    Source/GradientSignal_precompiled.h
    Include/GradientSignal/GradientSampler.h
    Include/GradientSignal/SmoothStep.h
    Include/GradientSignal/ImageAsset.h
    Include/GradientSignal/ImageSettings.h
    Include/GradientSignal/PerlinImprovedNoise.h
    Include/GradientSignal/Util.h
    Include/GradientSignal/GradientImageConversion.h
    Include/GradientSignal/Ebuses/GradientTransformRequestBus.h
    Include/GradientSignal/Ebuses/GradientRequestBus.h
    Include/GradientSignal/Ebuses/GradientPreviewRequestBus.h
    Include/GradientSignal/Ebuses/GradientPreviewContextRequestBus.h
    Include/GradientSignal/Ebuses/SectorDataRequestBus.h
    Include/GradientSignal/Ebuses/RandomGradientRequestBus.h
    Include/GradientSignal/Ebuses/PerlinGradientRequestBus.h
    Include/GradientSignal/Ebuses/DitherGradientRequestBus.h
    Include/GradientSignal/Ebuses/GradientTransformModifierRequestBus.h
    Include/GradientSignal/Ebuses/InvertGradientRequestBus.h
    Include/GradientSignal/Ebuses/LevelsGradientRequestBus.h
    Include/GradientSignal/Ebuses/PosterizeGradientRequestBus.h
    Include/GradientSignal/Ebuses/SmoothStepGradientRequestBus.h
    Include/GradientSignal/Ebuses/ThresholdGradientRequestBus.h
    Include/GradientSignal/Ebuses/ConstantGradientRequestBus.h
    Include/GradientSignal/Ebuses/ImageGradientRequestBus.h
    Include/GradientSignal/Ebuses/MixedGradientRequestBus.h
    Include/GradientSignal/Ebuses/ReferenceGradientRequestBus.h
    Include/GradientSignal/Ebuses/ShapeAreaFalloffGradientRequestBus.h
    Include/GradientSignal/Ebuses/SurfaceAltitudeGradientRequestBus.h
    Include/GradientSignal/Ebuses/SurfaceMaskGradientRequestBus.h
    Include/GradientSignal/Ebuses/SurfaceSlopeGradientRequestBus.h
    Include/GradientSignal/Ebuses/GradientSurfaceDataRequestBus.h
    Include/GradientSignal/Ebuses/SmoothStepRequestBus.h
    Source/Components/ConstantGradientComponent.cpp
    Source/Components/ConstantGradientComponent.h
    Source/Components/DitherGradientComponent.cpp
    Source/Components/DitherGradientComponent.h
    Source/Components/GradientSurfaceDataComponent.cpp
    Source/Components/GradientSurfaceDataComponent.h
    Source/Components/GradientTransformComponent.cpp
    Source/Components/GradientTransformComponent.h
    Source/Components/ImageGradientComponent.cpp
    Source/Components/ImageGradientComponent.h
    Source/Components/InvertGradientComponent.cpp
    Source/Components/InvertGradientComponent.h
    Source/Components/LevelsGradientComponent.cpp
    Source/Components/LevelsGradientComponent.h
    Source/Components/MixedGradientComponent.cpp
    Source/Components/MixedGradientComponent.h
    Source/Components/PerlinGradientComponent.cpp
    Source/Components/PerlinGradientComponent.h
    Source/Components/PosterizeGradientComponent.cpp
    Source/Components/PosterizeGradientComponent.h
    Source/Components/RandomGradientComponent.cpp
    Source/Components/RandomGradientComponent.h
    Source/Components/ReferenceGradientComponent.cpp
    Source/Components/ReferenceGradientComponent.h
    Source/Components/ShapeAreaFalloffGradientComponent.cpp
    Source/Components/ShapeAreaFalloffGradientComponent.h
    Source/Components/SmoothStepGradientComponent.cpp
    Source/Components/SmoothStepGradientComponent.h
    Source/Components/SurfaceAltitudeGradientComponent.cpp
    Source/Components/SurfaceAltitudeGradientComponent.h
    Source/Components/SurfaceMaskGradientComponent.cpp
    Source/Components/SurfaceMaskGradientComponent.h
    Source/Components/SurfaceSlopeGradientComponent.cpp
    Source/Components/SurfaceSlopeGradientComponent.h
    Source/Components/ThresholdGradientComponent.cpp
    Source/Components/ThresholdGradientComponent.h
    Source/GradientSampler.cpp
    Source/GradientSignalSystemComponent.cpp
    Source/GradientSignalSystemComponent.h
    Source/SmoothStep.cpp
    Source/ImageAsset.cpp
    Source/ImageSettings.cpp
    Source/PerlinImprovedNoise.cpp
    Source/Util.cpp
    Source/GradientImageConversion.cpp
)