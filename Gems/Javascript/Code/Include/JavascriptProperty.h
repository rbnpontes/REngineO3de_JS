#pragma once
#include <AzCore/RTTI/BehaviorContext.h>

namespace Javascript {
    class JavascriptInstance;
    class JavascriptProperty {
    public:
        JavascriptProperty(JavascriptInstance* instance, AZ::BehaviorProperty* property);
        JavascriptInstance* GetInstance();
        AZ::BehaviorClass* GetClass();
        AZ::BehaviorProperty* GetProperty() { return m_property; }
    private:
        JavascriptInstance* m_instance;
        AZ::BehaviorProperty* m_property;
    };
}
