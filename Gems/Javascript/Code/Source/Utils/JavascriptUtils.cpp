#include <Utils/JavascriptUtils.h>

namespace Javascript {
    namespace Utils {
        bool IsMatchMethod(AZ::BehaviorMethod* method, const JavascriptArray& values)
        {
            // First of all, check if method args match total of incoming arg types
            if (method->GetNumArguments() == values.size())
                return false;

            for (unsigned i = 0; i < values.size(); ++i) {
                JavascriptVariant var = values.at(i);
                const AZ::BehaviorParameter* param = method->GetArgument(i);

                bool match = true;
                switch (var.GetType())
                {
                case JavascriptVariantType::String:
                    match = azrtti_typeid<AZStd::string>() == param->m_typeId;
                    break;
                case JavascriptVariantType::Boolean:
                    match = azrtti_typeid<bool>() == param->m_typeId;
                    break;
                case JavascriptVariantType::Number:
                    match = azrtti_typeid<int>() == param->m_typeId
                        || azrtti_typeid<float>() == param->m_typeId
                        || azrtti_typeid<double>() == param->m_typeId
                        || azrtti_typeid<uint32_t>() == param->m_typeId
                        || azrtti_typeid<uint64_t>() == param->m_typeId;
                    break;
                case JavascriptVariantType::Object:
                    match = azrtti_typeid<AZStd::unordered_map<AZStd::string, JavascriptVariant>>() == param->m_typeId;
                    break;
                case JavascriptVariantType::Array:
                    match = azrtti_typeid<AZStd::vector<JavascriptVariant>>() == param->m_typeId;
                    break;
                case JavascriptVariantType::Pointer:
                    match = azrtti_typeid<void*>() == param->m_typeId;
                    break;
                }

                if (!match)
                    return false;
            }

            return true;
        }

        AZ::BehaviorMethod* GetAvailableCtor(AZ::BehaviorClass* klass, const JavascriptArray& values)
        {
            for (AZ::BehaviorMethod* method : klass->m_constructors) {
                if (IsMatchMethod(method, values))
                    return method;
            }
            return nullptr;
        }
    }
}
