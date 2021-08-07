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

    JavascriptMethodStatic::JavascriptMethodStatic(JavascriptString name, AZ::BehaviorClass* klass, AZ::BehaviorMethod* method) :
        m_name(name),
        m_class(klass),
        m_method(method)
    {
    }

    AZ::BehaviorClass* JavascriptMethodStatic::GetClass()
    {
        return m_class;
    }

    AZ::BehaviorMethod* JavascriptMethodStatic::GetMethod()
    {
        return m_method;
    }

    bool JavascriptMethodStatic::Call(duk_context* ctx, const JavascriptArray& args)
    {
        return false;
    }

}
