<?php require("menu.inc.php"); 
  StartPage("Holtz:: News", "News");
?>

<h2>News on Holtz</h2>
<?php readfile("../projnews.cache"); ?>

<?php EndPage(); ?>