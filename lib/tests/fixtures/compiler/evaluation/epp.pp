'environment/foo.epp'.epp.split("\n").each |$message| {
  notice $message
}
'foo/foo.epp'.epp.split("\n").each |$message| {
  notice $message
}
