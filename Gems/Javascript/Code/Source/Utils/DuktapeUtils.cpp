#include <Utils/DuktapeUtils.h>
#include <Utils/JavascriptUtils.h>
#include <sstream>
namespace Javascript {
    namespace Utils {
        JavascriptArray GetArray(duk_context* ctx, duk_idx_t idx)
        {
            unsigned length = duk_get_length(ctx, idx);
            JavascriptArray values(length);

            for (unsigned i = 0; i < length; ++i) {
                duk_get_prop_index(ctx, idx, i);
                values.push_back(GetValue(ctx, -1));
                duk_pop(ctx);
            }

            return values;
        }

        JavascriptVariant GetValue(duk_context* ctx, duk_idx_t idx)
        {
            duk_uint_t dukType = duk_get_type(ctx, idx);
            JavascriptVariant value;

            switch (dukType)
            {
            case DUK_TYPE_BOOLEAN:
                value = JavascriptVariant((bool)duk_get_boolean(ctx, idx));
                break;
            case DUK_TYPE_STRING:
                value = JavascriptVariant(AZStd::string(duk_get_string(ctx, idx)));
                break;
            case DUK_TYPE_NUMBER:
                value = JavascriptVariant(duk_get_number(ctx, idx));
            case DUK_TYPE_OBJECT: {
                if (duk_is_array(ctx, idx))
                    value = JavascriptVariant(GetArray(ctx, idx));
                else {
                    duk_get_prop_string(ctx, -1, InstanceKey);
                    if (duk_is_null_or_undefined(ctx, -1)) {
                        duk_pop(ctx);
                        value = JavascriptVariant(GetObject(ctx, idx));
                    } else {
                        value = JavascriptVariant(duk_get_pointer(ctx, -1));
                        duk_pop(ctx);
                    }
                }
            }
                break;
            case DUK_TYPE_POINTER:
                value.Set(duk_get_pointer(ctx, idx));
                break;
            }

            return value;
        }

        AZStd::vector<JavascriptVariant> GetArguments(duk_context* ctx)
        {
            AZStd::vector<JavascriptVariant> args;
            int argsCount = duk_get_top(ctx);

            for (unsigned i = 0; i < argsCount; ++i) {
                JavascriptVariant var = GetValue(ctx, i);
                if (var.GetType() != JavascriptVariantType::None)
                    args.push_back(var);
            }

            return args;
        }

        void PushValue(duk_context* ctx, JavascriptVariant value)
        {
            switch (value.GetType())
            {
            case JavascriptVariantType::Array: {
                auto arr = value.GetArray();
                duk_push_array(ctx);
                duk_set_length(ctx, -1, arr.size());
                for (unsigned i = 0; i < arr.size(); ++i) {
                    PushValue(ctx, arr.at(i));
                    duk_put_prop_index(ctx, -2, i);
                }
            }
                break;
            case JavascriptVariantType::Boolean: {
                duk_push_boolean(ctx, value.GetBoolean());
            }
                break;
            case JavascriptVariantType::Number: {
                duk_push_number(ctx, value.GetNumber());
            }
                break;
            case JavascriptVariantType::Object:
                PushObject(ctx, value.GetObject());
                break;
            case JavascriptVariantType::Pointer:
                duk_push_pointer(ctx, value.GetPointer());
                break;
            case JavascriptVariantType::String:
                duk_push_string(ctx, value.GetString().c_str());
                break;
            }
        }

        void PushObject(duk_context* ctx, const JavascriptObject& map)
        {
            duk_push_object(ctx);
            for (auto pair : map) {
                PushValue(ctx, pair.second);
                duk_put_prop_string(ctx, -2, pair.first.c_str());
            }
        }

        JavascriptObject GetObject(duk_context* ctx, duk_idx_t idx)
        {
            AZStd::unordered_map<AZStd::string, JavascriptVariant> result;
            duk_enum(ctx, idx, 0);
            while (duk_next(ctx, -1, 1)) {
                const char* key = duk_safe_to_string(ctx, -2);
                JavascriptVariant value = GetValue(ctx, -1);
                duk_pop_2(ctx);

                result[key] = value;
            }
            duk_pop(ctx);
            return result;
        }

        AZ::Script::Attributes::StorageType GetStorageType(duk_context* ctx, duk_idx_t idx)
        {
            duk_get_prop_string(ctx, idx, StorageKey);
            AZ::Script::Attributes::StorageType type = (AZ::Script::Attributes::StorageType)duk_get_int(ctx, -1);
            duk_pop(ctx);
            return type;
        }
    }
}
