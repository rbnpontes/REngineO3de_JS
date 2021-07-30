#pragma once
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <JavascriptComponent.h>

namespace Javascript {
    class JavascriptEditorComponent : public AzToolsFramework::Components::EditorComponentBase {
    public:
        AZ_EDITOR_COMPONENT(JavascriptEditorComponent, "{4BFDE90D-DCCA-49E5-8E7A-AC17DC9C38D1}");

        JavascriptEditorComponent();
        ~JavascriptEditorComponent();


        static void Reflect(AZ::ReflectContext* context);

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void BuildGameEntity(AZ::Entity* gameEntity) override;
    protected:
        JavascriptEditorComponent(const JavascriptEditorComponent&) = delete;
        void RunScriptCode();
    private:
        JavascriptComponent m_scriptComponent;
        JavascriptContext* m_context;
        AZStd::string m_scriptCode;
    };
}
