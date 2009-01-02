#include "relax.hpp"

using namespace relax;

int main()
{
  Standard_Ruleset ruleset;
  Game game(ruleset);
  
  Stones stones;
  stones.stone_count[ game.ruleset->get_stone(5, 7, 3) ] = 0;
  stones.stone_count[ game.ruleset->get_stone(1, 2, 4) ] = 0;
  stones.stone_count[ game.ruleset->get_stone(1, 2, 3) ] = 0;
  stones.stone_count[ game.ruleset->get_stone(5, 2, 4) ] = 0;
  stones.stone_count[ game.ruleset->get_stone(1, 7, 3) ] = 0;
  stones.stone_count[ game.ruleset->get_stone(9, 6, 4) ] = 0;
  stones.stone_count[ game.ruleset->get_stone(5, 2, 3) ] = 0;
  stones.stone_count[ game.ruleset->get_stone(9, 2, 8) ] = 0;

  Game best_game = find_optimal_solution( game, stones );
  
  return 0;
}
