<?php require("menu.inc.php"); 
  StartPage("Holtz:: Download", "Downloads");
?>

<h2>Download the Holtz program (source or binary)</h2>

<h3>Binary Packages:</h3>
<ul>
<li>Windows binary (<a href="http://sourceforge.net/projects/holtz/files/holtz/1.3.1/holtz-1.3.1-win32.zip/download">holtz-1.3.1-win32.zip</a>) - just extract and run - (MD5: 2438fc6c9e0d93078b4b5ff1c44a959a)</li>
<li>Linux RPM, OpenSUSE 11.0 
    (<a href="http://sourceforge.net/projects/holtz/files/holtz/1.3.1/holtz-1.3.1-1.x86_64.rpm/download">holtz-1.3.1-1.x86_64.rpm</a>) - (MD5: 483182fc0e97f31bb44cdc3295a7a4c0)</li>
<li>For other Linux distributions, see below</li>
</ul>

<h3>Sourcecode:</h3>
<ul>
    <li>Source (<a href="http://sourceforge.net/projects/holtz/files/holtz/1.3.1/holtz-1.3.1.tar.gz/download">holtz-1.3.1.tar.gz</a>) - source prepared with autotools, requires: wxWidgets 2.8 or 2.6 - (MD5: 1ec4b4560eefa0475bf81e7991398db9)</li>
   <li>RPM source package (<a href="http://sourceforge.net/projects/holtz/files/holtz/1.3.1/holtz-1.3.1-1.src.rpm/download">holtz-1.3.1-1.src.rpm</a>) - to compile a package for your RPM distribution, requires: wxWidgets 2.8 - (MD5: cbe3e52831d28fbc7a41a2a938fd96d6)</li>
</ul>
<p>Note: For compiling the sourcecode you either need to install a development package 
of wxWidgets (e.g. wxGTK-devel, libwxgtk2.6-dev, or libwxgtk2.8-dev) or download source directly 
from the <a href="http://wxwidgets.org">wxWidgets</a> homepage. Holtz was only 
tested with wxWidgets 2.8. When using "./configure --enable-wx_2_6" the source should be 
compatible with wxWidgets 2.6 as well.</p>

<h3>Binary packages for other Linux distributions:</h3>
<p>More binary packages for Fedora, Mandriva, and other SuSE distributions can be found at 
<a href="http://software.opensuse.org/search?p=1&q=holtz&baseproject=ALL">opensuse build service</a>.
Please <a href="mailto:windiana@users.sf.net">report</a> your experiences when 
using one of these packages since they are not intensively tested.
</p>

<h3>Older versions:</h3>
<p>Older versions can be found on the 
<a href="http://sourceforge.net/project/showfiles.php?group_id=74242">
download page</a> of the Holtz project hosted by sourceforge.</p>

<h3>Feedback:</h3>
<p>Descriptions of reproducible bugs and compiled packages for other distributions / operating 
Systems are always welcome. I am desperately looking for someone with knowlege about Debian 
and Ubuntu packaging. Please email <a href="mailto:windiana@users.sf.net">
windiana@users.sf.net</a>.</p>

<h2>Changes:</h2>

<h3>1.3.1:</h3>
<ul>
<li>Fixed bug in dvonn AI which made it loose intentionally on the final moves sometimes</li>
</ul>

<h3>1.3.0:</h3>
<ul>
<li>New Game supported: Relax</li>
</ul>

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
