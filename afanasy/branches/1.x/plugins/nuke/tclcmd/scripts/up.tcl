proc up {} {
   foreach pScript [glob -nocomplain "[getenv NUKE_AF_PATH]/scripts/*.tcl"] {
   source $pScript
   puts $pScript
   }
   return
}
