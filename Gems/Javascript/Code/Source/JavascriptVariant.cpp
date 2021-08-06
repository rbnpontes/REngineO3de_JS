#include <JavascriptVariant.h>
#include <JavascriptTypes.h>

namespace Javascript {
    JavascriptVariant::JavascriptVariant() {
        m_value.m_bool = false;
        m_type = JavascriptVariantType::None;
    }

    JavascriptVariant::JavascriptVariant(int value)
    {
        Set(value);
    }

    JavascriptVariant::JavascriptVariant(float value)
    {
        Set(value);
    }

    JavascriptVariant::JavascriptVariant(double value)
    {
        Set(value);
    }

    JavascriptVariant::JavascriptVariant(bool value)
    {
        Set(value);
    }

    JavascriptVariant::JavascriptVariant(AZStd::string value)
    {
        Set(value);
    }

    JavascriptVariant::JavascriptVariant(AZStd::vector<JavascriptVariant> value)
    {
        Set(value);
    }

    JavascriptVariant::JavascriptVariant(AZStd::unordered_map<AZStd::string, JavascriptVariant> value)
    {
        Set(value);
    }

    void JavascriptVariant::Set(int value)
    {
        m_type = JavascriptVariantType::Number;
        m_value.m_number = static_cast<double>(value);
    }

    void JavascriptVariant::Set(float value)
    {
        m_type = JavascriptVariantType::Number;
        m_value.m_number = static_cast<double>(value);
    }

    void JavascriptVariant::Set(double value)
    {
        m_type = JavascriptVariantType::Number;
        m_value.m_number = value;
    }

    void JavascriptVariant::Set(bool value)
    {
        m_type = JavascriptVariantType::Boolean;
        m_value.m_bool = value;
    }

    void JavascriptVariant::Set(AZStd::string value)
    {
        m_type = JavascriptVariantType::String;
        m_value.m_string = JavascriptString(value);
    }

    void JavascriptVariant::Set(AZStd::vector<JavascriptVariant> value)
    {
        m_type = JavascriptVariantType::Array;
        m_value.m_array = JavascriptArray(value);
    }

    void JavascriptVariant::Set(void* value)
    {
        m_type = JavascriptVariantType::Pointer;
        m_value.m_pointer = value;
    }

    void JavascriptVariant::Set(AZStd::unordered_map<AZStd::string, JavascriptVariant> value)
    {
        m_type = JavascriptVariantType::Object;
        m_value.m_object = JavascriptObject(value);
    }

    int JavascriptVariant::GetInt()
    {
        if (m_type != JavascriptVariantType::Number)
            return 0;
        return static_cast<int>(m_value.m_number);
    }

    float JavascriptVariant::GetFloat()
    {
        if (m_type != JavascriptVariantType::Number)
            return 0.0f;
        return static_cast<float>(m_value.m_number);
    }

    double JavascriptVariant::GetNumber()
    {
        if (m_type != JavascriptVariantType::Number)
            return false;
        return m_value.m_number;
    }

    AZStd::string JavascriptVariant::GetString()
    {
        if (m_type != JavascriptVariantType::String)
            return false;
        return m_value.m_string;
    }

    bool JavascriptVariant::GetBoolean()
    {
        if (m_type != JavascriptVariantType::Boolean)
            return false;
        return m_value.m_bool;
    }

    void* JavascriptVariant::GetPointer()
    {
        if (m_type != JavascriptVariantType::Pointer)
            return nullptr;
        return m_value.m_pointer;
    }

    AZStd::vector<JavascriptVariant> JavascriptVariant::GetArray()
    {
        if(m_type != JavascriptVariantType::Array)
            return AZStd::vector<JavascriptVariant>();
        return m_value.m_array;
    }

    AZStd::unordered_map<AZStd::string, JavascriptVariant> JavascriptVariant::GetObject()
    {
        if(m_type != JavascriptVariantType::Object)
            return AZStd::unordered_map<AZStd::string, JavascriptVariant>();
        return m_value.m_object;
    }

    JavascriptVariant& JavascriptVariant::operator=(const JavascriptVariant& value)
    {
        m_type = value.m_type;
        switch (m_type)
        {
        case Javascript::JavascriptVariantType::Number:
            m_value.m_number = value.m_value.m_number;
            break;
        case Javascript::JavascriptVariantType::Boolean:
            m_value.m_bool = value.m_value.m_bool;
            break;
        case Javascript::JavascriptVariantType::String:
            m_value.m_string = JavascriptString(value.m_value.m_string);
            break;
        case Javascript::JavascriptVariantType::Pointer:
            m_value.m_pointer = value.m_value.m_pointer;
            break;
        case Javascript::JavascriptVariantType::Array:
            m_value.m_array = JavascriptArray(value.m_value.m_array);
            break;
        case JavascriptVariantType::Object:
            m_value.m_object = JavascriptObject(value.m_value.m_object);
            break;
        }

        return *this;
    }
}
