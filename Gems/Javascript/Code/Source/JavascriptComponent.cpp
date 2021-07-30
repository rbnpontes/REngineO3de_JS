#include "JavascriptComponent.h"
#include "JavascriptSystemComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Component/Entity.h>
namespace Javascript {
    JavascriptComponent::JavascriptComponent() :
        m_script(""),
        m_context(0){
    }
    JavascriptComponent::~JavascriptComponent()
    {
        if (m_context)
            delete m_context;
    }
    void JavascriptComponent::Init()
    {
        if (m_context)
            return;
        if (m_script.length() > 0)
            SetScript(m_script);
    }
    void JavascriptComponent::Activate()
    {
        Javascript::JavascriptComponentRequestBus::Handler::BusConnect(GetEntityId());
        if (m_context)
            m_context->CallActivate();
    }
    void JavascriptComponent::Deactivate()
    {
        Javascript::JavascriptComponentRequestBus::Handler::BusDisconnect(GetEntityId());
        if (m_context)
            m_context->CallDeActivate();
    }
    void JavascriptComponent::SetScript(const AZStd::string& script)
    {
        m_script = script;
        UpdateScript();
    }
    void JavascriptComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context)) {
            if (serialize->FindClassData("{EE09F2F7-A016-48A1-841C-3384CD0E5A5F}") == nullptr) {
                serialize->Class<JavascriptComponent, AZ::Component>()
                    ->Version(0)
                    ->Field("Script", &JavascriptComponent::m_script);
            }
        }

        if (AZ::BehaviorContext* behaviourContext = azrtti_cast<AZ::BehaviorContext*>(context)) {
            behaviourContext->Class<JavascriptComponent>()
                ->Property("Script", &JavascriptComponent::GetScript, &JavascriptComponent::SetScript);
        }
    }
    void JavascriptComponent::RunScript(const AZStd::string script)
    {
        AZ_Assert(script.length() == 0, "Javascript script is empty.");
        m_script = script;
        UpdateScript();
    }
    void JavascriptComponent::UpdateScript()
    {
        JavascriptRequestBus::BroadcastResult(m_context, &JavascriptRequestBus::Events::GetContext, GetEntityId());
        AZ_Error("JavascriptComponent", m_context != 0, "Javascript Context isn't initialized!!!");
        m_context->RunScript(m_script);
    }
}
