touch __tmp__ || exit -1;
for i in *.xpm; do 
  cat $i | sed 's/^static char \*/static const char \*/' > __tmp__
  cat __tmp__ > $i;
done
rm __tmp__; 
