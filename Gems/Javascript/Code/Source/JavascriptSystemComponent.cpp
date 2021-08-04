
#include <JavascriptSystemComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Javascript
{
    void JavascriptSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<JavascriptSystemComponent, AZ::Component>()
                ->Version(0);
        }
    }

    void JavascriptSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("JavascriptService"));
    }

    void JavascriptSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("JavascriptService"));
    }

    void JavascriptSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void JavascriptSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    JavascriptSystemComponent::JavascriptSystemComponent()
    {
        if (JavascriptInterface::Get() == nullptr)
        {
            JavascriptInterface::Register(this);
        }
    }

    JavascriptSystemComponent::~JavascriptSystemComponent()
    {
        if (JavascriptInterface::Get() == this)
        {
            JavascriptInterface::Unregister(this);
        }
    }

    void JavascriptSystemComponent::Init()
    {
        AZ::BehaviorContext* behaviourCtx;
        AZ::ComponentApplicationBus::BroadcastResult(behaviourCtx, &AZ::ComponentApplicationBus::Events::GetBehaviorContext);

        InitializingJSEnviroment(behaviourCtx);
    }

    void JavascriptSystemComponent::Activate()
    {
        JavascriptRequestBus::Handler::BusConnect();
        //AZ::TickBus::Handler::BusConnect();
    }

    void JavascriptSystemComponent::Deactivate()
    {
        //AZ::TickBus::Handler::BusDisconnect();
        JavascriptRequestBus::Handler::BusDisconnect();
    }

    void JavascriptSystemComponent::InitializingJSEnviroment(AZ::BehaviorContext* context)
    {
        for (AZStd::pair<AZStd::string, AZ::BehaviorClass*> klass : context->m_classes)
            RegisterClass(klass);
        for (AZStd::pair<AZStd::string, AZ::BehaviorEBus*> ebus : context->m_ebuses)
            RegisterEBuses(ebus);
    }

    void JavascriptSystemComponent::RegisterClass(const AZStd::pair<AZStd::string, AZ::BehaviorClass*>& klass)
    {
        AZ_TracePrintf("JavascriptSystemComponent", "Registering Class: %s \n", klass.first);
    }

    void JavascriptSystemComponent::RegisterEBuses(const AZStd::pair<AZStd::string, AZ::BehaviorEBus*>& ebus)
    {
        /*if (ebus.first == "TickBus") {
            AZ::BehaviorEBusHandler* handler = 0;
            AZ::BehaviorValueParameter idParam;
            idParam.Set(ebus.second->m_idParam);

            if (ebus.second->m_createHandler) {
                ebus.second->m_createHandler->InvokeResult(handler);
            }

            if (handler) {
                const AZ::BehaviorEBusHandler::EventArray& events = handler->GetEvents();
                for (int iEvent = 0; iEvent < static_cast<int>(events.size()); ++iEvent) {
                    const AZ::BehaviorEBusHandler::BusForwarderEvent& e = events[iEvent];
                    AZ::BehaviorEBusHandler::GenericHookType hookFn = [](
                        void* userData,
                        const char* eventName,
                        int eventIndex,
                        AZ::BehaviorValueParameter* result,
                        int numParameters,
                        AZ::BehaviorValueParameter* parameters) {
                        AZ_Printf("JavascriptSystem", "Hello from Tick");
                    };
                    handler->InstallGenericHook(iEvent, hookFn);
                }
                handler->Connect(idParam);
            }
        }*/
    }

    JavascriptContext* JavascriptSystemComponent::GetContext(AZ::EntityId entityId)
    {
        AZStd::shared_ptr<JavascriptContext> ctx = m_contexts[entityId];
        if (ctx)
            return ctx.get();
        ctx = AZStd::shared_ptr<JavascriptContext>(new JavascriptContext());
        ctx->SetEntity(entityId);
        m_contexts[entityId] = ctx;
        return ctx.get();
    }

    void JavascriptSystemComponent::DestroyContext(AZ::EntityId entityId)
    {
        m_contexts[entityId] = nullptr;
    }
} // namespace Javascript
