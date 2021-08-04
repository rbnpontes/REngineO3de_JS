
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Javascript/JavascriptBus.h>
#include <JavascriptContext.h>

namespace Javascript
{
    class JavascriptSystemComponent
        : public AZ::Component
        , protected JavascriptRequestBus::Handler
    {
    public:
        AZ_COMPONENT(JavascriptSystemComponent, "{3900f916-805e-4ac2-b3ec-adf7ad04d26c}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        JavascriptSystemComponent();
        ~JavascriptSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // JavascriptRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        JavascriptContext* GetContext(AZ::EntityId entityId) override;
        void DestroyContext(AZ::EntityId entityId) override;

        void InitializingJSEnviroment(AZ::BehaviorContext* context);
        void RegisterClass(const AZStd::pair<AZStd::string, AZ::BehaviorClass*>& klass);
        void RegisterEBuses(const AZStd::pair<AZStd::string, AZ::BehaviorEBus*>& ebus);
    private:
        AZStd::unordered_map<AZ::EntityId, AZStd::shared_ptr<JavascriptContext>> m_contexts;
    };
} // namespace Javascript
