#pragma once
#include <JavascriptTypes.h>
#include <JavascriptProperty.h>
#include <JavascriptMethod.h>
namespace Javascript {
    class JavascriptInstance {
    public:
        JavascriptInstance(AZ::BehaviorClass* klass);
        ~JavascriptInstance();
        AZ::BehaviorClass* GetClass() { return m_class; }
        AZStd::shared_ptr<JavascriptProperty> GetProperty(AZStd::string key);

        JavascriptProperty* CreateProperty(JavascriptString name, AZ::BehaviorProperty* prop);
        JavascriptMethod* CreateMethod(JavascriptString name, AZ::BehaviorMethod* method);

        void* GetInstance() { return m_instance; }
        void SetInstance(void* instance) { m_instance = instance; }
        void SetArgValues(AZStd::vector<void*> args) {
            m_argValues = args;
        }
    private:
        void* m_instance;
        AZ::BehaviorClass* m_class;
        AZStd::vector<void*> m_argValues;
        AZStd::unordered_map<AZStd::string, AZStd::shared_ptr<JavascriptProperty>> m_properties;
        AZStd::unordered_map<AZStd::string, AZStd::shared_ptr<JavascriptMethod>> m_methods;
    };
}
