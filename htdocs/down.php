<?php require("menu.inc.php"); 
  StartPage("Holtz:: Downloads", "Downloads");
?>

<h2>Downloads of Holtz source and binaries</h2>

Note: If you download any source version, you have to download 
<a href="http://wxwidgets.org">wxWidgets</a> first. The source is only 
tested with wxWidgets 2.8.<br>
<br>
Downloads can be found on the <a href="http://sourceforge.net/project/showfiles.php?group_id=74242">
download page</a> of the Holtz project hosted by sourceforge. <br>
<br>
Available formats:
<ul>
<li>Windows binary (<a href="http://downloads.sourceforge.net/holtz/holtz-win32.zip">holtz-win32.zip</a>) - just extract and run</li>
<li>Linux binary package (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.0-1.i586.rpm">holtz-1.2.0-1.rpm</a>) - compiled with OpenSuse 10.2, wxWidgets is statically linked</li>
<li>RPM source package (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.0-1.src.rpm">holtz-1.2.0-1.src.rpm</a>) - to compile a package for your RPM distribution, requires: wxWidgets 2.8</li>
<li>Source (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.0.tar.gz">holtz-1.2.0.tar.gz</a>) - source prepared with autotools, requires: wxWidgets 2.8</li>
</ul>
<br>
Descriptions of reproducible bugs and compiled packages for other distributions / operating 
Systems are always welcome.
 
<?php EndPage(); ?>
