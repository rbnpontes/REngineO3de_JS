#include <JavascriptInstance.h>
namespace Javascript {
    JavascriptInstance::JavascriptInstance(AZ::BehaviorClass* klass) :
        m_instance(0),
        m_class(klass){

    }

    JavascriptInstance::~JavascriptInstance()
    {
        // Free allocated arg values
        for (void* ptr : m_argValues)
            delete ptr;
    }

    void JavascriptInstance::AddProperty(AZStd::string key, JavascriptProperty* prop)
    {
        m_properties[key] = AZStd::shared_ptr<JavascriptProperty>(prop);
    }

    AZStd::shared_ptr<JavascriptProperty> JavascriptInstance::GetProperty(AZStd::string key)
    {
        return m_properties[key];
    }

    JavascriptMethod* JavascriptInstance::CreateMethod(JavascriptString name, AZ::BehaviorMethod* method)
    {
        JavascriptMethod* jsMethod = new JavascriptMethod(this, method);
        m_methods[name] = AZStd::shared_ptr<JavascriptMethod>(jsMethod);
        return jsMethod;
    }
}
