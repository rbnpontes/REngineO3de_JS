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

#include "GraphToLua.h"

#include <AzCore/RTTI/BehaviorContextUtilities.h>
#include <AzCore/ScriptCanvas/ScriptCanvasOnDemandNames.h>
#include <AzCore/std/sort.h>
#include <ScriptCanvas/Core/Node.h>
#include <ScriptCanvas/Core/Nodeable.h>
#include <ScriptCanvas/Data/Data.h>
#include <ScriptCanvas/Debugger/ValidationEvents/DataValidation/ScopedDataConnectionEvent.h>
#include <ScriptCanvas/Debugger/ValidationEvents/GraphTranslationValidation/GraphTranslationValidations.h>
#include <ScriptCanvas/Debugger/ValidationEvents/ParsingValidation/ParsingValidations.h>
#include <ScriptCanvas/Execution/Interpreted/ExecutionInterpretedAPI.h>
#include <ScriptCanvas/Grammar/AbstractCodeModel.h>
#include <ScriptCanvas/Grammar/ParsingMetaData.h>
#include <ScriptCanvas/Grammar/ParsingUtilities.h>
#include <ScriptCanvas/Grammar/Primitives.h>
#include <ScriptCanvas/Grammar/PrimitivesExecution.h>

#include "GraphToLuaUtility.h"
#include "TranslationContext.h"
#include "TranslationContextBus.h"

namespace GraphToLuaCpp
{
    AZStd::string ToDependencyTableName(AZStd::string_view fileName)
    {
        return AZStd::string::format("%s%s", ScriptCanvas::Grammar::ToSafeName(fileName).c_str(), ScriptCanvas::Grammar::k_DependencySuffix);
    }

    AZStd::string FileNameToTableName(AZStd::string_view fileName)
    {
        return ScriptCanvas::Grammar::ToSafeName(fileName);
    }
}

namespace ScriptCanvas
{
    namespace Translation
    {
        const static size_t k_DefaultLoopLimit = 1000;

        Configuration CreateLuaConfig(const Grammar::AbstractCodeModel& source)
        {
            Configuration configuration;
            configuration.m_blockCommentClose = "--]]";
            configuration.m_blockCommentOpen = "--[[";
            configuration.m_dependencyDelimiter = "/";
            configuration.m_executionStateName = "executionState";

            configuration.m_executionStateEntityIdName = "m_entityId";
            configuration.m_executionStateEntityIdRef = "executionState:GetEntityId()";
            configuration.m_executionStateReference = source.IsPerEntityDataRequired() ? "self.executionState" : configuration.m_executionStateName;
            configuration.m_executionStateScriptCanvasIdName = "m_scriptCanvasId";
            configuration.m_executionStateScriptCanvasIdRef = "executionState:GetScriptCanvasId()";
            configuration.m_functionBlockClose = "end";
            configuration.m_functionBlockOpen = "";
            configuration.m_lexicalScopeDelimiter = ".";
            configuration.m_lexicalScopeVariable = ".";
            configuration.m_namespaceClose = "}";
            configuration.m_namespaceOpen = "{";
            configuration.m_scopeClose = "do";
            configuration.m_scopeOpen = "end";
            configuration.m_singleLineComment = "--";
            configuration.m_suffix = Grammar::k_internalRuntimeSuffix;
            return configuration;
        }

        CheckConversion::CheckConversion(Writer& writer, Grammar::VariableConstPtr source, const Grammar::ConversionByIndex& conversions, size_t index)
            : m_writer(writer)
            , m_source(source)
            , m_conversions(conversions)
            , m_index(index)
        {
            CheckConversionStringPre(m_writer, m_source, m_conversions, m_index);
        }

        CheckConversion::~CheckConversion()
        {
            CheckConversionStringPost(m_writer, m_source, m_conversions, m_index);
        }

        GraphToLua::GraphToLua(const Grammar::AbstractCodeModel& source)
            : GraphToX(CreateLuaConfig(source), source)
        {
            SystemRequestBus::BroadcastResult(m_systemConfiguration, &SystemRequests::GetSystemComponentConfiguration);
            MarkTranslationStart();
            RequestBus::BroadcastResult(m_context, &RequestTraits::GetTranslationContext);
            AZ_Assert(m_context, "Nothing is possible without the context");

            m_tableName = GraphToLuaCpp::FileNameToTableName(m_model.GetSource().m_name);
            m_tableName += m_configuration.m_suffix;

            m_runtimeInputs.m_executionCharacteristics = m_model.GetRuntimeCharacteristics();

            WriteHeader();
            TranslateDependencies();
            TranslateClassOpen();   
            ParseConstructionInputVariables();
            TranslateBody(BuildConfiguration::Release);
            TranslateBody(BuildConfiguration::Performance);
            TranslateBody(BuildConfiguration::Debug);
            TranslateClassClose();
            MarkTranslationStop();
        }

        const AZStd::string& GraphToLua::FindAbbreviation(AZStd::string_view dependency) const
        {
            return m_context->FindAbbreviation(dependency);
        }

        const AZStd::string& GraphToLua::FindLibrary(AZStd::string_view dependency) const
        {
            return m_context->FindLibrary(dependency);
        }

        AZStd::string_view GraphToLua::GetOperatorString(Grammar::ExecutionTreeConstPtr execution)
        {
            switch (execution->GetSymbol())
            {
            case Grammar::Symbol::OperatorAddition:
                return execution->GetInput(0).m_value->m_datum.GetType() == Data::Type::String() ? " .. " : " + ";
            case Grammar::Symbol::OperatorDivision:
                return " / ";
            case Grammar::Symbol::OperatorMultiplication:
                return " * ";
            case Grammar::Symbol::OperatorSubraction:
                return " - ";
            default:
                AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::UntranslatedArithmetic));
                return "";
            }
        }

        bool GraphToLua::IsCodeConstructable(Grammar::VariableConstPtr value)
        {
            return Data::IsValueType(value->m_datum.GetType())
                || value->m_datum.GetType().GetAZType() == azrtti_typeid<Nodeable>()
                || value->m_datum.GetAsDanger() == nullptr;
                ;
        }

        bool GraphToLua::IsDebugInfoWritten() const
        {
            return m_executionConfig == BuildConfiguration::Debug
                && m_functionBlockConfig == FunctionBlockConfig::Traced;
        }

        GraphToLua::IsNamed GraphToLua::IsInputNamed(Grammar::VariableConstPtr input, Grammar::ExecutionTreeConstPtr execution)
        {
            return input->m_source != execution || input->m_requiresCreationFunction ? IsNamed::Yes : IsNamed::No;
        }
        
        AZStd::pair<GraphToLua::NilCheck, AZStd::string> GraphToLua::IsReturnValueNilCheckRequired(ScriptCanvas::Grammar::ExecutionTreeConstPtr execution)
        {
            if (execution->GetEventType() != ScriptCanvas::EventType::Count)
            {
                auto localOutputPtr = execution->GetLocalOutput();
                if (localOutputPtr && !localOutputPtr->empty())
                {
                    const AZStd::vector<AZStd::pair<const Slot*, Grammar::OutputAssignmentConstPtr>>& localOutput = *localOutputPtr;

                    if (localOutput.size() == 1)
                    {
                        auto& slotOutputPair = localOutput[0];

                        if (IsValueType(slotOutputPair.second->m_source->m_datum.GetType()))
                        {
                            return { NilCheck::Single, Grammar::ToTypeSafeEBusResultName(slotOutputPair.second->m_source->m_datum.GetType()) };
                        }
                    }
                    else
                    {
                        const Grammar::FunctionCallDefaultMetaData* metaData = azrtti_cast<const Grammar::FunctionCallDefaultMetaData*>(execution->GetMetaData().get());
                        if (!metaData)
                        {
                            AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::MetaDataRequiredForEventCall));
                            return { NilCheck::None, "" };
                        }

                        if (metaData->multiReturnType.IsNull())
                        {
                            AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::MetaDataNeedsTupleTypeIdForEventCall));
                            return { NilCheck::None, "" };
                        }

                        return { NilCheck::Multiple, Execution::CreateStringFastFromId(metaData->multiReturnType) };
                    }
                }
            }

            return { NilCheck::None, "" };
        }

        TargetResult GraphToLua::MoveResult()
        {
            TargetResult result;
            result.m_text = m_dotLua.MoveOutput();
            result.m_runtimeInputs = AZStd::move(m_runtimeInputs);
            result.m_debugMap = m_model.m_debugMap;
            result.m_subgraphInterface = m_model.GetInterface();
            result.m_duration = GetTranslationDuration();
            return result;
        }
        
        void GraphToLua::OpenFunctionBlock(Writer& writer)
        {
            writer.Indent();
        }

        void GraphToLua::ParseConstructionInputVariables()
        {
            AZ::SerializeContext* serializeContext = AZ::EntityUtils::GetApplicationSerializeContext();
            AZStd::vector<AZStd::pair<AZ::EntityId, Nodeable*>> nodeablesById;
            AZStd::vector<VariableId> inputVariableIds;
            AZStd::unordered_map<VariableId, Grammar::VariableConstPtr> inputVariablesById;

            for (auto variable : m_model.GetVariables())
            {
                auto constructionRequirement = ParseConstructionRequirement(variable);

                switch (constructionRequirement)
                {
                case VariableConstructionRequirement::None:
                    break;

                case VariableConstructionRequirement::InputEntityId:
                    {
                        m_runtimeInputs.m_entityIds.emplace_back(variable->m_sourceVariableId, *variable->m_datum.GetAs<Data::EntityIDType>());
                    }
                    break;

                case VariableConstructionRequirement::InputNodeable:
                    {
                        // I solemnly swear no harm shall come to the nodeable
                        const Nodeable* nodeableSource = reinterpret_cast<const Nodeable*>(variable->m_datum.GetAsDanger());
                        AZ_Assert(nodeableSource != nullptr, "the must be a raw nodeable held by this pointer");
                        AZ_Assert(azrtti_typeid(nodeableSource) != azrtti_typeid<Nodeable>(), "type problem with nodeable");
                        nodeablesById.push_back({ variable->m_nodeableNodeId, const_cast<Nodeable*>(nodeableSource) });
                    }
                    break;

                case VariableConstructionRequirement::InputVariable:
                    {
                        inputVariableIds.push_back(variable->m_sourceVariableId);
                        inputVariablesById.insert({ variable->m_sourceVariableId, variable });
                        // sort revealed a datum copy issue: type is not preserved, workaround below
                        // m_runtimeInputs.m_variables.emplace_back(variable->m_sourceVariableId, variable->m_datum);
                    }
                    break;

                case VariableConstructionRequirement::Static:
                    m_runtimeInputs.m_staticVariables.push_back({ variable->m_sourceVariableId, variable->m_datum.ToAny() });
                    break;

                default:
                    AZ_Assert(constructionRequirement == VariableConstructionRequirement::None, "new requirement added and not handled");
                    break;
                }
            }

            m_runtimeInputs.m_nodeables.reserve(nodeablesById.size());
            for (auto& idAndNodeable : nodeablesById)
            {
                m_runtimeInputs.m_nodeables.push_back(idAndNodeable.second);
            }

            AZStd::sort(nodeablesById.begin(), nodeablesById.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

            // sort revealed a datum copy issue: type is not preserved, workaround below
            // AZStd::sort(m_runtimeInputs.m_variables.begin(), m_runtimeInputs.m_variables.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
            m_runtimeInputs.m_variables.reserve(inputVariableIds.size());
            AZStd::sort(inputVariableIds.begin(), inputVariableIds.end());
            for (auto variableId : inputVariableIds)
            {
                auto iter = inputVariablesById.find(variableId);
                AZ_Assert(iter != inputVariablesById.end(), "missing variable id from list just constructed");
                m_runtimeInputs.m_variables.emplace_back(iter->first, iter->second->m_datum);
            }

            AZStd::sort(m_runtimeInputs.m_entityIds.begin(), m_runtimeInputs.m_entityIds.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
            AZStd::sort(m_runtimeInputs.m_staticVariables.begin(), m_runtimeInputs.m_staticVariables.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

            auto& staticVariables = ModStaticVariables();
            staticVariables = m_model.ToVariableList(m_runtimeInputs.m_staticVariables);
            auto& staticVariableNames = ModStaticVariablesNames();
            for (auto staticVariable : staticVariables )
            {
                AZStd::string name = m_model.m_graphScope->AddVariableName(AZStd::string::format("s_%sCloneSource", staticVariable->m_name.c_str()));
                staticVariableNames.push_back({ staticVariable, name });
            }
        }

        GraphToLua::VariableConstructionRequirement GraphToLua::ParseConstructionRequirement(Grammar::VariableConstPtr variable)
        {
            if (Grammar::IsEntityIdThatRequiresRuntimeRemap(variable))
            {
                return VariableConstructionRequirement::InputEntityId;
            }
            else if (variable->m_isExposedToConstruction)
            {
                if (variable->m_sourceVariableId.IsValid())
                {
                    return VariableConstructionRequirement::InputVariable;
                }
                else if (variable->m_nodeableNodeId.IsValid())
                {
                    return VariableConstructionRequirement::InputNodeable;
                }
                else
                {
                    AZ_Assert(false, "A member variable in the model has no valid id");
                }
            }
            else if (variable->m_sourceVariableId.IsValid() && !IsCodeConstructable(variable))
            {
                if (m_model.GetVariableScopeMeaning() == Grammar::VariableScopeMeaning::ValueInitialization)
                {
                    return VariableConstructionRequirement::InputVariable;
                }
                else
                {
                    return VariableConstructionRequirement::Static;
                }
            }

            return VariableConstructionRequirement::None;
        }

        AZ::Outcome<TargetResult, ErrorList> GraphToLua::Translate(const Grammar::AbstractCodeModel& model)
        {
            GraphToLua translation(model);

            if (translation.IsSuccessfull())
            {
                return AZ::Success(translation.MoveResult());
            }
            else
            {
                ErrorList errors = { AZStd::string("provide translation failure details") };
                return AZ::Failure(errors);
            }
        }

        void GraphToLua::TranslateBody()
        {
            TranslateStaticInitialization();
            TranslateConstruction();
            TranslateDestruction();
            TranslateExecutionTrees();
        }

        void GraphToLua::TranslateBody(BuildConfiguration configuration)
        {
            m_executionConfig = configuration;

            if (configuration == BuildConfiguration::Release)
            {
                m_dotLua.WriteLine("-- release configuration, no debug information available, no performance markers");
                m_dotLua.WriteLine("if _G.%s then", Grammar::k_InterpretedConfigurationRelease);
                m_dotLua.WriteNewLine();
            }

            TranslateBody();

            if (configuration == BuildConfiguration::Release)
            {
                m_dotLua.WriteNewLine();
                m_dotLua.WriteLine("-- performance configuration, no debug information available, performance markers in place");
                m_dotLua.WriteLine("elseif _G.%s then", Grammar::k_InterpretedConfigurationPerformance);
                m_dotLua.WriteNewLine();

            }
            else if (configuration == BuildConfiguration::Performance)
            {
                m_dotLua.WriteNewLine();
                m_dotLua.WriteLine("-- debug configuration, debug information available upon when tracing is requested, no performance markers in place");
                m_dotLua.WriteLine("else");
                m_dotLua.WriteNewLine();
            }
            else
            {
                m_dotLua.WriteNewLine();
                m_dotLua.WriteLine("-- end debug configuration");
                m_dotLua.WriteLine("end");
            }
        }

        void GraphToLua::TranslateClassClose()
        {
            m_dotLua.WriteNewLine();
            m_dotLua.Write("return %s", m_tableName.data());
        }

        void GraphToLua::TranslateClassOpen()
        {
            m_dotLua.WriteLine("local %s = {}", m_tableName.data());
            m_dotLua.WriteNewLine();
        }

        void GraphToLua::TranslateConstruction()
        {
            if (m_model.IsPerEntityDataRequired())
            {
                TranslateInheritance();
            }
        }

        void GraphToLua::TranslateDependencies()
        {
            const auto& depencies = m_model.GetDependencies();

            for (const auto& dependency : depencies.nativeLibraries)
            {
                if (dependency.size() == 1)
                {
                    auto& library = FindLibrary(dependency[0]);
                    if (!library.empty())
                    {
                        auto& abbreviation = FindAbbreviation(dependency[0]);
                        if (!abbreviation.empty())
                        {
                            m_dotLua.WriteLine("local %s = %s", abbreviation.data(), library.data());
                        }
                    }
                }
            }

            for (const auto& dependency : depencies.userSubgraphs)
            {
                if (dependency.empty())
                {
                    continue;
                }
                if (dependency.size() == 1)
                {
                    m_dotLua.WriteLine("local %s%s = require'%s%s'"
                        , GraphToLuaCpp::ToDependencyTableName(*dependency.begin()).data()
                        , Grammar::k_executionStateVariableName
                        , dependency.begin()->data());
                }
                else
                {
                    auto dependencyIter = dependency.begin();
                    auto dependencySentinel = dependency.end();

                    m_dotLua.Write("local %s = require'%s"
                        , GraphToLuaCpp::ToDependencyTableName(dependency.back()).data()
                        , dependencyIter->data());

                    for (++dependencyIter; dependencyIter != dependencySentinel; ++dependencyIter)
                    {
                        m_dotLua.Write("%s%s", m_configuration.m_dependencyDelimiter.data(), dependencyIter->data());
                    }

                    m_dotLua.WriteLine("'");
                }
            }

            m_dotLua.WriteNewLine();
        }

        void GraphToLua::TranslateDestruction()
        {
        }

        void GraphToLua::TranslateExecutionTreeChildPost(Grammar::ExecutionTreeConstPtr execution, const Grammar::ExecutionChild& /*child*/, size_t index, size_t /*rootIndex*/)
        {
            switch (execution->GetSymbol())
            {
            case Grammar::Symbol::Cycle:
            case Grammar::Symbol::IfCondition:
            case Grammar::Symbol::RandomSwitch:
            case Grammar::Symbol::Switch:
                m_dotLua.Outdent();
                break;

            case Grammar::Symbol::ForEach:
                WriteForEachChildPost(execution, index);
                break;

            case Grammar::Symbol::While:
                if (index == 0)
                {
                    m_dotLua.Outdent();
                    m_dotLua.WriteLineIndented("end");
                }
                break;

                            
            default:
                break;
            }
        }

        void GraphToLua::TranslateExecutionTreeChildPre(Grammar::ExecutionTreeConstPtr execution, const Grammar::ExecutionChild& child, size_t index, size_t /*rootIndex*/)
        {
            const auto symbol = execution->GetSymbol();

            switch (symbol)
            {
            case Grammar::Symbol::ForEach:
                if (index == 0)
                {
                    WriteInfiniteLoopCheckPre(execution);
                    WriteForEachChildPre(execution);
                    WriteInfiniteLoopCheckPost(execution);
                }
                break;

            case Grammar::Symbol::IfCondition:
                if (index == 0)
                {
                    m_dotLua.Indent();
                    WriteDebugInfoOut(execution, 0, "if-true-out");
                }
                else
                {
                    m_dotLua.WriteLineIndented("else");
                    m_dotLua.Indent();
                    WriteDebugInfoOut(execution, 1, "if-false-out");
                }
                
                break;

            case Grammar::Symbol::Cycle:
            case Grammar::Symbol::RandomSwitch:
            case Grammar::Symbol::Switch:
                if (index > 0)
                {
                    m_dotLua.WriteIndented("elseif ");
                }
                else
                {
                    WritePreFirstCaseSwitch(execution, symbol);
                    m_dotLua.WriteIndented("if ");
                }

                WriteConditionalCaseSwitch(execution, symbol, child, index);
                m_dotLua.WriteLine(" then");
                m_dotLua.Indent();
                if (symbol == Grammar::Symbol::Cycle)
                {
                    WriteCycleBegin(execution);
                }
                WriteDebugInfoOut(execution, index, "switch-out TranslateExecutionTreeChildPre");
                break;

            case Grammar::Symbol::While:
                if (index == 0)
                {
                    WriteInfiniteLoopCheckPre(execution);
                    WriteDebugInfoIn(execution, "while-in TranslateExecutionTreeChildPre");
                    m_dotLua.WriteIndented("while ");
                    WriteFunctionCallInput(execution);
                    m_dotLua.WriteLine(" do");
                    m_dotLua.Indent();
                    WriteInfiniteLoopCheckPost(execution);
                }
                break;

            default:
                break;
            }
        }

        void GraphToLua::TranslateExecutionTreeEntry(Grammar::ExecutionTreeConstPtr execution, size_t index)
        {
            TranslateExecutionTreeEntryPre(execution, index);
            TranslateExecutionTreeEntryRecurse(execution, index);
            TranslateExecutionTreeEntryPost(execution, index);
        }

        void GraphToLua::TranslateExecutionTreeEntryPost(Grammar::ExecutionTreeConstPtr execution, size_t /*index*/)
        {
            const auto symbol = execution->GetSymbol();
            switch (execution->GetSymbol())
            {
            case Grammar::Symbol::Cycle:
            case Grammar::Symbol::RandomSwitch:
            case Grammar::Symbol::Switch:
                WriteSwitchEnd(symbol);
                break;

            case Grammar::Symbol::IfCondition:
                m_dotLua.WriteLineIndented("end");
                break;

            default:
                break;
            }
        }

        void GraphToLua::TranslateExecutionTreeEntryPre(Grammar::ExecutionTreeConstPtr execution, size_t /*index*/)
        {
            switch (execution->GetSymbol())
            {
            case Grammar::Symbol::IfCondition:
                // will write if the debug info is valid
                WriteDebugInfoIn(execution, "if-in !prefaced by expression TranslateExecutionTreeEntryPre");
                m_dotLua.WriteIndented("if ");
                WriteFunctionCallInput(execution);
                m_dotLua.WriteLine(" then");
                break;

            default:
                break;
            }
        }

        void GraphToLua::TranslateExecutionTreeEntryRecurse(Grammar::ExecutionTreeConstPtr execution, size_t index)
        {
            switch (execution->GetSymbol())
            {
            case Grammar::Symbol::Break:
                m_dotLua.WriteLineIndented("break");
                break;

            case Grammar::Symbol::UserOut:
                TranslateExecutionTreeUserOutCall(execution);
                break;

            case Grammar::Symbol::CompareEqual:
            case Grammar::Symbol::CompareGreater:
            case Grammar::Symbol::CompareGreaterEqual:
            case Grammar::Symbol::CompareLess:
            case Grammar::Symbol::CompareLessEqual:
            case Grammar::Symbol::CompareNotEqual:
            case Grammar::Symbol::IsNull:
            case Grammar::Symbol::LogicalAND:
            case Grammar::Symbol::LogicalNOT:
            case Grammar::Symbol::LogicalOR:
            case Grammar::Symbol::FunctionCall:
            case Grammar::Symbol::OperatorAddition:
            case Grammar::Symbol::OperatorDivision:
            case Grammar::Symbol::OperatorMultiplication:
            case Grammar::Symbol::OperatorSubraction:
            case Grammar::Symbol::VariableAssignment:
                TranslateExecutionTreeFunctionCall(execution);
                break;

            case Grammar::Symbol::VariableDeclaration:
            {
                auto variable = execution->GetInput(0).m_value;
                m_dotLua.WriteLineIndented("local %s = %s", variable->m_name.data(), ToValueString(variable->m_datum, m_configuration).data());
                break;
            }

            default:
                break;
            }

            for (size_t childIndex = 0; childIndex < execution->GetChildrenCount(); ++childIndex)
            {
                const auto& child = execution->GetChild(childIndex);

                if (child.m_execution && !child.m_execution->IsInternalOut())
                {
                    TranslateExecutionTreeChildPre(execution, child, childIndex, index + 1);
                    TranslateExecutionTreeEntry(child.m_execution, index + 1);
                    TranslateExecutionTreeChildPost(execution, child, childIndex, index + 1);
                }
            }
        }

        void GraphToLua::TranslateExecutionTreeFunctionCall(Grammar::ExecutionTreeConstPtr execution)
        {
            TranslateNodeableOuts(execution);
            WriteDebugInfoIn(execution, "TranslateExecutionTreeFunctionCall begin");
            m_dotLua.WriteIndent();
            WriteLocalOutputInitialization(execution);

            const bool isWrittenOutputPossible = execution->GetChildrenCount() == 1;

            if (isWrittenOutputPossible)
            {
                WriteVariableWrite(execution, execution->GetChild(0).m_output);
            }

            if (IsLogicalExpression(execution))
            {
                WriteLogicalExpression(execution);
                m_dotLua.WriteNewLine();
            }
            else if (IsVariableGet(execution))
            {
                WriteVariableRead(execution->GetInput(0).m_value);
                m_dotLua.WriteNewLine();
            }
            else if (IsVariableSet(execution) || execution->GetSymbol() == Grammar::Symbol::VariableAssignment)
            {
                WriteFunctionCallInput(execution);
                m_dotLua.WriteNewLine();
            }
            else if (IsExecutedPropertyExtraction(execution))
            {
                WriteFunctionCallInput(execution);
                m_dotLua.Write(".");
                m_dotLua.WriteLine(execution->GetExecutedPropertyExtraction()->m_name);
            }
            else if (IsWrittenMathExpression(execution))
            {
                WriteWrittenMathExpression(execution);
                m_dotLua.WriteNewLine();
            }
            else if (IsOperatorArithmetic(execution))
            {
                WriteOperatorArithmetic(execution);
                m_dotLua.WriteNewLine();
            }
            else if (IsEventConnectCall(execution))
            {
                WriteEventConnectCall(execution);
            }
            else if (IsEventDisconnectCall(execution))
            {
                WriteEventDisconnectCall(execution, PostDisconnectAction::SetToNil);
            }
            else
            {
                const bool isNullCheckRequired = Grammar::IsFunctionCallNullCheckRequired(execution);

                if (isNullCheckRequired)
                {
                    WriteFunctionCallNullCheckPre(execution);
                    m_dotLua.WriteIndent();
                }

                WriteFunctionCallOfNode(execution);
                m_dotLua.WriteLine(")");

                if (isNullCheckRequired)
                {
                    WriteFunctionCallNullCheckPost(execution);
                }
            }

            if (isWrittenOutputPossible)
            {
                WriteOnVariableWritten(execution, execution->GetChild(0).m_output);
            }

            WriteOutputAssignments(execution);
            WriteDebugInfoOut(execution, 0, "TranslateExecutionTreeFunctionCall end");
        }

        void GraphToLua::TranslateExecutionTrees()
        {
            if (auto start = m_model.GetStart())
            {
                TranslateFunction(start, IsNamed::Yes);
                m_dotLua.WriteNewLine();
            }

            auto functions = m_model.GetFunctions();

            if (!functions.empty())
            {
                m_dotLua.WriteNewLine();

                for (auto function : functions)
                {
                    TranslateFunction(function, IsNamed::Yes);
                    m_dotLua.WriteNewLine();
                }
            }
        }

        void GraphToLua::TranslateExecutionTreeUserOutCall(Grammar::ExecutionTreeConstPtr execution)
        {
            // \todo revisit with per-entity run time storage that keeps execution out calls
            m_dotLua.WriteIndented("%s(self, \"%s\"", Grammar::k_NodeableCallInterpretedOut, execution->GetName().data());

            if (execution->GetInputCount() > 0)
            {
                m_dotLua.Write(", ");
                WriteFunctionCallInput(execution);
            }

            m_dotLua.WriteLine(")");
        }

        void GraphToLua::TranslateFunction(Grammar::ExecutionTreeConstPtr execution, IsNamed lex)
        {
            // get the signature of the function
            // start a block with the signature
            // translate the block, with the parameter information passed in
            TranslateFunctionDefinition(execution, lex);

            if (m_executionConfig == BuildConfiguration::Debug)
            {
                m_dotLua.Indent();

                if (m_model.IsPerEntityDataRequired())
                {
                    m_dotLua.WriteLineIndented("if %s(%s) then", Grammar::k_DebugIsTracedName, m_configuration.m_executionStateReference.data());
                }
                else
                {
                    m_dotLua.WriteLineIndented("if %s(%s) then", Grammar::k_DebugIsTracedName, m_configuration.m_executionStateName.data());
                }

                TranslateFunctionBlock(execution, FunctionBlockConfig::Traced, lex);
                m_dotLua.WriteLineIndented("else");
            }
            
            TranslateFunctionBlock(execution, FunctionBlockConfig::Ignored, lex);

            if (m_executionConfig == BuildConfiguration::Debug)
            {
                m_dotLua.WriteLineIndented("end");
                m_dotLua.Outdent();
            }

            m_dotLua.WriteIndented("end");
        }

        void GraphToLua::TranslateFunctionBlock(Grammar::ExecutionTreeConstPtr functionBlock, IsNamed /*lex*/)
        {
            ScopedIndent indent(m_dotLua);

            if (m_model.IsPerEntityDataRequired())
            {
                // \todo move to the config
                m_dotLua.WriteLineIndented("local %s = %s", m_configuration.m_executionStateName.data(), m_configuration.m_executionStateReference.data());
            }

            if (functionBlock->IsInfiniteLoopDetectionPoint())
            {
                WriteInfiniteLoopCheckPre(functionBlock);
            }

            WriteDebugInfoOut(functionBlock, 0, "TranslateFunctionBlock begin");
            WriteLocalInputCreation(functionBlock);
            WriteOutputAssignments(functionBlock);
            WriteLocalVariableInitializion(functionBlock);
            WriteReturnValueInitialization(functionBlock);

            if (functionBlock->GetChildrenCount() > 0)
            {
                TranslateExecutionTreeEntry(functionBlock->GetChild(0).m_execution, 0);
            }

            if (functionBlock->IsInfiniteLoopDetectionPoint())
            {
                WriteInfiniteLoopCheckPost(functionBlock);
            }

            WriteReturnStatement(functionBlock);
        }

        void GraphToLua::TranslateFunctionBlock(Grammar::ExecutionTreeConstPtr functionBlock, FunctionBlockConfig config, IsNamed lex)
        {
            if (m_executionConfig == BuildConfiguration::Debug || config == FunctionBlockConfig::Ignored)
            {
                m_functionBlockConfig = config;
                TranslateFunctionBlock(functionBlock, lex);
            }
        }

        void GraphToLua::TranslateFunctionDefinition(Grammar::ExecutionTreeConstPtr execution, IsNamed isNamed)
        {
            m_dotLua.WriteIndented("function");

            if (isNamed == IsNamed::Yes)
            {
                m_dotLua.Write(" %s", m_tableName.data());
                // function TableName
                m_dotLua.Write(m_model.IsPerEntityDataRequired() ? ":" : ".");
                // function TableName. OR function TableName:
                m_dotLua.Write(execution->GetName());
            }

            m_dotLua.Write("(");

            if (execution->GetChildrenCount() > 0)
            {
                const auto& output = execution->GetChild(0).m_output;

                int inputIndex = 0;

                if (isNamed == IsNamed::Yes)
                {
                    if (execution->IsPure())
                    {
                        m_dotLua.Write(Grammar::k_executionStateVariableName);

                        if (!output.empty())
                        {
                            m_dotLua.Write(", ");
                        }
                    }
                    else if (!m_model.IsUserNodeable())
                    {
                        inputIndex = 1;
                    }
                }
                else if (execution->IsPure())
                {
                    m_dotLua.Write(Grammar::k_executionStateVariableName);

                    if (!output.empty())
                    {
                        m_dotLua.Write(", ");
                    }
                }

                // function[name](arg0
                if (inputIndex < output.size())
                {
                    m_dotLua.Write(output[inputIndex].second->m_source->m_name.data());
                    ++inputIndex;
                }

                // function[name](arg0
                for (; inputIndex < output.size(); ++inputIndex)
                {
                    m_dotLua.Write(", %s", output[inputIndex].second->m_source->m_name.data());
                    // function[name](arg0, ..., argN
                }
            }

            m_dotLua.WriteLine(")");
            // function function[name](arg0, ..., argN) end
        }

        void GraphToLua::TranslateEBusEvents(Grammar::EBusHandlingConstPtr ebusHandling, AZStd::string_view leftValue)
        {
            for (const auto& nameAndEventThread : ebusHandling->m_events)
            {
                const AZStd::string& eventName = nameAndEventThread.first;
                const Grammar::ExecutionTreeConstPtr& eventThread = nameAndEventThread.second;

                const bool hasResults = eventThread->HasReturnValues();

                m_dotLua.WriteNewLine();
                m_dotLua.WriteLineIndented("%s(%s.%s, '%s',"
                    , hasResults ? Grammar::k_EBusHandlerHandleEventResultName : Grammar::k_EBusHandlerHandleEventName
                    , leftValue.data()
                    , ebusHandling->m_handlerName.data()
                    , eventThread->GetName().data());

                m_dotLua.Indent();
                TranslateFunction(eventThread, IsNamed::No);
                m_dotLua.WriteLine(")");

                m_dotLua.Outdent();
            }
        }

        void GraphToLua::TranslateEBusHandlerCreation(Grammar::EBusHandlingConstPtr ebusHandling, AZStd::string_view leftValue)
        {
            if (ebusHandling->m_startsConnected)
            {
                if (ebusHandling->m_isAddressed)
                {
                    m_dotLua.WriteIndented("%.*s.%s = %s(%s, '%s', \""
                        , static_cast<int>(leftValue.size()), leftValue.data()
                        , ebusHandling->m_handlerName.data()
                        , Grammar::k_EBusHandlerCreateAndConnectToName
                        , Grammar::k_executionStateVariableName
                        , ebusHandling->m_ebusName.data());

                    m_dotLua.Write(Execution::CreateStringFastFromId(ebusHandling->m_startingAdress->m_datum.GetType().GetAZType()).data());
                    m_dotLua.Write("\", ");
                    m_dotLua.Write(AZStd::string::format("%s.%s", leftValue.data(), ebusHandling->m_startingAdress->m_name.data()));
                    m_dotLua.WriteLine(")");
                }
                else
                {
                    m_dotLua.WriteLineIndented("%.*s.%s = %s(%s, '%s')"
                        , static_cast<int>(leftValue.size()), leftValue.data()
                        , ebusHandling->m_handlerName.data()
                        , Grammar::k_EBusHandlerCreateAndConnectName
                        , Grammar::k_executionStateVariableName
                        , ebusHandling->m_ebusName.data());
                }
            }
            else
            {
                m_dotLua.WriteLineIndented("%.*s.%s = %s(%s, '%s')"
                    , static_cast<int>(leftValue.size()), leftValue.data()
                    , ebusHandling->m_handlerName.data()
                    , Grammar::k_EBusHandlerCreateName
                    , Grammar::k_executionStateVariableName
                    , ebusHandling->m_ebusName.data());
            }
        }

        void GraphToLua::TranslateEBusHandling(AZStd::string_view leftValue)
        {
            const auto& handlingByNode = m_model.GetEBusHandlings();
            for (const auto& eventHandling : handlingByNode)
            {
                TranslateEBusHandlerCreation(eventHandling, leftValue);
                TranslateEBusEvents(eventHandling, leftValue);
                m_dotLua.WriteNewLine();
            }
        }
        
        void GraphToLua::TranslateNodeableParse(AZStd::string_view leftValue)
        {
            for (auto& nodeAndParse : m_model.GetNodeableParse())
            {
                for (auto onInputChange : nodeAndParse->m_onInputChanges)
                {
                    TranslateExecutionTreeFunctionCall(onInputChange);
                }

                for (auto& out : nodeAndParse->m_latents)
                {
                    m_dotLua.WriteNewLine();
                    TranslateNodeableParse(out.second, leftValue);
                }

                if (!nodeAndParse->m_latents.empty())
                {
                    m_dotLua.WriteNewLine();
                }
            }
        }
        
        void GraphToLua::TranslateNodeableParse(Grammar::ExecutionTreeConstPtr nodeable, AZStd::string_view /*leftValue*/)
        {
            TranslateNodeableOut(nodeable);
        }

        void GraphToLua::TranslateInheritance()
        {
            if (m_model.IsUserNodeable())
            {
                // setmetatable(Subgraph, { __index = Nodeable })
                m_dotLua.WriteLine("setmetatable(%s, { __index = %s })", m_tableName.c_str(), Grammar::k_NodeableUserBaseClassName);
                // local SubgraphInstance_MT = { __index = Subgraph }
                m_dotLua.WriteLine("local %s%s = { __index = %s }", m_tableName.c_str(), Grammar::k_MetaTableSuffix, m_tableName.c_str());
            }
            else
            {
                m_dotLua.WriteLine("%s.__index = %s", m_tableName.c_str(), m_tableName.c_str());
            }

            m_dotLua.WriteNewLine();
            m_dotLua.Write("function %s.new(executionState", m_tableName.c_str());
            WriteConstructionInput(IsLeadingCommaRequired::Yes);
            m_dotLua.WriteLine(")");

            OpenFunctionBlock(m_dotLua);
            {
                if (m_model.IsUserNodeable())
                {
                    // local self = OverrideNodeableMetatable(Nodeable(), SubgraphInstance_MT)
                    m_dotLua.WriteLineIndented("local self = %s(%s(%s), %s%s)"
                        , Grammar::k_OverrideNodeableMetatableName
                        , Grammar::k_NodeableUserBaseClassName
                        , Grammar::k_executionStateVariableName
                        , m_tableName.c_str()
                        , Grammar::k_MetaTableSuffix);

                    // initialize outs to no-ops
                    const auto& outKeys = m_model.GetInterface().GetOutKeys();
                    if (!outKeys.empty())
                    {
                        m_dotLua.WriteIndented("%s(self", Grammar::k_InitializeNodeableOutKeys);

                        for (auto& key : outKeys)
                        {
                            m_dotLua.Write(", %u", AZ::u32(key));
                        }

                        m_dotLua.WriteLine(")");
                    }
                }
                else
                {
                    m_dotLua.WriteLineIndented("local self = setmetatable({}, %s)", m_tableName.c_str());
                }

                m_dotLua.WriteLineIndented("self.%s = %s", Grammar::k_executionStateVariableName, Grammar::k_executionStateVariableName);
                TranslateVariableInitialization("self");
                m_dotLua.WriteLineIndented("return self");
            }

            CloseFunctionBlock(m_dotLua);
            m_dotLua.WriteNewLine();
        }

        void GraphToLua::TranslateNodeableOut(Grammar::ExecutionTreeConstPtr execution)
        {
            const auto setExecutionOutName = Grammar::IsUserFunctionDefinition(execution)
                ? Grammar::k_NodeableSetExecutionOutUserSubgraphName
                : execution->HasReturnValues()
                    ? Grammar::k_NodeableSetExecutionOutResultName
                    : Grammar::k_NodeableSetExecutionOutName;

            m_dotLua.WriteLineIndented("%s(self.%s, '%s',"
                , setExecutionOutName
                , execution->GetNodeable()->m_name.data() 
                , execution->GetName().data());

            m_dotLua.Indent();
            TranslateFunction(execution, IsNamed::No);
            m_dotLua.WriteLine(")");

            m_dotLua.Outdent();
        }

        void GraphToLua::TranslateNodeableOuts(Grammar::ExecutionTreeConstPtr execution)
        {
            const auto outs = execution->GetInternalOuts();
            
            for (const auto& out : outs)
            {
                m_dotLua.WriteNewLine();
                TranslateNodeableOut(out);
            }

            if (!outs.empty())
            {
                m_dotLua.WriteNewLine();
            }
        }

        void GraphToLua::TranslateStaticInitialization()
        {
            if (m_runtimeInputs.m_staticVariables.empty())
            {
                return;
            }

            
            m_dotLua.WriteIndented("function %s.%s(self, ", m_tableName.c_str(), Grammar::k_InitializeStaticsName);
            WriteStaticInitializerInput(IsLeadingCommaRequired::No);
            m_dotLua.WriteLine(")");
            m_dotLua.Indent();

            auto& variables = GetStaticVariables();
            auto& staticSource = GetStaticVariablesNames();

            for (size_t i = 0; i < variables.size(); ++i)
            {
                m_dotLua.WriteLineIndented("rawset(self, '%s', %s)", staticSource[i].second.c_str(), variables[i]->m_name.c_str());
            }

            m_dotLua.Outdent();
            m_dotLua.WriteLine("end");
        }

        void GraphToLua::TranslateVariableInitialization(AZStd::string_view leftValue)
        {
            auto& variables = m_model.GetVariables();
            for (auto& variable : variables)
            {
                if (variable->m_isDebugOnly && m_executionConfig != BuildConfiguration::Debug)
                {
                    continue;
                }

                if (variable->m_datum.GetType().GetAZType() == azrtti_typeid<Nodeable>())
                {
                    auto nodeableName = variable->m_name;
                    if (nodeableName.starts_with(Grammar::k_memberNamePrefix))
                    {
                        nodeableName.erase(0, AZStd::string_view(Grammar::k_memberNamePrefix).size());
                    }

                    if (nodeableName.ends_with(Grammar::k_reservedWordProtection))
                    {
                        nodeableName.resize(nodeableName.size() - AZStd::string_view(Grammar::k_reservedWordProtection).size());
                    }

                    if (m_model.IsUserNodeable(variable))
                    {
                        nodeableName = GraphToLuaCpp::ToDependencyTableName(nodeableName);
                    }

                    m_dotLua.WriteLineIndented("%s.%s = %s.new(%s)"
                        , leftValue.data()
                        , variable->m_name.data()
                        , nodeableName.data()
                        , Grammar::k_executionStateVariableName);
                }
                else if (variable->m_isMember)
                {
                    const auto constructionRequirement = ParseConstructionRequirement(variable);

                    if (constructionRequirement == VariableConstructionRequirement::None)
                    {
                        m_dotLua.WriteLineIndented("%s.%s = %s", leftValue.data(), variable->m_name.data(), ToValueString(variable->m_datum, m_configuration).data());
                    }
                    else if (constructionRequirement == VariableConstructionRequirement::InputNodeable)
                    {
                        m_dotLua.WriteLineIndented("%s:InitializeExecutionState(%s)", variable->m_name.data(), Grammar::k_executionStateVariableName);
                        m_dotLua.WriteLineIndented("%s.%s = %s", leftValue.data(), variable->m_name.data(), variable->m_name.data());
                    }
                    else if (constructionRequirement != VariableConstructionRequirement::Static)
                    {
                        m_dotLua.WriteLineIndented("%s.%s = %s", leftValue.data(), variable->m_name.data(), variable->m_name.data());
                    }
                }
            }

            // translate the event handling...initialize to nil, check for nil before disconnecting
            TranslateEBusHandling(leftValue);
            TranslateNodeableParse(leftValue);
        }

        void GraphToLua::WriteConditionalCaseSwitch(Grammar::ExecutionTreeConstPtr execution, Grammar::Symbol symbol, const Grammar::ExecutionChild& child, size_t index)
        {
            if (symbol == Grammar::Symbol::RandomSwitch)
            {
                auto controlValue = execution->GetInput(execution->GetInputCount() - 2).m_value;
                auto weightX = execution->GetInput(execution->GetChildrenCount() + index).m_value->m_name.data();

                m_dotLua.Write("%s <= %s"
                    , controlValue->m_name.data()
                    , weightX);
            }
            else
            {
                WriteFunctionCallInput(execution);
                m_dotLua.Write(" == ");
                m_dotLua.Write(Grammar::SlotNameToIndexString(*child.m_slot));
            }
        }

        void GraphToLua::WriteConstructionInput(IsLeadingCommaRequired commaRequired)
        {
            AZStd::vector<Grammar::VariableConstPtr> constructionArguments = m_model.CombineVariableLists(m_runtimeInputs.m_nodeables, m_runtimeInputs.m_variables, m_runtimeInputs.m_entityIds);

            if (!constructionArguments.empty())
            {
                if (commaRequired == IsLeadingCommaRequired::Yes)
                {
                    m_dotLua.Write(", ");
                }

                m_dotLua.Write(constructionArguments[0]->m_name.data());

                for (size_t i = 1; i < constructionArguments.size(); ++i)
                {
                    m_dotLua.Write(", %s", constructionArguments[i]->m_name.data());
                }
            }
        }

        void GraphToLua::WriteCycleBegin(Grammar::ExecutionTreeConstPtr execution)
        {
            auto variable = execution->GetInput(0).m_value;
            m_dotLua.WriteIndent();
            WriteVariableReference(variable);
            m_dotLua.Write(" = (");
            WriteVariableReference(variable);
            m_dotLua.WriteLine(" + 1) %% %d", execution->GetChildrenCount());
        }

        void GraphToLua::WriteDebugInfoIn(Grammar::ExecutionTreeConstPtr execution, AZStd::string_view reason)
        {
            WriteDebugInfoIn(execution, reason, execution->GetInputCount());
        }

        void GraphToLua::WriteDebugInfoIn(Grammar::ExecutionTreeConstPtr execution, AZStd::string_view reason, size_t inputCountOverride)
        {
            if (IsDebugInfoWritten())
            {
                if (auto* debugIndex = m_model.GetDebugInfoInIndex(execution))
                {
                    if (m_model.GetVariableScopeMeaning() == Grammar::VariableScopeMeaning::ValueInitialization)
                    {
                        m_dotLua.WriteIndented("%s(executionState, %d", Grammar::k_DebugSignalInName, *debugIndex);
                    }
                    else
                    {
                        m_dotLua.WriteIndented("%s(executionState, '%s', %d", Grammar::k_DebugSignalInSubgraphName, m_model.GetSourceString().data(), *debugIndex);
                    }

                    if (execution->GetInputCount() > 0)
                    {
                        m_dotLua.Write(", ");
                        WriteFunctionCallInput(execution, inputCountOverride);
                    }

                    m_dotLua.WriteLine(") -- %s", reason.data());
                }
            }
        }

        void GraphToLua::WriteDebugInfoOut(Grammar::ExecutionTreeConstPtr execution, size_t childIndex, AZStd::string_view reason)
        {
            if (IsDebugInfoWritten())
            {
                if (auto* debugIndex = m_model.GetDebugInfoOutIndex(execution, childIndex))
                {
                    if (m_model.GetVariableScopeMeaning() == Grammar::VariableScopeMeaning::ValueInitialization)
                    {
                        m_dotLua.WriteIndented("%s(executionState, %d", Grammar::k_DebugSignalOutName, *debugIndex);
                    }
                    else
                    {
                        m_dotLua.WriteIndented("%s(executionState, '%s', %d", Grammar::k_DebugSignalOutSubgraphName, m_model.GetSourceString().data(), *debugIndex);
                    }

                    const auto& output = execution->GetChild(childIndex).m_output;

                    for (size_t index(0), sentinel(output.size()); index < sentinel; ++index)
                    {
                        m_dotLua.Write(", ");
                        WriteVariableReference(output[index].second->m_source);
                    }

                    m_dotLua.WriteLine(") -- %s", reason.data());
                }
            }
        }

        void GraphToLua::WriteDebugInfoReturn(Grammar::ExecutionTreeConstPtr execution, AZStd::string_view reason)
        {
            if (IsDebugInfoWritten())
            {
                if (auto* debugIndex = m_model.GetDebugInfoReturnIndex(execution))
                {
                    if (m_model.GetVariableScopeMeaning() == Grammar::VariableScopeMeaning::ValueInitialization)
                    {
                        m_dotLua.WriteIndented("%s(executionState, %d", Grammar::k_DebugSignalReturnName, *debugIndex);
                    }
                    else
                    {
                        m_dotLua.WriteIndented("%s(executionState, '%s', %d", Grammar::k_DebugSignalReturnSubgraphName, m_model.GetSourceString().data(), *debugIndex);
                    }

                    for (size_t index(0), sentinel(execution->GetReturnValueCount()); index < sentinel; ++index)
                    {
                        m_dotLua.Write(", ");
                        WriteVariableReference(execution->GetReturnValue(index).second->m_source);
                    }

                    m_dotLua.WriteLine(") -- %s", reason.data());
                }
            }
        }

        void GraphToLua::WriteDebugInfoVariableChange(Grammar::ExecutionTreeConstPtr execution, Grammar::OutputAssignmentConstPtr output)
        {
            if (IsDebugInfoWritten())
            {
                if (auto* debugIndex = m_model.GetDebugInfoVariableSetIndex(output))
                {
                    if (m_model.GetVariableScopeMeaning() == Grammar::VariableScopeMeaning::ValueInitialization)
                    {
                        m_dotLua.WriteIndented("%s(executionState, %d, ", Grammar::k_DebugVariableChangeName, *debugIndex);
                        WriteVariableReference(output->m_source);
                        m_dotLua.WriteLine(")");
                    }
                    else
                    {
                        m_dotLua.WriteIndented("%s(executionState, '%s', %d, ", Grammar::k_DebugVariableChangeSubgraphName, m_model.GetSourceString().data(), *debugIndex);
                        WriteVariableReference(output->m_source);
                        m_dotLua.WriteLine(")");
                    }
                }

                for (size_t assignmentIndex(0); assignmentIndex < output->m_assignments.size(); ++assignmentIndex)
                {
                    auto& assignment = output->m_assignments[assignmentIndex];

                    if (auto* debugIndex = m_model.GetDebugInfoVariableAssignmentIndex(output, assignmentIndex))
                    {
                        if (m_model.GetVariableScopeMeaning() == Grammar::VariableScopeMeaning::ValueInitialization)
                        {
                            m_dotLua.WriteIndented("%s(executionState, %d, ", Grammar::k_DebugVariableChangeName, *debugIndex);
                            WriteVariableReference(assignment);
                            m_dotLua.WriteLine(")");
                        }
                        else
                        {
                            m_dotLua.WriteIndented("%s(executionState, '%s', %d, ", Grammar::k_DebugVariableChangeSubgraphName, m_model.GetSourceString().data(), *debugIndex);
                            WriteVariableReference(assignment);
                            m_dotLua.WriteLine(")");
                        }
                    }
                }
            }
        }

        void GraphToLua::WriteEventConnectCall(Grammar::ExecutionTreeConstPtr execution)
        {
            auto eventHandling = m_model.GetEventHandling(execution->GetId().m_node);
            if (!eventHandling)
            {
                AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::BadEventHandlingAccounting));
                return;
            }

            WriteEventDisconnectCall(execution, PostDisconnectAction::None);
            m_dotLua.WriteIndent();
            WriteVariableReference(eventHandling->m_handler);
            m_dotLua.Write(" = ");
            WriteEventConnectCaller(execution, eventHandling);
            m_dotLua.WriteLine(":%s(", Grammar::k_AzEventHandlerConnectName);
            m_dotLua.Indent();
            TranslateFunction(eventHandling->m_eventHandlerFunction, IsNamed::No);
            m_dotLua.WriteLine(")");
            m_dotLua.Outdent();
        }

        void GraphToLua::WriteEventConnectCaller(Grammar::ExecutionTreeConstPtr execution, Grammar::EventHandlingConstPtr)
        {
            auto parent = execution->GetParent();
            if (!parent)
            {
                AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::EventNodeConnectMissingParent));
                return;
            }

            if (parent->GetChildrenCount() != 1)
            {
                AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::EventNodeConnectMissingChild));
                return;
            }

            const auto& output = parent->GetChild(0).m_output;
            if (output.size() != 1)
            {
                AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::EventNodeConnectMissingChildOutput));
                return;
            }

            const auto& firstOutput = output[0].second;
            if (firstOutput->m_source->m_source != parent)
            {
                AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::EventNodeConnectMissingChildOutputSourceNotSet));
                return;
            }

            if (firstOutput->m_source->m_isMember)
            {
                AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::EventNodeConnectMissingChildOutputNotLocal));
                return;
            }

            m_dotLua.Write(firstOutput->m_source->m_name.c_str());
        }

        void GraphToLua::WriteEventDisconnectCall(Grammar::ExecutionTreeConstPtr execution, PostDisconnectAction postAction)
        {
            WriteFunctionCallNullCheckPre(execution);
            m_dotLua.WriteIndent();
            WriteFunctionCallOfNode(execution, Grammar::k_AzEventHandlerDisconnectName, 1);
            m_dotLua.WriteLine(")");

            if (postAction == PostDisconnectAction::SetToNil)
            {
                m_dotLua.WriteIndent();
                WriteVariableReference(execution->GetInput(0).m_value);
                m_dotLua.WriteLine(" = nil");
            }

            WriteFunctionCallNullCheckPost(execution);
        }

        void GraphToLua::WriteFunctionCallNamespace(Grammar::ExecutionTreeConstPtr execution)
        {
            const Grammar::LexicalScope lexicalScope = execution->GetNameLexicalScope();

            switch (lexicalScope.m_type)
            {
            case Grammar::LexicalScopeType::Class:
            case Grammar::LexicalScopeType::Namespace:
                if (!lexicalScope.m_namespaces.empty())
                {
                    if (IsUserFunctionCall(execution) && !lexicalScope.m_namespaces.back().empty())
                    {
                        AZStd::string dependencyTableName = GraphToLuaCpp::ToDependencyTableName(lexicalScope.m_namespaces.back());
                        m_dotLua.Write("%s%.*s", dependencyTableName.c_str(),
                            aznumeric_cast<int>(m_configuration.m_lexicalScopeDelimiter.size()), m_configuration.m_lexicalScopeDelimiter.data());
                    }
                    else
                    {
                        const AZStd::string resolvedScope = ResolveScope(lexicalScope.m_namespaces);

                        auto& abbreviation = FindAbbreviation(resolvedScope);

                        if (!abbreviation.empty())
                        {
                            m_dotLua.Write("%s%.*s", abbreviation.c_str(),
                                aznumeric_cast<int>(m_configuration.m_lexicalScopeDelimiter.size()), m_configuration.m_lexicalScopeDelimiter.data());
                        }
                        else if (!resolvedScope.empty())
                        {
                            m_dotLua.Write("%s%.*s", resolvedScope.c_str(),
                                aznumeric_cast<int>(m_configuration.m_lexicalScopeDelimiter.size()), m_configuration.m_lexicalScopeDelimiter.data());
                        }
                    }
                }
                break;

            case Grammar::LexicalScopeType::Variable:
                {
                    WriteFunctionCallInput(execution, 0, IsFormatStringInput::No);
                    m_dotLua.Write(m_configuration.m_lexicalScopeVariable);
                }
                break;

            default:
                break;
            }
        }

        void GraphToLua::WriteFunctionCallNullCheckPost(Grammar::ExecutionTreeConstPtr execution)
        {
            m_dotLua.Outdent();
            m_dotLua.WriteLineIndented("end");
        }

        void GraphToLua::WriteFunctionCallNullCheckPre(Grammar::ExecutionTreeConstPtr execution)
        {
            m_dotLua.Write("if ");
            WriteVariableReference(execution->GetInput(0).m_value);
            m_dotLua.WriteLine(" ~= nil then");
            m_dotLua.Indent();
        }

        void GraphToLua::WriteFunctionCallOfNode(Grammar::ExecutionTreeConstPtr execution, AZStd::string nameOverride, size_t inputOverride)
        {
            const AZStd::string name = nameOverride.empty() ? execution->GetName() : nameOverride;
            if (name.empty())
            {
                AddError(execution, aznew InvalidFunctionCallNameValidation(execution->GetId().m_node->GetEntityId(), execution->GetId().m_slot->GetId()));
                return;
            }

            const auto isNilCheckRequired = IsReturnValueNilCheckRequired(execution);

            if (isNilCheckRequired.first == NilCheck::Single)
            {
                m_dotLua.Write("%s(", isNilCheckRequired.second.data());
            }
            else if (isNilCheckRequired.first == NilCheck::Multiple)
            {
                m_dotLua.Write("%s(", Grammar::k_TypeSafeEBusMultipleResultsName);
            }

            WriteFunctionCallNamespace(execution);

            switch (execution->GetEventType())
            {
            case ScriptCanvas::EventType::Broadcast:
                m_dotLua.Write("Broadcast.%s(", Grammar::ToIdentifier(name).data());
                break;
            case ScriptCanvas::EventType::BroadcastQueue:
                m_dotLua.Write("QueueBroadcast.%s(", Grammar::ToIdentifier(name).data());
                break;
            case ScriptCanvas::EventType::Event:
                m_dotLua.Write("Event.%s(", Grammar::ToIdentifier(name).data());
                break;
            case ScriptCanvas::EventType::EventQueue:
                m_dotLua.Write("QueueEvent.%s(", Grammar::ToIdentifier(name).data());
                break;
            case ScriptCanvas::EventType::Count:
                m_dotLua.Write("%s(", Grammar::ToIdentifier(name).data());
                break;
            default:
                AddError(execution, aznew InvalidFunctionCallNameValidation(execution->GetId().m_node->GetEntityId(), execution->GetId().m_slot->GetId()));
                break;
            }

            WriteFunctionCallInput(execution, inputOverride);

            if (isNilCheckRequired.first == NilCheck::Single)
            {
                m_dotLua.Write(")");
            }
            else if (isNilCheckRequired.first == NilCheck::Multiple)
            {
                m_dotLua.Write("), \"%s\"", isNilCheckRequired.second.data());
            }
        }

        void GraphToLua::WriteForEachChildPost(Grammar::ExecutionTreeConstPtr execution, size_t index)
        {
            using namespace Grammar;

            if (index != 0)
            {
                return;
            }

            auto meta = AZStd::rtti_pointer_cast<const ForEachMetaData>(execution->GetMetaData());
            // nextFunc(iter)
            m_dotLua.WriteLineIndented("%s(%s)"
                , meta->m_nextFunctionVariableName.c_str()
                , meta->m_iteratorVariableName.c_str());

            m_dotLua.Outdent();
            m_dotLua.WriteLineIndented("end");
        }

        void GraphToLua::WriteForEachChildPre(Grammar::ExecutionTreeConstPtr execution)
        {
            using namespace Grammar;

            WriteDebugInfoIn(execution, "for-each-body WriteForEachChildPre");
            auto meta = AZStd::rtti_pointer_cast<const ForEachMetaData>(execution->GetMetaData());
            auto sourceVariable = execution->GetInput(0).m_value;

            /***
             * \note resist the temptation to put to much of these function calls in the ACM.
             * They do not belong there. The ACM represents the grammar of Script Canvas (and the syntactical sugar of the nodes).
             * The ACM does NOT represent the grammar of the back-ends. Don't let it overly accommodate them.
             */

            // local iter = source:Iterate_VM()
            m_dotLua.WriteIndented("local %s = ", meta->m_iteratorVariableName.c_str());
            WriteVariableReference(sourceVariable);
            m_dotLua.WriteLine(":%s()", AZ::k_iteratorConstructorName);
            
            if (meta->m_isKeyRequired)
            {
                // local getKeyFunc = iter.GetKey
                m_dotLua.WriteLineIndented("local %s = %s.%s"
                    , meta->m_keyFunctionVariableName.c_str()
                    , meta->m_iteratorVariableName.c_str()
                    , AZ::k_iteratorGetKeyName);
            }

            // local getValueFunc = iter.GetValue
            m_dotLua.WriteLineIndented("local %s = %s.%s"
                , meta->m_valueFunctionVariableName.c_str()
                , meta->m_iteratorVariableName.c_str()
                , AZ::k_iteratorModValueName);

            // local isNotAtEndFunc = iter.IsNotAtEnd
            m_dotLua.WriteLineIndented("local %s = %s.%s"
                , meta->m_isNotAtEndFunctionVariableName.c_str()
                , meta->m_iteratorVariableName.c_str()
                , AZ::k_iteratorIsNotAtEndName);

            // local nextFunc = iter.Next
            m_dotLua.WriteLineIndented("local %s = %s.%s"
                , meta->m_nextFunctionVariableName.c_str()
                , meta->m_iteratorVariableName.c_str()
                , AZ::k_iteratorNextName);

            // while isNotAtEndFunc(iter) do
            m_dotLua.WriteLineIndented("while %s(%s) do"
                , meta->m_isNotAtEndFunctionVariableName.c_str()
                , meta->m_iteratorVariableName.c_str());

            m_dotLua.Indent();
        }

        void GraphToLua::WriteFunctionCallInput(Grammar::ExecutionTreeConstPtr execution)
        {
            WriteFunctionCallInput(execution, execution->GetInputCount());
        }

        void GraphToLua::WriteFunctionCallInput(Grammar::ExecutionTreeConstPtr execution, size_t inputCountOverride)
        {
            const size_t inputCount = execution->GetInputCount();
            const size_t inputMax = AZStd::min(inputCount, inputCountOverride != inputCount ? inputCountOverride : inputCount);
            const size_t startingIndex = WriteFunctionCallInputThisPointer(execution);

            if (startingIndex < inputMax)
            {
                if (startingIndex > 0)
                {
                    m_dotLua.Write(", ");
                }

                const IsFormatStringInput convertToStrings = (execution->GetId().m_node && execution->GetId().m_node->ConvertsInputToStrings()) ? IsFormatStringInput::Yes : IsFormatStringInput::No;
                WriteFunctionCallInput(execution, startingIndex, convertToStrings);

                for (size_t i = startingIndex + 1; i < inputMax; ++i)
                {
                    m_dotLua.Write(", ");
                    WriteFunctionCallInput(execution, i, convertToStrings);
                }
            }
        }
        
        void GraphToLua::WriteFunctionCallInput(Grammar::ExecutionTreeConstPtr execution, size_t index, IsFormatStringInput isformatStingInput)
        {
            auto canWriteValue = [&](auto isformatStingInput, auto input)->bool
            {
                return isformatStingInput == IsFormatStringInput::No
                    || input->m_datum.GetType() == Data::Type::Number()
                    || input->m_datum.GetType() == Data::Type::String();
            };
            
            const auto isNamed = IsInputNamed(execution->GetInput(index).m_value, execution);
            auto& input = execution->GetInput(index).m_value;

            if (isNamed == IsNamed::No)
            {
                // just write the value
                if (canWriteValue(isformatStingInput, input))
                {
                    CheckConversion converter(m_dotLua, execution->GetInput(index).m_value, execution->GetConversions(), index);
                    m_dotLua.Write(ToValueString(input->m_datum, m_configuration));
                }
                else
                {
                    if (!input->m_datum.Empty())
                    {
                        m_dotLua.Write("tostring(");
                        CheckConversion converter(m_dotLua, execution->GetInput(index).m_value, execution->GetConversions(), index);
                        m_dotLua.Write("%s)", ToValueString(input->m_datum, m_configuration).data());
                    }
                    else
                    {
                        m_dotLua.Write("''");
                    }
                }
            }
            else 
            {
                // write the by name reference
                if (canWriteValue(isformatStingInput, input))
                {
                    CheckConversion converter(m_dotLua, execution->GetInput(index).m_value, execution->GetConversions(), index);
                    WriteVariableReference(input);
                }
                else
                {
                    m_dotLua.Write("tostring(");
                    CheckConversion converter(m_dotLua, execution->GetInput(index).m_value, execution->GetConversions(), index);
                    WriteVariableReference(input);
                    m_dotLua.Write(")");
                }
            }
        }

        size_t GraphToLua::WriteFunctionCallInputThisPointer(Grammar::ExecutionTreeConstPtr execution)
        {
            if (IsUserFunctionCallPure(execution))
            {
                m_dotLua.Write("%s", Grammar::k_executionStateVariableName);

                if (execution->GetInputCount() > 0)
                {
                    m_dotLua.Write(", ");
                }
            }
            else if (execution->GetId().m_node)
            {
                const auto eventHandlingType = Grammar::CheckEventHandlingType(execution);
                if (eventHandlingType == Grammar::EventHandingType::Event || eventHandlingType == Grammar::EventHandingType::EBus)
                {
                    if (eventHandlingType == Grammar::EventHandingType::EBus)
                    {
                        m_dotLua.Write("self.%s", execution->GetInput(0).m_value->m_datum.GetAs<AZStd::string>()->data());

                        if (execution->GetInputCount() > 1)
                        {
                            // the address is supplied, Lua needs to know the type, the value will be written right after this call
                            auto azType = execution->GetInput(1).m_value->m_datum.GetType().GetAZType();
                            m_dotLua.Write(", \'%s\'", Execution::CreateStringFastFromId(azType).data());
                        }
                    }
                    else
                    {
                        m_dotLua.Write("self.%s", execution->GetInput(0).m_value->m_name.c_str());
                    }
                    
                    return 1;
                }
            }

            return 0;
        }
        
        void GraphToLua::WriteHeader()
        {
            // no one will ever the see header or the do not modify, so these will not be necessary
            // WriteCopyright(m_dotLua);
            // WriteDoNotModify(m_dotLua);
            // m_dotLua.WriteNewLine();
            
            // m_dotLua.WriteLine("local sch = ScriptCanvasHost");
            // m_dotLua.WriteNewLine();
        }

        void GraphToLua::WriteInfiniteLoopCheckPost(Grammar::ExecutionTreeConstPtr execution)
        {
            if (m_executionConfig == BuildConfiguration::Debug)
            {
                if (auto controlVariable = m_model.GetImplicitVariable(execution))
                {
                    if (controlVariable->m_isMember)
                    {
                        const char* variableName = controlVariable->m_name.data();
                        m_dotLua.WriteLineIndented("self.%s = self.%s - 1", variableName, variableName);
                    }
                    else
                    {
                        const auto loopLimit = m_systemConfiguration.m_maxIterationsForInfiniteLoopDetection;
                        const char* variableName = controlVariable->m_name.data();
                        m_dotLua.WriteLineIndented("if %s < %d then", variableName, loopLimit > 0 ? loopLimit : k_DefaultLoopLimit);
                        m_dotLua.Indent();
                        m_dotLua.WriteLineIndented("%s = %s + 1", variableName, variableName);
                        m_dotLua.Outdent();
                        m_dotLua.WriteLineIndented("else");
                        m_dotLua.Indent();
                        m_dotLua.WriteLineIndented("%s(executionState, \"%s: Hit runtime loop limit in block: %s, symbol: %s\")"
                            , Grammar::k_DebugRuntimeErrorName
                            , m_tableName.c_str()
                            , execution->GetName().c_str()
                            , Grammar::GetSymbolName(execution->GetSymbol()));
                        m_dotLua.Outdent();
                        m_dotLua.WriteLineIndented("end");
                    }
                }
                else
                {
                    AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::MissingInfiniteLoopDetectionVariable));
                }
            }
        }

        void GraphToLua::WriteInfiniteLoopCheckPre(Grammar::ExecutionTreeConstPtr execution)
        {
            if (m_executionConfig == BuildConfiguration::Debug)
            {
                if (auto controlVariable = m_model.GetImplicitVariable(execution))
                {
                    if (controlVariable->m_isMember)
                    {
                        const auto loopLimit = m_systemConfiguration.m_maxHandlerStackDepth;
                        const char* variableName = controlVariable->m_name.data();
                        m_dotLua.WriteLineIndented("if self.%s < %d then", variableName, loopLimit > 0 ? loopLimit : k_DefaultLoopLimit);
                        m_dotLua.Indent();
                        m_dotLua.WriteLineIndented("self.%s = self.%s + 1", variableName, variableName);
                        m_dotLua.Outdent();
                        m_dotLua.WriteLineIndented("else");
                        m_dotLua.Indent();
                        m_dotLua.WriteLineIndented("%s(executionState, \"%s: Hit max handler stack depth in %s: \")", Grammar::k_DebugRuntimeErrorName, m_tableName.c_str(), execution->GetRoot()->GetName().c_str());
                        m_dotLua.Outdent();
                        m_dotLua.WriteLineIndented("end");
                    }
                    else
                    {
                        m_dotLua.WriteLineIndented("local %s = 0", controlVariable->m_name.data());
                    }
                }
                else
                {
                    AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::MissingInfiniteLoopDetectionVariable));
                }
            }
        }

        void GraphToLua::WriteLocalInputCreation(Grammar::ExecutionTreeConstPtr execution)
        {
            if (m_model.GetVariableScopeMeaning() == Grammar::VariableScopeMeaning::FunctionPrototype)
            {
                const auto& staticVariables = GetStaticVariablesNames();

                for (const auto& intitializer : staticVariables)
                {
                    AZ_Assert(!IsCodeConstructable(intitializer.first), "parsing of local input creation requirement failed");
                    m_dotLua.WriteLineIndented("local %s = %s(rawget(%s, '%s'))", intitializer.first->m_name.c_str(), Grammar::k_CloneSourceFunctionName, m_tableName.c_str(), intitializer.second.c_str());
                }
            }
        }

        void GraphToLua::WriteLocalOutputInitialization(Grammar::ExecutionTreeConstPtr execution)
        {
            if (auto output = execution->GetLocalOutput())
            {
                if (output->size() <= 1)
                {
                    // it will be done one line
                    return;
                }

                for (const auto& outputIter : *output)
                {
                    if (outputIter.second->m_source->m_source == execution)
                    {
                        // until a need arises, don't bother initializing at start, wait for the first assignment
                        // m_dotLua.WriteLine("local %s = %s", output->m_source->m_name.data(), ToValueString(output->m_source->m_datum, m_configuration).data());
                        m_dotLua.WriteLine("local %s", outputIter.second->m_source->m_name.data());
                        m_dotLua.WriteIndent();
                    }
                }

                if (!Grammar::IsUserFunctionCall(execution))
                {
                    m_dotLua.WriteLine("local %s", AddMultiReturnName().data());
                    m_dotLua.WriteIndent();
                }
            }
        }

        void GraphToLua::WriteLocalVariableInitializion(Grammar::ExecutionTreeConstPtr execution)
        {
            if (execution->GetSymbol() == Grammar::Symbol::FunctionDefinition && m_model.GetVariableScopeMeaning() == Grammar::VariableScopeMeaning::FunctionPrototype)
            {
                for (size_t index(0); index < execution->GetInputCount(); ++index)
                {
                    if (auto variable = execution->GetInput(index).m_value)
                    {
                        if (variable->m_source == execution)
                        {
                            m_dotLua.WriteLineIndented("local %s = %s", variable->m_name.data(), ToValueString(variable->m_datum, m_configuration).data());
                        }
                    }
                    else
                    {
                        AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::MissingInputVariableInFunctionDefinition));
                    }
                }

                auto localVariables = m_model.GetLocalVariablesUser();
                for (const auto& variable : localVariables)
                {
                    if (IsCodeConstructable(variable) && !variable->m_isExposedToConstruction)
                    {
                        m_dotLua.WriteLineIndented("local %s = %s", variable->m_name.data(), ToValueString(variable->m_datum, m_configuration).data());
                    }
                }
            }
        }

        void GraphToLua::WriteFloatingPointErrorNumberEqualityComparision(Grammar::ExecutionTreeConstPtr execution)
        {
            m_dotLua.Write("math.abs(");
            WriteFunctionCallInput(execution, 0, IsFormatStringInput::No);
            m_dotLua.Write(" - ");
            WriteFunctionCallInput(execution, 1, IsFormatStringInput::No);

            if (execution->GetSymbol() == Grammar::Symbol::CompareEqual)
            {
                // math.abs(lhs - rhs) <= 0.000001
                m_dotLua.Write(") <= %s", Grammar::k_LuaEpsilonString);
            }
            else
            {
                // math.abs(lhs - rhs) > 0.000001
                m_dotLua.Write(") > %s", Grammar::k_LuaEpsilonString);
            }
        }

        void GraphToLua::WriteLogicalExpression(Grammar::ExecutionTreeConstPtr execution)
        {
            if (execution->GetSymbol() == Grammar::Symbol::IsNull)
            {
                WriteFunctionCallInput(execution, 0, IsFormatStringInput::No);
                m_dotLua.Write(" == nil ");
            }
            else if (execution->GetSymbol() == Grammar::Symbol::LogicalNOT)
            {
                m_dotLua.Write("not ");
                WriteFunctionCallInput(execution, 0, IsFormatStringInput::No);
            }
            else
            {
                if (Grammar::IsFloatingPointNumberEqualityComparison(execution))
                {
                    WriteFloatingPointErrorNumberEqualityComparision(execution);
                }
                else
                {
                    WriteFunctionCallInput(execution, 0, IsFormatStringInput::No);

                    switch (execution->GetSymbol())
                    {
                    case Grammar::Symbol::CompareEqual:
                        m_dotLua.Write(" == ");
                        break;
                    case Grammar::Symbol::CompareGreater:
                        m_dotLua.Write(" > ");
                        break;
                    case Grammar::Symbol::CompareGreaterEqual:
                        m_dotLua.Write(" >= ");
                        break;
                    case Grammar::Symbol::CompareLess:
                        m_dotLua.Write(" < ");
                        break;
                    case Grammar::Symbol::CompareLessEqual:
                        m_dotLua.Write(" <= ");
                        break;
                    case Grammar::Symbol::CompareNotEqual:
                        m_dotLua.Write(" ~= ");
                        break;
                    case Grammar::Symbol::LogicalAND:
                        m_dotLua.Write(" and ");
                        break;
                    case Grammar::Symbol::LogicalOR:
                        m_dotLua.Write(" or ");
                        break;
                    default:
                        break;
                    }

                    WriteFunctionCallInput(execution, 1, IsFormatStringInput::No);
                }
            }
        }

        void GraphToLua::WritePreFirstCaseSwitch(Grammar::ExecutionTreeConstPtr execution, Grammar::Symbol symbol)
        {
            m_dotLua.WriteLineIndented("--[[ begin switch for Grammar::%s --]]", Grammar::GetSymbolName(symbol));
            if (symbol == Grammar::Symbol::RandomSwitch)
            {
                size_t randomCount = execution->GetChildrenCount();
                auto controlValue = execution->GetInput(execution->GetInputCount() - 2).m_value;
                auto runningTotalName = execution->GetInput(execution->GetInputCount() - 1).m_value->m_name;

                // local runningTotal = 0
                m_dotLua.WriteLineIndented("local %s = 0", runningTotalName.data());

                for (size_t index = 0; index < randomCount; ++index)
                {
                    auto weightX = execution->GetInput(randomCount + index).m_value->m_name.data();
                    // local weightX = runningTotal + inputX 
                    m_dotLua.WriteIndented("local %s = %s + "
                        , weightX
                        , runningTotalName.data());
                    WriteFunctionCallInput(execution, index, IsFormatStringInput::No);
                    m_dotLua.WriteNewLine();

                    // runningTotal = weightX
                    m_dotLua.WriteLineIndented("%s = %s", runningTotalName.data(), weightX);
                }

                // local switchControl = RandomSwitchControlNumberFunctionName(runningTotal)
                m_dotLua.WriteLineIndented("local %s = %s(%s)"
                    , controlValue->m_name.data()
                    , Grammar::k_GetRandomSwitchControlNumberName
                    , runningTotalName.data());

                WriteDebugInfoIn(execution, "random-in TranslateExecutionTreeChildPre", randomCount);
            }
            else
            {
                WriteDebugInfoIn(execution, "switch-in TranslateExecutionTreeChildPre");
            }
        }

        void GraphToLua::WriteOnVariableWritten(Grammar::ExecutionTreeConstPtr execution, const AZStd::vector<AZStd::pair<const Slot*, Grammar::OutputAssignmentConstPtr>>& output)
        {
            if (!output.empty())
            {
                auto firstOutput = output[0].second;

                if (!(output.size() == 1 && firstOutput->m_source->m_source == execution))
                {
                    if (output.size() > 1)
                    {
                        if (!Grammar::IsUserFunctionCall(execution))
                        {
                            const auto& multiReturnName = GetMultiReturnName();
                            for (int i = 0; i < output.size(); ++i)
                            {
                                m_dotLua.WriteIndent();

                                if (output[i].second->m_source->m_isMember && !m_model.GetVariableHandling(output[i].second->m_source).empty())
                                {
                                    // variable required inequality check
                                    m_dotLua.Write("local %s_Copy", output[i].second->m_source->m_name.data());
                                }
                                else
                                {
                                    WriteVariableReference(output[i].second->m_source);
                                }

                                m_dotLua.WriteLine(" = %s:Get%d()", multiReturnName.data(), i);

                                WriteOnVariableWritten(output[i].second->m_source);
                            }
                        }
                    }
                    else
                    {
                        WriteOnVariableWritten(firstOutput->m_source);
                    }
                }
            }
        }

        bool GraphToLua::WriteOnVariableWritten(Grammar::VariableConstPtr variable)
        {
            auto variableHandlingSet = m_model.GetVariableHandling(variable);
            if (variableHandlingSet.empty())
            {
                return false;
            }

            if (variable->m_isMember)
            {
                m_dotLua.WriteLineIndented("if self.%s ~= %s_Copy then", variable->m_name.data(), variable->m_name.data());
                m_dotLua.Indent();
                m_dotLua.WriteLineIndented("self.%s = %s_Copy", variable->m_name.data(), variable->m_name.data());
            }

            for (auto handling : variableHandlingSet)
            {
                const bool requiresConnectionControl = handling->RequiresConnectionControl();
                if (requiresConnectionControl)
                {
                    // \todo handle pure here execution if it ever comes up
                    m_dotLua.WriteLineIndented("if self.%s then", handling->m_connectionVariable->m_name.data());
                    m_dotLua.Indent();
                }

                m_dotLua.WriteLineIndented("self:%s()", handling->m_function->GetName().data());

                if (requiresConnectionControl)
                {
                    m_dotLua.Outdent();
                    m_dotLua.WriteLineIndented("end");
                }
            }

            if (variable->m_isMember)
            {
                m_dotLua.Outdent();
                m_dotLua.WriteLineIndented("end");
            }

            return true;
        }

        void GraphToLua::WriteOperatorArithmetic(Grammar::ExecutionTreeConstPtr execution)
        {
            // \todo write safety check code for all divisors against near zero division

            const auto count = execution->GetInputCount();

            if (count < 2)
            {
                AddError(execution, aznew Internal::ParseError(execution->GetNodeId(), ParseErrors::NotEnoughInputForArithmeticOperator));
                return;
            }

            const AZStd::string_view operatorString = GetOperatorString(execution);

            for (size_t i(0); i < (count - 1); ++i)
            {
                m_dotLua.Write("(");
            }

            // write operand 0 + operand 1
            WriteFunctionCallInput(execution, 0, IsFormatStringInput::No);
            m_dotLua.Write(operatorString);
            WriteFunctionCallInput(execution, 1, IsFormatStringInput::No);
            m_dotLua.Write(")");

            for (size_t i(2); i < count; ++i)
            {
                m_dotLua.Write(operatorString);
                WriteFunctionCallInput(execution, i, IsFormatStringInput::No);
                m_dotLua.Write(")");
            }
        }

        void GraphToLua::WriteOutputAssignments(Grammar::ExecutionTreeConstPtr execution)
        {
            if (const auto output = execution->GetLocalOutput())
            {
                WriteOutputAssignments(execution, *output);
            }
        }

        void GraphToLua::WriteOutputAssignments(Grammar::ExecutionTreeConstPtr execution, const AZStd::vector<AZStd::pair<const Slot*, Grammar::OutputAssignmentConstPtr>>& output)
        {
            for (auto outputIter : output)
            {
                for (size_t i(0); i < outputIter.second->m_assignments.size(); ++i)
                {
                    auto& assignment = outputIter.second->m_assignments[i];
                    const bool variableRequiresInequalityCheck = assignment->m_isMember && !m_model.GetVariableHandling(assignment).empty();
                    m_dotLua.WriteIndent();
                    if (variableRequiresInequalityCheck)
                    {
                        m_dotLua.Write("local %s_Copy = ", assignment->m_name.data());
                    }
                    else
                    {
                        WriteVariableReference(assignment);
                        m_dotLua.Write(" = ");
                    }
                    WriteVariableReadConvertible(outputIter.second->m_sourceConversions, i, outputIter.second->m_source);
                    m_dotLua.WriteNewLine();
                    WriteOnVariableWritten(assignment);
                }

                WriteDebugInfoVariableChange(execution, outputIter.second);
            }
        }

        void GraphToLua::WriteReturnStatement(Grammar::ExecutionTreeConstPtr execution)
        {
            if (execution->HasReturnValues() && !execution->ReturnValuesRoutedToOuts())
            {
                WriteDebugInfoReturn(execution, "WriteReturnStatement");
                m_dotLua.WriteIndented("return ");
                WriteVariableReference(execution->GetReturnValue(0).second->m_source);

                for (int i = 1; i < execution->GetReturnValueCount(); ++i)
                {
                    m_dotLua.Write(", ");
                    WriteVariableReference(execution->GetReturnValue(i).second->m_source);
                }

                m_dotLua.WriteNewLine();
            }
        }

        void GraphToLua::WriteReturnValueInitialization(Grammar::ExecutionTreeConstPtr execution)
        {
            if (execution->HasReturnValues())
            {
                for (size_t index(0), sentinel(execution->GetReturnValueCount()); index < sentinel; ++index)
                {
                    const auto& slotAndReturnValue = execution->GetReturnValue(index);
                    const auto& returnValue = slotAndReturnValue.second;

                    if (returnValue->m_isNewValue)
                    {
                        if (returnValue->m_initializationValue)
                        {
                            m_dotLua.WriteIndented("local %s = ", returnValue->m_source->m_name.data());
                            WriteVariableRead(returnValue->m_initializationValue);
                            m_dotLua.WriteNewLine();
                        }
                        else
                        {
                            m_dotLua.WriteLineIndented("local %s = %s", returnValue->m_source->m_name.data(), ToValueString(returnValue->m_source->m_datum, m_configuration).data());
                        }
                    }
                }
            }
        }

        void GraphToLua::WriteSwitchEnd(Grammar::Symbol symbol)
        {
            m_dotLua.WriteLineIndented("else");
            m_dotLua.Indent();
            m_dotLua.WriteLineIndented("--[[ report an error --]]");
            m_dotLua.Outdent();
            m_dotLua.WriteLineIndented("end");
            m_dotLua.WriteLineIndented("--[[ end switch for Grammar::%s --]]", Grammar::GetSymbolName(symbol));
        }

        void GraphToLua::WriteStaticInitializerInput(IsLeadingCommaRequired commaRequired)
        {
            AZ_Assert(!m_runtimeInputs.m_staticVariables.empty(), "don't write static initialization without needing to");
            
            if (commaRequired == IsLeadingCommaRequired::Yes)
            {
                m_dotLua.Write(", ");
            }

            auto& variables = GetStaticVariables();
            m_dotLua.Write(variables[0]->m_name.data());

            for (size_t i = 1; i < variables.size(); ++i)
            {
                m_dotLua.Write(", %s", variables[i]->m_name.data());
            }
        }

        void GraphToLua::WriteVariableRead(Grammar::VariableConstPtr variable)
        {
            AZ_Assert(variable, "non valid variable");
            
            WriteVariableReference(variable);
            
            if (IsReferenceInLuaAndValueInScriptCanvas(variable->m_datum.GetType()))
            {
                m_dotLua.Write(":Clone()");
            }
        }

        void GraphToLua::WriteVariableReadConvertible(const Grammar::ConversionByIndex& conversions, size_t index, Grammar::VariableConstPtr source)
        {
            auto iter = conversions.find(index);
            if (iter != conversions.end())
            {
                CheckConversion converter(m_dotLua, source, conversions, index);
                WriteVariableReference(source);
            }
            else
            {
                WriteVariableRead(source);
            }
        }

        void GraphToLua::WriteVariableReference(Grammar::VariableConstPtr variable)
        {
            AZ_Assert(variable, "non valid variable");
            
            if (variable->m_isMember)
            {
                m_dotLua.Write("self.");
            }

            m_dotLua.Write(variable->m_name.data());
        }

        void GraphToLua::WriteVariableWrite(Grammar::ExecutionTreeConstPtr execution, const AZStd::vector<AZStd::pair<const Slot*, Grammar::OutputAssignmentConstPtr>>& output)
        {
            if (!output.empty())
            {
                if (output.size() > 1)
                {
                    if (Grammar::IsUserFunctionCall(execution))
                    {
                        auto firstOutput = output[0].second;
                        WriteVariableReference(firstOutput->m_source);

                        for (size_t multiReturnIndex(1), sentinel(output.size()); multiReturnIndex < sentinel; ++ multiReturnIndex)
                        {
                            m_dotLua.Write(", ");
                            WriteVariableReference(output[multiReturnIndex].second->m_source);
                        }

                        m_dotLua.Write(" = ");
                    }
                    else
                    {
                        const auto& multiReturnName = GetMultiReturnName();
                        m_dotLua.Write("%s = ", multiReturnName.data());
                    }
                }
                else
                {
                    auto firstOutput = output[0].second;
                    const bool variableRequiresInequalityCheck = firstOutput->m_source->m_isMember && !m_model.GetVariableHandling(firstOutput->m_source).empty();

                    if (firstOutput->m_source->m_source == execution)
                    {
                        AZ_Assert(!firstOutput->m_source->m_isMember, "this should never be true");
                        m_dotLua.Write("local %s = ", firstOutput->m_source->m_name.data());
                    }
                    else if (variableRequiresInequalityCheck)
                    {
                        m_dotLua.Write("local %s_Copy = ", firstOutput->m_source->m_name.data());
                    }
                    else
                    {
                        WriteVariableReference(firstOutput->m_source);
                        m_dotLua.Write(" = ");
                    }
                }
            }
        }

        void GraphToLua::WriteWrittenMathExpression(Grammar::ExecutionTreeConstPtr execution)
        {
            auto meta = AZStd::rtti_pointer_cast<const Grammar::MathExpressionMetaData>(execution->GetMetaData());
            AZ_Assert(meta, "math expression needs meta data");

            size_t inputIndex = 0;

            const size_t inputStringSentinel = meta->m_expressionString.size();
            const auto& inputString = meta->m_expressionString;

            for (size_t inputStringIndex = 0; inputStringIndex < inputStringSentinel; inputStringIndex++)
            {
                auto character = inputString[inputStringIndex];
                if (character == '@')
                {
                    WriteFunctionCallInput(execution, inputIndex, IsFormatStringInput::No);
                    ++inputIndex;
                }
                else
                {
                    AZStd::string oneChar;
                    oneChar.push_back(character);
                    m_dotLua.Write(oneChar);
                }
            }
        }

    } 
} 