#include <iostream>

#include "States.h"
#include "Game.h"
#include "StateManager.h"
#include "TextureManager.h"
#include "EventManager.h"
#include "TiledLevel.h"
#include "PlatformPlayer.h"
#include "SoundManager.h"
#include "PlayButton.h"

// Begin TitleState
void TitleState::Enter()
{
	std::cout << "Entering TitleState..." << std::endl;
	SoundManager::LoadMusic("Assets/Sound/Music/Menu.mp3", "MainMenuMusic");
	SoundManager::PlayMusic("MainMenuMusic");

	TextureManager::Load("Assets/Images/Buttons/play.png", "play");

	int buttonWidth = 400;
	int buttonHeight = 100;
	float buttonX = Game::GetInstance().kWidth / 2 - buttonWidth / 2.0f;
	float buttonY = Game::GetInstance().kHeight / 2 - buttonHeight / 2.0f;
	m_objects.emplace("play", new PlayButton({ 0, 0, buttonWidth, buttonHeight }, { buttonX, buttonY, (float)buttonWidth, (float)buttonHeight }, "play"));
}

void TitleState::Update(float deltaTime)
{
	for (auto const& i : m_objects)
	{
		i.second->Update(deltaTime);
		if (StateManager::StateChanging()) 
			return;
	}
}

void TitleState::Render()
{
	SDL_SetRenderDrawColor(Game::GetInstance().GetRenderer(), 255, 0, 255, 255);
	SDL_RenderClear(Game::GetInstance().GetRenderer());

	for (auto const& i : m_objects)
		i.second->Render();
}

void TitleState::Exit()
{
	std::cout << "Exiting TitleState..." << std::endl;
	SoundManager::StopMusic();
	SoundManager::UnloadMusic("MainMenuMusic");

	TextureManager::Unload("play");
	for (auto& i : m_objects)
	{
		delete i.second;
		i.second = nullptr;
	}
	m_objects.clear();
}
// End TitleState

// Begin GameState
void GameState::Enter() // Used for initialization.
{
	std::cout << "Entering GameState..." << std::endl;

	m_RectangleTransform.x = Game::kWidth / 2;
	m_RectangleTransform.y = Game::kHeight / 2;
	m_RectangleTransform.w = 60;
	m_RectangleTransform.h = 100;

	TextureManager::Load("Assets/Images/Tiles.png", "tiles");
	TextureManager::Load("Assets/Images/Player.png", "player");


	m_objects.emplace("level", new TiledLevel(24, 32, 32, 32, "Assets/Data/Tiledata.txt", "Assets/Data/Level1.txt", "tiles"));
	m_objects.emplace("player", new PlatformPlayer({ 0,0,128,128 }, { 50,500,64,64 }));

	SoundManager::LoadMusic("Assets/Sound/Music/Level.mp3", "LevelMusic");
	SoundManager::PlayMusic("LevelMusic");
}

void GameState::Update(float deltaTime)
{
	if (EventManager::KeyPressed(SDL_SCANCODE_X))
	{
		StateManager::ChangeState(new TitleState()); // Change to new TitleState
	}
	else if (EventManager::KeyPressed(SDL_SCANCODE_P))
	{
		StateManager::PushState(new PauseState()); // Add new PauseState
	}
	else
	{
		for (auto const& i : m_objects)
		{
			i.second->Update(deltaTime);
		}
		
		// Check collision.
		SDL_FRect* playerColliderTransform = m_objects["player"]->GetDestinationTransform(); // Copies address of player m_dst.

		float playerLeft = playerColliderTransform->x;
		float playerRight = playerColliderTransform->x + playerColliderTransform->w;
		float playerTop = playerColliderTransform->y;
		float playerBottom = playerColliderTransform->y + playerColliderTransform->h;

		PlatformPlayer* pPlayer = static_cast<PlatformPlayer*>(m_objects["player"]);
		for (unsigned int i = 0; i < static_cast<TiledLevel*>(m_objects["level"])->GetObstacles().size(); i++)
		{
			SDL_FRect* obstacleColliderTransform = static_cast<TiledLevel*>(m_objects["level"])->GetObstacles()[i]->GetDestinationTransform();

			float obstacleLeft = obstacleColliderTransform->x;
			float obstacleRight = obstacleColliderTransform->x + obstacleColliderTransform->w;
			float obstacleTop = obstacleColliderTransform->y;
			float obstacleBottom = obstacleColliderTransform->y + obstacleColliderTransform->h;

			// Check if they overlap horizontally
			// by comparing right vs. left and left vs. right.
			bool xOverlap = playerLeft < obstacleRight && playerRight > obstacleLeft;

			// Check if they also overlap vertically
			// by comparing top vs. bottom and bottom vs. top.
			bool yOverlap = playerTop < obstacleBottom && playerBottom > obstacleTop;

			// Used to determine which direction the collision came from
			float bottomCollision = obstacleBottom - playerColliderTransform->y;
			float topCollision = playerBottom - obstacleColliderTransform->y;
			float leftCollision = playerRight - obstacleColliderTransform->x;
			float rightCollision = obstacleRight - playerColliderTransform->x;

			// If they overlap both horizontally and vertically,
			// then they truly overlap.
			if (xOverlap && yOverlap)
			{
				// Top collision
				if (topCollision < bottomCollision && topCollision < leftCollision && topCollision < rightCollision)
				{
					pPlayer->StopY();
					pPlayer->SetY(playerColliderTransform->y - topCollision);
					pPlayer->SetGrounded(true);
				}
				// Bottom collision
				if (bottomCollision < topCollision && bottomCollision < leftCollision && bottomCollision < rightCollision)
				{
					pPlayer->StopY();
					pPlayer->SetY(playerColliderTransform->y + bottomCollision);
				}
				// Left collision
				if (leftCollision < rightCollision && leftCollision < topCollision && leftCollision < bottomCollision)
				{
					pPlayer->StopX();
					pPlayer->SetX(playerColliderTransform->x - leftCollision);
				}
				// Right collision
				if (rightCollision < leftCollision && rightCollision < topCollision && rightCollision < bottomCollision)
				{
					pPlayer->StopX();
					pPlayer->SetX(playerColliderTransform->x + rightCollision);
				}
			}
		}
	}
}

void GameState::Render()
{
	SDL_Renderer* pRenderer = Game::GetInstance().GetRenderer();

	SDL_SetRenderDrawColor(pRenderer, 0, 0, 255, 255);
	SDL_RenderClear(pRenderer);

	for (auto const& i : m_objects)
		i.second->Render();


	SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
	SDL_RenderFillRectF(pRenderer, &m_RectangleTransform);

}

void GameState::Exit()
{
	std::cout << "Exiting GameState..." << std::endl;

	TextureManager::Unload("tiles");
	TextureManager::Unload("player");
	for (auto& i : m_objects)
	{
		delete i.second;
		i.second = nullptr;
	}
	m_objects.clear();

	SoundManager::StopMusic();
	SoundManager::UnloadMusic("LevelMusic");
}

void GameState::Pause()
{
	std::cout << "Pauseing GameState..." << std::endl;
	SoundManager::PauseMusic();
}

void GameState::Resume()
{
	std::cout << "Resuming GameState..." << std::endl;
	SoundManager::ResumeMusic();
}
// End GameState

// Begin PauseState
void PauseState::Enter()
{
	std::cout << "Entering PauseState..." << std::endl;
}

void PauseState::Update(float deltaTime)
{
	if (EventManager::KeyPressed(SDL_SCANCODE_R))
	{
		StateManager::PopState();
	}
}

void PauseState::Render()
{
	// First render the GameState
	StateManager::GetStates().front()->Render();
	// Now render rest of PauseState
	SDL_SetRenderDrawBlendMode(Game::GetInstance().GetRenderer(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(Game::GetInstance().GetRenderer(), 128, 128, 128, 128);
	SDL_Rect rect = { 256, 128, 512, 512 };
	SDL_RenderFillRect(Game::GetInstance().GetRenderer(), &rect);
}

void PauseState::Exit()
{
	std::cout << "Exiting PauseState..." << std::endl;
}
// End PauseState