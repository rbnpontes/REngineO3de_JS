#pragma once
#include <JavascriptTypes.h>

namespace Javascript {
    /// <summary>
    /// JavascriptStackValue is used to store and manage method arguments
    /// </summary>
    class JavascriptStackValue {
    public:
        JavascriptStackValue() {}
        ~JavascriptStackValue();

        void* AllocateInt32(int32_t defaultValue = 0);
        void* AllocateUint32(uint32_t defaultValue = 0U);
        void* AllocateInt64(int64_t defaultValue = 0);
        void* AllocateUint64(uint64_t defaultValue = 0U);
        void* AllocateFloat(float_t defaultValue = 0.0f);
        void* AllocateDouble(double_t defaultValue = 0.0);
        void* AllocateBool(bool defaultValue = false);
        void* AllocateString(JavascriptString defaultValue = "");
        void* AllocateArray(JavascriptArray defaultValue);
        void* AllocateObject(JavascriptObject defaultValue);
        void* AllocatePointer(void* defaultValue = nullptr);
        void* FromVariant(JavascriptVariant variant, AZ::Uuid auxiliarType);
        void* FromType(AZ::Uuid type);
        void* Get(unsigned idx) { return m_values.at(idx); }
        /// <summary>
        /// This method will mark on values list item as nullptr
        /// On this way, when clean method is called this item will
        /// be ignored
        /// </summary>
        /// <param name="idx"></param>
        void Detach(unsigned idx);

        void Clean();
        unsigned Size() { return m_values.size(); }
    private:
        AZStd::vector<void*> m_values;
    };
}
