#pragma once
#include <AzCore/RTTI/BehaviorContext.h>
#include <JavascriptTypes.h>
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

    class JavascriptMethodStatic {
    public:
        JavascriptMethodStatic(JavascriptString name, AZ::BehaviorClass* klass, AZ::BehaviorMethod* method);
        JavascriptString GetName() { return m_name; }
        AZ::BehaviorClass* GetClass();
        AZ::BehaviorMethod* GetMethod();
        bool Call(duk_context* ctx, const JavascriptArray& args);
    private:
        JavascriptString m_name;
        AZ::BehaviorClass* m_class;
        AZ::BehaviorMethod* m_method;
    };
}
