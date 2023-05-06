#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <thread>

struct VerletObj {
	sf::Vector2f pos;
	sf::Vector2f pos_old;
	sf::Vector2f acc;

	float rad = 10.f;
	sf::Color col = sf::Color::White;

	VerletObj() = default; // What it do

	VerletObj(sf::Vector2f pos_, float rad_)
		: pos{ pos_ },
		pos_old{ pos_ }, 
		acc{ 0.f, 0.f },
		rad{ rad_ }
	{}

	void update(float dt) {
		// Get displacement
		const sf::Vector2f disp = pos - pos_old;
		// Move the obj
		pos_old = pos;
		pos = pos + disp + acc * (dt * dt);
		// Reset acc
		acc = {};
	}

	void accel(sf::Vector2f acc_new) {
		acc += acc_new;
	}

	void setVel(sf::Vector2f vel, float dt) {
		pos_old = pos - (vel * dt);
	}

	void addVel(sf::Vector2f vel_add, float dt) {
		pos_old -= vel_add * dt;
	}

	[[nodiscard]]	// This shit wild
	sf::Vector2f getVel(float dt) const {
		return (pos - pos_old) / dt;
	}
};

typedef std::vector<uint64_t> Cell;

class Solver {
public:
	//Solver() = default;
	Solver() {
		for (int y = 0; y < 100; ++y) {
			std::vector<Cell> row;
			cells.push_back(row);
			for (int x = 0; x < 200; ++x) {
				Cell c;
				cells[y].push_back(c);
			}
		}
	}

	VerletObj& addObject(sf::Vector2f position, float radius) {
		m_objects.emplace_back(position, radius);

		return m_objects.back();
	}

	void updateNaive() {
		time += frameDT;
		const float stepDT = getStepDT();
		for (uint32_t i{ subSteps }; --i;) {
			applyGravity();
			checkCollisionsNaive();
			applyConstraint();
			updateObjs(stepDT);

			//updateGrid();
			
		}
	}

	void updateOptim() {
		time += frameDT;
		const float stepDT = getStepDT();
		for (uint32_t i{ subSteps }; --i;) {
			applyGravity();
			checkCollisionsOptim();
			applyConstraint();
			updateObjs(stepDT);

			updateGrid();
		}
	}

	void updateThrea() {
		time += frameDT;
		const float stepDT = getStepDT();
		uint32_t range = 200 / 5;

		for (uint32_t i{ subSteps }; --i;) {
			applyGravity();

			uint8_t threadNum = 0;
			for (int start = 0; start < 200; start += range) {
				int end = start + range;

				threads[threadNum] = std::thread(&Solver::checkCollisionsThrea, this, start, end);
				++threadNum;

			}
			//checkCollisionsThrea();

			for (auto& t : threads) {
				t.join();
			}

			applyConstraint();
			updateObjs(stepDT);

			updateGrid();
		}
	}

	void setSimUpdateRate(uint32_t r) {
		frameDT = 1.f / static_cast<float>(r);
	}

	void setConstraint(sf::Vector2f size) {
		constraint_dim = size;
	}

	void setSubSteps(uint32_t sS) {
		subSteps = sS;
	}

	void setObjVel(VerletObj& o, sf::Vector2f vel) {
		o.setVel(vel, getStepDT());
	}

	[[nodiscard]]
	const std::vector<VerletObj>& getObjs() const {
		return m_objects;
	}

	[[nondiscard]]
	const std::vector<std::vector<Cell>>& getGrid() const {
		return cells;
	}

	[[nodiscard]]
	sf::Vector2f getConstraint() const {
		return { constraint_dim.x, constraint_dim.y };
	}

	[[nodiscard]]
	uint64_t getObjCount() const {
		return m_objects.size();
	}

	[[nodiscard]]
	float getTime() const {
		return time;
	}

	[[nodiscard]]
	float getStepDT() const {
		return frameDT / static_cast<float>(subSteps);
	}

private:
	uint32_t subSteps = 1;
	sf::Vector2f gravity = { 0.f, 1000.f };

	sf::Vector2f constraint_dim;

	std::vector<VerletObj> m_objects;
	float time = 0.f;
	float frameDT = 0.f;

	std::vector<std::vector<Cell>> cells;

	std::thread threads[5];

	void applyGravity() {
		for (auto& o : m_objects) {
			o.accel(gravity);
		}
	}

	void checkCollisionsNaive() {
		const float res = .75f; // Dampening of bounces
		const uint64_t c = m_objects.size();

		for (uint64_t i = 0; i < c; ++i) {
			VerletObj& o1 = m_objects[i];
			for (uint64_t ii = i + 1; ii < c; ++ii) {
				VerletObj& o2 = m_objects[ii];

				const sf::Vector2f vel = o1.pos - o2.pos;
				const float dDist = vel.x * vel.x + vel.y * vel.y;
				const float min = o1.rad + o2.rad;

				if (dDist < min * min) {
					const float dist = sqrt(dDist);
					const sf::Vector2f n = vel / dist;
					const float mRat1 = o1.rad / (o1.rad + o2.rad);
					const float mRat2 = o2.rad / (o1.rad + o2.rad);
					const float delta = .5f * res * (dist - min);

					// Shift if needed
					o1.pos -= n * (mRat2 * delta);
					o2.pos += n * (mRat1 * delta);
				}
			}
		}
	}

	void checkCollisionsOptim() {
		for (int x = 0; x < 200; ++x) {
			for (int y = 0; y < 100; ++y) {
				if ((x == 0 && y == 0) || (x == 0 && y == 99) || (x == 199 && y == 99) || (x == 199 && y == 0)) {
					// Corner, resolveCell 5 times
					for (uint8_t i{ 5 }; --i;) {
						auto& c = cells[y][x];
						resolveCell(c);
					}
				}
				else if (x == 0 || y == 0 || x == 199 || y == 99) {
					// Edge, resolveCell 3 times
					for (uint8_t i{ 3 }; --i;) {
						auto& c = cells[y][x];
						resolveCell(c);
					}
				}
				
				

				auto& c1 = cells[y][x];

				for (int dx = -1; dx <= 1; ++dx) {
					
					for (int dy = -1; dy <= 1; ++dy) {
						if (x + dx < 0 || x + dx > 199) { continue; }
						if (y + dy < 0 || y + dy > 99) { continue; }
						auto& c2 = cells[y + dy][x + dx];

						resolveCellCelL(c1, c2);
					}
				}
			}
		}
	}

	void checkCollisionsThrea(int min, int max) {
		for (int y = 0; y < 100; ++y) {
			for (int x = min; x < max; ++x) {
				if ((x == 0 && y == 0) || (x == 0 && y == 99) || (x == 99 && y == 99) || (x == 99 && y == 0)) {
					// Corner, resolveCell 5 times
					for (uint8_t i{ 5 }; --i;) {
						auto& c = cells[y][x];
						resolveCell(c);
					}
				}
				else if (x == 0 || y == 0 || x == 99 || y == 99) {
					// Edge, resolveCell 3 times
					for (uint8_t i{ 3 }; --i;) {
						auto& c = cells[y][x];
						resolveCell(c);
					}
				}



				auto& c1 = cells[y][x];

				for (int dx = -1; dx <= 1; ++dx) {

					for (int dy = -1; dy <= 1; ++dy) {
						if (x + dx < 0 || x + dx > 99) { continue; }
						if (y + dy < 0 || y + dy > 99) { continue; }
						auto& c2 = cells[y + dy][x + dx];

						resolveCellCelL(c1, c2);
					}
				}
			}
		}
	}

	void resolveCellCelL(Cell& c1, Cell& c2) {
		for (uint64_t i1 : c1) {
			for (uint64_t i2 : c2) {
				if (i1 != i2) {
					VerletObj& o1 = m_objects[i1];
					VerletObj& o2 = m_objects[i2];

					resolve(o1, o2);
				}
			}
		}
	}

	void resolveCell(Cell& c1) {
		for (uint64_t i1 : c1) {
			for (uint64_t i2 : c1) {
				if (i1 != i2) {
					VerletObj& o1 = m_objects[i1];
					VerletObj& o2 = m_objects[i2];

					resolve(o1, o2);
				}
			}
		}
	}

	void resolve(VerletObj& o1, VerletObj& o2) {
		const float res = .75f;

		const sf::Vector2f vel = o1.pos - o2.pos;
		const float dDist = vel.x * vel.x + vel.y * vel.y;
		const float min = o1.rad + o2.rad;

		if (dDist < min * min) {
			const float dist = sqrt(dDist);
			const sf::Vector2f n = vel / dist;
			const float mRat1 = o1.rad / (o1.rad + o2.rad);
			const float mRat2 = o2.rad / (o1.rad + o2.rad);
			const float delta = .5f * res * (dist - min);

			// Shift if needed
			o1.pos -= n * (mRat2 * delta);
			o2.pos += n * (mRat1 * delta);
		}
	}

	void applyConstraint() {/*
		for (auto& o : m_objects) {
			const sf::Vector2f vec = constraint_center - o.pos;
			const float dist = sqrt(vec.x * vec.x + vec.y * vec.y);
			if (dist > (constraint_radius - o.rad)) {
				const sf::Vector2f n = vec / dist;
				o.pos = constraint_center - n * (constraint_radius - o.rad);
			}
		}
		*/

		for (auto& o : m_objects) {
			if (o.pos.x < o.rad) { 
				o.pos.x = o.rad;
			}
			else if (o.pos.x > constraint_dim.x - o.rad) { 
				o.pos.x = constraint_dim.x - o.rad; 
			}

			if (o.pos.y < o.rad) { 
				o.pos.y = o.rad;
			}
			else if (o.pos.y > constraint_dim.y - o.rad) { 
				o.pos.y = constraint_dim.y - o.rad; 
			}
		}
	}

	void updateObjs(float dt) {
		for (auto& o : m_objects) {
			o.update(dt);
		}
	}

	uint32_t mapToGrid(float val, float in, float out) {
		return 0 + (val - 0) * out / in;
	}

	void updateGrid() {
		for (int x = 0; x < 100; ++x) {
			for (int y = 0; y < 100; ++y) {
				cells[y][x].clear();
			}
		}

		// Reset grid;
		for (int i = 0; i < m_objects.size(); ++i) {
			VerletObj& o = m_objects[i];
			uint32_t gridX = mapToGrid(o.pos.x, 1000, 100);
			if (gridX < 0) { gridX = 0; }
			if (gridX > 99) { gridX = 99; }
			uint32_t gridY = mapToGrid(o.pos.y, 1000, 100);
			if (gridY < 0) { gridY = 0; }
			if (gridY > 99) { gridY = 99; }

			cells[gridY][gridX].push_back(i);
		}
	}
};
