#include "DialogueManager.h"
#include "Log.h"
#include "Engine.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include "Input.h"

DialogueManager::DialogueManager()
{



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
    case DialogueEngine::RAIDEDVILLAGE:
        dialogueType = "RAIDEDVILLAGE";
        break;
    case DialogueEngine::EMPTY:
        dialogueType = "EMPTY";
        break;

    default:
        LOG("Unknown dialogue type");
        return;
    }

    pugi::xml_node dialoguesNode = dialogFile.child("dialogues").child(dialogueType.c_str()).child("dialogues");

    if (!dialoguesNode)
    {
        LOG("Dialogue type '%s' not found in XML!", dialogueType.c_str());
        return;
    }

    dialogues.clear();

    for (pugi::xml_node dialogue = dialoguesNode.first_child(); dialogue; dialogue = dialogue.next_sibling())
    {
        std::string character = dialogue.attribute("character").as_string();
        std::string text = dialogue.attribute("text").as_string();
        dialogues.push_back({ character, text });
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

        Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::DIALOGUE, 1, character.c_str(), namePos, module);
        Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::DIALOGUE, 1, text.c_str(), dialoguePos, module);

        currentDialogueIndex++;
    }
    else
    {
        LOG("No more dialogues to display!");
        dialogueEnded = true;
    }
}