#include "randomMapGenerator.h"
#include "globalfunctions.h"
#include "gamestate.h"
#include <iostream>
#include "objects.h"
#include "actors.h"
#include "player.h"

void generatePerlinNoise(float scaleBias, int octaves, float* noiseSeed, float* output)
{
	for (int x = 0; x < MAP_WIDTH; x++) {
		for (int y = 0; y < MAP_HEIGHT; y++) {

			float noise = 0.0f;
			float totalScale = 0.0f;
			float scale = 1.0f;
			
			for (int o = 0; o < octaves; o++) {
				int pitch = MAP_WIDTH >> o;

				int sampleX1 = (x / pitch) * pitch;
				int sampleY1 = (y / pitch) * pitch;

				int sampleX2 = (sampleX1 / pitch) % MAP_WIDTH;
				int sampleY2 = (sampleY1 / pitch) % MAP_WIDTH;
				
				float blendX = (float)(x - sampleX1) / (float)pitch;
				float blendY = (float)(y - sampleY1) / (float)pitch;

				float sampleT = (1.0f - blendX) * noiseSeed[sampleY1 * MAP_WIDTH + sampleX1] + blendX * noiseSeed[sampleY1 * MAP_WIDTH + sampleX2];
				float sampleB = (1.0f - blendX) * noiseSeed[sampleY2 * MAP_WIDTH + sampleX2] + blendX * noiseSeed[sampleY2 * MAP_WIDTH + sampleX2];

				totalScale += scale;
				noise += (blendY * (sampleB - sampleT) + sampleT) * scale;
				scale = scale / scaleBias;
			}
			output[y * MAP_WIDTH + x] = noise / totalScale;
		}
	}
}

void convertPerlinNoiseToMap(float* noisemap) {
	for (int x = 0; x < MAP_WIDTH; x++)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			int depth = (int)(noisemap[y * MAP_WIDTH + x] * 3.0f);
			switch (depth)
			{
			case 0: currentGame.currentMap[x][y] = 7; break;
			case 1: currentGame.currentMap[x][y] = 1; break;
			case 2: currentGame.currentMap[x][y] = 2; break;
			default: currentGame.currentMap[x][y] = 1; break;
			}
		}
	}
}

int roll(int min, int max)
{
	return  min + (rand() % static_cast<int>(max - min + 1));
}

void placeTrees() {
	srand(time(NULL));
	for (int x = 0; x < MAP_WIDTH; x++)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			if (roll(0, 20) > 19) {
				if (currentGame.currentMap[x][y] == 1) {
					objects newObject(roll(1, 3), x, y, listOfObjects.size());
					listOfObjects.push_back(newObject);
				}
				else if (currentGame.currentMap[x][y] == 2)
				{
					objects newObject(0, x, y, listOfObjects.size());
					listOfObjects.push_back(newObject);
				}
			}
		}
	}
}

void spawmFoodStoneGold(int resource)
{
	int gridMinX=0;
	int gridMinY=0;
	for (int gridMaxX = 32; gridMaxX < 256; gridMaxX += 32) {
		for (int gridMaxY = 32; gridMaxY < 256; gridMaxY += 32) {
			bool resourcePlaced = false;
			while (!resourcePlaced) {
				mouseWorldCord suggestedCords = { roll(gridMinX,gridMaxX), roll(gridMinY,gridMaxY) };
				if (currentGame.isPassable(suggestedCords.x, suggestedCords.y))
				{
					objects newObject(resource, suggestedCords.x, suggestedCords.y, listOfObjects.size());
					listOfObjects.push_back(newObject);
					resourcePlaced = true;
				}
			}
			gridMinY = gridMaxY;
		}
		gridMinX = gridMaxX;
	}
}

void spawmFirstVillager() {
	bool villagerIsPlaced = false;
	while (!villagerIsPlaced) {
		mouseWorldCord suggestedVillagerCords = { roll(0,256), roll(0,256) };
		if (currentGame.isPassable(suggestedVillagerCords.x, suggestedVillagerCords.y))
		{
			listOfActorsMutex.lock();
			actors newActor(0, suggestedVillagerCords.x, suggestedVillagerCords.y, currentPlayer.getTeam(), listOfActors.size());
			listOfActors.push_back(newActor);
			listOfActorsMutex.unlock();
			villagerIsPlaced = true;
		}
	}
}

void centerViewOnVillager() {
	for (int i = 0; i < listOfActors.size(); i++) {
		if (listOfActors[i].getTeam() == currentPlayer.getTeam()) {
			viewOffsetX = worldSpace(listOfActors[i].getLocation().x, listOfActors[i].getLocation().y, true);
			viewOffsetY = worldSpace(listOfActors[i].getLocation().x, listOfActors[i].getLocation().y, false);
		}
	}
}

void generateRandomMap() {
	srand(time(NULL));
	float* noiseMap = nullptr;
	float* noiseSeed = nullptr;
	noiseSeed = new float[MAP_WIDTH * MAP_HEIGHT];
	noiseMap = new float[MAP_WIDTH * MAP_HEIGHT];
	
	for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
		noiseSeed[i] = (float)rand() / (float)RAND_MAX;
	};
	generatePerlinNoise(1.4f, 5, noiseSeed, noiseMap);
	convertPerlinNoiseToMap(noiseMap);
	placeTrees();
	spawmFirstVillager();
	centerViewOnVillager();
	spawmFoodStoneGold(4);
	spawmFoodStoneGold(5);
	spawmFoodStoneGold(6);
	delete noiseMap;
	delete noiseSeed;
}