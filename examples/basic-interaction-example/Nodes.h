#pragma once

#include <string>
#include <vector>
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

// Nodes.h

enum class PinType
{

    Trigger,

    Ability,   
    ABILITY,   
    Action,    
    BOOLEAN,   
    CONSTANT,  
    CUBE,      
    DIRECTION, 
    double_,  
    DOUBLE,   
    Info,     
    Int,      
    List,     
    NAME,     
    PERK,     
    POSITION, 
    STACKING, 
    String,   
    STRING,   
    TIME,     
    WORD      
};

enum class PinKind { Output, Input };
enum class NodeType { Basic, Primary, Constant}; //basic has 1 input and n outputs, primary has no inputs and n outputs and constant has 1 input and 0 outputs
// Nodes.h

inline const char* PinTypeToString(PinType t)
{
    switch (t)
    {
    case PinType::Trigger:   return "Trigger";

    case PinType::Ability:   return "Ability";
    case PinType::ABILITY:   return "ABILITY";
    case PinType::Action:    return "Action";
    case PinType::BOOLEAN:   return "BOOLEAN";
    case PinType::CONSTANT:  return "CONSTANT";
    case PinType::CUBE:      return "CUBE";
    case PinType::DIRECTION: return "DIRECTION";
    case PinType::double_:   return "double";
    case PinType::DOUBLE:    return "DOUBLE";
    case PinType::Info:      return "Info";
    case PinType::Int:       return "int";
    case PinType::List:      return "List";
    case PinType::NAME:      return "NAME";
    case PinType::PERK:      return "PERK";
    case PinType::POSITION:  return "POSITION";
    case PinType::STACKING:  return "STACKING";
    case PinType::String:    return "String";
    case PinType::STRING:    return "STRING";
    case PinType::TIME:      return "TIME";
    case PinType::WORD:      return "WORD";
    }
    return "Unknown";
}

// Nodes.h

inline bool PinTypeFromString(const std::string& s, PinType& out)
{
    if (s == "Trigger") { out = PinType::Trigger;   return true; }

    if (s == "Ability") { out = PinType::Ability;   return true; }
    if (s == "ABILITY") { out = PinType::ABILITY;   return true; }
    if (s == "Action") { out = PinType::Action;    return true; }
    if (s == "BOOLEAN") { out = PinType::BOOLEAN;   return true; }
    if (s == "CONSTANT") { out = PinType::CONSTANT;  return true; }
    if (s == "CUBE") { out = PinType::CUBE;      return true; }
    if (s == "DIRECTION") { out = PinType::DIRECTION; return true; }
    if (s == "double") { out = PinType::double_;   return true; }
    if (s == "DOUBLE") { out = PinType::DOUBLE;    return true; }
    if (s == "Info") { out = PinType::Info;      return true; }
    if (s == "int") { out = PinType::Int;       return true; }
    if (s == "List") { out = PinType::List;      return true; }
    if (s == "NAME") { out = PinType::NAME;      return true; }
    if (s == "PERK") { out = PinType::PERK;      return true; }
    if (s == "POSITION") { out = PinType::POSITION;  return true; }
    if (s == "STACKING") { out = PinType::STACKING;  return true; }
    if (s == "String") { out = PinType::String;    return true; }
    if (s == "STRING") { out = PinType::STRING;    return true; }
    if (s == "TIME") { out = PinType::TIME;      return true; }
    if (s == "WORD") { out = PinType::WORD;      return true; }

    return false;
}


struct PinTypeInfo
{
    PinType     type;
    const char* label;
};

inline const std::vector<PinTypeInfo>& GetAllPinTypes()
{
    static const std::vector<PinTypeInfo> s = {
        { PinType::Trigger,   "Trigger" },

        { PinType::Ability,   "Ability" },
        { PinType::ABILITY,   "ABILITY" },
        { PinType::Action,    "Action" },
        { PinType::BOOLEAN,   "BOOLEAN" },
        { PinType::CONSTANT,  "CONSTANT" },
        { PinType::CUBE,      "CUBE" },
        { PinType::DIRECTION, "DIRECTION" },
        { PinType::double_,   "double" },
        { PinType::DOUBLE,    "DOUBLE" },
        { PinType::Info,      "Info" },
        { PinType::Int,       "int" },
        { PinType::List,      "List" },
        { PinType::NAME,      "NAME" },
        { PinType::PERK,      "PERK" },
        { PinType::POSITION,  "POSITION" },
        { PinType::STACKING,  "STACKING" },
        { PinType::String,    "String" },
        { PinType::STRING,    "STRING" },
        { PinType::TIME,      "TIME" },
        { PinType::WORD,      "WORD" }
    };
    return s;
}

// Nodes.h

inline ImColor GetPinColor(PinType type)
{
    switch (type)
    {
    case PinType::Trigger:   return ImColor(255, 255, 255);

    case PinType::Ability:   return ImColor(80, 160, 255);
    case PinType::ABILITY:   return ImColor(80, 200, 255);
    case PinType::Action:    return ImColor(255, 200, 80);
    case PinType::BOOLEAN:   return ImColor(120, 220, 120);
    case PinType::CONSTANT:  return ImColor(200, 200, 200);
    case PinType::CUBE:      return ImColor(200, 150, 255);
    case PinType::DIRECTION: return ImColor(180, 180, 255);
    case PinType::double_:   return ImColor(180, 180, 80);
    case PinType::DOUBLE:    return ImColor(220, 220, 80);
    case PinType::Info:      return ImColor(80, 255, 255);
    case PinType::Int:       return ImColor(180, 120, 80);
    case PinType::List:      return ImColor(120, 120, 120);
    case PinType::NAME:      return ImColor(255, 120, 120);
    case PinType::PERK:      return ImColor(255, 160, 255);
    case PinType::POSITION:  return ImColor(255, 120, 255);
    case PinType::STACKING:  return ImColor(220, 180, 80);
    case PinType::String:    return ImColor(180, 255, 180);
    case PinType::STRING:    return ImColor(160, 255, 160);
    case PinType::TIME:      return ImColor(160, 160, 255);
    case PinType::WORD:      return ImColor(255, 255, 160);
    }
    return ImColor(255, 0, 255);
}
inline bool CanConnectPinTypes(PinType inputType, PinType outputType)
{
    // Exact match is always fine
    if (inputType == outputType)
        return true;

    // Table of implicit conversions:
    // outputType  --->  inputType
    struct Conversion
    {
        PinType to;   // input  type
        PinType from; // output type
    };

    static const Conversion s_conversions[] = {
        // numeric-ish
        { PinType::DOUBLE,  PinType::double_ },
        { PinType::DOUBLE,  PinType::Int     },

        {PinType::Int, PinType::DOUBLE},
        {PinType::Int, PinType::double_},
        {PinType::Int, PinType::CONSTANT},
        {PinType::Int, PinType::STACKING},
        {PinType::Int, PinType::TIME },

        {PinType::CONSTANT, PinType::DOUBLE},
        {PinType::CONSTANT, PinType::double_},
        {PinType::CONSTANT, PinType::Int},
        {PinType::CONSTANT, PinType::STACKING},
        {PinType::CONSTANT, PinType::TIME },

        {PinType::TIME, PinType::DOUBLE},
        {PinType::TIME, PinType::double_},
        {PinType::TIME, PinType::Int},
        {PinType::TIME, PinType::STACKING},
        {PinType::TIME, PinType::CONSTANT},



        {PinType::STACKING, PinType::DOUBLE},
        {PinType::STACKING, PinType::double_},
        {PinType::STACKING, PinType::Int},
        {PinType::STACKING, PinType::CONSTANT},
        {PinType::STACKING, PinType::TIME },

        // string-ish
        { PinType::STRING,  PinType::String  },
        { PinType::STRING,  PinType::NAME  },
        { PinType::STRING,  PinType::WORD  },

        { PinType::NAME,  PinType::String  },
        { PinType::NAME,  PinType::STRING },
        { PinType::NAME,  PinType::WORD  },

        { PinType::WORD,  PinType::String  },
        { PinType::WORD,  PinType::STRING },
        { PinType::WORD,  PinType::NAME  },

        // ability variants
        { PinType::ABILITY, PinType::Ability },
    };

    for (const auto& c : s_conversions)
    {
        if (c.to == inputType && c.from == outputType)
            return true;
    }

    return false;
}



class LinkInfo
{public:
    ed::LinkId Id;
    ed::PinId  InputId;
    ed::PinId  OutputId;
};

class Node; // forward declaration

class Pin
{
public:
    ed::PinId   ID;
    Node* NodePtr = nullptr;
    std::string Name;
    PinType     Type;
    PinKind     Kind;


    Pin(ed::PinId ID, PinType Type, PinKind Kind) {
        this->ID = ID;
        this->Type = Type;
        this->Kind = Kind;

        this->Name = PinTypeToString(Type);
    }
};
class Node
{
public:
    ed::NodeId ID;
    std::string Name;
    Pin* InputPin;
    std::vector<Pin*> OutputPins;
    Node* InputNode = nullptr;
    std::vector<Node*> OutputNodes;
    ImColor Color;
    NodeType Type = NodeType::Basic;
    ImVec2 Start_pos;
    
    std::string description = "";

    Node(ed::NodeId ID, std::string Name, Pin* InputPin, std::vector<Pin*> OutputPins, ImVec2 Start_pos, std::string desc = "", NodeType nodetype = NodeType::Basic)
    {
        this->ID = ID;
        this->Name = Name;
        this->InputPin = InputPin;
        this->OutputPins = OutputPins;
        OutputNodes.resize(OutputPins.size(),nullptr);
        this->Start_pos = Start_pos;
        this->Type = nodetype;

        this->InputPin->NodePtr = this;
        for (Pin* pin : this->OutputPins)
        {
            pin->NodePtr = this;
        }
        this->description = desc;

    }
    // Remove this node's pins from a global/shared pin list
    void RemovePinsFrom(std::vector<Pin*>& allPins) const
    {
        auto erase_one = [&](Pin* p)
            {
                if (!p) return;
                auto it = std::find(allPins.begin(), allPins.end(), p);
                if (it != allPins.end())
                    allPins.erase(it);
            };

        // remove input pin
        erase_one(InputPin);

        // remove all output pins
        for (Pin* p : OutputPins)
            erase_one(p);
    }
    

    ~Node()
    {
        delete InputPin;
        for (auto pin : OutputPins)
        {
            delete pin;
        }

    }

};



struct function
{
    std::string Name;
    PinType input;
    PinType output[10];
    int output_size;
    std::string description = "";
};


