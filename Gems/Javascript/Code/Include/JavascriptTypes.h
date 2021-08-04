#pragma once
#include <AzCore/std/string/string.h>
#include <JavascriptVariant.h>
namespace Javascript {
    typedef AZStd::string JavascriptString;
    typedef AZStd::unordered_map<JavascriptString, JavascriptVariant> JavascriptObject;
    typedef AZStd::vector<JavascriptVariant> JavascriptArray;
}
