#include <imgui.h>
#include <imgui_node_editor.h>
#include <application.h>
#include <vector>
#include <fstream>
#include <cctype>
#include "Nodes.h"

namespace ed = ax::NodeEditor;


static bool StartsWithCaseInsensitive(const std::string& text, const std::string& prefix)
{
    if (prefix.empty())
        return true;

    if (text.size() < prefix.size())
        return false;

    for (std::size_t i = 0; i < prefix.size(); ++i)
    {
        unsigned char c1 = static_cast<unsigned char>(text[i]);
        unsigned char c2 = static_cast<unsigned char>(prefix[i]);
        if (std::tolower(c1) != std::tolower(c2))
            return false;
    }

    return true;
}



struct Example:
    public Application
{
    int uniqueId = 1;
   
    std::vector<Node*> Nodes{};
    std::vector<LinkInfo*> Links{};
    std::vector<Pin*> Pins{};

    std::vector<function> funcs{};


    // --- Quick node creation UI ("Shift + A") ---
    bool   m_ShowCreateNode = false;
    bool   m_FocusCreateNodeSearch = false;
    ImVec2 m_CreateNodePosCanvas = ImVec2(0.0f, 0.0f);
    ImVec2 m_CreateNodePopupPos = ImVec2(0.0f, 0.0f);
    char   m_CreateNodeFilter[64] = { 0 };   // text filter (function name prefix)
    int    m_CreateNodeTypeFilter = 0;       // 0 = All, 1..N = specific PinType


    Pin* MakePin(PinType type, PinKind kind)
    {
        auto* pin = new Pin{ ed::PinId(uniqueId++), type, kind };
        Pins.push_back(pin);
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

    Node* NodeFromFunciton(const function& f, ImVec2 startPos)
    {

        Node* node = nullptr;

        switch (f.output_size)
        {
        case 0:
            node = MakeBasicNode(f.Name, f.input, {}, startPos);
            break;
        case 1:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0] }, startPos);
            break;
        case 2:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1] }, startPos);
            break;
        case 3:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1], f.output[2] }, startPos);
            break;
        case 4:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1], f.output[2], f.output[3] }, startPos);
            break;
        case 5:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1], f.output[2], f.output[3],
                  f.output[4] }, startPos);
            break;
        case 6:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1], f.output[2], f.output[3],
                  f.output[4], f.output[5] }, startPos);
            break;
        case 7:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1], f.output[2], f.output[3],
                  f.output[4], f.output[5], f.output[6] }, startPos);
            break;
        case 8:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1], f.output[2], f.output[3],
                  f.output[4], f.output[5], f.output[6], f.output[7] }, startPos);
            break;
        case 9:
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1], f.output[2], f.output[3],
                  f.output[4], f.output[5], f.output[6], f.output[7],
                  f.output[8] }, startPos);
            break;
        default: // 10 or more: clamp to first 10 (struct only stores 10)
            node = MakeBasicNode(f.Name, f.input,
                { f.output[0], f.output[1], f.output[2], f.output[3],
                  f.output[4], f.output[5], f.output[6], f.output[7],
                  f.output[8], f.output[9] }, startPos);
            break;
        }

        if (node)
        {
            node->description = f.description;

        }

        return node;
    }



    void ParseModInfo(std::vector<function>& dir)
    {
        dir.clear();

        std::ifstream file("E:\\Steam stuff\\steamapps\\common\\Cube Chaos\\ModdingInfo.txt");
        if (!file.is_open())
            return; 

        std::string line;
        bool hasCategory = false;
        PinType currentCategory = PinType::Trigger; // dummy init

        while (std::getline(file, line))
        {
            // trim whitespace from both ends
            std::size_t start = 0;
            while (start < line.size() && std::isspace(static_cast<unsigned char>(line[start])))
                ++start;

            std::size_t end = line.size();
            while (end > start && std::isspace(static_cast<unsigned char>(line[end - 1])))
                --end;

            if (start >= end)
                continue;

            std::string trimmed = line.substr(start, end - start);

            if (trimmed == "Modding Info")
                continue;

            // category header: e.g. "Trigger:"
            if (trimmed.back() == ':')
            {
                std::string cat = trimmed.substr(0, trimmed.size() - 1);

                hasCategory = true;
                if (cat == "Trigger")   currentCategory = PinType::Trigger;
                else if (cat == "Action")    currentCategory = PinType::Action;
                else if (cat == "BOOLEAN")   currentCategory = PinType::Boolean;
                else if (cat == "CUBE")      currentCategory = PinType::Cube;
                else if (cat == "DIRECTION") currentCategory = PinType::Direction;
                else if (cat == "DOUBLE")    currentCategory = PinType::Double;
                else if (cat == "PERK")      currentCategory = PinType::Perk;
                else if (cat == "POSITION")  currentCategory = PinType::Position;
                else if (cat == "STRING")    currentCategory = PinType::String;
                else if (cat == "ABILITY")   currentCategory = PinType::Ability;
                else
                    hasCategory = false; // unknown category, skip until next known

                continue;
            }

            if (!hasCategory)
                continue;

            // split into before/inside quotes
            std::string beforeQuote = trimmed;
            std::string description;

            std::size_t firstQuote = trimmed.find('"');
            if (firstQuote != std::string::npos)
            {
                beforeQuote = trimmed.substr(0, firstQuote);

                std::size_t lastQuote = trimmed.find_last_of('"');
                if (lastQuote != std::string::npos && lastQuote > firstQuote)
                {
                    // raw description (keeps colour codes etc.; you can clean it later if you want)
                    description = trimmed.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                }
            }

            // re-trim beforeQuote
            std::size_t bs = 0;
            while (bs < beforeQuote.size() && std::isspace(static_cast<unsigned char>(beforeQuote[bs])))
                ++bs;
            std::size_t be = beforeQuote.size();
            while (be > bs && std::isspace(static_cast<unsigned char>(beforeQuote[be - 1])))
                --be;

            if (bs >= be)
                continue;

            beforeQuote = beforeQuote.substr(bs, be - bs);

            // tokenize by whitespace
            std::vector<std::string> tokens;
            std::size_t pos = 0;
            while (pos < beforeQuote.size())
            {
                while (pos < beforeQuote.size() &&
                    std::isspace(static_cast<unsigned char>(beforeQuote[pos])))
                    ++pos;

                if (pos >= beforeQuote.size())
                    break;

                std::size_t j = pos;
                while (j < beforeQuote.size() &&
                    !std::isspace(static_cast<unsigned char>(beforeQuote[j])))
                    ++j;

                tokens.emplace_back(beforeQuote.substr(pos, j - pos));
                pos = j;
            }

            if (tokens.empty())
                continue;

            function f;
            f.Name = tokens[0];
            f.input = currentCategory;
            f.output_size = 0;
            f.description = description;

            // map argument tokens to PinType outputs
            for (std::size_t i = 1; i < tokens.size() && f.output_size < 10; ++i)
            {
                const std::string& t = tokens[i];
                PinType pt;
                bool isType = true;

                if (t == "Ability")                 pt = PinType::Ability;
                else if (t == "Action")                  pt = PinType::Action;
                else if (t == "Boolean")                 pt = PinType::Boolean;
                else if (t == "CUBE" || t == "Cube")     pt = PinType::Cube;
                else if (t == "DIRECTION" || t == "Direction")
                    pt = PinType::Direction;
                else if (t == "DOUBLE" || t == "Double" ||
                    t == "TIME" || t == "Time" ||
                    t == "CONSTANT" || t == "Constant" ||
                    t == "STACKING" || t == "Stacking")
                    pt = PinType::Double;
                else if (t == "PERK" || t == "Perk")     pt = PinType::Perk;
                else if (t == "POSITION" || t == "Position")
                    pt = PinType::Position;
                else if (t == "STRING" || t == "String" ||
                    t == "WORD" || t == "Name" || t == "NAME")
                    pt = PinType::String;
                else if (t == "TRIGGER" || t == "Trigger")
                    pt = PinType::Trigger;
                else
                    isType = false;

                if (isType)
                {
                    f.output[f.output_size++] = pt;
                }
            }

            dir.push_back(f);
        }
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
        ParseModInfo(funcs);
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




        // ---------------------------------------------------------------------
   // 1) Keyboard shortcut: Shift + A opens the "create node" search popup
   // ---------------------------------------------------------------------
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        {
            // Don't trigger while another input is active
            if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyItemFocused())
            {
                // Map ImGuiKey_A -> index in io.KeysDown[]
                int keyAIndex = ImGui::GetKeyIndex(ImGuiKey_A);

                if (io.KeyShift && ImGui::IsKeyPressed(keyAIndex, false))
                {
                    m_ShowCreateNode = true;
                    m_FocusCreateNodeSearch = true;

                    ImVec2 mouseScreen = ImGui::GetMousePos();
                    

                    m_CreateNodePopupPos = mouseScreen;
                    m_CreateNodePosCanvas = ed::ScreenToCanvas(mouseScreen);


                    m_CreateNodeFilter[0] = '\0';
                    m_CreateNodeTypeFilter = 0;
                }
            }
        }
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
            ed::PinId InputPinId, OutputPinId;
            if (ed::QueryNewLink(&InputPinId, &OutputPinId))
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

                bool Aisinput = false;
                bool Bisinput = false;

                Node* n1 = nullptr;
                Node* n2 = nullptr; //check if same node input and output. 
                    for (Pin* pin : Pins)
                    {
                        if (Aisinput && Bisinput) { break; }

                        if (pin->ID == InputPinId)
                        {
                            n1 = pin->NodePtr;
                            if (pin->Kind == PinKind::Input)
                            {
                                Aisinput = true;
                            }
                            
                        }
                        if (pin->ID == OutputPinId)
                        {
                            n2 = pin->NodePtr;
                            if (pin->Kind == PinKind::Input)
                            {
                                Bisinput = true;
                            }
            
                        }
                    }
                    

                if (InputPinId && OutputPinId && (Aisinput ^ Bisinput)&&(n1!=n2)) // both are valid, let's accept link
                {
                    // ed::AcceptNewItem() return true when user release mouse button.
                    if (ed::AcceptNewItem())
                    {
                        // Since we accepted new link, lets add one to our list of links.
                        m_Links.push_back({ ed::LinkId(m_NextLinkId++), InputPinId, OutputPinId });

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
                            node->RemovePinsFrom(Pins);
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



        if (m_ShowCreateNode)
        {
            // Position popup near where the user opened it
            ImGui::SetNextWindowPos(m_CreateNodePopupPos, ImGuiCond_Always);

            ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize
                | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoCollapse;

            if (ImGui::Begin("Create Node", &m_ShowCreateNode, flags))
            {
                if (m_FocusCreateNodeSearch)
                {
                    ImGui::SetKeyboardFocusHere();
                    m_FocusCreateNodeSearch = false;
                }

                ImGui::Text("Search function:");
                ImGui::InputText("##Filter", m_CreateNodeFilter, IM_ARRAYSIZE(m_CreateNodeFilter));

                // Type filter combo
                // 0 = All, 1..N map to PinType values
                static const PinType s_TypeOptions[] = {
                    PinType::Trigger,
                    PinType::Action,
                    PinType::Boolean,
                    PinType::Cube,
                    PinType::Direction,
                    PinType::Double,
                    PinType::Perk,
                    PinType::Position,
                    PinType::String,
                    PinType::Ability
                };

                const char* typeLabels[] = {
                    "All",
                    "Trigger",
                    "Action",
                    "Boolean",
                    "Cube",
                    "Direction",
                    "Double",
                    "Perk",
                    "Position",
                    "String",
                    "Ability"
                };

                static_assert(IM_ARRAYSIZE(typeLabels) == IM_ARRAYSIZE(s_TypeOptions) + 1,
                    "Labels and type options size mismatch");

                ImGui::Separator();

                ImGui::Text("Input type:");
                ImGui::SameLine();
                if (ImGui::BeginCombo("##TypeFilter", typeLabels[m_CreateNodeTypeFilter]))
                {
                    for (int i = 0; i < IM_ARRAYSIZE(typeLabels); ++i)
                    {
                        bool selected = (i == m_CreateNodeTypeFilter);
                        if (ImGui::Selectable(typeLabels[i], selected))
                            m_CreateNodeTypeFilter = i;
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::Separator();
                ImGui::Text("Results:");

                // Close with Escape
                if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                {
                    m_ShowCreateNode = false;
                }

                // List of matching functions inside a scrollable region
                ImGui::BeginChild("##FunctionList", ImVec2(300.0f, 300.0f), true);

                const std::string prefix = m_CreateNodeFilter;

                for (std::size_t i = 0; i < funcs.size(); ++i)
                {
                    const function& f = funcs[i];

                    // Type filter
                    if (m_CreateNodeTypeFilter != 0)
                    {
                        PinType expected = s_TypeOptions[m_CreateNodeTypeFilter - 1];
                        if (f.input != expected)
                            continue;
                    }

                    // Name prefix filter (case-insensitive)
                    if (!prefix.empty() && !StartsWithCaseInsensitive(f.Name, prefix))
                        continue;

                    if (ImGui::Selectable(f.Name.c_str()))
                    {
                        // Create node at stored canvas position
                        Node* created = NodeFromFunciton(f, m_CreateNodePosCanvas);
                        if (created)
                        {
                            ed::SetNodePosition(created->ID, m_CreateNodePosCanvas);
                        }

                        m_ShowCreateNode = false;
                        break;
                    }
                }

                ImGui::EndChild();

                ImGui::End();
            }
            else
            {
                // Window closed via close button
                m_ShowCreateNode = false;
            }
        }





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