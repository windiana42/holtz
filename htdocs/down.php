<?php require("menu.inc.php"); 
  StartPage("Holtz:: Downloads", "Downloads");
?>

<h2>Downloads of Holtz source and binaries</h2>

Note: If you download any source version, you have to download 
<a href="http://wxwidgets.org">wxWidgets</a> first. The source is only 
tested with wxWidgets 2.8<br>
<br>
Downloads can be found on the <a href="http://sourceforge.net/project/showfiles.php?group_id=74242">
download page</a> of the holtz project hosted by sourceforge. <br>
<br>
Available formats:
<ul>
<li>Source (.tar.gz) - source prepared with autotools, prerequisites: wxWidgets 2.8</li>
<li>Linux binary package (.rpm) - compiled with OpenSuse 10.2, wxWidgets is statically linked</li>
<li>Windows binary (.zip) - just extract and run</li>
</ul>

<?php EndPage(); ?>
