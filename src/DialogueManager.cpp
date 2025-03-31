#include "DialogueManager.h"
#include "Log.h"
#include "Engine.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include "Input.h"

DialogueManager::DialogueManager()
{
    // Attempt to load the dialogue XML file
    pugi::xml_parse_result result = dialogFile.load_file("dialogues.xml");
    if (result)
    {
        LOG("dialogues.xml parsed without errors");
    }
    else
    {
        LOG("Error loading dialogues.xml: %s", result.description());
    }

    dialogFile.child("dialogues");
}

void DialogueManager::SetModule(Module* module)
{
    this->module = module;
}

void DialogueManager::CastDialogue(DialogueEngine dialogueEngine)
{
    // Convert the enum to a string representation
    std::string dialogueType;
    switch (dialogueEngine)
    {
    case DialogueEngine::MENTORSHIP:
        dialogueType = "MENTORSHIP";
        break;
    case DialogueEngine::RAIDEDVILLAGE:
        dialogueType = "RAIDEDVILLAGE";
        break;
    case DialogueEngine::ISAMU:
		dialogueType = "ISAMU";
		break;
    case DialogueEngine::KAEDE:
        dialogueType = "KAEDE";
        break;
    case DialogueEngine::HANZO:
        dialogueType = "HANZO";
        break;
    case DialogueEngine::EMPTY:
        dialogueType = "EMPTY";
        break;
 
    default:
        LOG("Unknown dialogue type");
        return;
    }

    // Load the correct dialogue type from XML
    pugi::xml_node dialoguesNode = dialogFile.child("dialogues").child(dialogueType.c_str()).child("dialogues");

    if (!dialoguesNode)
    {
        LOG("Dialogue type '%s' not found in XML!", dialogueType.c_str());
        return;
    }

    dialogues.clear();
    isBranching = false; // Reset branching status before loading new dialogues

    // Load dialogues and check if they are branching
    for (pugi::xml_node dialogue = dialoguesNode.first_child(); dialogue; dialogue = dialogue.next_sibling())
    {
        std::string character = dialogue.attribute("character").as_string();
        std::string text = dialogue.attribute("text").as_string();
        int branching = dialogue.attribute("branching").as_int(); // Get branching attribute as int

        dialogues.push_back({ character, text });

        // If a dialogue has branching options, set isBranching to true
        if (branching == 1)
        {
            isBranching = true;
        }
    }

    if (!dialogues.empty())
    {
        currentDialogueIndex = 0;
        ShowNextDialogue();
    }
    else
    {
        LOG("No dialogues found for '%s'!", dialogueType.c_str());
    }
}

void DialogueManager::Update()
{
   

    if (!dialogueEnded && Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
    {
        Engine::GetInstance().guiManager->ClearControlsOfType(GuiControlType::DIALOGUE);
        ShowNextDialogue();
    }
}

void DialogueManager::ShowNextDialogue()
{
    Engine::GetInstance().guiManager->ClearControlsOfType(GuiControlType::DIALOGUE);

    if (currentDialogueIndex < dialogues.size())
    {
        std::string character = dialogues[currentDialogueIndex].first;
        std::string text = dialogues[currentDialogueIndex].second;

        // If not branching, move to the next dialogue automatically
        if (!isBranching)
        {
            currentDialogueIndex++;
        }
        else
        {

            std::string optionA = "Option A";
            std::string optionB = "Option B";

            Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::OPTIONA, 1, optionA.c_str(), optionAPos, module);
            Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::OPTIONB, 1, optionB.c_str(), optionBPos, module);
            
        }

        // Display the current dialogue and character name
        Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::DIALOGUE, 1, text.c_str(), dialoguePos, module);
        Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::DIALOGUE, 1, character.c_str(), namePos, module);

    }
    else
    {
        LOG("No more dialogues to display!");
        dialogueEnded = true;
    }
}
