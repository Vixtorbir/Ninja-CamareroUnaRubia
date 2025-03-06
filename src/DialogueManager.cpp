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
    switch (dialogueEngine)
    {
    case DialogueEngine::RAIDEDVILLAGE:
    {
        pugi::xml_node dialoguesNode = dialogFile.child("dialogues").child("RAIDEDVILLAGE").child("dialogues");

        for (pugi::xml_node dialogue = dialoguesNode.first_child(); dialogue; dialogue = dialogue.next_sibling())
        {
            std::string character = dialogue.attribute("character").as_string();
            std::string text = dialogue.attribute("text").as_string();
            dialogues.push_back({ character, text });
        }

        currentDialogueIndex = 0;
        ShowNextDialogue();

        break;
    }
    }
}

void DialogueManager::Update()
{
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
    {
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
    }
}