if $is_virtual {
  # Our NTP module is not supported on virtual machines:
  warning( 'Tried to include class ntp on virtual machine; this node may be misclassified.' )
}
elsif $operatingsystem == 'Darwin' {
  warning( 'This NTP module does not yet work on our Mac laptops.' )
}
else {
  # Normal node, include the class.
  include ntp
}

if false {

} elsif true {

} elsif true and false {

}

if false {}
