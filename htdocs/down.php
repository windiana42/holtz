<?php require("menu.inc.php"); 
  StartPage("Downloads", "Downloads");
?>

<h2>Downloads of Holtz source and binaries</h2>
<?php 
$fp = fopen('http://sourceforge.net/project/showfiles.php?group_id=74242', 'r') 
  or die("Cannot read Project: Files page???");
$fcontents = fread($fp, 100000); 
fclose($fp);
$pos = strpos($fcontents, "Below is a list");
$pos = strpos($fcontents, "<table", $pos); 

$endpos = strpos($fcontents, "Project Totals", $pos);
/* it doesn't work (but why)
$endpos = strpos($fcontents, "</table", $endpos);
print "endpos: $endpos <br>\n";
$endpos = strpos($fcontents, ">", $endpos) + 1;
print "endpos: $endpos <br>\n";
*/
$data = substr($fcontents, $pos, $endpos - $pos);
print $data;
print "</table>";
print "Note: If you download any source version, you have to download
	<a href="http://wxwindows.org">wxWindows</a> first";
?>
<!--
<h2>Not yet implemented</h2>
Sorry, downloads are not yet available through this home page. Use
the Project Page or the Released Files link instead. 
-->
<?php EndPage(); ?>