#pragma once
#include <duktape.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/RTTI/BehaviorContext.h>
namespace Javascript {
    typedef duk_c_function JavascriptFunction;
    class JavascriptContext {
    public:
        struct JavascriptEventDesc {
            duk_context* m_context;
            AZ::BehaviorEBus* m_ebus;
            AZ::BehaviorEBusHandler* m_ebusHandler;
            AZStd::string m_eventName;
            AZStd::string m_eventId;
        };
        JavascriptContext();
        ~JavascriptContext();
        duk_context* GetContext() { return m_context; }
        void RunScript(const AZStd::string& script);
        void AddGlobalFunction(const AZStd::string& functionName, JavascriptFunction fn, duk_idx_t argsCount = DUK_VARARGS);
        void CallActivate();
        void CallDeActivate();
        void SetEntity(AZ::EntityId id);
    protected:
        JavascriptContext::JavascriptEventDesc* CreateOrGetEventDesc(const char* eventName, AZ::BehaviorEBus* ebus, AZ::BehaviorEBusHandler* ebusHandler);
    private:
        static const char* ScriptContextKey;
        static const char* EBusKey;
        static const char* EBusHandlerKey;
        static const char* EBusListenersKey;
        static const char* BehaviorClassKey;

        void RegisterDefaultClasses();
        void RegisterDefaultMethods();
        void RegisterClass(AZ::BehaviorClass* klass);
        static void DeclareEBusHandler(duk_context* ctx);
        static duk_ret_t OnCreateClass(duk_context* ctx);
        static duk_ret_t OnGetter(duk_context* ctx);
        static duk_ret_t OnSetter(duk_context* ctx);
        static duk_ret_t OnCreateEBusHandler(duk_context* ctx);
        static duk_ret_t OnSetEBusEvent(duk_context* ctx);
        static duk_ret_t OnConnectEBus(duk_context* ctx);
        static duk_ret_t OnDisconnectEBus(duk_context* ctx);
        static duk_ret_t OnBroadcastEBus(duk_context* ctx);
        static duk_ret_t OnCheckBusConnected(duk_context* ctx);
        static duk_ret_t OnLogMethod(duk_context* ctx);
        static JavascriptContext* GetCurrentContext(duk_context* ctx);
        static void SetGlobalEBusListener(duk_context* ctx, const char* id, duk_idx_t stackIdx);
        static int GetEBusHandlerEventIndex(AZ::BehaviorEBusHandler* handler, const char* evtName);
        static void HandleEBusGenericHook(
            void* userData,
            const char* eventName,
            int eventIndex,
            AZ::BehaviorValueParameter* result,
            int numParameters,
            AZ::BehaviorValueParameter* parameters
        );
        static AZStd::string GetEventId(const char* eventName, AZ::BehaviorEBus* ebus);

        duk_context* m_context;
        AZStd::unordered_map<AZStd::string, AZStd::shared_ptr<JavascriptEventDesc>> m_events;
        AZ::BehaviorContext* m_behaviorContext;
    };
}
