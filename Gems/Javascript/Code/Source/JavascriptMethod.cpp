#include <JavascriptMethod.h>
#include <JavascriptInstance.h>
namespace Javascript {
    JavascriptMethod::JavascriptMethod(JavascriptInstance* instance, AZ::BehaviorMethod* method) :
        m_instance(instance),
        m_method(method){

    }

    AZ::BehaviorClass* JavascriptMethod::GetClass()
    {
        return m_instance->GetClass();
    }

    AZ::BehaviorMethod* JavascriptMethod::GetMethod()
    {
        return m_method;
    }

    JavascriptInstance* JavascriptMethod::GetInstance()
    {
        return m_instance;
    }

    bool JavascriptMethod::Call(duk_context* ctx, const JavascriptArray& args)
    {

        return false;
    }
}
