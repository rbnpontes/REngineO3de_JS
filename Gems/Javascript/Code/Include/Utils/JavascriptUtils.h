#pragma once
#include <AzCore/RTTI/BehaviorContext.h>
#include <JavascriptTypes.h>
namespace Javascript {
    namespace Utils {
        inline const char* StorageKey = DUK_HIDDEN_SYMBOL("__storage");
        inline const char* BehaviorClassKey = DUK_HIDDEN_SYMBOL("__class");
        inline const char* InstanceKey = DUK_HIDDEN_SYMBOL("__instance");
        inline const char* TypeUuidKey = DUK_HIDDEN_SYMBOL("__typeUuid");
        inline const char* PropertyKey = DUK_HIDDEN_SYMBOL("__property");
        inline const char* MethodKey = DUK_HIDDEN_SYMBOL("__method");

        bool IsMemberMethod(AZ::BehaviorMethod* method, AZ::BehaviorClass* klass);

        bool IsMatchMethod(AZ::BehaviorMethod* method, const JavascriptArray& values);
        AZ::BehaviorMethod* GetAvailableCtor(AZ::BehaviorClass* klass, const JavascriptArray& values);
        JavascriptVariant ConvertToVariant(void* value, const AZ::BehaviorParameter* param);
        void* AllocateValue(AZ::Uuid typeId);
        void* AllocateValue(JavascriptVariant value, AZ::Uuid type);
        void ToCamelCase(JavascriptString& value);
    }
}
