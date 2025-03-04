#pragma once

#include "GuiControl.h"
#include "Vector2D.h"
#include "Dialogue.h"
#include "Module.h"

enum class DialogueEngine
{
	RAIDEDVILLAGE
};

class DialogueManager
{

public:

	DialogueManager();
	void SetModule(Module* module);
	void CastDialogue(DialogueEngine dialogueEngine);

	const char* GetText(DialogueEngine dialogueEngine);

	SDL_Rect namePos = { 0, 100, 150,150 };
	SDL_Rect dialoguePos = { 0, 700, 1920,300 };

	pugi::xml_node dialogueParameters;
	pugi::xml_document dialogFile;

	Dialogue* dialogue;
	Module* module;
};

