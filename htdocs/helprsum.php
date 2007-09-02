<?php require("menu.inc.php"); 
  StartPage("Rules", "");
?>

<h2>Z&egrave;rtz Rule summary</h2>

The Z&egrave;rtz game is originally meant to be played on a hexagonal board composed of 37 fields, by two players. <br>
There are five white, seven gray and nine black stones which may be placed on the fields. 
<h3>Moves</h3>
The players move alternately. They can choose between two kinds of moves:
<ol>
<li><b>Setting a stone</b>
    <ul>
    <li>Take a stone from the common stones (below the board) and
      place it on one of the empty fields.
    <li>Remove one of the empty border fields, which can be moved 
      away from the board without moving other fields. 
      (If there are no such fields, no field is removed.)
    </ul>
<li><b>Knock-out move</b>
    <ul>
    <li>Knock out a stone by jumping with a stone on the board over 
      an adjacent one to an empty field.
    <li>You get the stone(s) which you jumped over, i.e. they are
      removed from the board and put to your stones (on the right).
    <li>If you can jump again with the same stone, starting from its 
      new position, you have to do so. 
    </ul>  
    No fields are removed from the board in a knock-out move. 
</li>
</ol>
However, if a knock-out move is possible, it must be performed. 

<h3>Goal of the game</h3>
The goal is to get eithe three white, four gray, or five black stones. 
You also win if you have got two stones of each colour.

<h3>Additional rules</h3>
<ul>
<li>If there are islands of fields (i.e. parts of the board which are no longer connected with the rest of the board) filled with stones after the move is finished, 
this island is removed completely, and the player whose turn it was, gets all stones which were on the island. 
<li>If there are no more common stones, but no knock-out move is possible, the player has to take one of his own stones and use it instead. (This happens quite rarely.)
</ul>

<h3>Tournament rules</h3>
The tournament rules are a slight variation of the rules to make the game  more challenging for experienced players. 
<ul>
  <li>there are six white, eight gray and ten black stones
  <li>the goal is to get four white, five gray, or six black stones; 
    alternatively three stones of each type.
</ul>

<?php EndPage(); ?>