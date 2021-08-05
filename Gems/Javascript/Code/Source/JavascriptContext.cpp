#include "JavascriptContext.h"
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/RTTI/AttributeReader.h>
#include <Utils/JavascriptUtils.h>
#include <Utils/DuktapeUtils.h>
#include <JavascriptInstance.h>
#include <JavascriptProperty.h>
#include <sstream>

namespace Javascript {
    const char* JavascriptContext::ScriptContextKey = DUK_HIDDEN_SYMBOL("__instance");
    const char* JavascriptContext::EBusKey = DUK_HIDDEN_SYMBOL("__ebus");
    const char* JavascriptContext::EBusHandlerKey = DUK_HIDDEN_SYMBOL("__ebusHandler");
    const char* JavascriptContext::EBusListenersKey = DUK_HIDDEN_SYMBOL("__ebusListeners");
    const char* JavascriptContext::BehaviorClassKey = DUK_HIDDEN_SYMBOL("__classHandler");

    JavascriptContext::JavascriptContext()
    {
        m_context = duk_create_heap_default();

        AZ::ComponentApplicationBus::BroadcastResult(m_behaviorContext, &AZ::ComponentApplicationBus::Events::GetBehaviorContext);

        // Store Current context, this can'be useful later
        duk_push_pointer(m_context, this);
        duk_put_global_string(m_context, ScriptContextKey);
        // Create Listener EBus table
        duk_push_object(m_context);
        duk_put_global_string(m_context, EBusListenersKey);

        RegisterDefaultClasses();
        RegisterDefaultMethods();
    }

    JavascriptContext::~JavascriptContext()
    {
        duk_destroy_heap(m_context);
    }

    void JavascriptContext::RunScript(const AZStd::string& script)
    {
        duk_eval_string(m_context, script.c_str());
    }

    void JavascriptContext::AddGlobalFunction(const AZStd::string& functionName, JavascriptFunction fn, duk_idx_t argsCount)
    {
        duk_push_c_function(m_context, fn, argsCount);
        duk_put_global_string(m_context, functionName.c_str());
    }

    void JavascriptContext::CallActivate()
    {
        duk_get_global_string(m_context, "OnActivate");
        if (duk_is_null_or_undefined(m_context, -1))
            return;
        duk_call(m_context, 0);
    }

    void JavascriptContext::CallDeActivate()
    {
        duk_get_global_string(m_context, "OnDeactivate");
        if (duk_is_null_or_undefined(m_context, -1))
            return;
        duk_call(m_context, 0);
    }

    void JavascriptContext::SetEntity(AZ::EntityId id)
    {
        uint64_t entityId = (uint64_t)id;
        std::ostringstream entityIdStr;
        entityIdStr << entityId;
        duk_push_string(m_context, entityIdStr.str().c_str());
        duk_put_global_string(m_context, "entity");
    }

    JavascriptContext::JavascriptEventDesc* JavascriptContext::CreateOrGetEventDesc(const char* eventName, AZ::BehaviorEBus* ebus, AZ::BehaviorEBusHandler* ebusHandler)
    {
        AZStd::string eventId = GetEventId(eventName, ebus);
        AZStd::shared_ptr<JavascriptContext::JavascriptEventDesc> ptr = m_events[eventId];

        if (ptr)
            return ptr.get();
        JavascriptContext::JavascriptEventDesc* evt = new JavascriptContext::JavascriptEventDesc();
        evt->m_context = m_context;
        evt->m_ebus = ebus;
        evt->m_ebusHandler = ebusHandler;
        evt->m_eventId = eventId;
        evt->m_eventName = eventName;

        m_events[eventId] = AZStd::shared_ptr<JavascriptContext::JavascriptEventDesc>(evt);
        return evt;
    }

    void JavascriptContext::RegisterDefaultClasses()
    {
        auto classes = m_behaviorContext->m_classes;
        AZStd::string className;
        for (auto classPair : classes) {
            AZ::BehaviorClass* klass = classPair.second;
            className = klass->m_name;
            if (className.contains("VM") || className.contains("Iterator") || className.contains("String") || className.contains("EBusHandlerw"))
                continue;

            RegisterClass(klass);
        }
    }

    void JavascriptContext::RegisterDefaultMethods()
    {
        AddGlobalFunction("log", &JavascriptContext::OnLogMethod);
        DeclareEBusHandler(m_context);
    }

    void JavascriptContext::RegisterClass(AZ::BehaviorClass* klass)
    {
        // The code below is same of ScriptContext used in LUA
        AZ::Script::Attributes::StorageType storageType = AZ::Script::Attributes::StorageType::ScriptOwn;
        {
            if (AZ::FindAttribute(AZ::Script::Attributes::Ignore, klass->m_attributes))
                return;
            if (!AZ::Internal::IsInScope(klass->m_attributes, AZ::Script::Attributes::ScopeFlags::Launcher))
                return;
            AZ::Attribute* ownershipAttribute = AZ::FindAttribute(AZ::Script::Attributes::Storage, klass->m_attributes);
            if (ownershipAttribute) {
                AZ::AttributeReader ownershipAttrReader(nullptr, ownershipAttribute);
                ownershipAttrReader.Read<AZ::Script::Attributes::StorageType>(storageType);

                if (storageType == AZ::Script::Attributes::StorageType::Value) {
                    bool isError = false;

                    if (klass->m_cloner == nullptr)
                    {
                        AZ_Error("Javascript", false, "Class %s was reflected to be stored by value, however class can't be copy constructed!", klass->m_name.c_str());
                        isError = true;
                    }

                    if (klass->m_alignment > 16)
                    {
                        AZ_Error("Script", false, "Class %s was reflected to be stored by value, however it has alignment %d which is more than maximum support of 16 bytes!", klass->m_name.c_str(), klass->m_alignment);
                        isError = true;
                    }

                    if (isError)
                    {
                        return;
                    }
                }
            }
        }
        
        duk_push_c_function(m_context, &JavascriptContext::OnCreateClass, 0);

        {
            // Declare Internal Values
            duk_push_pointer(m_context, klass);
            duk_put_prop_string(m_context, -2, Utils::BehaviorClassKey);
            duk_push_int(m_context, (int)storageType);
            duk_put_prop_string(m_context, -2, Utils::StorageKey);
        }

        duk_put_global_string(m_context, klass->m_name.c_str());
    }

    void JavascriptContext::DeclareEBusHandler(duk_context* ctx)
    {
        duk_push_c_function(ctx, &JavascriptContext::OnCreateEBusHandler, 1);
        duk_push_object(ctx);

        duk_push_c_function(ctx, &JavascriptContext::OnConnectEBus, DUK_VARARGS);
        duk_put_prop_string(ctx, -2, "connect");

        duk_push_c_function(ctx, &JavascriptContext::OnDisconnectEBus, 2);
        duk_put_prop_string(ctx, -2, "disconnect");

        duk_push_c_function(ctx, &JavascriptContext::OnBroadcastEBus, DUK_VARARGS);
        duk_put_prop_string(ctx, -2, "broadcast");

        duk_push_c_function(ctx, &JavascriptContext::OnCheckBusConnected, 0);
        duk_put_prop_string(ctx, -2, "isConnected");

        duk_push_c_function(ctx, &JavascriptContext::OnSetEBusEvent, 2);
        duk_put_prop_string(ctx, -2, "setEvent");

        duk_put_prop_string(ctx, -2, "prototype");
        duk_put_global_string(ctx, "EBusHandler");
    }

    duk_ret_t JavascriptContext::OnCreateClass(duk_context* ctx)
    {
        AZStd::vector<JavascriptVariant> args = Utils::GetArguments(ctx);
        AZ::BehaviorClass* klass = nullptr;
        AZ::Script::Attributes::StorageType storageType = AZ::Script::Attributes::StorageType::ScriptOwn;

        duk_push_current_function(ctx);
        {
            duk_get_prop_string(ctx, -1, Utils::BehaviorClassKey);
            klass = Utils::GetPointer<AZ::BehaviorClass>(ctx, -1);
            duk_pop(ctx);
            storageType = Utils::GetStorageType(ctx, -1);
        }
        duk_pop(ctx);

        AZ_Assert(klass != nullptr, "Can´t instantiate this object because it is invalid!");
        if (!klass)
            return DUK_RET_ERROR;

        if (!duk_is_constructor_call(ctx)) {
            AZ_Printf("Class %s must be called with new operator", klass->m_name.c_str());
            return DUK_RET_TYPE_ERROR;
        }

        JavascriptInstance* instance = new JavascriptInstance(klass);
        void* obj = klass->Allocate();
        AZStd::vector<void*> pointers;
        if (AZ::BehaviorMethod* bestCtor = Utils::GetAvailableCtor(klass, args)) {
            AZStd::vector<AZ::BehaviorValueParameter> methodArgs;

            {
                AZ::BehaviorValueParameter instance;
                instance.m_traits = AZ::BehaviorParameter::TR_POINTER;
                methodArgs.push_back(instance);
            }

            void* p_Value = nullptr;
            for (unsigned i = 0; i < args.size(); ++i) {
                AZ::BehaviorValueParameter value;
                const AZ::BehaviorParameter* parameter = bestCtor->GetArgument(i);
                Javascript::JavascriptVariant argument = args.at(i);

                switch (argument.GetType())
                {
                case JavascriptVariantType::Array:
                    break;
                case JavascriptVariantType::Boolean: {
                    bool* val = new bool();
                    *val = argument.GetBoolean();
                    p_Value = val;
                }
                    break;
                case JavascriptVariantType::Number: {
                    if (parameter->m_typeId == azrtti_typeid<int>()) {
                        int* val = new int();
                        *val = argument.GetInt();
                        p_Value = val;
                    }
                    else if (parameter->m_typeId == azrtti_typeid<float>()) {
                        float* val = new float();
                        *val = argument.GetFloat();
                        p_Value = val;
                    }
                    else if (parameter->m_typeId == azrtti_typeid<double>()) {
                        double* val = new double();
                        *val = argument.GetNumber();
                        p_Value = val;
                    }
                }
                    break;
                case JavascriptVariantType::Object: {
                    JavascriptObject* val = new JavascriptObject(argument.GetObject().begin(), argument.GetObject().end());
                    p_Value = val;
                }
                    break;
                case JavascriptVariantType::Pointer:
                    p_Value = argument.GetPointer();
                    break;
                case JavascriptVariantType::String: {
                    size_t memSize = (argument.GetString().length() * sizeof(char));
                    p_Value = malloc(memSize + sizeof(char)/*Strings must be terminated as null*/);
                    memset(p_Value, 0, memSize); // Reset memory

                    strcpy_s(static_cast<char*>(p_Value), memSize, argument.GetString().c_str());
                }
                    break;
                }

                value.Set(p_Value);
                pointers.push_back(p_Value);
            }

            bestCtor->Call(methodArgs.data(), methodArgs.size());
        }
        else {
            klass->m_defaultConstructor(obj, klass->m_userData);
        }
        
        instance->SetInstance(obj);
        instance->SetArgValues(pointers);

        duk_push_this(ctx);
        {
            // Define Instance pointer
            duk_push_pointer(ctx, instance);
            duk_put_prop_string(ctx, -2, Utils::InstanceKey);
        }

        {
            // Define Accessors
            duk_get_global_string(ctx, "Object");
            duk_get_prop_string(ctx, -1, "defineProperties");
            // Add current instance as arg
            duk_push_this(ctx);
            duk_push_object(ctx); // Add property configs
            {
                for (auto propPair : klass->m_properties) {
                    if (AZ::FindAttribute(AZ::Script::Attributes::Ignore, propPair.second->m_attributes))
                        continue;
                    JavascriptProperty* prop = new JavascriptProperty(instance, propPair.second);
                    instance->AddProperty(propPair.first, prop);

                    duk_push_object(ctx);
                    {
                        // Enumerable property
                        duk_push_boolean(ctx, true);
                        duk_put_prop_string(ctx, -2, "enumerable");
                    }
                    {
                        // Configurable property
                        duk_push_boolean(ctx, true);
                        duk_put_prop_string(ctx, -2, "configurable");
                    }
                    {
                        // Getter
                        duk_push_c_function(ctx, &JavascriptContext::OnGetter, 1);
                        duk_push_pointer(ctx, prop); // Store JavascriptProperty inside getter function
                        duk_put_prop_string(ctx, -2, Utils::PropertyKey);
                        duk_put_prop_string(ctx, -2, "get");
                    }
                    {
                        // Setter
                        duk_push_c_function(ctx, &JavascriptContext::OnSetter, 2);
                        duk_push_pointer(ctx, prop); // Store JavascriptProperty inside setter function
                        duk_put_prop_string(ctx, -2, Utils::PropertyKey);
                        duk_put_prop_string(ctx, -2, "set");
                    }

                    duk_put_prop_string(ctx, -2, propPair.first.c_str());
                }
            }
            duk_call(ctx, 2);
            duk_pop_2(ctx);
        }

        return 0;
    }

    duk_ret_t JavascriptContext::OnGetter(duk_context* ctx)
    {
        const char* key = duk_get_string(ctx, -1);
        JavascriptProperty* prop = nullptr;
        {
            duk_push_current_function(ctx);
            duk_get_prop_string(ctx, -1, Utils::PropertyKey);
            prop = Utils::GetPointer<JavascriptProperty>(ctx, -1);
            duk_pop_2(ctx);
        }

        AZ_Assert(prop, "JavascriptProperty not found on this object.");
        if (!prop)
            return DUK_RET_ERROR;

        AZ::BehaviorMethod* method = prop->GetProperty()->m_getter;
        const AZ::BehaviorParameter* resultType = method->GetResult();

        AZ::BehaviorObject obj(prop->GetInstance()->GetInstance(), prop->GetClass()->m_azRtti);

        void* value = Utils::AllocateValue(resultType->m_typeId);

        AZ::BehaviorValueParameter arguments[40];
        AZ::BehaviorValueParameter result;

        result.Set(*resultType);
        result.m_value = value;

        arguments[0].Set(&obj);
        arguments[0].m_traits = AZ::BehaviorParameter::TR_POINTER;
        arguments[1].Set(*resultType);
        arguments[1].m_value = value;

        if (!method->Call(arguments, 2, &result)) {
            AZ_Error("Javascript", false, "Error has ocurred on access property data");
            return DUK_RET_ERROR;
        }

        JavascriptVariant var = Utils::ConvertToVariant(value, resultType);
        Utils::PushValue(ctx, var);

        delete value;
        return 1;
    }

    duk_ret_t JavascriptContext::OnSetter(duk_context* ctx)
    {
        JavascriptVariant var = Utils::GetValue(ctx, 0);
        const char* key = duk_get_string(ctx, 1);
        JavascriptProperty* prop = nullptr;
        {
            duk_push_current_function(ctx);
            duk_get_prop_string(ctx, -1, Utils::PropertyKey);
            prop = Utils::GetPointer<JavascriptProperty>(ctx, -1);
            duk_pop_2(ctx);
        }

        AZ_Assert(prop, "JavascriptProperty not found on this object.");
        if (!prop)
            return DUK_RET_ERROR;


        AZ::BehaviorMethod* method = prop->GetProperty()->m_setter;
        const AZ::BehaviorParameter* setterType = method->GetArgument(1);

        AZ::BehaviorObject obj(prop->GetInstance()->GetInstance(), prop->GetClass()->m_azRtti);

        void* value = Utils::AllocateValue(var, setterType->m_typeId);

        AZ::BehaviorValueParameter arguments[40];
        arguments[0].Set(&obj);
        arguments[0].m_traits = AZ::BehaviorParameter::TR_POINTER;
        arguments[1].Set(*setterType);
        arguments[1].m_value = value;


        duk_ret_t result = 0;
        if (!method->Call(arguments, 2)) {
            AZ_Error("Javascript", false, "Error has ocurred on access property data");
            result = DUK_RET_ERROR;
        }

        delete value;
        return result;
    }

    duk_ret_t JavascriptContext::OnCreateEBusHandler(duk_context* ctx)
    {
        if (!duk_is_constructor_call(ctx)) {
            AZ_Printf("Javascript", "EBusHandler must be called with new instead a function.");
            return DUK_RET_TYPE_ERROR;
        }

        duk_require_string(ctx, 0);

        const char* busName = duk_get_string(ctx, 0);

        AZ::BehaviorContext* behaviorContext = GetCurrentContext(ctx)->m_behaviorContext;

        if (!behaviorContext) {
            AZ_Error("Javascript", behaviorContext != 0, "Can´t get EBus because behaviourContext is not available.");
            return DUK_RET_ERROR;
        }
        
        AZ::BehaviorEBus* ebus = behaviorContext->m_ebuses[busName];
        if (!ebus) {
            AZ_TracePrintf("Javascript", "EBus %s not found", busName);
            duk_push_null(ctx);
            return 1;
        }

        AZ::BehaviorEBusHandler* handler = 0;

        AZ_Assert(ebus->m_createHandler, "EBus doesn´t contains `m_createHandler`");
        if (!ebus->m_createHandler)
            return DUK_RET_ERROR;

        ebus->m_createHandler->InvokeResult(handler);

        AZ_Assert(handler, "Can´t create EBus handler");
        if (!handler)
            return DUK_RET_ERROR;

        duk_push_this(ctx);
        duk_push_pointer(ctx, ebus);
        duk_put_prop_string(ctx, -2, EBusKey);

        duk_push_pointer(ctx, handler);
        duk_put_prop_string(ctx, -2, EBusHandlerKey);

        duk_push_string(ctx, busName);
        duk_put_prop_string(ctx, -2, "name");

        return 0;
    }

    duk_ret_t JavascriptContext::OnSetEBusEvent(duk_context* ctx)
    {
        duk_require_string(ctx, 0);

        const char* evtName = duk_get_string(ctx, 0);

        duk_push_this(ctx);

        duk_get_prop_string(ctx, -1, EBusKey);
        AZ::BehaviorEBus* ebus = static_cast<AZ::BehaviorEBus*>(duk_get_pointer(ctx, -1));
        duk_pop(ctx);

        duk_get_prop_string(ctx, -1, EBusHandlerKey);
        AZ::BehaviorEBusHandler* ebusHandler = static_cast<AZ::BehaviorEBusHandler*>(duk_get_pointer(ctx, -1));
        duk_pop_2(ctx);

        AZ_Assert(ebus, "Can´t get EBus pointer");
        AZ_Assert(ebusHandler, "Can´t get EBus Handler pointer");
        if (!ebus || !ebusHandler)
            return DUK_RET_ERROR;

        int eventIdx = GetEBusHandlerEventIndex(ebusHandler, evtName);

        AZ_Warning("Javascript", eventIdx != -1, "Not found event with name %s", evtName);
        if (eventIdx == -1) {
            return 0;
        }

        AZStd::string eventId = GetEventId(evtName, ebus);
        JavascriptContext::JavascriptEventDesc* userData = 0;

        if (!duk_is_null_or_undefined(ctx, 1)) {
            userData = GetCurrentContext(ctx)->CreateOrGetEventDesc(evtName, ebus, ebusHandler);
        }

        duk_get_global_string(ctx, EBusListenersKey);
        duk_dup(ctx, -2);
        if (duk_is_function(ctx, -1))
            AZ_Printf("Javascript", "Is function");
        duk_put_prop_string(ctx, -2, eventId.c_str());

        ebusHandler->InstallGenericHook(eventIdx, &JavascriptContext::HandleEBusGenericHook, userData);
        return 0;
    }

    duk_ret_t JavascriptContext::OnConnectEBus(duk_context* ctx)
    {
        duk_push_this(ctx);

        duk_get_prop_string(ctx, -1, EBusKey);
        AZ::BehaviorEBus* ebus = static_cast<AZ::BehaviorEBus*>(duk_get_pointer(ctx, -1));
        duk_pop(ctx);

        duk_get_prop_string(ctx, -1, EBusHandlerKey);
        AZ::BehaviorEBusHandler* ebusHandler = static_cast<AZ::BehaviorEBusHandler*>(duk_get_pointer(ctx, -1));
        duk_pop(ctx);

        AZ_Assert(ebus, "Can´t get EBus pointer");
        AZ_Assert(ebusHandler, "Can´t get EBus Handler pointer");
        if (!ebus || !ebusHandler)
            return DUK_RET_ERROR;

        AZ::BehaviorValueParameter idParam;
        idParam.Set(ebus->m_idParam);

        ebusHandler->Connect(idParam);
        return 0;
    }

    duk_ret_t JavascriptContext::OnDisconnectEBus(duk_context* ctx)
    {
        duk_push_this(ctx);

        duk_get_prop_string(ctx, -1, EBusHandlerKey);
        AZ::BehaviorEBusHandler* ebusHandler = static_cast<AZ::BehaviorEBusHandler*>(duk_get_pointer(ctx, -1));
        duk_pop(ctx);

        AZ_Assert(ebusHandler, "Can´t get EBus Handler pointer");
        if (!ebusHandler)
            return DUK_RET_ERROR;

        ebusHandler->Disconnect();
        return 0;
    }

    duk_ret_t JavascriptContext::OnBroadcastEBus(duk_context* ctx)
    {
        return 0;
    }

    duk_ret_t JavascriptContext::OnCheckBusConnected(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, EBusHandlerKey);
        AZ::BehaviorEBusHandler* ebusHandler = static_cast<AZ::BehaviorEBusHandler*>(duk_get_pointer(ctx, -1));
        duk_push_boolean(ctx, ebusHandler->IsConnected());
        return 1;
    }

    duk_ret_t JavascriptContext::OnLogMethod(duk_context* ctx)
    {
        int argsLen = duk_get_top(ctx);
        for (unsigned i = 0; i < argsLen; i++)
            AZ_TracePrintf("Javascript", "%s\n", duk_to_string(ctx, i));
        return 1;
    }

    JavascriptContext* JavascriptContext::GetCurrentContext(duk_context* ctx)
    {
        duk_get_global_string(ctx, ScriptContextKey);
        JavascriptContext* javascriptCtx = static_cast<JavascriptContext*>(duk_get_pointer(ctx, -1));
        duk_pop(ctx);
        return javascriptCtx;
    }

    void JavascriptContext::SetGlobalEBusListener(duk_context* ctx, const char* id, duk_idx_t stackIdx)
    {
        duk_get_global_string(ctx, EBusListenersKey);
        duk_dup(ctx, stackIdx);
        duk_put_prop_string(ctx, -1, id);
        duk_pop(ctx);
    }

    int JavascriptContext::GetEBusHandlerEventIndex(AZ::BehaviorEBusHandler* handler, const char* evtName)
    {
        const AZ::BehaviorEBusHandler::EventArray& events = handler->GetEvents();
        AZStd::string evtNameStr = evtName;
        
        for (int iEvent = 0; iEvent < static_cast<int>(events.size()); ++iEvent) {
            const AZ::BehaviorEBusHandler::BusForwarderEvent& e = events[iEvent];
            if (AZStd::string(e.m_name) == evtNameStr)
                return iEvent;
        }
        return -1;
    }

    void JavascriptContext::HandleEBusGenericHook(
        [[maybe_unused]] void* userData,
        [[maybe_unused]] const char* eventName,
        [[maybe_unused]] int eventIndex,
        [[maybe_unused]] AZ::BehaviorValueParameter* result,
        [[maybe_unused]] int numParameters,
        [[maybe_unused]] AZ::BehaviorValueParameter* parameters)
    {
        if (!userData)
            return;

        JavascriptContext::JavascriptEventDesc* eventDesc = static_cast<JavascriptContext::JavascriptEventDesc*>(userData);

        if (eventDesc->m_eventName != AZStd::string(eventName))
            return;

        duk_context* ctx = eventDesc->m_context;
        
        duk_get_global_string(ctx, EBusListenersKey);
        duk_get_prop_string(ctx, -1, eventDesc->m_eventId.c_str());
        if (duk_is_function(ctx, -1))
            duk_call(ctx, 0);
        duk_pop_2(ctx);
    }

    AZStd::string JavascriptContext::GetEventId(const char* eventName, AZ::BehaviorEBus* ebus)
    {
        AZStd::string evtId = ebus->m_name;
        evtId.append("_");
        evtId.append(eventName);
        return evtId;
    }
}
