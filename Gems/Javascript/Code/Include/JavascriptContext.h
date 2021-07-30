#pragma once
#include <duktape.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Component/Entity.h>
namespace Javascript {
    typedef duk_c_function JavascriptFunction;
    class JavascriptContext {
    public:
        JavascriptContext();
        ~JavascriptContext();
        duk_context* GetContext() { return m_context; }
        void RunScript(const AZStd::string& script);
        void AddGlobalFunction(const AZStd::string& functionName, JavascriptFunction fn, duk_idx_t argsCount = DUK_VARARGS);
        void CallActivate();
        void CallDeActivate();
        void SetEntity(AZ::EntityId id);
    private:
        void RegisterDefaultMethods();
        static duk_ret_t OnLogMethod(duk_context* ctx);
        duk_context* m_context;
    };
}
