set ver=1.5.2
set location=http://rubyforge.org/frs/download.php/70936
set filename=Mxx_ru-%ver%.gem
set link=%location%/%filename%

if not exist %filename% (
   wget %link%
) else (
   echo File %filename% already exists.
)
