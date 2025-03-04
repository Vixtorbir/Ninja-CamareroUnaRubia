#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

enum class DialogueEngine
{
	RAIDEDVILLAGE
};

class DialogueManager
{

public:

	DialogueManager();
	virtual ~DialogueManager();

	const char* GetText(DialogueEngine dialogueEngine);
};

