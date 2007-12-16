<?php require("menu.inc.php"); 
  StartPage("Holtz:: Download", "Downloads");
?>

<h2>Download the Holtz program (source or binary)</h2>

Note: If you download any source version, you have to download 
<a href="http://wxwidgets.org">wxWidgets</a> first. The source is only 
tested with wxWidgets 2.8.<br>
<br>
<h3>Binary Packages:</h3>
<ul>
<li>Windows binary (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.2-win32.zip">holtz-1.2.2-win32.zip</a>) - just extract and run</li>
<li>Linux RPM, OpenSUSE 10.3 
(<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.2-2.i586.rpm">holtz-1.2.2-2.rpm</a>)</li>
</ul>

<h3>Sourcecode:</h3>
<ul>
<li>Source (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.2.tar.gz">holtz-1.2.2.tar.gz</a>) - source prepared with autotools, requires: wxWidgets 2.8</li>
<li>RPM source package (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.2-2.src.rpm">holtz-1.2.2-2.src.rpm</a>) - to compile a package for your RPM distribution, requires: wxWidgets 2.8</li>
</ul>

<h3>Untested binary packages created with <a href="http://build.opensuse.org/">
opensuse build service</a>:</h3>


<br>
Older versions can be found on the 
<a href="http://sourceforge.net/project/showfiles.php?group_id=74242">
download page</a> of the Holtz project hosted by sourceforge. <br>
<br>
Descriptions of reproducible bugs and compiled packages for other distributions / operating 
Systems are always welcome. I am desperately looking for someone with knowlege about Debian 
and Ubuntu packaging. Please email to <a href="mailto:windiana@users.sf.net">
windiana@users.sf.net</a>.<br>
<br>

<h2>Changes:</h2>
<h3>1.2.2:</h3>
<ul>
<li>Visualize variant tree and enable navigation</li>
<li>Fixed bug in loading PBM games</li>
<li>Fixed bug in configure.ac causing compile problems with wxWidgets library</li>
<li>RPM is dynamically linked to wxGTK</li>
<li>Backward compatibility with wxWidgets 2.6 when using "./configure --enable-wx_2_6"</li>
<li>Spec files for several RPM based distributions used with opensuse build service</li>
</ul>

<h3>1.2.1:</h3>
<ul>
<li> Ruleset options of Zertz are now: 
<ul><li>Basic Rules: Win Condition 3/4/5/2 marbles, board with 37 rings</li>
<li>Standard Rules: Win Condition 4/5/6/3 marbles, board with 37 rings</li>
<li>Tournament Rules: Win Condition 4/5/6/3 marbles, board with 48 rings</li>
</ul>
</li>
</ul>

<?php EndPage(); ?>
