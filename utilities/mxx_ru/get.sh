#!/bin/bash

http://rubyforge.org/frs/download.php/53915/Mxx_ru-1.4.10.gem

ver=1.4.10
lcation="http://rubyforge.org/frs/download.php/53915"
filename="Mxx_ru-$ver.gem"
link="$lcation/$filename"

wget $link
