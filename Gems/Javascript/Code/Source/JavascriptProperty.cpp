#include <JavascriptProperty.h>
#include <JavascriptInstance.h>

namespace Javascript {
    JavascriptProperty::JavascriptProperty(JavascriptInstance* instance, AZ::BehaviorProperty* prop):
        m_instance(instance),
        m_property(prop)
    {
    }

    JavascriptInstance* JavascriptProperty::GetInstance()
    {
        return m_instance;
    }

    AZ::BehaviorClass* JavascriptProperty::GetClass()
    {
        return m_instance->GetClass();
    }

}
