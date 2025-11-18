# ChaosNode — Visual Ability Editor for CubeChaos

ChaosNode is a visual node-based editor that helps you **design abilities for CubeChaos** without hand-writing long, fragile scripts.

Instead of typing everything by hand, you can:
- Connect **typed nodes** together (Trigger, Action, Boolean, etc.)
- Preview how abilities flow
- Export/import them as plain text the game understands

>  TL;DR: ChaosNode turns CubeChaos modding into a node graph editor with bidirectional text support.

---

##  Features

- **Node-based editor for abilities**  
  Build abilities by connecting nodes that represent triggers, actions, conditions, and data types.

- **Text <-> Node graph parsing**  
  - **Parse nodes → text**: turn your node graph into a clean text representation.  
  - **Parse text → nodes**: paste ability text and visualize it as a graph.


-  **Quick node creation (Shift + A)**  
  - Press **Shift + A** over the canvas  
  - Search by function name  
  - Filter by input type  
  - Create:
    - Nodes based on functions from `ModdingInfo.txt`
    - **Constant nodes** for hardcoded values
    - **Root nodes** (not used in the game but help with the node editor) with a chosen output type. each graph starts from a root.
    - 

- **Constant & Root nodes**  
  - **Root nodes** represent entry points (e.g. ability start).  
  - **Constant nodes** let you store literal values (with inline editable text).

---

## How to use
- **To parse text**: copy ability text onto the textbox and click **"Parse text"**, assuming there are no errors you should see the graph appear (errors appear inside the textbox, appended to the end of what you have written). if you want to have multiple abilities you need to start new lines with Root, if no Roots are used it is assumed teh whole block of text is 1 ability.
- **To build/edit graph manually**: Shift+A opens the Node Creation UI. from there you can select any function from ModInfo.txt that is in the directory of the .exe, Generate a constant of any type and create Root nodes that serve as entry points. Drag Input pins to Output pins to create links, starting from each root text is generated in orded using existing links.
- **To parse nodes to text**: simply click the **"Parse nodes"** button. If you are copying this text in the program you **MUST** remove the starting Root for this is not a valid keyword the actual game understands but only this editor.

##Common issues
- **ERROR: CANT READ ABILITY: (Root)**: You forgot to remove Root from the text written by the node editor. ROOT is used soley to tell the node editor from where to start the parsing of the graph, it is not a game command
- **Shift + A Menu shows no functions**: "ModdingInfo.txt" was not in the .exe directory

## 📷 Screenshots

> _Add actual screenshots or GIFs here once your repo is public._

```text
[ ChaosNode main editor screenshot ]
[ Text parser window screenshot ]
[ Create Node ("Shift + A") popup screenshot ]
