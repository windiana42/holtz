<?php require("menu.inc.php"); 
  StartPage("Downloads", "Downloads");
?>

<h2>Downloads of Holtz source and binaries</h2>

Note: If you download any source version, you have to download 
<a href="http://wxwidgets.org">wxWidgets</a> first <br>
<br>
<?php 
$fp = fopen('http://sourceforge.net/project/showfiles.php?group_id=74242', 'r') 
  or die("Cannot read Project: Files page???");
$fcontents = "";
while( strlen($fcontents < 50000) && !feof($fp) ) {
  $fcontents = $fcontents.fread($fp, 10000); 
}
fclose($fp);
$pos = strpos($fcontents, "Latest File Releases");
$pos = strpos($fcontents, "<table"/*, $pos*/); 

$endpos = strpos($fcontents, "</table", $pos);
//$endpos = strpos($fcontents, "Project Totals", $pos);
/* it doesn't work (but why)
$endpos = strpos($fcontents, "</table", $endpos);
print "endpos: $endpos <br>\n";
$endpos = strpos($fcontents, ">", $endpos) + 1;
print "endpos: $endpos <br>\n";
*/
$data = substr($fcontents, $pos, $endpos - $pos);
print $data;
print "</table>";
?>
<?php EndPage(); ?>
