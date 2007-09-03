<?php 



function PrintMenuItem($itemtext, $itemlink, $activeitem, $last = 0)
{
  print "<td style=\"background-image:url(line-20-bg.gif)\" valign=\"middle\" align=\"center\">&nbsp;&nbsp;";
  if($activeitem == $itemtext) print "<span class=\"active\">$itemtext</span>";
  else print "<a href=\"$itemlink\">$itemtext</a>";
  print "&nbsp;&nbsp;";
  if(!$last) print "<td><img alt=\"interrupt\" src=\"line-20-interrupt.gif\">";
  else print "<td><img alt=\"right\" src=\"line-20-right.gif\">";
}

function PrintMenu($activepage) {
  print "<div align=\"center\"><table cellpadding=0 border=0 cellspacing=0><tr>";
  print "<td><img alt=\"left\" src=\"line-20-left.gif\">";
  PrintMenuItem("Home", "index.php", $activepage);
  PrintMenuItem("Z&egrave;rtz", "zertz.php", $activepage);
  PrintMenuItem("Dvonn", "dvonn.php", $activepage);
  PrintMenuItem("Screenshots", "helpprog.php", $activepage);
  PrintMenuItem("Downloads", "down.php", $activepage);
  PrintMenuItem("Links", "links.php", $activepage, 1);
  print "</tr></table></div>";
}

function PrintSidebar($sidebarImg, $bigImg) {
  if($sidebarImg != "") {
    print "<td valign=\"top\">";
    if($bigImg != "") { 
      print "<a href=\"$bigImg\">";
    }
    print "<img src=\"$sidebarImg\">";
    if($bigImg != "") {
      print "</a>";
    }
    print "</td></tr>\n";
    print "<tr><td valign=\"bottom\"><img src=\"sidebar.png\" alt=\"Holtz/Dvonn/Zertz logo\"></td>\n";
  }
  else {
    print "<td valign=\"middle\" width=\"230\"><img src=\"sidebar.png\" alt=\"Holtz/Dvonn/Zertz logo\"></td></tr><tr><td>&nbsp;</td>\n";
  }
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
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
  <meta name="author" content="Martin Trautmann, Florian Fischer">
  <meta name="keywords" content="Zertz, zertz, Zèrtz, Z&egrave;ertz, zèrtz, z&egrave;ertz, Dvonn, dvonn, Holtz, holtz,
	abstract strategy game, strategy game, abstract game, Denkspiel, Brettspiel, board game">
  <meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">
  <title><?php print $title; ?></title>
  <link rel="stylesheet" href="holtz.css">
</head>
<body background="bg-wood.jpg">
  <table border=0 cellpadding=2 cellspacing=15 width="100%">
    <tr><td colspan=2 align="center"><h1><?php print $title; ?></h1>
      <?php PrintMenu($menuname); ?>
    </tr>
    <tr>
      <td valign="top" width="*" <?php if($showSidebar) print "rowspan=2"; else print "colspan=2"; ?>>
<?php
}

function EndPage($showSidebar = 1, $sidebarImg = "", $bigImg = "") {
?>
      </td>
      <?php if($showSidebar) { ?><?php PrintSidebar($sidebarImg, $bigImg); } ?>
    </tr>
    <tr><td colspan=2><hr>by Martin Trautmann<br>
      <i>Hosted by <A href="http://sourceforge.net"> 
      <img src="http://sourceforge.net/sflogo.php?group_id=74242&amp;type=5" 
        width="105" height="31" border="0" align="middle" alt="SourceForge.net Logo"></A></i>
      &nbsp;&nbsp;&nbsp; -- &nbsp;<a href="http://sourceforge.net/projects/holtz/">Project Page</a>
    </tr>
  </table>
</body>
</html>
<?php
}

?>
