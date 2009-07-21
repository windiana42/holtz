<?php require("menu.inc.php"); 
  StartPage("Relax:: Rules", "");
?>

<h2>Relax Rule Summary</h2>

Relax can be played alone or with up to 10 players. <br>

<h3>Moves</h3>

There are hexagonal pieces with 3 color coded numbers -- one for
each direction in which pieces can form a row on a hexagonal board. 
Pieces are chosen at random by the computer and each player has to
place the pieces on empty fields of his board, one by one. In each
round the piece for the round is given at the top of the game panel
and all players have to place this piece on their board. The position
of placed pieces can never be changed again. The game ends when no
player has an empty field left on his board.

<h3>Goal of the game</h3>

The goal of Relax is to end up with a board where as many as possible
rows are filled only with pieces that have the same enpicted number in
the direction of the row. For each such row you get points which are
calculated by multiplying the number of pieces in that row with the
enpicted number which is common to all pieces in the row. Rows which
have pieces showing different enpicted numbers in the direction of the
row, count no points.

<?php EndPage(1, "relax.jpg", "relax-board-big.png"); ?>
