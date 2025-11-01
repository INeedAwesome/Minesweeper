#include "Game.h"

#include <chrono>
#include <random>
#include <queue>

#include <iostream>

void Game::Init(uint32_t amountMines)
{
	m_AmountMines = amountMines;
	m_FlagsLeft = amountMines;
	m_RevealedSquaresAmount = 0;
	m_State = Playing;

	for (int i = 0; i < m_Cells.size(); i++)// Just simply initialize the cells to the correct values.
	{
		int x = i % CELLS_WIDTH;			// Get the x and y 
		int y = i / CELLS_WIDTH;

		m_Cells[i].Type = None;				// Set the cells type
		m_Cells[i].CellPosition = { x, y }; // And position
	}

	for (int i = 0; i < m_ShowingCells.size(); i++)// Just simply initialize the cells to the correct values.
	{
		int x = i % CELLS_WIDTH;			// Get the x and y 
		int y = i / CELLS_WIDTH;

		m_ShowingCells[i].Type = NonRevealed;		// Set the cells type
		m_ShowingCells[i].CellPosition = { x, y };  // And position
	}

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine engine(seed);
	
	for (uint32_t i = 0; i < m_AmountMines; i++)
	{
		uint32_t randIdx = engine() % (CELLS_WIDTH * CELLS_WIDTH);
		if (m_Cells[randIdx].Type != DrawableType::Bomb)
		{
			m_Cells[randIdx].Type = Bomb;
		}
		else 
			i--;
	}

	for (int i = 0; i < m_Cells.size(); i++)
	{
		// 1 2 3
		// 4 c 5
		// 6 7 8

		int valCell1 = 0;
		if (i - CELLS_WIDTH - 1 >= 0)
			if (m_Cells[i].CellPosition.x - 1 == m_Cells[i - CELLS_WIDTH - 1].CellPosition.x)
				valCell1 = m_Cells[i - CELLS_WIDTH - 1].Type == DrawableType::Bomb ? 1 : 0;
		
		int valCell2 = 0;
		if (i - CELLS_WIDTH >= 0)
			if (m_Cells[i].CellPosition.x == m_Cells[i - CELLS_WIDTH].CellPosition.x && m_Cells[i].CellPosition.y - 1 == m_Cells[i - CELLS_WIDTH].CellPosition.y)
				valCell2 = m_Cells[i - CELLS_WIDTH].Type     == DrawableType::Bomb ? 1 : 0;
		
		int valCell3 = 0;
		if (i - CELLS_WIDTH + 1 >= 0)
			if (m_Cells[i].CellPosition.x + 1 == m_Cells[i - CELLS_WIDTH + 1].CellPosition.x && m_Cells[i].CellPosition.y - 1 == m_Cells[i - CELLS_WIDTH + 1].CellPosition.y)
				valCell3 = m_Cells[i - CELLS_WIDTH + 1].Type == DrawableType::Bomb ? 1 : 0;


		int valCell4 = 0;
		if (i - 1 >= 0)
			if (m_Cells[i - 1].Type == DrawableType::Bomb)
				if (m_Cells[i].CellPosition.x - 1 == m_Cells[i - 1].CellPosition.x && m_Cells[i].CellPosition.y == m_Cells[i - 1].CellPosition.y)
					valCell4 = 1;

		int valCell5 = 0;
		if (i + 1 < m_Cells.size())
			if (m_Cells[i + 1].Type == DrawableType::Bomb)
				if (m_Cells[i].CellPosition.x + 1 == m_Cells[i + 1].CellPosition.x && m_Cells[i].CellPosition.y == m_Cells[i + 1].CellPosition.y)
					valCell5 = 1;


		int valCell6 = 0;
		if (i + CELLS_WIDTH - 1 < m_Cells.size())
			if (m_Cells[i].CellPosition.x - 1 == m_Cells[i + CELLS_WIDTH - 1].CellPosition.x && m_Cells[i].CellPosition.y + 1 == m_Cells[i + CELLS_WIDTH - 1].CellPosition.y)
				valCell6 = m_Cells[i + CELLS_WIDTH - 1].Type == DrawableType::Bomb ? 1 : 0;
		
		int valCell7 = 0;
		if (i + CELLS_WIDTH < m_Cells.size())
			if (m_Cells[i].CellPosition.x == m_Cells[i + CELLS_WIDTH].CellPosition.x && m_Cells[i].CellPosition.y + 1 == m_Cells[i + CELLS_WIDTH].CellPosition.y)
				valCell7 = m_Cells[i + CELLS_WIDTH].Type == DrawableType::Bomb ? 1 : 0;
		
		int valCell8 = 0;
		if (i + CELLS_WIDTH + 1 < m_Cells.size())
			if (m_Cells[i].CellPosition.x + 1 == m_Cells[i + CELLS_WIDTH + 1].CellPosition.x && m_Cells[i].CellPosition.y + 1 == m_Cells[i + CELLS_WIDTH + 1].CellPosition.y)
				valCell8 = m_Cells[i + CELLS_WIDTH + 1].Type == DrawableType::Bomb ? 1 : 0;


		int valAll = valCell1 + valCell2 + valCell3 + valCell4 + valCell5 + valCell6 + valCell7 + valCell8;
		
		if (m_Cells[i].Type != DrawableType::Bomb)
			m_Cells[i].Type = (DrawableType)valAll;
	}

}

void Game::Cleanup()
{
}

void Game::TryExpose(int xpos, int ypos)
{
	if (m_State == Lost)
		return;

	int idx = ypos * CELLS_WIDTH + xpos;
	if (m_ShowingCells[idx].Type == Flag)
		return;
	if (m_Cells[idx].Type != Bomb)
	{
		RevealEmptyArea(Vector2{ xpos, ypos });

		bool won = false;
		if (m_RevealedSquaresAmount + m_AmountMines == CELLS_WIDTH * CELLS_WIDTH)
			m_State = Won;

	}
	else // Bomb clicked
	{
		// You Lost
		m_State = Lost;

		// Reveal bombs
		for (int i = 0; i < m_Cells.size(); i++)
		{
			if (m_Cells[i].Type == Bomb)
			{
				/*if (m_ShowingCells[i].Type == NonRevealed)
				{
					m_ShowingCells[i].Type = Bomb;
				}*/
				m_ShowingCells[i].Type = Bomb;
			}
		}
	}
}

void Game::TryPlaceFlag(int xpos, int ypos)
{
	if (m_State == Lost)
		return;

	int idx = ypos * CELLS_WIDTH + xpos;
	if (m_ShowingCells[idx].Type == None || 
		m_ShowingCells[idx].Type == One || 
		m_ShowingCells[idx].Type == Two|| 
		m_ShowingCells[idx].Type == Three || 
		m_ShowingCells[idx].Type == Four || 
		m_ShowingCells[idx].Type == Five || 
		m_ShowingCells[idx].Type == Six || 
		m_ShowingCells[idx].Type == Seven || 
		m_ShowingCells[idx].Type == Eight 
	)
		return;

	if (m_ShowingCells[idx].Type == Flag)
	{
		m_ShowingCells[idx].Type = NonRevealed;
		m_FlagsLeft++;
	}
	else
	{
		m_ShowingCells[idx].Type = Flag;
		m_FlagsLeft--;
	}

}

void Game::Reset()
{
	m_State = Playing;
	Init(60);
}

std::vector<Vector2> Game::GetNeighbors(const Vector2& cell)
{
	std::vector<Vector2> neighbors;

	int i = cell.y * CELLS_WIDTH + cell.x;
	
	if (i - CELLS_WIDTH - 1 >= 0)
		if (m_Cells[i].CellPosition.x - 1 == m_Cells[i - CELLS_WIDTH - 1].CellPosition.x && m_Cells[i].CellPosition.y - 1 == m_Cells[i - CELLS_WIDTH].CellPosition.y)
			neighbors.push_back({ m_Cells[i].CellPosition.x - 1,  m_Cells[i].CellPosition.y - 1 });
	
	if (i - CELLS_WIDTH >= 0)
		if (m_Cells[i].CellPosition.x == m_Cells[i - CELLS_WIDTH].CellPosition.x && m_Cells[i].CellPosition.y - 1 == m_Cells[i - CELLS_WIDTH].CellPosition.y)
			neighbors.push_back({ m_Cells[i].CellPosition.x,  m_Cells[i].CellPosition.y - 1 });

	if (i - CELLS_WIDTH + 1 >= 0)
		if (m_Cells[i].CellPosition.x + 1 == m_Cells[i - CELLS_WIDTH + 1].CellPosition.x && m_Cells[i].CellPosition.y - 1 == m_Cells[i - CELLS_WIDTH + 1].CellPosition.y)
			neighbors.push_back({ m_Cells[i].CellPosition.x + 1,  m_Cells[i].CellPosition.y - 1 });


	if (i - 1 >= 0)
		if (m_Cells[i].CellPosition.x - 1 == m_Cells[i - 1].CellPosition.x && m_Cells[i].CellPosition.y == m_Cells[i - 1].CellPosition.y)
			neighbors.push_back({ m_Cells[i].CellPosition.x - 1,  m_Cells[i].CellPosition.y });

	if (i + 1 < m_Cells.size())
		if (m_Cells[i].CellPosition.x + 1 == m_Cells[i + 1].CellPosition.x && m_Cells[i].CellPosition.y == m_Cells[i + 1].CellPosition.y)
			neighbors.push_back({ m_Cells[i].CellPosition.x + 1,  m_Cells[i].CellPosition.y });


	if (i + CELLS_WIDTH - 1 < m_Cells.size())
		if (m_Cells[i].CellPosition.x - 1 == m_Cells[i + CELLS_WIDTH - 1].CellPosition.x && m_Cells[i].CellPosition.y + 1 == m_Cells[i + CELLS_WIDTH - 1].CellPosition.y)
			neighbors.push_back({ m_Cells[i].CellPosition.x - 1,  m_Cells[i].CellPosition.y + 1 });

	if (i + CELLS_WIDTH < m_Cells.size())
		if (m_Cells[i].CellPosition.x == m_Cells[i + CELLS_WIDTH].CellPosition.x && m_Cells[i].CellPosition.y + 1 == m_Cells[i + CELLS_WIDTH].CellPosition.y)
			neighbors.push_back({ m_Cells[i].CellPosition.x,  m_Cells[i].CellPosition.y + 1 });

	if (i + CELLS_WIDTH + 1 < m_Cells.size())
		if (m_Cells[i].CellPosition.x + 1 == m_Cells[i + CELLS_WIDTH + 1].CellPosition.x && m_Cells[i].CellPosition.y + 1 == m_Cells[i + CELLS_WIDTH + 1].CellPosition.y)
			neighbors.push_back({ m_Cells[i].CellPosition.x + 1,  m_Cells[i].CellPosition.y + 1 });

	return neighbors;
}

void Game::RevealEmptyArea(const Vector2& startCell)
{
	std::queue<Vector2> q;
	q.push(startCell);

	while (!q.empty())
	{
		Vector2 cell = q.front();
		q.pop();

		int idx = startCell.y * CELLS_WIDTH + startCell.x;

		DrawableObject& hidden = m_Cells[idx];
		DrawableObject& shown = m_ShowingCells[idx];

		// Reveal it
		shown.Type = hidden.Type;
		m_RevealedSquaresAmount++;

		// If it’s empty, we expand
		if (hidden.Type == DrawableType::None)
		{
			for (auto& neighbor : GetNeighbors(cell))
			{
				int nidx = neighbor.y * CELLS_WIDTH + neighbor.x;
				DrawableObject& neighborHidden = m_Cells[nidx];
				DrawableObject& neighborShown = m_ShowingCells[nidx];

				if (neighborShown.Type == DrawableType::NonRevealed)
				{
					if (neighborHidden.Type == DrawableType::None)
					{
						q.push(neighbor); // Keep spreading

						// Reveal bordering numbers
						neighborShown.Type = neighborHidden.Type;
					}
					else if (neighborHidden.Type != DrawableType::Bomb)
					{
						m_RevealedSquaresAmount++;
						// Reveal bordering numbers
						neighborShown.Type = neighborHidden.Type;
					}
				}
			}
		}
	}
}

