#pragma once
#include <AzCore/std/string/string.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Javascript {
    enum class JavascriptVariantType {
        None        = 0,
        Number      = 1,
        Boolean     = 2,
        String      = 3,
        Pointer     = 4,
        Array       = 5,
        Object      = 6
    };
    class JavascriptVariant;

    union JavascriptVariantValue {
        double m_number;
        bool m_bool;
        AZStd::string m_string;
        void* m_pointer;
        AZStd::vector<JavascriptVariant> m_array;
        AZStd::unordered_map<AZStd::string, JavascriptVariant> m_object;

        JavascriptVariantValue() :
            m_number(0),
            m_bool(false),
            m_string(""),
            m_array(),
            m_pointer(nullptr){
        }
        JavascriptVariantValue(const JavascriptVariantValue& value) = delete;
        ~JavascriptVariantValue() {}
    };

    class JavascriptVariant {
    public:
        AZ_TYPE_INFO(JavascriptVariant, "{552CF9C7-B74E-407D-AFCF-46EA8B0110AE}");

        JavascriptVariant();
        JavascriptVariant(const JavascriptVariant& value) { *this = value; }
        JavascriptVariant(int value);
        JavascriptVariant(float value);
        JavascriptVariant(double value);
        JavascriptVariant(bool value);
        JavascriptVariant(AZStd::string value);
        JavascriptVariant(AZStd::vector<JavascriptVariant> value);
        JavascriptVariant(AZStd::unordered_map<AZStd::string, JavascriptVariant> value);

        void Set(int value);
        void Set(float value);
        void Set(double value);
        void Set(bool value);
        void Set(AZStd::string value);
        void Set(AZStd::vector<JavascriptVariant> value);
        void Set(void* value);
        void Set(AZStd::unordered_map<AZStd::string, JavascriptVariant> value);

        int GetInt();
        float GetFloat();
        double GetNumber();
        AZStd::string GetString();
        bool GetBoolean();
        void* GetPointer();
        AZStd::vector<JavascriptVariant> GetArray();
        AZStd::unordered_map<AZStd::string, JavascriptVariant> GetObject();
        JavascriptVariantType GetType() { return m_type; }

        JavascriptVariant& operator =(const JavascriptVariant& value);
    private:
        JavascriptVariantValue m_value;
        JavascriptVariantType m_type;
    };
}
