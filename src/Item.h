#pragma once  

#include "Entity.h"  
#include "SDL2/SDL.h"  
#include "Animation.h"  
#include "Physics.h"  
#include "Player.h"  

struct SDL_Texture;  

class Item : public Entity  
{  
public:  

   Item();  
   virtual ~Item();  

   bool Awake();  

   bool Start();  

   bool Update(float dt);  

   void SetPosition(Vector2D pos);  

   void ApplyEffect();

   bool CleanUp();

   void OnPickup(Player* player);  

   void SetParameters(pugi::xml_node parameters) {  
       this->parameters = parameters;  
   }  

public:  

   bool isPicked = false;  

   std::string name;  
   int quantity;  
   std::string description;  
   SDL_Texture* icon = nullptr;    

   SDL_Texture* texture;  
   const char* texturePath;  
   int texW, texH;  
   pugi::xml_node parameters;  
   Animation* currentAnimation = nullptr;  
   Animation idle;  
   
   int effect;
   //L08 TODO 4: Add a physics to an item  
   PhysBody* pbody;  

};
