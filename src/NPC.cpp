#include "NPC.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "GuiManager.h"
//#include "tracy/Tracy.hpp"


NPC::NPC() : Entity(EntityType::NPC)
{
}

NPC::NPC(DialogueEngine name) : Entity(EntityType::NPC)
{
	this->dialogueName = name;
}

NPC::~NPC() {

}

bool NPC::Awake() {

	return true;
}

bool NPC::Start() {

	//L03: TODO 2: Initialize NPC parameters
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));

	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)position.getY(), texW + 50, texH, bodyType::STATIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	pbody->ctype = ColliderType::NPC;

	pbody->body->SetFixedRotation(true);


	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	//initialize audio effect

	return true;
}

bool NPC::Update(float dt)
{

	//ZoneScoped;
	// Code you want to profile

	if (Engine::GetInstance().scene.get()->currentState != GameState::PLAYING) {
		return true;
	}

	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

	if (!parameters.attribute("gravity").as_bool()) {
		velocity = b2Vec2(0, 0);
	}


	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawEntity(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1, 0, 0, 0, (int)npcDirection);
	currentAnimation->Update();

	if ((Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN || Engine::GetInstance().input.get()->GetControllerButton(SDL_CONTROLLER_BUTTON_Y) == KEY_DOWN )&& inside)
	{
		bubble = !bubble;

	}
	
	if(once)
	{
		if (bubble) {
			showcaseDialogue = true;
			once = false;

		}
	}

	
	return true;
}
float NPC::Lerp(float start, float end, float factor) {
	return start + factor * (end - start);
}
bool NPC::CleanUp()
{
	Engine::GetInstance().textures.get()->UnLoad(texture);
	

	if (pbody != nullptr) {
		Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	}
	active = false;
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void NPC::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		inside = true;
		
		//FIX interact mapping
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
		{

			LOG("a");

		}
		break;
	}
}
void NPC::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		showcaseDialogue = false;

		inside = false;
		once = false; // Change from false to true
		Engine::GetInstance().guiManager->ClearControlsOfType(GuiControlType::DIALOGUE);
		break;
	}
}

void NPC::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D NPC::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}
