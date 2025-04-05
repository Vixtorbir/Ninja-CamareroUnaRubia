#pragma once

#include "GuiControl.h"
#include "Vector2D.h"
#include "Dialogue.h"
#include "Module.h"
#include <vector>

enum class DialogueEngine
{
	MENTORSHIP, MENTORSHIP1, MENTORSHIP2,
	RAIDEDVILLAGE,
	ISAMU,
	KAEDE,
	HANZO,
	EMPTY
};

class DialogueManager : public Module
{

public:

	DialogueManager();
	
	void Update();

	DialogueEngine GetDialogueEngineFromString(const std::string& dialogueName);

	std::vector<std::pair<std::string, std::string>> GetCurrentOptions();

	void ShowNextDialogue();

	void ShowNextDialogueWithIndex(int optionDialogueIndex);
	
	void SetModule(Module* module);
	void CastDialogue(DialogueEngine dialogueEngine);

	const char* GetText(DialogueEngine dialogueEngine);


	pugi::xml_node dialogueParameters;

	Dialogue* name;
	Dialogue* dialogue;

	const char* currentText;
	const char* currentName;

	Module* module;

	int screenWidth;
	int screnHeight;
	SDL_Texture* Hanzo = nullptr;

private:

	bool dialogueEnded = false;
	SDL_Rect namePos = { 500, 550, 300,150 };
	SDL_Rect dialoguePos = { 0, 700, 1920,300 };

	SDL_Rect optionAPos = { 1000, 550, 300,150 };
	SDL_Rect optionBPos = { 1000, 800, 300,150 };
	std::vector<std::pair<std::string, std::string>> branchingOptions;
	int isBranching = 0;

	pugi::xml_document dialogFile;
	std::vector<std::pair<std::string, std::string>> dialogues;
	int currentDialogueIndex = 0;

	int optionIndex = 0;
};

