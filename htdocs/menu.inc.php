<?php 



function PrintMenuItem($itemtext, $itemlink, $activeitem, $last = 0)
{
  print "<td background=\"line-20-bg.gif\" valign=\"middle\" align=\"center\">&nbsp;&nbsp;";
  if($activeitem == $itemtext) print "<span class=\"active\">$itemtext</span>";
  else print "<a href=\"$itemlink\">$itemtext</a>";
  print "&nbsp;&nbsp;";
  if(!$last) print "<td><img alt=\"interrupt\" src=\"line-20-interrupt.gif\">";
  else print "<td><img alt=\"right\" src=\"line-20-right.gif\">";
}

function PrintMenu($activepage) {
  print "<div align=\"center\"><table cellpadding=0 border=0 cellspacing=0><tr>";
  print "<td><img alt=\"left\" src=\"line-20-left.gif\">";
  PrintMenuItem("Home page", "zertz.php", $activepage);
  PrintMenuItem("News", "news.php", $activepage);
  PrintMenuItem("Downloads", "http://sourceforge.net/project/showfiles.php?group_id=74242", $activepage);
  PrintMenuItem("Information", "help.php", $activepage);
  PrintMenuItem("Project Page", "http://sourceforge.net/projects/holtz/", $activepage);
  PrintMenuItem("Links", "links.php", $activepage, 1);
  print "</tr></table></div>";
}

function PrintSidebar() {
/*
  print "<table cellpadding=0 border=0 cellspacing=0>";
  // /////// make a box ////////
  // upper border
  print '<tr><td><img alt="" src="box-nw.gif"><td background="box-n.gif">'
   	.'<span style="font-size:1px;">&nbsp;</span><td><img alt="" src="box-ne.gif"></tr>';
  // left border
  print '<tr><td background="box-w.gif"><span style="font-size:1px;">&nbsp;</span>';
  // actual cell
  print '<td background="box-bg.gif" style="padding:7px;"><small>';
  readfile("../projhtml.cache");
  print '</small></td>';
  // right border
  print '<td background="box-e.gif"><span style="font-size:1px;">&nbsp;</span></tr>';
  // bottom border
  print '<tr><td><img alt="" src="box-sw.gif"><td background="box-s.gif">'
   	.'<span style="font-size:1px;">&nbsp;</span><td><img alt="" src="box-se.gif"></tr>';
  // //////// end box //////////
  print '</table>';
*/
}

function StartPage($title, $menuname, $showSidebar = 1) {
?>
<html>
<head>
  <meta name="author" content="Martin Trautmann, Florian Fischer">
  <meta name="keywords" content="Zertz, zertz, Zèrtz, Z&egrave;ertz, zèrtz, z&egrave;ertz, Holtz, holtz,
	abstract strategy game, strategy game, abstract game, Denkspiel, Brettspiel, board game">
  <meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">
  <title>Holtz:: <?php print $title; ?></title>
  <link rel="stylesheet" href="holtz.css">
</head>
<body background="bg-wood.jpg">
  <table border=0 cellpadding=2 cellspacing=15 width="100%">
    <tr><td colspan=2 align="center"><h1>Holtz:: <?php print $title; ?></h1>
      <?php PrintMenu($menuname); ?>
    </tr>
    <tr>
      <td valign="top" width="*" <?php if(!$showSidebar) print "colspan=2"; ?>>
<?php
}

function EndPage($showSidebar = 1) {
?>
      </td>
      <?php if($showSidebar) { ?><td valign="top" width="230"><?php PrintSidebar(); } ?>
    </tr>
    <tr><td colspan=2><hr>by Martin Trautmann<br><i>Hosted by <A href="http://sourceforge.net"> 
      <img src="http://sourceforge.net/sflogo.php?group_id=74242&amp;type=5" 
        width="105" height="31" border="0" align="middle" alt="SourceForge.net Logo"></A></i>
    </tr>
  </table>
</body>
</html>
<?php
}

?>
