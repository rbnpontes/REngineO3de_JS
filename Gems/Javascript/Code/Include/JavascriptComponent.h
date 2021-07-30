#ifndef R_JAVASCRIPT_COMPONENT_H
#define R_JAVASCRIPT_COMPONENT_H

#include <AzCore/Component/Component.h>
#include <AzCore/std/string/string.h>
#include <JavascriptContext.h>
#include <Javascript/JavascriptBus.h>

namespace Javascript {
    class JavascriptComponent : public AZ::Component, protected Javascript::JavascriptComponentRequestBus::Handler {
    public:
        friend class JavascriptEditorComponent;
        AZ_COMPONENT(JavascriptComponent, "{EE09F2F7-A016-48A1-841C-3384CD0E5A5F}", AZ::Component);

        JavascriptComponent();
        ~JavascriptComponent();

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void SetScript(const AZStd::string& script);
        AZStd::string GetScript() { return m_script; }

        static void Reflect(AZ::ReflectContext* context);
    protected:
        void RunScript(const AZStd::string script) override;

    private:
        void UpdateScript();
        JavascriptContext* m_context;
        AZStd::string m_script;
    };
}

#endif
