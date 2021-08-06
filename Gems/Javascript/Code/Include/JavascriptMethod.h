#pragma once
#include <AzCore/RTTI/BehaviorContext.h>
#include <duktape.h>
namespace Javascript {
    class JavascriptInstance;
    class JavascriptMethod {
    public:
        JavascriptMethod(JavascriptInstance* instance, AZ::BehaviorMethod* method);
        AZ::BehaviorClass* GetClass();
        AZ::BehaviorMethod* GetMethod();
        JavascriptInstance* GetInstance();
        bool Call(duk_context* ctx, const JavascriptArray& args);
    private:
        JavascriptInstance* m_instance;
        AZ::BehaviorMethod* m_method;
    };
}
