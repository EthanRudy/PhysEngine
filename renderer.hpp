#pragma once
#include "solver.hpp"

class Renderer {
public:
	explicit
	Renderer(sf::RenderTarget& target) : m_target{ target } { }

	int threadNum = 5;
	std::thread threads[5];

	void render(const Solver& solver) const {
		// I hate that i cant shorten constraint to const
		const sf::Vector2f constraint = solver.getConstraint();

		
		sf::RectangleShape rect({ 5, 5 });
		const auto& cells = solver.getGrid();
		for (int x = 0; x < 200; ++x) {
			for (int y = 0; y < 100; ++y) {
				if (!cells[y][x].empty()) {
					rect.setPosition({ x * 5.f, y * 5.f });
					rect.setFillColor(sf::Color::White);
					m_target.draw(rect);
				}
			}
		}
		
		
		
		
		

		// Haha funny cicle
		sf::CircleShape circle(1.0f);
		circle.setPointCount(32);
		// Center the object on the new origin (center, not the TL)
		circle.setOrigin(1.0f, 1.0f);
		
		
		// Autos fr saving my ass
		//const uint32_t size = solver.getObjCount();
		const auto& objects = solver.getObjs();
		
		for (const auto& o : objects) {
			circle.setPosition(o.pos);
			circle.setScale(o.rad, o.rad);
			circle.setFillColor(o.col);
			m_target.draw(circle);
		}
		
	}
private:
	sf::RenderTarget& m_target;
};
