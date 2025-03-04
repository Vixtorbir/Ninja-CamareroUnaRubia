#include "DialogueManager.h"
#include "Log.h"
#include "Engine.h"
#include "GuiControl.h"
#include "GuiManager.h"

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
		dialogFile.child("RAIDEDVILLAGE");
		dialogue = (Dialogue*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::DIALOGUE, 1, "Ren", namePos, module);
		dialogue = (Dialogue*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::DIALOGUE, 1, "dialogo moment", dialoguePos, module);

		break;

	}
}
