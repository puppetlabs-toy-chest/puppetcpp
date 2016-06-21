unless $memorysize > 1024 {
  $maxclient = 500
  notice $maxclient
} else {
  $maxclient = 1000
}

$a = unless false {}
