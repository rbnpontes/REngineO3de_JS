#pragma once
#include <AzCore/RTTI/BehaviorContext.h>
#include <JavascriptTypes.h>
namespace Javascript {
    namespace Utils {
        const char* StorageKey = DUK_HIDDEN_SYMBOL("__storage");
        const char* BehaviorClassKey = DUK_HIDDEN_SYMBOL("__class");
        const char* InstanceKey = DUK_HIDDEN_SYMBOL("__instance");
        const char* TypeUuidKey = DUK_HIDDEN_SYMBOL("__typeUuid");
        const char* PropertyKey = DUK_HIDDEN_SYMBOL("__property");

        bool IsMatchMethod(AZ::BehaviorMethod* method, const JavascriptArray& values);
        AZ::BehaviorMethod* GetAvailableCtor(AZ::BehaviorClass* klass, const JavascriptArray& values);
        JavascriptVariant ConvertToVariant(void* value, const AZ::BehaviorParameter* param);
        void* AllocateValue(AZ::Uuid typeId);
        void* AllocateValue(JavascriptVariant value, AZ::Uuid type);
    }
}
