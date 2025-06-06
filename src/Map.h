#pragma once

#include "Module.h"
#include "Physics.h"
#include <list>
#include <vector>
#include <map>

// L10: TODO 2: Define a property to store the Map Orientation and Load it from the map
enum MapOrientation
{
    ORTOGRAPHIC = 0,
    ISOMETRIC
};

// L09: TODO 5: Add attributes to the property structure
struct Properties
{
    struct Property
    {
        std::string name;
        bool value; //We assume that we are going to work only with bool for the moment
    };

    std::list<Property*> propertyList;

    ~Properties()
    {
        for (const auto& property : propertyList)
        {
            delete property;
        }

        propertyList.clear();
    }

    // L09: DONE 7: Method to ask for the value of a custom property
    Property* GetProperty(const char* name);

};

struct MapLayer
{
    // L07: TODO 1: Add the info to the MapLayer Struct
    int id;
    std::string name;
    int width;
    int height;
    std::vector<int> tiles;
    Properties properties;

    // L07: TODO 6: Short function to get the gid value of i,j
    int Get(int i, int j) const
    {
        return tiles[(j * width) + i];
    }
};
struct AnimationFrame {
    int tileId;
    int duration;
};

struct AnimatedTile {
    int gid;
    std::vector<AnimationFrame> frames;
    int totalDuration;
};
// L06: TODO 2: Create a struct to hold information for a TileSet
// Ignore Terrain Types and Tile Types for now, but we want the image!

struct TileSet
{
    int firstGid;
    std::string name;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    int tileCount;
    int columns;
    SDL_Texture* texture;
    std::map<int, AnimatedTile> animatedTiles;
    // L07: TODO 7: Implement the method that receives the gid and returns a Rect
    SDL_Rect GetRect(unsigned int gid) {
        SDL_Rect rect = { 0 };

        int relativeIndex = gid - firstGid;
        rect.w = tileWidth;
        rect.h = tileHeight;
        rect.x = margin + (tileWidth + spacing) * (relativeIndex % columns);
        rect.y = margin + (tileHeight + spacing) * (relativeIndex / columns);

        return rect;
    }

};

struct Object {
    float x;
    float y;
    float width;
    float height;
};

struct ObjectGroup {
    std::string name;
    std::vector<Object*> objects;
};

// L06: TODO 1: Create a struct needed to hold the information to Map node
struct MapData
{
	int width;
	int height;
	int tileWidth;
	int tileHeight;
    std::list<TileSet*> tilesets;
    // L10: TODO 2: Define a property to store the Map Orientation and Load it from the map
    MapOrientation orientation;

    // L07: TODO 2: Add the info to the MapLayer Struct
    std::list<MapLayer*> layers;
    std::vector<ObjectGroup*> objectGroups;
};

class Map : public Module
{
public:

    Map();

    // Destructor
    virtual ~Map();

    // Called before render is available
    bool Awake();

    // Called before the first frame
    bool Start();

    // Called each loop iteration
    bool Update(float dt);

    void UpdateAnimatedTiles(float dt);
    // Called before quitting
    bool CleanUp();

    // Load new map
    bool Load(std::string path, std::string mapFileName);
    void DeleteCollisionBodies();
    const std::vector<ObjectGroup*>& GetObjectGroups() const { return mapData.objectGroups; }
    // L07: TODO 8: Create a method that translates x,y coordinates from map positions to world positions
    Vector2D MapToWorld(int x, int y) const;

    // L10: TODO 5: Add method WorldToMap to obtain  map coordinates from screen coordinates 
    Vector2D WorldToMap(int x, int y);

    // L09: TODO 2: Implement function to the Tileset based on a tile id
    TileSet* GetTilesetFromTileId(int gid) const;
    MapLayer* GetNavigationLayer() const {
        for (const auto& layer : mapData.layers) {
            if (layer->name == "Navigation") return layer;
        }
        return nullptr;
    }

    // Add this to get map dimensions
    void GetMapDimensions(int& width, int& height) const {
        width = mapData.width;
        height = mapData.height;
    }
    // L09: TODO 6: Load a group of properties 
    bool LoadProperties(pugi::xml_node& node, Properties& properties);

    int GetWidth() {
        return mapData.width;
    }

    int GetHeight() {
        return mapData.height;
    }

    int GetTileWidth() {
        return mapData.tileWidth;
    }

    int GetTileHeight() {
        return mapData.tileHeight;
    }

    MapLayer* GetNavigationLayer();
  
    bool IsTileCollidable(int x, int y);

    bool IsObjectGroupCollidable(int x, int y);

    MapLayer* GetLayer(const char* name) {
        for (auto layer : mapData.layers) {
            if (layer->name == name) return layer;
        }
        return nullptr;
    }

	

public: 
    std::string mapFileName;
    std::string mapPath;
    std::vector<PhysBody*> collisionBodies;
	std::vector<PhysBody*> collisionBodies2;
    MapData mapData;
    bool collisionBodies2Deleted = false;

private:
    bool mapLoaded;
    // L06: DONE 1: Declare a variable data of the struct MapData

};