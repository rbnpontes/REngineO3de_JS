#include <JavascriptEditorComponent.h>
#include <AzCore/Serialization/EditContext.h>
#include <Javascript/JavascriptBus.h>
namespace Javascript {
    JavascriptEditorComponent::JavascriptEditorComponent() : m_context(0)
    {
    }
    JavascriptEditorComponent::~JavascriptEditorComponent()
    {
        if (m_context)
            JavascriptRequestBus::Broadcast(&JavascriptRequestBus::Events::DestroyContext, GetEntityId());
    }
    void JavascriptEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context)) {
            JavascriptComponent::Reflect(context);

            serializeContext->Class<JavascriptEditorComponent, EditorComponentBase>()
                ->Field("ScriptComponent", &JavascriptEditorComponent::m_scriptComponent)
                ->Field("Script", &JavascriptEditorComponent::m_scriptCode);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext()) {
                editContext->Class<JavascriptEditorComponent>("Javascript", "The Javascript component allows you to add arbitrary Javascript logic to an entity in the form of a Javascript.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::Category, "Scripting")
                    ->DataElement(AZ::Edit::UIHandlers::MultiLineEdit, &JavascriptEditorComponent::m_scriptCode, "Script", "Place any Javascript code to run")
                    ->UIElement(AZ::Edit::UIHandlers::Button, "", "Run Javascript code into Entity")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &JavascriptEditorComponent::RunScriptCode)
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Run");
            }
        }
    }
    void JavascriptEditorComponent::Init()
    {
        if (m_context)
            return;
        JavascriptRequestBus::BroadcastResult(m_context, &JavascriptRequestBus::Events::GetContext, GetEntityId());
    }
    void JavascriptEditorComponent::Activate()
    {
        if (m_context)
            m_context->CallActivate();
    }
    void JavascriptEditorComponent::Deactivate()
    {
        if (m_context)
            m_context->CallDeActivate();
    }
    void JavascriptEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        AZ::SerializeContext* context = 0;
        AZ::ComponentApplicationBus::BroadcastResult(context, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        if (!context) {
            AZ_Error("JavascriptEditorComponent", false, "Can't get serialize context from component application.");
            return;
        }

        JavascriptComponent* component = context->CloneObject(&m_scriptComponent);
        component->m_script = m_scriptCode;
        gameEntity->AddComponent(component);
    }
    void JavascriptEditorComponent::RunScriptCode()
    {
        if (m_context)
            m_context->RunScript(m_scriptCode);
    }
}
