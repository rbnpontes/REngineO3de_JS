#pragma once
#include <duktape.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <JavascriptTypes.h>

namespace Javascript {
    namespace Utils {
        JavascriptArray GetArray(duk_context* ctx, duk_idx_t idx);
        JavascriptVariant GetValue(duk_context* ctx, duk_idx_t idx);
        JavascriptArray GetArguments(duk_context* ctx);
        template<class T>
        T* GetPointer(duk_context* ctx, duk_idx_t idx) {
            void* pointer = duk_get_pointer(ctx, idx);
            return static_cast<T*>(pointer);
        }
        void PushValue(duk_context* ctx, JavascriptVariant value);
        void PushObject(duk_context* ctx, const JavascriptObject& map);
        JavascriptObject GetObject(duk_context* ctx, duk_idx_t idx);
        AZ::Script::Attributes::StorageType GetStorageType(duk_context* ctx, duk_idx_t idx);
        void SetFinalizer(duk_context* ctx, duk_idx_t targetIdx, duk_c_function finalizerFn);
    }
}
