#include <JavascriptStackValues.h>
#include <JavascriptInstance.h>

namespace Javascript {
    JavascriptStackValue::~JavascriptStackValue()
    {
        Clean();
    }

    void* JavascriptStackValue::AllocateInt32(int32_t defaultValue)
    {
        int32_t* value = new int32_t();
        *value = defaultValue;
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::AllocateUint32(uint32_t defaultValue)
    {
        uint32_t* value = new uint32_t();
        *value = defaultValue;
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::AllocateInt64(int64_t defaultValue)
    {
        int64_t* value = new int64_t();
        *value = defaultValue;
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::AllocateUint64(uint64_t defaultValue)
    {
        uint64_t* value = new uint64_t();
        *value = defaultValue;
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::AllocateFloat(float_t defaultValue)
    {
        float_t* value = new float_t();
        *value = defaultValue;
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::AllocateDouble(double_t defaultValue)
    {
        double_t* value = new double_t();
        *value = defaultValue;
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::AllocateBool(bool defaultValue)
    {
        return nullptr;
    }

    void* JavascriptStackValue::AllocateString(JavascriptString defaultValue)
    {
        JavascriptString* value = new JavascriptString(defaultValue.begin(), defaultValue.end());
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::AllocateArray(JavascriptArray defaultValue)
    {
        return nullptr;
    }

    void* JavascriptStackValue::AllocateObject(JavascriptObject defaultValue)
    {
        JavascriptObject* value = new JavascriptObject(defaultValue.begin(), defaultValue.end());
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::AllocatePointer(void* defaultValue)
    {
        size_t size = sizeof(long long);
        void* value = malloc(size); // This engine will compile at 64bits
        if (defaultValue)
            memcpy(value, defaultValue, size);
        else
            memset(value, 0, size);
        m_values.push_back(value);
        return value;
    }

    void* JavascriptStackValue::FromVariant(JavascriptVariant variant, AZ::Uuid auxiliarType)
    {
        switch (variant.GetType())
        {
        case JavascriptVariantType::Array:
            return AllocateArray(variant.GetArray());
            break;
        case JavascriptVariantType::Boolean:
            return AllocateBool(variant.GetBoolean());
            break;
        case JavascriptVariantType::Number: {
            if (auxiliarType == azrtti_typeid<int32_t>())
                return AllocateInt32(variant.GetNumber());
            else if (auxiliarType == azrtti_typeid<uint32_t>())
                return AllocateUint32(variant.GetNumber());
            else if (auxiliarType == azrtti_typeid<int64_t>())
                return AllocateInt64(variant.GetNumber());
            else if (auxiliarType == azrtti_typeid<uint64_t>())
                return AllocateUint64(variant.GetNumber());
            else if (auxiliarType == azrtti_typeid<float_t>())
                return AllocateFloat(variant.GetNumber());
            else if (auxiliarType == azrtti_typeid<double_t>())
                return AllocateDouble(variant.GetNumber());
        }
            break;
        case JavascriptVariantType::Object:
            return AllocateObject(variant.GetObject());
            break;
        case JavascriptVariantType::String:
            return AllocateString(variant.GetString());
            break;
        case JavascriptVariantType::Pointer: {
            JavascriptInstance* instance = static_cast<JavascriptInstance*>(variant.GetPointer());
            return AllocatePointer(instance->GetInstance());
        }
            break;
        }
        return nullptr;
    }

    void* JavascriptStackValue::FromType(AZ::Uuid type)
    {
        if (AZ::AzTypeInfo<const char*>::Uuid() == type || azrtti_typeid<JavascriptString>() == type)
            return AllocateString("");
        else if (type == azrtti_typeid<JavascriptArray>())
            return AllocateArray(JavascriptArray());
        else if (type == azrtti_typeid<JavascriptObject>())
            return AllocateObject(JavascriptObject());
        else if (type == azrtti_typeid<int32_t>())
            return AllocateInt32();
        else if (type == azrtti_typeid<uint32_t>())
            return AllocateUint32();
        else if (type == azrtti_typeid<int64_t>())
            return AllocateInt64();
        else if (type == azrtti_typeid<uint64_t>())
            return AllocateUint64();
        else if (type == azrtti_typeid<float_t>())
            return AllocateFloat();
        else if (type == azrtti_typeid<double_t>())
            return AllocateDouble();
        return AllocatePointer();
    }

    void JavascriptStackValue::Detach(unsigned idx)
    {
        if(idx < m_values.size())
            m_values[idx] = nullptr;
    }

    void JavascriptStackValue::Clean()
    {
        for (void* ptr : m_values) {
            if(ptr)
                delete ptr;
        }
        m_values.clear();
    }

}
