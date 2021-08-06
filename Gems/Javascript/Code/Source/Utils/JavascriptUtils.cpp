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

        JavascriptVariant ConvertToVariant(void* value, const AZ::BehaviorParameter* param)
        {
            JavascriptVariant result;
            AZ::Uuid type = param->m_typeId;

            if (type == azrtti_typeid<int>()) {
                int* val = reinterpret_cast<int*>(value);
                result.Set(*val);
            }
            else if (type == azrtti_typeid<float>()) {
                float* val = reinterpret_cast<float*>(value);
                result.Set(*val);
            }
            else if (type == azrtti_typeid<double>()) {
                double* val = reinterpret_cast<double*>(value);
                result.Set(*val);
            }
            else if (type == azrtti_typeid<JavascriptString>()) {
                JavascriptString* str = static_cast<JavascriptString*>(value);
                result.Set(JavascriptString(str->begin(), str->end()));
            }
            else if (type == azrtti_typeid<const char*>()) {
                const char* buffer = static_cast<const char*>(value);
                result.Set(buffer);
            }
            else if (type == azrtti_typeid<JavascriptObject>()) {
                JavascriptObject* obj = static_cast<JavascriptObject*>(value);
                result.Set(JavascriptObject(obj->begin(), obj->end()));
            }
            else if (type == azrtti_typeid<JavascriptArray>()) {
                JavascriptArray* arr = static_cast<JavascriptArray*>(value);
                result.Set(JavascriptArray(arr->begin(), arr->end()));
            }
            else {
                result.Set(value);
            }

            return result;
        }

        void* AllocateValue(AZ::Uuid typeId)
        {
            if (typeId == azrtti_typeid<int>())
                return new int();
            else if (typeId == azrtti_typeid<float>())
                return new float();
            else if (typeId == azrtti_typeid<double>())
                return new double();
            else if (typeId == azrtti_typeid<JavascriptString>())
                return new JavascriptString();
            else if (typeId == azrtti_typeid<bool>())
                return new bool();
            return nullptr;
        }
        void* AllocateValue(JavascriptVariant value, AZ::Uuid type)
        {
            void* ptr = nullptr;
            switch (value.GetType()) {
            case JavascriptVariantType::Array: {
                JavascriptArray* arr = new JavascriptArray(value.GetArray().begin(), value.GetArray().end());
                ptr = arr;
            }
                break;
            case JavascriptVariantType::Number: {
                if (type == azrtti_typeid<int>()) {
                    int* val = new int();
                    *val = value.GetNumber();
                    ptr = val;
                }
                else if (type == azrtti_typeid<float>()) {
                    float* val = new float();
                    *val = value.GetNumber();
                    ptr = val;
                }
                else if (type == azrtti_typeid<double>()) {
                    double* val = new double();
                    *val = value.GetNumber();
                    ptr = val;
                }
            }
                break;
            case JavascriptVariantType::Object: {
                JavascriptObject* obj = new JavascriptObject(value.GetObject().begin(), value.GetObject().end());
                ptr = obj;
            }
                break;
            case JavascriptVariantType::Boolean: {
                bool* val = new bool();
                *val = value.GetBoolean();
                ptr = val;
            }
                break;
            case JavascriptVariantType::String: {
                JavascriptString* str = new JavascriptString(value.GetString().begin(), value.GetString().end());
                ptr = str;
            }
                break;
            case JavascriptVariantType::Pointer:
                ptr = value.GetPointer();
                break;
            }

            return ptr;
        }

        void ToCamelCase(JavascriptString& value)
        {
            if (value.size() < 1)
                return;
            value[0] = tolower(value[0]);
        }
    }
}
