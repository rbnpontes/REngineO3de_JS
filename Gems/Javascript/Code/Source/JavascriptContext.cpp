#include "JavascriptContext.h"
#include <sstream>
namespace Javascript {
    JavascriptContext::JavascriptContext()
    {
        m_context = duk_create_heap_default();
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
    void JavascriptContext::RegisterDefaultMethods()
    {
        AddGlobalFunction("log", &JavascriptContext::OnLogMethod);
    }
    duk_ret_t JavascriptContext::OnLogMethod(duk_context* ctx)
    {
        int argsLen = duk_get_top(ctx);
        for (unsigned i = 0; i < argsLen; i++)
            AZ_TracePrintf("Javascript", "%s\n", duk_to_string(ctx, i));
        return 1;
    }
}
