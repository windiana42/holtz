<?php require("menu.inc.php"); 
  StartPage("Bloks:: Rules", "");
?>

<h2>Bloks Rule Summary</h2>

Bloks can be played with two or four players. <br>

<h3>Stones</h3>

Each player has twenty-one stones, each of which is shaped differently
and consists of one to five little squares. The board consists of 20 x 20
fields. <br>
The players place one stone alternatingly, starting in the corners. 
The first stone of each color must use the player's corner square. <br>
Stones must always touch another of the same color diagonally, but may
never touch along the sides. However, foreign stones may be touched arbitrarily. 


<h3>Goal of the game</h3>

Players move until no stones can be placed anymore. <br>
The goal is to place as many stones as possible. As space on the board is tight, 
it is usually impossible to place all your stones. <br>
When the game ends, the squares of which the remaining stones consist are counted. 
The player who has the least remaining squares wins. 

<?php EndPage(1, "bloks.jpg", "bloks-board-big.png"); ?>
