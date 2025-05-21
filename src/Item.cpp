#include "Item.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

Item::Item() : Entity(EntityType::ITEM)
{
	name = "item";
}

Item::~Item() {}

bool Item::Awake() {
	return true;
}

bool Item::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	name = parameters.attribute("name").as_string("Unnamed Item");
	description = parameters.attribute("description").as_string("No description available.");
	texturePath = parameters.attribute("texture").as_string("");
	quantity = parameters.attribute("quantity").as_int(1);
	icon = Engine::GetInstance().textures.get()->Load(parameters.attribute("icon").as_string());
	effect = parameters.attribute("effect").as_int();
	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;
	
	// L08 TODO 4: Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::STATIC);

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::ITEM;
	pbody->listener = this;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Item::Update(float dt)
{
	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  

	if (Engine::GetInstance().scene.get()->currentState != GameState::PLAYING) return true;

	
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	return true;
}

void Item::SetPosition(Vector2D pos)
{
	position = pos;
}

void Item::ApplyEffect()
{
	switch (effect)
	{
	case 1:
		Engine::GetInstance().scene.get()->player->HP += 20;

		break;

	case 2:
		Engine::GetInstance().scene.get()->player->speed += 20;

		break;
	}

}
bool Item::CleanUp()
{
	LOG("Cleanup item");
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	Engine::GetInstance().scene.get()->items.clear();
	return true;
}

void Item::OnPickup(Player* player) {
	player->AddItem(InventoryItem(name, quantity, description));
	Engine::GetInstance().audio.get()->PlayFx(player->pickUpItemFxId); // Sonido
    Engine::GetInstance().render.get()->DrawText(("Picked up: " + name).c_str(), 600, 200, 750, 255); // Mensaje en pantalla
	this->active = false; // Desactivar el objeto
}

void Item::Puzzle() {

	pbody->ctype = ColliderType::PUZZLE;

}

