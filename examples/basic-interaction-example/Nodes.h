#pragma once

#include <string>
#include <vector>
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

enum class PinType { Ability,Action,Boolean,Cube,Direction,Double,Perk,Position,String,Trigger };
enum class PinKind { Output, Input };
enum class NodeType { Basic, Primary, Constant}; //basic has 1 input and n outputs, primary has no inputs and n outputs and constant has 1 input and 0 outputs
inline const char* ToString(PinType type)
{
    switch (type)
    {
    case PinType::Ability:   return "Ability";
    case PinType::Action:    return "Action";
    case PinType::Boolean:   return "Boolean";
    case PinType::Cube:      return "Cube";
    case PinType::Direction: return "Direction";
    case PinType::Double:    return "Double";
    case PinType::Perk:      return "Perk";
    case PinType::Position:  return "Position";
    case PinType::String:    return "String";
    case PinType::Trigger:   return "Trigger";
    }
    return "Unknown"; // safety
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

        this->Name = ToString(Type);
    }
};
class Node
{
public:
    ed::NodeId ID;
    std::string Name;
    Pin* InputPin;
    std::vector<Pin*> OutputPins;
    Node* InputNode;
    std::vector<Node*> OutputNode;
    ImColor Color;
    NodeType Type = NodeType::Basic;
    ImVec2 Start_pos;


    Node(ed::NodeId ID, std::string Name, Pin* InputPin, std::vector<Pin*> OutputPins, ImVec2 Start_pos)
    {
        this->ID = ID;
        this->Name = Name;
        this->InputPin = InputPin;
        this->OutputPins = OutputPins;
        this->Start_pos = Start_pos;

        this->InputPin->NodePtr = this;
        for (Pin* pin : this->OutputPins)
        {
            pin->NodePtr = this;
        }

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




