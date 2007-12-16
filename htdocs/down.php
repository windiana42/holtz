<?php require("menu.inc.php"); 
  StartPage("Holtz:: Download", "Downloads");
?>

<h2>Download the Holtz program (source or binary)</h2>

<br>
<h3>Binary Packages:</h3>
<ul>
<li>Windows binary (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.2-win32.zip">holtz-1.2.2-win32.zip</a>) - just extract and run</li>
<li>Linux RPM, OpenSUSE 10.3 
(<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.2-1.i586.rpm">holtz-1.2.2-1.rpm</a>)</li>
<li>See below for other Linux distributions</li>
</ul>

<h3>Sourcecode:</h3>
<p>Note: For compiling the sourcecode you either need to install a development package 
of wxWidgets (e.g. wxGTK-devel/libwxgtk2.6-dev/libwxgtk2.8-dev) or download source directly 
from the <a href="http://wxwidgets.org">wxWidgets</a> homepage. Holtz was only 
tested with wxWidgets 2.8. When using "./configure --enable-wx_2_6" the source should be 
compatible with wxWidgets 2.6 as well.</p>
<ul>
<li>Source (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.2.tar.gz">holtz-1.2.2.tar.gz</a>) - source prepared with autotools, requires: wxWidgets 2.8 or 2.6</li>
<li>RPM source package (<a href="http://downloads.sourceforge.net/holtz/holtz-1.2.2-1.src.rpm">holtz-1.2.2-1.src.rpm</a>) - to compile a package for your RPM distribution, requires: wxWidgets 2.8</li>
</ul>

<h3>Binary packages for other Linux distributions:</h3>
<p>Untested binary packages for Fedora, Mandriva, and other SUSE distributions can be found at 
<a href="http://software.opensuse.org/search?p=1&q=holtz&baseproject=ALL">opensuse build service</a>.
Please <a href="mailto:windiana@users.sf.net">report</a> your experiences when 
using one of these packages so more binary linux packages can be provided on this page.
</p>

<h3>Older versions:</h3>
<p>Older versions can be found on the 
<a href="http://sourceforge.net/project/showfiles.php?group_id=74242">
download page</a> of the Holtz project hosted by sourceforge.</p>

<h3>Feedback:</h3>
<p>Descriptions of reproducible bugs and compiled packages for other distributions / operating 
Systems are always welcome. I am desperately looking for someone with knowlege about Debian 
and Ubuntu packaging. Please email to <a href="mailto:windiana@users.sf.net">
windiana@users.sf.net</a>.</p>

<h2>Changes:</h2>
<h3>1.2.2:</h3>
<ul>
<li>Visualize variant tree and enable navigation</li>
<li>Fixed bug in loading PBM games</li>
<li>Fixed bug in configure.ac causing compile problems with wxWidgets library</li>
<li>Fixed bug when deleting AI thread (e.g. when switching between Zertz and Dvonn)</li>
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
