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

        if (m_instance) {
            m_class->m_destructor(m_instance, m_class->m_userData);
            m_class->Deallocate(m_instance);
        }
    }

    AZStd::shared_ptr<JavascriptProperty> JavascriptInstance::GetProperty(AZStd::string key)
    {
        return m_properties[key];
    }

    JavascriptProperty* JavascriptInstance::CreateProperty(JavascriptString name, AZ::BehaviorProperty* prop)
    {
        AZStd::shared_ptr<JavascriptProperty> property = m_properties[name];
        if (property)
            return property.get();
        property = AZStd::shared_ptr<JavascriptProperty>(new JavascriptProperty(this, prop));
        m_properties[name] = property;
        return property.get();
    }

    JavascriptMethod* JavascriptInstance::CreateMethod(JavascriptString name, AZ::BehaviorMethod* method)
    {
        AZStd::shared_ptr<JavascriptMethod> jsMethod = m_methods[name];
        if (jsMethod)
            return jsMethod.get();
        jsMethod = AZStd::shared_ptr<JavascriptMethod>(new JavascriptMethod(this, method));
        m_methods[name] = jsMethod;
        return jsMethod.get();
    }
}
