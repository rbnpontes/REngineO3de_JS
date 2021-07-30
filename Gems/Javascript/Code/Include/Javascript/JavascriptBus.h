
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>
#include <JavascriptContext.h>
namespace Javascript
{
    class JavascriptRequests
    {
    public:
        AZ_RTTI(JavascriptRequests, "{34a3df39-1501-499f-9196-110026b49299}");
        virtual ~JavascriptRequests() = default;

        virtual JavascriptContext* GetContext(AZ::EntityId entityId) = 0;
        virtual void DestroyContext(AZ::EntityId entityId) = 0;
        // Put your public methods here
    };
    
    class JavascriptBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using JavascriptRequestBus = AZ::EBus<JavascriptRequests, JavascriptBusTraits>;
    using JavascriptInterface = AZ::Interface<JavascriptRequests>;


    class JavascriptComponentRequests : public AZ::ComponentBus {
    public:
        AZ_RTTI(JavascriptComponentRequests, "{2D5E657C-E7C2-4931-8752-7DD79F28C647}");
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        virtual void RunScript(const AZStd::string script) = 0;
    };
    using JavascriptComponentRequestBus = AZ::EBus<JavascriptComponentRequests>;
} // namespace Javascript
