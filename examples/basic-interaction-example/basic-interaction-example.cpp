#include <imgui.h>
#include <imgui_node_editor.h>
#include <application.h>
#include <vector>
#include "Nodes.h"

namespace ed = ax::NodeEditor;

struct Example:
    public Application
{
    int uniqueId = 1;
   
    std::vector<Node*> Nodes{};
    std::vector<LinkInfo*> Links{};

    Pin* MakePin(PinType type, PinKind kind)
    {
        auto* pin = new Pin{ ed::PinId(uniqueId++), type, kind };
        return pin;
    }

    Node* MakeBasicNode(const std::string& name,
        PinType inputType,
        std::initializer_list<PinType> outputTypes,
        ImVec2 startPos)
    {
        Pin* inputPin = MakePin(inputType, PinKind::Input);

        std::vector<Pin*> outputs;
        outputs.reserve(outputTypes.size());
        for (auto t : outputTypes)
            outputs.push_back(MakePin(t, PinKind::Output));

        auto* node = new Node{ ed::NodeId(uniqueId++), name, inputPin, std::move(outputs), startPos };
        Nodes.push_back(node);

        return node;
    }

    void DrawNode(Node* node) const
    {
        if (m_FirstFrame)
            ed::SetNodePosition(node->ID, node->Start_pos);

        ed::BeginNode(node->ID);
        ImGui::Text(node->Name.c_str());

        // Left side: input
        ImGui::BeginGroup();
        if (node->Type != NodeType::Primary && node->InputPin)
        {
            ed::BeginPin(node->InputPin->ID, ed::PinKind::Input);
            ImGui::Text(ToString(node->InputPin->Type));
            ed::EndPin();
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        // Right side: outputs
        ImGui::BeginGroup();
        for (auto* out : node->OutputPins)
        {
            ed::BeginPin(out->ID, ed::PinKind::Output);
            ImGui::Text(ToString(out->Type));
            ed::EndPin();
        }
        ImGui::EndGroup();

        ed::EndNode();
    }


    void DrawNodes(std::vector<Node*> nodes)
    {
        for (Node* node : nodes)
        {
            DrawNode(node);
        }
    }

    using Application::Application;

    void OnStart() override
    {
        ed::Config config;
        config.SettingsFile = "BasicInteraction.json";
        m_Context = ed::CreateEditor(&config);

        MakeBasicNode("DirectionFromPosToPos", PinType::Direction, { PinType::Position, PinType::Direction }, { 0,0 });
        MakeBasicNode("hi", PinType::Direction, { PinType::Position, PinType::Direction }, { 0,0 });
        MakeBasicNode("love!", PinType::Direction, { PinType::Position, PinType::Direction }, { 0,0 });
        ;
    }

    void OnStop() override
    {
        ed::DestroyEditor(m_Context);
    }

    void ImGuiEx_BeginColumn()
    {
        ImGui::BeginGroup();
    }

    void ImGuiEx_NextColumn()
    {
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
    }

    void ImGuiEx_EndColumn()
    {
        ImGui::EndGroup();
    }

    void OnFrame(float deltaTime) override
    {
        auto& io = ImGui::GetIO();

        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

        ImGui::Separator();

        ed::SetCurrentEditor(m_Context);

        // Start interaction with editor.
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));
        DrawNodes(Nodes);

        // Submit Links
        for (auto& linkInfo : m_Links)
            ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);

        //
        // 2) Handle interactions
        //

        // Handle creation action, returns true if editor want to create new object (node or link)
        if (ed::BeginCreate())
        {
            ed::PinId inputPinId, outputPinId;
            if (ed::QueryNewLink(&inputPinId, &outputPinId))
            {
                // QueryNewLink returns true if editor want to create new link between pins.
                //
                // Link can be created only for two valid pins, it is up to you to
                // validate if connection make sense. Editor is happy to make any.
                //
                // Link always goes from input to output. User may choose to drag
                // link from output pin or input pin. This determine which pin ids
                // are valid and which are not:
                //   * input valid, output invalid - user started to drag new ling from input pin
                //   * input invalid, output valid - user started to drag new ling from output pin
                //   * input valid, output valid   - user dragged link over other pin, can be validated

                if (inputPinId && outputPinId) // both are valid, let's accept link
                {
                    // ed::AcceptNewItem() return true when user release mouse button.
                    if (ed::AcceptNewItem())
                    {
                        // Since we accepted new link, lets add one to our list of links.
                        m_Links.push_back({ ed::LinkId(m_NextLinkId++), inputPinId, outputPinId });

                        // Draw new link.
                        ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
                    }

                    // You may choose to reject connection between these nodes
                    // by calling ed::RejectNewItem(). This will allow editor to give
                    // visual feedback by changing link thickness and color.
                }
            }
        }
        ed::EndCreate(); // Wraps up object creation action handling.


        // Handle deletion action
        if (ed::BeginDelete())
        {
            ed::NodeId DeletedNode;
            while (ed::QueryDeletedNode(&DeletedNode))   //delete a node
            {
                if (ed::AcceptDeletedItem())
                {
                    for (size_t i = 0; i < Nodes.size(); ++i) {
                        Node* node = Nodes[i];

                        if (node->ID == DeletedNode) {
                            delete node;                     
                            Nodes.erase(Nodes.begin() + i);  
                            --i;                             
                        }
                    }
                }
            }



            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
                if (ed::AcceptDeletedItem())
                {
                    for (auto& link : m_Links)
                    {
                        if (link.Id == deletedLinkId)
                        {
                            m_Links.erase(&link);
                            break;
                        }
                    }
                }

            }
        }
        ed::EndDelete(); // Wrap up deletion action



        // End of interaction with editor.
        ed::End();

        if (m_FirstFrame)
            ed::NavigateToContent(0.0f);

        ed::SetCurrentEditor(nullptr);

        m_FirstFrame = false;

        // ImGui::ShowMetricsWindow();
    }

    ed::EditorContext*   m_Context = nullptr;    // Editor context, required to trace a editor state.
    bool                 m_FirstFrame = true;    // Flag set for first frame only, some action need to be executed once.
    ImVector<LinkInfo>   m_Links;                // List of live links. It is dynamic unless you want to create read-only view over nodes.
    int                  m_NextLinkId = 100;     // Counter to help generate link ids. In real application this will probably based on pointer to user data structure.
};

int Main(int argc, char** argv)
{
    Example exampe("Basic Interaction", argc, argv);

    if (exampe.Create())
        return exampe.Run();

    return 0;
}