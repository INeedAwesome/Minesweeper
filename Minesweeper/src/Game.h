#pragma once

#include <array>
#include <string>
#include <vector>

struct Vector2
{
	int x, y;
	Vector2() : x(0), y(0) {}
	Vector2(int _x, int _y) : x(_x), y(_y) {}
};

enum GameState
{
	Playing, Won, Lost
};

enum DrawableType
{
	None, 
	One, Two, Three, Four, Five, Six, Seven, Eight, Nine, 
	Bomb, 
	Flag,
	NonRevealed
};

struct DrawableObject
{
	DrawableType Type{};
	Vector2 CellPosition{};
};

#define CELLS_WIDTH 20

class Game
{
public:
	Game() = default;
	~Game() = default;

	void Init(uint32_t amountMines);
	void Cleanup();

	void TryExpose(int xpos, int ypos);
	void TryPlaceFlag(int xpos, int ypos);

	void Reset();

	int GetAmountRevealed() const { return m_RevealedSquaresAmount; }
	int GetAmountFlagsLeft() const { return m_FlagsLeft; }
	bool GetLost() const { return m_State == Lost; }
	bool GetWon() const { return m_State == Won; }
	std::array<DrawableObject, CELLS_WIDTH * CELLS_WIDTH>& GetCells() { return m_ShowingCells; }

	
private:
	std::vector<Vector2> GetNeighbors(const Vector2& cell);
	void RevealEmptyArea(const Vector2& startCell);
	
private:
	std::array<DrawableObject, CELLS_WIDTH* CELLS_WIDTH> m_Cells;
	std::array<DrawableObject, CELLS_WIDTH* CELLS_WIDTH> m_ShowingCells;
	
	int m_AmountMines = 0;
	int m_FlagsLeft = 0;

	int m_RevealedSquaresAmount = 0;

	GameState m_State = GameState::Playing;

};

