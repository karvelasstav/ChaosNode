#include <imgui.h>
#include <imgui_node_editor.h>
#include <application.h>
#include <vector>
#include <fstream>
#include <cctype>
#include <unordered_set>
#include <functional>
#include <algorithm>
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



struct Example :
    public Application
{
    int uniqueId = 1;

    std::vector<Node*> Nodes{};
    std::vector<LinkInfo*> Links{};
    std::vector<Pin*> Pins{};

    std::vector<function> funcs{};
    std::string g_TextToParse = "";

    std::vector<Node*> root_nodes{};

    // --- Quick node creation UI ("Shift + A") ---
    bool   m_ShowCreateNode = false;
    bool   m_FocusCreateNodeSearch = false;
    ImVec2 m_CreateNodePosCanvas = ImVec2(0.0f, 0.0f);
    ImVec2 m_CreateNodePopupPos = ImVec2(0.0f, 0.0f);
    char   m_CreateNodeFilter[64] = { 0 };   // text filter (function name prefix)
    int    m_CreateNodeTypeFilter = 0;       // 0 = All, 1..N = specific PinType




    const float LAYOUT_X_STEP = 230.0f;    // horizontal spacing between columns
    const float LAYOUT_Y_STEP = 90.0f;     // vertical spacing between siblings
    const float LAYOUT_ROOT_GAP = 120.0f;    // vertical gap between different root trees

    // Recursively layout a subtree. Returns "height" in rows.
    float LayoutSubtree(Node* node, int depth, float& yCursor,
        std::unordered_set<Node*>& visited)
    {
        if (!node)
            return 0.0f;

        // Avoid infinite recursion on cycles
        if (visited.count(node))
            return 0.0f;
        visited.insert(node);

        // Gather real children (skip nulls)
        std::vector<Node*> children;
        children.reserve(node->OutputNodes.size());
        for (Node* c : node->OutputNodes)
            if (c)
                children.push_back(c);

        // Leaf node: just place it and advance y
        if (children.empty())
        {
            node->Start_pos = ImVec2(depth * LAYOUT_X_STEP, yCursor);
            yCursor += LAYOUT_Y_STEP;
            return 1.0f;
        }

        // Internal node: first layout children, then place node in the vertical middle
        float subtreeStartY = yCursor;
        float totalRows = 0.0f;

        for (Node* c : children)
        {
            totalRows += LayoutSubtree(c, depth + 1, yCursor, visited);
        }

        if (totalRows <= 0.0f)
            totalRows = 1.0f; // safety

        float centerY = subtreeStartY + (totalRows * LAYOUT_Y_STEP) * 0.5f;
        node->Start_pos = ImVec2(depth * LAYOUT_X_STEP, centerY);

        return totalRows;
    }

    // Layout all graphs starting from root_nodes
    void AutoLayoutGraphs()
    {
        std::unordered_set<Node*> visited;
        float rootY = 0.0f;

        for (Node* root : root_nodes)
        {
            if (!root)
                continue;

            float yCursor = rootY;
            float rows = LayoutSubtree(root, /*depth=*/0, yCursor, visited);

            // Leave some gap before the next root tree
            rootY = yCursor + LAYOUT_ROOT_GAP;
        }
    }

    Pin* MakePin(PinType type, PinKind kind)
    {
        auto* pin = new Pin{ ed::PinId(uniqueId++), type, kind };
        Pins.push_back(pin);
        return pin;
    }

    Node* MakeBasicNode(const std::string& name,
        PinType inputType,
        std::initializer_list<PinType> outputTypes,
        ImVec2 startPos, std::string desc = "", NodeType nodetype = NodeType::Basic)
    {
        Pin* inputPin = MakePin(inputType, PinKind::Input);

        std::vector<Pin*> outputs;
        outputs.reserve(outputTypes.size());
        for (auto t : outputTypes)
            outputs.push_back(MakePin(t, PinKind::Output));

        auto* node = new Node{ ed::NodeId(uniqueId++), name, inputPin, std::move(outputs), startPos, desc,nodetype };

        inputPin->NodePtr = node;
        for (auto t : outputs)
        {
            t->NodePtr = node;
        }
        Nodes.push_back(node);
        if (nodetype == NodeType::Primary) { root_nodes.push_back(node); }
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

        std::ifstream file("ModdingInfo.txt");
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

                
                PinType pt;
                if (PinTypeFromString(cat, pt))
                {
                    currentCategory = pt;
                    hasCategory = true;
                }
                else
                {
                    hasCategory = false;
                }
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
                bool isType = PinTypeFromString(t, pt);

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

        // Title
        if (node->Type == NodeType::Constant)
            ImGui::Text("Constant");
        else
            ImGui::Text("%s", node->Name.c_str());


// Left side: input
        ImGui::BeginGroup();
        if (node->Type != NodeType::Primary && node->InputPin)
        {
            ed::BeginPin(node->InputPin->ID, ed::PinKind::Input);
            ImColor col = GetPinColor(node->InputPin->Type);
            ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)col);
            ImGui::Text("%s", PinTypeToString(node->InputPin->Type));
            ImGui::PopStyleColor();
            ed::EndPin();
        }
        ImGui::EndGroup();

        ImGui::SameLine();



       // Right side: outputs (none for constants)
        if (node->Type != NodeType::Constant)
        {
            ImGui::BeginGroup();
            for (auto* out : node->OutputPins)
            {
                ed::BeginPin(out->ID, ed::PinKind::Output);
                ImColor col = GetPinColor(out->Type);
                ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)col);
                ImGui::Text("%s", PinTypeToString(out->Type));
                ImGui::PopStyleColor();
                ed::EndPin();
            }
            ImGui::EndGroup();
        }


        // --- Constant value widget ---
        if (node->Type == NodeType::Constant)
        {
            auto& io = ImGui::GetIO();
            // Disable node editor shortcuts when typing (like the widget example)
            ed::EnableShortcuts(!io.WantTextInput);

            ImGui::Text("Value:");
            ImGui::SameLine();

            // Make a stable ID per node so multiple nodes can each have an InputText
            ImGui::PushID(node);

            // Use a small buffer; copy from node->Name
            char buf[128];
            std::snprintf(buf, sizeof(buf), "%s", node->Name.c_str());

            ImGui::PushItemWidth(120.0f);
            if (ImGui::InputText("##ConstName", buf, IM_ARRAYSIZE(buf)))
            {
                // Write the edited name back into the node
                node->Name = buf;
            }
            ImGui::PopItemWidth();

            ImGui::PopID();
        }

        ed::EndNode();
    }


    void DrawNodes(std::vector<Node*> nodes)
    {
        for (Node* node : nodes)
        {
            DrawNode(node);
        }
    }



    void ParseNodes(std::string& text)
    {
        // If there are no root nodes, just append the message and return
        if (root_nodes.empty())
        {
            if (!text.empty() && text.back() != '\n')
                text += '\n';

            text += "a root node is needed";
            return;
        }

        // For cycle detection and to avoid reprocessing nodes
        std::unordered_set<Node*> visited;
        std::unordered_set<Node*> recursionStack;

        bool cycleDetected = false;

        // Depth-first traversal
        std::function<void(Node*)> dfs = [&](Node* node)
            {
                if (!node)
                    return;

                // Cycle detection: if the node is already in the current recursion stack
                if (recursionStack.find(node) != recursionStack.end())
                {
                    cycleDetected = true;
                    // Do not recurse further from this node to avoid infinite loop
                    return;
                }

                // If we've already fully processed this node, skip it
                if (visited.find(node) != visited.end())
                    return;

                visited.insert(node);
                recursionStack.insert(node);

                // Append this node's name on its own line
                if (!text.empty() && text.back() != '\n')
                    text += ' ';

                text += node->Name;

                // If this is a Constant node, treat it as a leaf and go back up
                if (node->Type == NodeType::Constant )
                {
                    recursionStack.erase(node);
                    return;
                }

                // Go down each output node in order (depth-first)
                for (Node* out : node->OutputNodes)
                {
                    dfs(out);
                }

                // Done exploring this path
                recursionStack.erase(node);
            };

        // Start DFS from each root node
        for (Node* root : root_nodes)
        {
            dfs(root);
            text += '\n';
        }

        // If a cycle was detected anywhere, note it at the end of the string
        if (cycleDetected)
        {
            if (!text.empty() && text.back() != '\n')
                text += '\n';

            text += "cycle detected";
        }
    }
    void ParseText(const std::string& text)
    {
        // 0) Clear existing graph
        for (Node* n : Nodes)
            delete n;
        Nodes.clear();
        Pins.clear();
        m_Links.clear();
        root_nodes.clear();

        uniqueId = 1;
        m_NextLinkId = 100;

        // Helper: split string into lines
        std::vector<std::string> lines;
        {
            std::string cur;
            for (char c : text)
            {
                if (c == '\n')
                {
                    lines.push_back(cur);
                    cur.clear();
                }
                else
                    cur.push_back(c);
            }
            lines.push_back(cur);
        }

        // Trim helper
        auto trim = [](std::string& s)
            {
                size_t b = 0;
                while (b < s.size() && std::isspace((unsigned char)s[b])) ++b;
                size_t e = s.size();
                while (e > b && std::isspace((unsigned char)s[e - 1])) --e;
                s = s.substr(b, e - b);
            };

        // Tokenize a line
        auto tokenize = [](const std::string& s) -> std::vector<std::string>
            {
                std::vector<std::string> out;
                std::string cur;
                for (char c : s)
                {
                    if (std::isspace((unsigned char)c))
                    {
                        if (!cur.empty())
                        {
                            out.push_back(cur);
                            cur.clear();
                        }
                    }
                    else
                        cur.push_back(c);
                }
                if (!cur.empty())
                    out.push_back(cur);
                return out;
            };

        // 1) Determine if we're in explicit-root mode
        bool explicitRootMode = false;
        for (auto l : lines)
        {
            trim(l);
            if (l.empty()) continue;
            auto toks = tokenize(l);
            if (!toks.empty() && toks[0] == "Root")
            {
                explicitRootMode = true;
                break;
            }
            else
            {
                // first non-empty line is not Root => implicit-root mode
                explicitRootMode = false;
                break;
            }
        }

        // 2) Build graphs = vector< vector<string> >
        std::vector<std::vector<std::string>> graphs;

        if (!explicitRootMode)
        {
            // All text is one graph, newlines are just whitespace
            std::vector<std::string> all;
            for (auto l : lines)
            {
                trim(l);
                if (l.empty()) continue;
                auto toks = tokenize(l);
                all.insert(all.end(), toks.begin(), toks.end());
            }
            if (!all.empty())
                graphs.push_back(std::move(all));
        }
        else
        {
            // Explicit-root mode: each line starting with Root begins a new graph
            std::vector<std::string> current;

            for (auto l : lines)
            {
                trim(l);
                if (l.empty()) continue;
                auto toks = tokenize(l);
                if (toks.empty()) continue;

                if (toks[0] == "Root")
                {
                    // Start new graph
                    if (!current.empty())
                    {
                        graphs.push_back(std::move(current));
                        current.clear();
                    }
                    // Drop the "Root" token itself, rest belong to this graph
                    for (size_t i = 1; i < toks.size(); ++i)
                        current.push_back(toks[i]);
                }
                else
                {
                    // Continuation of current graph (ignore newline)
                    for (auto& t : toks)
                        current.push_back(t);
                }
            }

            if (!current.empty())
                graphs.push_back(std::move(current));
        }

        if (graphs.empty())
            return;


//3)
       // a) Any function with this name (used when we don't care about overloads)
        auto findFuncAny = [&](const std::string& name) -> const function*
            {
                for (const auto& f : funcs)
                    if (f.Name == name)
                        return &f;
                return nullptr;
            };

        // b) Overload-aware: pick function whose input matches `expected` if possible
        auto findFunc = [&](const std::string& name, PinType expected) -> const function*
            {
                const function* fallback = nullptr;

                for (const auto& f : funcs)
                {
                    if (f.Name != name)
                        continue;

                    // Perfect match: same name + same input type
                    if (f.input == expected)
                        return &f;

                    // Remember the first function with that name as a fallback
                    if (!fallback)
                        fallback = &f;
                }

                return fallback; // may be nullptr if name not found, or "closest" one
            };


        // 4) Recursive expression parser
        std::function<Node* (const std::vector<std::string>&, size_t&, PinType)> ParseExpr =
            [&](const std::vector<std::string>& toks, size_t& idx, PinType expected) -> Node*
            {
                if (idx >= toks.size())
                {
                    g_TextToParse += "\nerror: unexpected end of input while expecting ";
                    g_TextToParse += PinTypeToString(expected);
                    return nullptr;
                }

                const std::string& tok = toks[idx++];
                const function* f = findFunc(tok, expected);

                if (f)
                {
                    // If input type matches, normal function handling.
                    if (CanConnectPinTypes(f->input, expected))
                    {
                        Node* node = NodeFromFunciton(*f, ImVec2(0, 0));
                        if (!node) return nullptr;

                        for (int p = 0; p < f->output_size; ++p)
                        {
                            Node* child = ParseExpr(toks, idx, f->output[p]);
                            if (!child)
                                return node; // keep partial graph, error already reported

                            node->OutputNodes[p] = child;
                            child->InputNode = node;

                            m_Links.push_back({
                                ed::LinkId(m_NextLinkId++),
                                child->InputPin->ID,
                                node->OutputPins[p]->ID
                                });
                        }

                        return node;
                    }

                    // --- New special-case logic: treat Trigger->void functions as ABILITY constants ---
                    bool hasAnyOverload = false;
                    bool allTriggerVoid = true;

                    for (const auto& cand : funcs)
                    {
                        if (cand.Name != tok)
                            continue;

                        hasAnyOverload = true;

                        // We only "whitelist" functions that are Trigger input and no outputs.
                        if (!(cand.input == PinType::Trigger && cand.output_size == 0))
                        {
                            allTriggerVoid = false;
                            break;
                        }
                    }

                    if (hasAnyOverload && allTriggerVoid && (expected == PinType::ABILITY|| expected==PinType::Ability))
                    {
                        // Interpret this token as a constant of type ABILITY, as requested.
                        Node* c = MakeBasicNode(
                            tok,
                            PinType::ABILITY,          // constant type you wanted
                            {},
                            ImVec2(0, 0),
                            "",
                            NodeType::Constant
                        );
                        return c;
                    }

                    // Fallback: real type mismatch error like before
                    g_TextToParse += "\nerror: type mismatch at token '";
                    g_TextToParse += tok;
                    g_TextToParse += "': function expects ";
                    g_TextToParse += PinTypeToString(f->input);
                    g_TextToParse += " but parent needed ";
                    g_TextToParse += PinTypeToString(expected);
                    return nullptr;
                }
                else
                {
                    // Still no function with that name at all: create constant of the expected type (old behaviour)
                    Node* c = MakeBasicNode(tok, expected, {}, ImVec2(0, 0), "", NodeType::Constant);
                    return c;
                }

            };

        // 5) Parse each graph, create Primary Root
        for (auto& g : graphs)
        {
            if (g.empty())
                continue;

            // Infer root output type from first token if possible
            PinType rootOutputType = PinType::Trigger;
            if (!g.empty())
            {
                if (const function* f0 = findFuncAny(g[0]))
                    rootOutputType = f0->input;
            }

            // Create primary root node; MakeBasicNode will add it to root_nodes
            Node* root = MakeBasicNode(
                "Root",
                PinType::Action,         // dummy input
                { rootOutputType },      // one output
                ImVec2(0, 0),
                "",
                NodeType::Primary
            );

            size_t idx = 0;
            Node* child = ParseExpr(g, idx, rootOutputType);

            if (child)
            {
                root->OutputNodes[0] = child;
                child->InputNode = root;

                m_Links.push_back({
                    ed::LinkId(m_NextLinkId++),
                    child->InputPin->ID,
                    root->OutputPins[0]->ID
                    });
            }

            if (idx < g.size())
            {
                g_TextToParse += "\nwarning: extra tokens at end of line starting with '";
                g_TextToParse += g[0];
                g_TextToParse += "'";
            }
        }

        AutoLayoutGraphs();
        ed::SetCurrentEditor(m_Context);
        for (Node* n : Nodes)
            ed::SetNodePosition(n->ID, n->Start_pos);
    }





    using Application::Application;

    void OnStart() override
    {
        ed::Config config;
        config.SettingsFile = "BasicInteraction.json";
        m_Context = ed::CreateEditor(&config);

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

        //window with textbox
        {

            // Separate "Text Parser" window with global string & buttons
            ImGui::Begin("Text Parser");

            
            auto Clamp = [](float v, float lo, float hi)
                {
                    return (v < lo) ? lo : (v > hi) ? hi : v;
                };


            ImVec2 size = ImGui::GetWindowSize();

            // Set your constraints
            float fixedHeight = 360.0f;
            float minWidth = 300.0f;
            float maxWidth = 1500.0f;

            // Clamp width, force height
            size.x = Clamp(size.x, minWidth, maxWidth);
            size.y = fixedHeight;

            ImGui::SetWindowSize(size);


            // Local editable buffer for ImGui (backed by the global string)
            static char textBuf[80192]; //TODO: i know there is a way to use a string and dynamically resize it but the buffer should realistically be sufficient
            static bool initialized = false;


            std::snprintf(textBuf, sizeof(textBuf), "%s", g_TextToParse.c_str());
            initialized = true;

            // Multiline textbox
            if (ImGui::InputTextMultiline(
                "##GlobalText",
                textBuf,
                IM_ARRAYSIZE(textBuf),
                ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16.0f),
                ImGuiInputTextFlags_AllowTabInput))
            {
                // If user edited it, write back to the global string
                g_TextToParse = textBuf;
            }

            // Buttons
            if (ImGui::Button("Parse nodes"))
            {
                ParseNodes(g_TextToParse);
            }

            ImGui::SameLine();

            if (ImGui::Button("Parse text"))
            {
                ParseText(g_TextToParse);
            }

            ImGui::End();
        }

        ed::SetCurrentEditor(m_Context);









        // Keyboard shortcut: Shift + A opens the "create node" search popup

        {
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
        }
        // Start interaction with editor.
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));

        DrawNodes(Nodes);

        // Submit Links
        for (auto& linkInfo : m_Links)
        {
            Pin* outputPin = nullptr;
            for (Pin* p : Pins)
                if (p->ID == linkInfo.OutputId)
                    outputPin = p;

            ImVec4 color = GetPinColor(outputPin ? outputPin->Type : PinType::Trigger);

            ed::Link(
                linkInfo.Id,
                linkInfo.InputId,
                linkInfo.OutputId,
                color,
                2.0f  // thicker is nicer
            );
        }

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
                ed::PinId actual_input_pin; //after following for loop stores the id of the actual input pin

                PinType Atype;
                PinType Btype;
                for (Pin* pin : Pins)
                {
                    if (Aisinput && Bisinput) { break; }

                    if (pin->ID == InputPinId)
                    {
                        Atype = pin->Type;
                        n1 = pin->NodePtr;
                        if (pin->Kind == PinKind::Input)
                        {
                            Aisinput = true;
                            actual_input_pin = pin->ID;
                        }

                    }
                    if (pin->ID == OutputPinId)
                    {
                        Btype = pin->Type;
                        n2 = pin->NodePtr;
                        if (pin->Kind == PinKind::Input)
                        {
                            Bisinput = true;
                            actual_input_pin = pin->ID;
                        }

                    }
                }
                PinType inputType;
                PinType outputType;

                if (Aisinput)
                {
                    inputType = Btype;
                    outputType = Atype;
                }
                else
                {
                    inputType = Atype;
                    outputType = Btype;
                }



                if (InputPinId && OutputPinId && (Aisinput ^ Bisinput) && (n1 != n2) && CanConnectPinTypes(inputType, outputType)) // both are valid, let's accept link
                {



                    Node* input_node; //node that has the input pin
                    Node* output_node; //node that has the output pin
                    if (Aisinput) { input_node = n1;  output_node = n2; }
                    else { input_node = n2; output_node = n1; ed::PinId t = InputPinId; InputPinId = OutputPinId; OutputPinId = t; }


                    // ed::AcceptNewItem() return true when user release mouse button.
                    if (ed::AcceptNewItem())
                    {
                       
                        for (size_t i = 0; i < output_node->OutputPins.size(); i++)
                        {
                            if (output_node->OutputPins[i]->ID == OutputPinId) {
                                if (output_node->OutputNodes[i] != nullptr || input_node->InputNode != nullptr) { goto SkipCreation; }
                                output_node->OutputNodes[i] = input_node;
                            }
                        }
                        input_node->InputNode = output_node; //very spaghetti but should work

                        // Since we accepted new link, lets add one to our list of links.
                        m_Links.push_back({ ed::LinkId(m_NextLinkId++), InputPinId, OutputPinId });

                        // Draw new link.
                        ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
                    }


                }
                else { ed::RejectNewItem(); }
            }
        }
        SkipCreation:
        ed::EndCreate(); // Wraps up object creation action handling.


        // Handle deletion action
        if (ed::BeginDelete())
        {




            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
                if (!ed::AcceptDeletedItem())
                    continue;

                for (int i = 0; i < m_Links.size(); ++i)
                {
                    auto& link = m_Links[i];
                    if (link.Id != deletedLinkId)
                        continue;

                    Pin* input_pin = nullptr;
                    Pin* output_pin = nullptr;

                    // Find the two pins
                    for (Pin* pin : Pins)
                    {
                        if (link.InputId == pin->ID) input_pin = pin;
                        if (link.OutputId == pin->ID) output_pin = pin;
                    }

                    // Make sure input_pin is actually an Input and output_pin an Output
                    if (input_pin && input_pin->Kind == PinKind::Output)
                        std::swap(input_pin, output_pin);

                    Node* input_node = input_pin ? input_pin->NodePtr : nullptr;
                    Node* output_node = output_pin ? output_pin->NodePtr : nullptr;

                    // Clear the child's InputNode
                    if (input_node)
                        input_node->InputNode = nullptr;

                    // Clear the parent's OutputNodes[] slot that corresponds to this output pin
                    if (output_node && output_pin)
                    {
                        for (size_t j = 0; j < output_node->OutputPins.size(); ++j)
                        {
                            if (output_node->OutputPins[j]->ID == output_pin->ID)
                            {
                                output_node->OutputNodes[j] = nullptr;
                                break;
                            }
                        }
                    }

                    // Remove link from list
                    m_Links.erase(m_Links.begin() + i);
                    break;
                }
            }
       
            ed::NodeId DeletedNode;
            while (ed::QueryDeletedNode(&DeletedNode))   //delete a node
            {
                if (ed::AcceptDeletedItem())
                {


                    for (size_t i = 0; i < Nodes.size(); ++i) {
                        Node* node = Nodes[i];

                        if (node->ID == DeletedNode) {

                            if (node->Type == NodeType::Primary)
                            {
                                for (size_t k = 0; k < root_nodes.size(); ++k)
                                {
                                    root_nodes.erase(root_nodes.begin() + k);
                                }
                            }

                            node->RemovePinsFrom(Pins);
                            delete node;
                            Nodes.erase(Nodes.begin() + i);
                            --i;
                        }
                    }
                }
            }
        
        
        }
        ed::EndDelete(); // Wrap up deletion action



        // End of interaction with editor.
        ed::End();


        if (auto hoveredId = ed::GetHoveredNode())
        {
            Node* hoveredNode = nullptr;
            for (Node* n : Nodes)
            {
                if (n->ID == hoveredId)
                {
                    hoveredNode = n;
                    break;
                }
            }

            if (hoveredNode && !hoveredNode->description.empty())
            {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(hoveredNode->Name.c_str());
                ImGui::Separator();

                // Fix: manually choose wrap width (e.g. 40 characters-ish)
                float wrapWidth = ImGui::GetFontSize() * 40.0f;
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrapWidth);
                ImGui::TextUnformatted(hoveredNode->description.c_str());
                ImGui::PopTextWrapPos();

                ImGui::EndTooltip();
            }
        }



        //search with shift+A
        if (m_ShowCreateNode)
        {
            // Position popup near where the user opened it
            ImGui::SetNextWindowPos(m_CreateNodePopupPos, ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoCollapse;

            if (ImGui::Begin("Create Node", &m_ShowCreateNode, flags))
            {
                const auto& allTypes = GetAllPinTypes();

                if (m_FocusCreateNodeSearch)
                {
                    ImGui::SetKeyboardFocusHere();
                    m_FocusCreateNodeSearch = false;
                }

                ImGui::Text("Search function:");
                ImGui::InputText("##Filter", m_CreateNodeFilter, IM_ARRAYSIZE(m_CreateNodeFilter));

                // -----------------------------
                // Type filter combo (0 = All)
                // -----------------------------
                ImGui::Separator();

                ImGui::Text("Input type:");
                ImGui::SameLine();

                const char* typeFilterLabel =
                    (m_CreateNodeTypeFilter == 0)
                    ? "All"
                    : allTypes[m_CreateNodeTypeFilter - 1].label;

                if (ImGui::BeginCombo("##TypeFilter", typeFilterLabel))
                {
                    // "All"
                    bool selected = (m_CreateNodeTypeFilter == 0);
                    if (ImGui::Selectable("All", selected))
                        m_CreateNodeTypeFilter = 0;
                    if (selected)
                        ImGui::SetItemDefaultFocus();

                    // Actual types
                    for (int i = 0; i < (int)allTypes.size(); ++i)
                    {
                        bool sel = (m_CreateNodeTypeFilter == i + 1);
                        if (ImGui::Selectable(allTypes[i].label, sel))
                            m_CreateNodeTypeFilter = i + 1;
                        if (sel)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                // -----------------------------
                // Quick create
                // -----------------------------
                ImGui::Separator();
                ImGui::Text("Quick create:");

                // Constant type selection
                static int s_ConstTypeIndex = 0; // index into allTypes (0..N-1)
                ImGui::Text("Constant of type:");
                ImGui::SameLine();

                const char* constTypeLabel = allTypes[s_ConstTypeIndex].label;

                if (ImGui::BeginCombo("##ConstType", constTypeLabel))
                {
                    for (int i = 0; i < (int)allTypes.size(); ++i)
                    {
                        bool selected = (i == s_ConstTypeIndex);
                        if (ImGui::Selectable(allTypes[i].label, selected))
                            s_ConstTypeIndex = i;
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Create Constant"))
                {
                    PinType chosenType = allTypes[s_ConstTypeIndex].type;
                    std::string nodeName = "Constant";

                    // 1 input (chosen type), no outputs, NodeType::Constant
                    Node* constantNode = MakeBasicNode(
                        nodeName,
                        chosenType,
                        {},
                        m_CreateNodePosCanvas,
                        "",
                        NodeType::Constant
                    );

                    if (constantNode)
                        ed::SetNodePosition(constantNode->ID, m_CreateNodePosCanvas);

                    m_ShowCreateNode = false;
                }

                // Root type selection
                static int s_RootTypeIndex = 0; // index into allTypes
                ImGui::Text("Root of type:");
                ImGui::SameLine();

                const char* rootTypeLabel = allTypes[s_RootTypeIndex].label;

                if (ImGui::BeginCombo("##RootType", rootTypeLabel))
                {
                    for (int i = 0; i < (int)allTypes.size(); ++i)
                    {
                        bool selected = (i == s_RootTypeIndex);
                        if (ImGui::Selectable(allTypes[i].label, selected))
                            s_RootTypeIndex = i;
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Create Root"))
                {
                    PinType chosenType = allTypes[s_RootTypeIndex].type;
                    std::string nodeName = "Root";

                    // Use Action as dummy input type as requested.
                    // NodeType::Primary hides the input pin in DrawNode.
                    Node* rootNode = MakeBasicNode(
                        nodeName,
                        PinType::Action,           // dummy input type
                        { chosenType },            // one output of chosen type
                        m_CreateNodePosCanvas,
                        "",
                        NodeType::Primary
                    );

                    if (rootNode)
                        ed::SetNodePosition(rootNode->ID, m_CreateNodePosCanvas);

                    m_ShowCreateNode = false;
                }

                // -----------------------------------------------------------------
                // Existing function search results
                // -----------------------------------------------------------------
                ImGui::Separator();
                ImGui::Text("Results:");

                // Close with Escape
                if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                {
                    m_ShowCreateNode = false;
                }

                ImVec2 avail = ImGui::GetContentRegionAvail();
                ImGui::BeginChild("##FunctionList", avail, true);

                const std::string prefix = m_CreateNodeFilter;
                for (std::size_t i = 0; i < funcs.size(); ++i)
                {
                    const function& f = funcs[i];

                    // Type filter
                    if (m_CreateNodeTypeFilter != 0)
                    {
                        PinType expected = allTypes[m_CreateNodeTypeFilter - 1].type;
                        if (f.input != expected)
                            continue;
                    }

                    // Name prefix filter (case-insensitive)
                    if (!prefix.empty() && !StartsWithCaseInsensitive(f.Name, prefix))
                        continue;

                    ImGui::PushID(static_cast<int>(i));

                    // Main selectable: function name
                    bool clicked = ImGui::Selectable(f.Name.c_str());

                    // Build inline signature: "InputType -> Out1, Out2" / "InputType -> void"
                    std::string sig;
                    sig += PinTypeToString(f.input);
                    sig += " -> ";

                    if (f.output_size <= 0)
                    {
                        sig += "void";
                    }
                    else
                    {
                        for (int oi = 0; oi < f.output_size; ++oi)
                        {
                            if (oi > 0)
                                sig += ", ";
                            sig += PinTypeToString(f.output[oi]);
                        }
                    }

                    // Draw signature on same line, dimmer as secondary text
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s", sig.c_str());

                    ImGui::PopID();

                    if (clicked)
                    {
                        // Create node at stored canvas position
                        Node* created = NodeFromFunciton(f, m_CreateNodePosCanvas);
                        if (created)
                            ed::SetNodePosition(created->ID, m_CreateNodePosCanvas);

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

    ed::EditorContext* m_Context = nullptr;    // Editor context, required to trace a editor state.
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